#include "pch.h"
#include "UIBridge.h"
#include "../skyrimnet/GameMasterController.h"
#include "../keyhandler/keyhandler.h"

namespace SkyrimNetUI {
namespace UI {

static PRISMA_UI_API::IVPrismaUI1* g_prismaUI = nullptr;
static PrismaView g_view = 0;
#ifdef PRISMAUI_ENABLE_INSPECTOR
static bool g_inspectorInitialized = false;
#endif
static KeyHandler* g_keyHandler = nullptr;
static KeyHandlerEvent g_toggleEventHandler = 0;
#ifdef PRISMAUI_ENABLE_INSPECTOR
static KeyHandlerEvent g_inspectorEventHandler = 0;
#endif

constexpr uint32_t TOGGLE_FOCUS_KEY = 0x3E;       // F4 key
#ifdef PRISMAUI_ENABLE_INSPECTOR
constexpr uint32_t TOGGLE_INSPECTOR_KEY = 0x41;   // F7 key

static bool EnsureInspectorSetup();
#endif

void Initialize() {
  // Check if already fully initialized
  if (g_prismaUI && g_view != 0) {
    logger::warn("UI already initialized with view [{}]", g_view);
    return;
  }

  // Only request API once
  if (!g_prismaUI) {
    g_prismaUI = static_cast<PRISMA_UI_API::IVPrismaUI1*>(
        PRISMA_UI_API::RequestPluginAPI(PRISMA_UI_API::InterfaceVersion::V1));

    if (!g_prismaUI) {
      logger::critical("PrismaUI API pointer is null. Cannot create view.");
      return;
    }
  }

  constexpr const char* kViewPath = "PrismaUI-SkyrimNet-UI/index.html";

  // Only create view once - check both that g_view is set AND valid
  if (g_view == 0 || !g_prismaUI->IsValid(g_view)) {
    logger::info("Creating PrismaUI view with path: '{}'.", kViewPath);

    g_view = g_prismaUI->CreateView(kViewPath, []([[maybe_unused]] PrismaView v) {
      logger::info("View DOM is ready {}", v);

      g_prismaUI->InteropCall(g_view, "toggleSkyrimNetUIDiv", "hide");

      // Register key handlers
      if (g_keyHandler) {
        if (!g_toggleEventHandler) {
          g_toggleEventHandler = g_keyHandler->Register(
              TOGGLE_FOCUS_KEY, KeyEventType::KEY_DOWN, ToggleView);
        }
#ifdef PRISMAUI_ENABLE_INSPECTOR
        if (!g_inspectorEventHandler) {
          g_inspectorEventHandler = g_keyHandler->Register(
              TOGGLE_INSPECTOR_KEY, KeyEventType::KEY_DOWN, ToggleInspector);
        }
#endif
      } else {
        logger::warn("KeyHandler not ready when DOM became ready");
      }

      // Note: GameMaster polling is started when view is shown (in ToggleView),
      // not during initialization. This prevents unnecessary HTTP requests when
      // the view is hidden.
    });

    // Note: View is created asynchronously on UI thread, so g_view is valid immediately
    // even though the actual Ultralight View won't exist until later.
    logger::info("View [{}] creation requested successfully.", g_view);
  }

  // Register JS listeners (only once, outside the creation block)
  // These will be active once the DOM is ready
  if (g_view != 0) {
    g_prismaUI->RegisterJSListener(g_view, "closePrismaUIWindow",
                                   [](const char*) -> void {
                                     logger::info("Received close from JS");
                                     if (g_prismaUI) {
                                       g_prismaUI->Unfocus(g_view);
                                       g_prismaUI->InteropCall(g_view, "toggleSkyrimNetUIDiv", "hide");
#ifdef PRISMAUI_ENABLE_INSPECTOR
                                       if (g_prismaUI->IsInspectorVisible(g_view)) {
                                         g_prismaUI->SetInspectorVisibility(g_view, false);
                                       }
#endif
                                     }
                                   });

    g_prismaUI->RegisterJSListener(g_view, "onGameMasterToggle",
                                   [](const char*) -> void {
                                     logger::info("GameMaster toggle requested from JS");
                                     SkyrimNet::GetController().Toggle();
                                   });
  }

  // Initialize key handler early so it's ready when DOM callback fires
  KeyHandler::RegisterSink();
  g_keyHandler = KeyHandler::GetSingleton();

  logger::info("UI initialized successfully");
}

void Shutdown() {
  SkyrimNet::GetController().StopPolling();
  g_prismaUI = nullptr;
  g_view = 0;
  logger::info("UI shutdown complete");
}

void ToggleView() {
  logger::info("ToggleView called with g_view = [{}], g_prismaUI = {}", g_view, (void*)g_prismaUI);
  
  if (!g_prismaUI) {
    logger::critical("PrismaUI API pointer is null. Cannot toggle view.");
    return;
  }
  
  logger::info("About to call IsValid on view [{}]...", g_view);
  const bool isValid = g_prismaUI->IsValid(g_view);
  logger::info("IsValid returned: {}", isValid);
  
  if (!isValid) {
    logger::critical("Failed to create PrismaUI view. View handle is invalid.");
    return;
  }

  logger::info("Current view value before toggle: '{}'", g_view);

  const bool hasFocus = g_prismaUI->HasFocus(g_view);
  if (!hasFocus) {
    g_prismaUI->InteropCall(g_view, "toggleSkyrimNetUIDiv", "show");
    g_prismaUI->Focus(g_view, false);
#ifdef PRISMAUI_ENABLE_INSPECTOR
    EnsureInspectorSetup();
#endif
    // Start polling when view becomes visible
    SkyrimNet::GetController().StartPolling();
    logger::info("Called InteropCall show to 'skyrimnet-ui' div. GameMaster polling started.");
  } else {
#ifdef PRISMAUI_ENABLE_INSPECTOR
    if (g_prismaUI->IsInspectorVisible(g_view)) {
      g_prismaUI->SetInspectorVisibility(g_view, false);
      logger::info("Inspector overlay hidden while closing SkyrimNet UI view.");
    }
#endif
    // Stop polling when view becomes hidden
    SkyrimNet::GetController().StopPolling();
    g_prismaUI->Unfocus(g_view);
    g_prismaUI->InteropCall(g_view, "toggleSkyrimNetUIDiv", "hide");
    logger::info("Called InteropCall hide to 'skyrimnet-ui' div. GameMaster polling stopped.");
  }
}

void UpdateGameMasterStatus(bool enabled) {
  logger::info("UIBridge::UpdateGameMasterStatus called with enabled={}", enabled);
  
  if (!g_prismaUI || !g_prismaUI->IsValid(g_view)) {
    logger::warn("UIBridge::UpdateGameMasterStatus: PrismaUI or view is not valid");
    return;
  }
  
  const char* enabledStr = enabled ? "true" : "false";
  logger::info("UIBridge::UpdateGameMasterStatus: Calling InteropCall with '{}'", enabledStr);
  g_prismaUI->InteropCall(g_view, "updateGameMasterStatus", enabledStr);
  logger::info("UIBridge::UpdateGameMasterStatus: InteropCall completed");
}

PrismaView GetView() {
  return g_view;
}

bool HasFocus() {
  if (!g_prismaUI || !g_prismaUI->IsValid(g_view)) {
    return false;
  }
  return g_prismaUI->HasFocus(g_view);
}

#ifdef PRISMAUI_ENABLE_INSPECTOR
static bool EnsureInspectorSetup() {
  if (!g_prismaUI || !g_prismaUI->IsValid(g_view)) {
    return false;
  }

  if (!g_inspectorInitialized) {
    logger::info("Requesting inspector view for SkyrimNet UI.");
    g_prismaUI->CreateInspectorView(g_view);

    constexpr float inspectorPosX = 48.0f;
    constexpr float inspectorPosY = 48.0f;
    constexpr uint32_t inspectorWidth = 1200u;
    constexpr uint32_t inspectorHeight = 640u;

    g_prismaUI->SetInspectorBounds(g_view, inspectorPosX, inspectorPosY, inspectorWidth, inspectorHeight);
    g_prismaUI->SetInspectorVisibility(g_view, false);

    g_inspectorInitialized = true;
    logger::info("Inspector view initialized with default placement.");
  }

  return true;
}

void ToggleInspector() {
  if (!g_prismaUI || !g_prismaUI->IsValid(g_view)) {
    logger::warn("Inspector toggle requested but UI view is not ready");
    return;
  }

  // Inspector can be toggled even when main UI doesn't have focus
  if (!g_inspectorInitialized) {
    logger::warn("Inspector not yet initialized. Open the interface (F4) first to initialize it.");
    return;
  }

  const bool currentlyVisible = g_prismaUI->IsInspectorVisible(g_view);
  g_prismaUI->SetInspectorVisibility(g_view, !currentlyVisible);

  if (!currentlyVisible) {
    logger::info("Inspector overlay opened.");
  } else {
    logger::info("Inspector overlay hidden.");
  }
}
#endif // PRISMAUI_ENABLE_INSPECTOR

} // namespace UI
} // namespace SkyrimNetUI
