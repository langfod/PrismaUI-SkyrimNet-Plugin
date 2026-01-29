#pragma once

#include <atomic>
#include <string>
#include <thread>

namespace SkyrimNetUI::SkyrimNet {

    /**
     * @brief Manages GameMaster agent status monitoring and control
     */
    class Controller {
    public:
        Controller();
        ~Controller();

        // Prevent copying
        Controller(const Controller&) = delete;
        Controller& operator=(const Controller&) = delete;

        /**
         * @brief Start background polling of GameMaster status
         */
        void StartPolling();

        /**
         * @brief Stop background polling
         */
        void StopPolling();

        /**
         * @brief Toggle GameMaster enabled state
         * Retrieves current config, updates gamemaster.enabled and gamemaster.agentEnabled,
         * then sends complete config back to server
         */
        void Toggle();

        /**
         * @brief Get current GameMaster enabled state
         */
        bool IsEnabled() const noexcept { return enabled_.load(); }

        /**
         * @brief Update UI with current GameMaster status
         */
        void UpdateUI(bool enabled);

    private:
        void PollStatus();
        bool ParseStatus(const std::string& jsonResponse);
        std::string UpdateConfigFields(const std::string& configJson, bool newState);

        std::atomic<bool> enabled_{false};
        std::atomic<bool> pollingActive_{false};
        std::thread pollingThread_;
    };

    // Global singleton instance
    Controller& GetController();

}  // namespace SkyrimNetUI::SkyrimNet
