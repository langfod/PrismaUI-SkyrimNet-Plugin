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
  REL::Module::reset();

  auto g_messaging = reinterpret_cast<SKSE::MessagingInterface*>(
      a_skse->QueryInterface(SKSE::LoadInterface::kMessaging));

  if (!g_messaging) {
    logger::critical("Failed to load messaging interface! This error is fatal, plugin will not load.");
    return false;
  }

  logger::info("{} v{}", Plugin::NAME, Plugin::VERSION.string());

  SKSE::Init(a_skse);
  SKSE::AllocTrampoline(1 << 10);

  g_messaging->RegisterListener("SKSE", SKSEMessageHandler);

  return true;
}

// Clean up on plugin unload
extern "C" DLLEXPORT void SKSEAPI SKSEPlugin_Unload() {
  SkyrimNetUI::UI::Shutdown();
}
