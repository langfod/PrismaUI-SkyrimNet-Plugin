


// Ensure pch.h is included first for logger and SKSE types
#include "pch.h"
#include "PrismaUI_API.h"
#include "keyhandler/keyhandler.h"


PRISMA_UI_API::IVPrismaUI1* PrismaUI =nullptr;
static PrismaView view = 0;
// Toggle the PrismaUI view on keypress
static void TogglePrismaUIView() {
    if (!PrismaUI) {
        logger::critical("PrismaUI API pointer is null. Cannot toggle view.");
        return;
    }

    logger::info("Current view value before toggle: '{}'", view);

    constexpr const char* kViewPath = "PrismaUI-SkyrimNet-UI/index.html";
    // Create the view once if not already created
    if (!PrismaUI->IsValid(view)) {
        logger::info("Creating PrismaUI view with path: '{}'.", kViewPath);
        view = PrismaUI->CreateView(kViewPath, [](PrismaView v) {
            logger::info("View DOM is ready {}", v);
            PrismaUI->Focus(view, false);
        });
        if (!PrismaUI->IsValid(view)) {
            logger::critical("Failed to create PrismaUI view. View handle is invalid.");
            return;
        }
    }
    auto hasFocus = PrismaUI->HasFocus(view);
    if (!hasFocus) {
        PrismaUI->InteropCall(view, "toggleSkyrimNetUIDiv", "show");
        PrismaUI->Focus(view, false);
        logger::info("Called InteropCall show to 'skyrimnet-ui' div.");
    } else {
        PrismaUI->Unfocus(view);
        PrismaUI->InteropCall(view, "toggleSkyrimNetUIDiv", "hide");
        logger::info("Called InteropCall hide to 'skyrimnet-ui' div.");

    }
}

// SKSE message handler for plugin initialization
static void SKSEMessageHandler(SKSE::MessagingInterface::Message* message)
{
    switch (message->type) {
    case SKSE::MessagingInterface::kDataLoaded:
        // Initialize PrismaUI API
        PrismaUI = static_cast<PRISMA_UI_API::IVPrismaUI1*>(PRISMA_UI_API::RequestPluginAPI(PRISMA_UI_API::InterfaceVersion::V1));

        // Register key event handler for toggling view
        KeyHandler::RegisterSink();
        KeyHandler* keyHandler = KeyHandler::GetSingleton();
        const uint32_t TOGGLE_FOCUS_KEY = 0x3E; // F4 key

        KeyHandlerEvent toggleEventHandler = keyHandler->Register(TOGGLE_FOCUS_KEY, KeyEventType::KEY_DOWN, TogglePrismaUIView);

        // If you want to unregister the key event handlers:
        // keyHandler->Unregister(toggleEventHandler);
        break;
    }
}


// SKSE plugin entry point
extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
    REL::Module::reset();

    auto g_messaging = reinterpret_cast<SKSE::MessagingInterface*>(a_skse->QueryInterface(SKSE::LoadInterface::kMessaging));

    if (!g_messaging) {
        logger::critical("Failed to load messaging interface! This error is fatal, plugin will not load.");
        return false;
    }

    logger::info("{} v{}"sv, Plugin::NAME, Plugin::VERSION.string());

    SKSE::Init(a_skse);
    SKSE::AllocTrampoline(1 << 10);

    g_messaging->RegisterListener("SKSE", SKSEMessageHandler);

    return true;
}
