#pragma once

#include "PrismaUI_API.h"

namespace SkyrimNetUI {
namespace UI {

/**
 * @brief Initialize the UI bridge and create the view
 */
void Initialize();

/**
 * @brief Cleanup UI resources
 */
void Shutdown();

/**
 * @brief Toggle the main UI view visibility
 */
void ToggleView();

/**
 * @brief Update GameMaster status indicator in UI
 * @param enabled Current GameMaster enabled state
 */
void UpdateGameMasterStatus(bool enabled);

/**
 * @brief Get the current PrismaUI view handle
 */
PrismaView GetView();

/**
 * @brief Check if UI has focus
 */
bool HasFocus();

#ifdef PRISMAUI_ENABLE_INSPECTOR
/**
 * @brief Toggle inspector overlay visibility
 */
void ToggleInspector();
#endif

} // namespace UI
} // namespace SkyrimNetUI
