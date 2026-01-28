// Ensure pch.h is included first for logger and SKSE types
#include "pch.h"
#include "plugin.h"
#include "ui/UIBridge.h"

// SKSE message handler for plugin initialization
static void SKSEMessageHandler(SKSE::MessagingInterface::Message* message) {
  switch (message->type) {
    case SKSE::MessagingInterface::kDataLoaded:
      SkyrimNetUI::UI::Initialize();
      break;
  }
}

// SKSE plugin entry point
extern "C" DLLEXPORT bool SKSEAPI
SKSEPlugin_Load(const SKSE::LoadInterface* a_skse) {
  //REL::Module::reset();
  SKSE::Init(a_skse, false); // false = don't initialize logger by default
  logger::init();
  // pattern: [2024-01-01 12:00:00.000] [info] [1234] [sourcefile.cpp:123] Log message
  spdlog::set_pattern("[%Y-%m-%d %T.%e] [%l] [%t] [%s:%#] %v");

  logger::info("{} v{}", Plugin::NAME, Plugin::VERSION.string());
  logger::info("  built using CommonLibSSE-NG v{}", COMMONLIBSSE_VERSION);
  logger::info("  Running on Skyrim v{}", REL::Module::get().version().string());
  
  auto g_messaging = reinterpret_cast<SKSE::MessagingInterface*>(
      a_skse->QueryInterface(SKSE::LoadInterface::kMessaging));

  if (!g_messaging) {
    logger::critical("Failed to load messaging interface! This error is fatal, plugin will not load.");
    return false;
  }

  SKSE::AllocTrampoline(1 << 10);

  g_messaging->RegisterListener("SKSE", SKSEMessageHandler);

  return true;
}

// Clean up on plugin unload
extern "C" DLLEXPORT void SKSEAPI SKSEPlugin_Unload() {
  SkyrimNetUI::UI::Shutdown();
}
