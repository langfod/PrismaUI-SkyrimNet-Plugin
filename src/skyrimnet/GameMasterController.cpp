#include "pch.h"
#include "GameMasterController.h"
#include "../http/HttpClient.h"
#include "../ui/UIBridge.h"
#include <chrono>

namespace SkyrimNetUI {
namespace SkyrimNet {

static Controller* g_controller = nullptr;

Controller& GetController() {
  if (!g_controller) {
    g_controller = new Controller();
  }
  return *g_controller;
}

Controller::Controller() = default;

Controller::~Controller() {
  StopPolling();
}

void Controller::StartPolling() {
  if (pollingActive_) {
    return;
  }
  
  pollingActive_ = true;
  pollingThread_ = std::thread([this]() { PollStatus(); });
  logger::info("Started GameMaster status polling");
}

void Controller::StopPolling() {
  if (!pollingActive_) {
    return;
  }
  
  pollingActive_ = false;
  if (pollingThread_.joinable()) {
    pollingThread_.join();
  }
  logger::info("Stopped GameMaster status polling");
}

void Controller::PollStatus() {
  while (pollingActive_) {
    try {
      std::string response = Http::Get(L"http://localhost:8080/?api=gamemaster-status");
      
      if (!response.empty()) {
        logger::info("Poll: Received GameMaster status response: {}", response);
        bool newState = ParseStatus(response);
        bool previousState = enabled_.load();
        
        logger::info("Poll: ParseStatus={}, previousState={}, enabled_.load()={}", 
                     newState, previousState, enabled_.load());
        
        enabled_.store(newState);
        
        if (previousState != newState) {
          logger::info("Poll: State changed from {} to {}, calling UpdateUI({})", 
                       previousState, newState, newState);
          UpdateUI(newState);
        } else {
          logger::info("Poll: State unchanged ({}), skipping UpdateUI", newState);
        }
      }
    } catch (const std::exception& e) {
      logger::warn("Error polling GameMaster status: {}", e.what());
    }
    
    // Sleep for 5 seconds
    for (int i = 0; i < 50 && pollingActive_; ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
}

bool Controller::ParseStatus(const std::string& jsonResponse) {
  try {
    // Simple JSON parsing for agent_enabled
    size_t pos = jsonResponse.find("\"agent_enabled\"");
    if (pos == std::string::npos) {
      return false;
    }
    
    pos = jsonResponse.find(":", pos);
    if (pos == std::string::npos) {
      return false;
    }
    
    // Skip whitespace
    pos++;
    while (pos < jsonResponse.length() && (jsonResponse[pos] == ' ' || jsonResponse[pos] == '\t')) {
      pos++;
    }
    
    if (pos + 4 <= jsonResponse.length() && jsonResponse.substr(pos, 4) == "true") {
      return true;
    }
    
    return false;
  } catch (...) {
    logger::error("Failed to parse GameMaster status JSON");
    return false;
  }
}

void Controller::UpdateUI(bool enabled) {
  logger::info("UpdateUI called with enabled={}", enabled);
  UI::UpdateGameMasterStatus(enabled);
  logger::info("UpdateUI: UI::UpdateGameMasterStatus({}) completed", enabled);
}

static std::string ReplaceBooleanValue(const std::string& json, size_t searchStart, const std::string& fieldName, bool newValue) {
  size_t fieldPos = json.find("\"" + fieldName + "\"", searchStart);
  if (fieldPos == std::string::npos) {
    return json;
  }
  
  size_t colonPos = json.find(":", fieldPos);
  if (colonPos == std::string::npos) {
    return json;
  }
  
  // Skip whitespace after colon
  size_t valueStart = colonPos + 1;
  while (valueStart < json.length() && (json[valueStart] == ' ' || json[valueStart] == '\t' || json[valueStart] == '\n' || json[valueStart] == '\r')) {
    valueStart++;
  }
  
  // Find end of value (comma, closing brace, or closing bracket)
  size_t valueEnd = json.find_first_of(",}]", valueStart);
  if (valueEnd == std::string::npos) {
    return json;
  }
  
  // Skip trailing whitespace before delimiter
  size_t actualEnd = valueEnd;
  while (actualEnd > valueStart && (json[actualEnd-1] == ' ' || json[actualEnd-1] == '\t' || json[actualEnd-1] == '\n' || json[actualEnd-1] == '\r')) {
    actualEnd--;
  }
  
  // Replace the value
  std::string result = json.substr(0, valueStart) + (newValue ? "true" : "false") + json.substr(actualEnd);
  return result;
}

void Controller::Toggle() {
  bool currentState = enabled_.load();
  bool newState = !currentState;
  
  logger::info("Toggling GameMaster agent from {} to {}", currentState, newState);
  
  // Stop polling during toggle to prevent race condition
  bool wasPolling = pollingActive_.load();
  if (wasPolling) {
    logger::info("Stopping polling during toggle operation");
    StopPolling();
  }
  
  // Step 1: Get the current config values
  std::string configResponse = Http::Get(L"http://localhost:8080/config?api=get&name=game");
  
  if (configResponse.empty()) {
    logger::error("Failed to retrieve game config");
    // Restart polling if it was active
    if (wasPolling) {
      StartPolling();
    }
    return;
  }
  
  logger::info("Retrieved current config (length: {} bytes)", configResponse.length());
  
  // Step 2: Find the gamemaster section and update both fields
  size_t gamemasterPos = configResponse.find("\"gamemaster\"");
  if (gamemasterPos == std::string::npos) {
    logger::error("Could not find gamemaster section in config response");
    // Restart polling if it was active
    if (wasPolling) {
      StartPolling();
    }
    return;
  }
  
  // Update agentEnabled field
  std::string updatedConfig = ReplaceBooleanValue(configResponse, gamemasterPos, "agentEnabled", newState);
  
  // Update enabled field (search from gamemaster position again since string changed)
  gamemasterPos = updatedConfig.find("\"gamemaster\"");
  updatedConfig = ReplaceBooleanValue(updatedConfig, gamemasterPos, "enabled", newState);
  
  if (updatedConfig == configResponse) {
    logger::error("Failed to update gamemaster fields - no changes detected");
    // Restart polling if it was active
    if (wasPolling) {
      StartPolling();
    }
    return;
  }
  
  logger::info("Updated config (length: {} bytes), sending to server", updatedConfig.length());
  
  // Step 3: Send the complete updated config back
  bool success = Http::Post(L"http://localhost:8080/config?api=update", updatedConfig);
  
  if (success) {
    logger::info("Successfully toggled GameMaster to {}", newState);
    
    // Fetch actual server state before updating UI
    std::string statusResponse = Http::Get(L"http://localhost:8080/?api=gamemaster-status");
    logger::info("Toggle: Status response = '{}'", statusResponse);
    
    if (!statusResponse.empty()) {
      bool actualState = ParseStatus(statusResponse);
      logger::info("Toggle: ParseStatus returned {} (expected {})", actualState, newState);
      
      enabled_.store(actualState);
      logger::info("Toggle: Calling UpdateUI({})", actualState);
      UpdateUI(actualState);  
      logger::info("Toggle: Updated UI with server-confirmed state: {}", actualState);
    } else {
      // Fallback to expected state if status check fails
      enabled_.store(newState);
      logger::info("Toggle: Calling UpdateUI({}) with expected value", newState);
      UpdateUI(newState);
      logger::warn("Could not verify server state, using expected value: {}", newState);
    }
  } else {
    logger::error("Failed to toggle GameMaster state");
  }
  
  // Restart polling if it was active before toggle
  if (wasPolling) {
    logger::info("Restarting polling after toggle operation");
    StartPolling();
  }
}

} // namespace SkyrimNet
} // namespace SkyrimNetUI
