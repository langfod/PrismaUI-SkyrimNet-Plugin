/*
* For modders: Copy this file into your own project if you wish to use this API.
* This is the EXTENDED API with inspector support (requires newer PrismaUI).
*/
#pragma once

#include <functional>
#include <queue>
#include <stdint.h>
#include <iostream>

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
	#define NOMINMAX
#endif
#include <windows.h>

typedef uint64_t PrismaView;

namespace PRISMA_UI_API
{
	constexpr const auto PrismaUIPluginName = "PrismaUI";

	enum class InterfaceVersion : uint8_t
	{
		V1
	};

	typedef void (*OnDomReadyCallback)(PrismaView view);
	typedef void (*JSCallback)(const char* result);
	typedef void (*JSListenerCallback)(const char* argument);

	// PrismaUI modder interface v1 (with inspector support)
	class IVPrismaUI1
	{
	public:
		// Create view.
		virtual PrismaView CreateView(const char* htmlPath, OnDomReadyCallback onDomReadyCallback = nullptr) noexcept = 0;

		// Send JS code to UI.
		virtual void Invoke(PrismaView view, const char* script, JSCallback callback = nullptr) noexcept = 0;

		// Call JS function through JS Interop API (best performance).
		virtual void InteropCall(PrismaView view, const char* functionName, const char* argument) noexcept = 0;

		// Register JS listener.
		virtual void RegisterJSListener(PrismaView view, const char* functionName, JSListenerCallback callback) noexcept = 0;

		// Returns true if view has focus.
		virtual bool HasFocus(PrismaView view) noexcept = 0;

		// Set focus on view.
		virtual bool Focus(PrismaView view, bool pauseGame = false) noexcept = 0;

		// Remove focus from view.
		virtual void Unfocus(PrismaView view) noexcept = 0;

		// Get scroll size in pixels.
		virtual int GetScrollingPixelSize(PrismaView view) noexcept = 0;

		// Set scroll size in pixels.
		virtual void SetScrollingPixelSize(PrismaView view, int pixelSize) noexcept = 0;

		// Returns true if view exists.
		virtual bool IsValid(PrismaView view) noexcept = 0;

		// Completely destroy view.
		virtual void Destroy(PrismaView view) noexcept = 0;

		// Get raw Ultralight View pointer for advanced operations (e.g., Inspector)
		virtual void* GetUltralightView(PrismaView view) noexcept = 0;

		// ========== INSPECTOR METHODS (requires newer PrismaUI) ==========

		// Create an Inspector View for debugging (requires inspector assets)
		// Note: This triggers inspector creation via ViewListener callback
		// The inspector view is managed internally by PrismaUI
		virtual void CreateInspectorView(PrismaView view) noexcept = 0;

		// Show or hide the overlay inspector that PrismaUI renders
		virtual void SetInspectorVisibility(PrismaView view, bool visible) noexcept = 0;

		// Returns true when the PrismaUI-managed inspector overlay is visible
		virtual bool IsInspectorVisible(PrismaView view) noexcept = 0;

		// Configure on-screen placement (top-left position in pixels) and size for the inspector overlay
		virtual void SetInspectorBounds(PrismaView view, float topLeftX, float topLeftY, uint32_t width, uint32_t height) noexcept = 0;

		// Retrieve the raw Ultralight View pointer for the inspector overlay (nullptr when unavailable)
		virtual void* GetInspectorView(PrismaView view) noexcept = 0;

		// =================================================================

		// Show the view.
		virtual void Show(PrismaView view) noexcept = 0;
		
		// Hide the view.
		virtual void Hide(PrismaView view) noexcept = 0;
		
		// Returns true if view is hidden.
		virtual bool IsHidden(PrismaView view) noexcept = 0;

		// Set view order.
		virtual void SetOrder(PrismaView view, int order) noexcept = 0;

		// Get view order.
		virtual int GetOrder(PrismaView view) noexcept = 0;
	};

	typedef void* (*_RequestPluginAPI)(const InterfaceVersion interfaceVersion);

	/// <summary>
	/// Request the PrismaUI API interface.
	/// Recommended: Send your request during or after SKSEMessagingInterface::kMessage_PostLoad to make sure the dll has already been loaded
	/// </summary>
	/// <param name="a_interfaceVersion">The interface version to request</param>
	/// <returns>The pointer to the API singleton, or nullptr if request failed</returns>
	[[nodiscard]] inline void* RequestPluginAPI(const InterfaceVersion a_interfaceVersion = InterfaceVersion::V1)
	{
		auto pluginHandle = GetModuleHandle("PrismaUI.dll");
		_RequestPluginAPI requestAPIFunction = (_RequestPluginAPI)GetProcAddress(pluginHandle, "RequestPluginAPI");

		if (requestAPIFunction) {
			return requestAPIFunction(a_interfaceVersion);
		}

		return nullptr;
	}
}
