#pragma once

#include "esphome/core/component.h"
#include "balboaspa.h"
#include <functional>

namespace esphome
{
    namespace balboa_spa
    {
        /**
         * @brief Base class for components that need toggle logic with retry capability
         *
         * This class provides shared toggle logic for both switches and fans that control
         * jets. It includes MAX_TOGGLE_ATTEMPTS functionality to handle cases where the
         * spa doesn't allow state changes (e.g., during heating or filter cycles).
         */
        class JetToggleComponentBase
        {
        public:
            JetToggleComponentBase(const char *tag, const char *jet_name)
                : tag(tag), jet_name(jet_name) {};

            void set_parent(BalboaSpa *parent);
            void set_max_toggle_attempts(uint8_t value) { this->max_toggle_attempts = value; }

        protected:
            /**
             * @brief Get the current jet state from spa state
             * @param spaState Current spa state
             * @return Current jet state (0=OFF, 1=LOW, 2=HIGH)
             */
            virtual uint8_t get_jet_state(const SpaState *spaState) = 0;

            /**
             * @brief Trigger a toggle command for this jet.
             *        on_sent is called when the command is actually transmitted.
             */
            virtual void toggle_jet(std::function<void()> on_sent) = 0;

            /**
             * @brief Update logic called when spa state changes
             * @param spaState Current spa state
             * @param desired_state Desired jet state (0=OFF, 1=LOW, 2=HIGH)
             * @param current_state Reference to current component state for comparison
             * @param publish_callback Callback to publish new state
             */
            /**
             * @brief Sync ESPHome component state from the spa — no toggling.
             *        Call from register_listener on every state update.
             */
            void sync_from_spa(
                const SpaState *spaState,
                uint8_t &current_state,
                std::function<void(uint8_t)> publish_callback);

            /**
             * @brief Drive the spa toward the desired state via toggle commands.
             *        Call from register_retry_listener (once per second).
             */
            void retry_toggle();

            /**
             * @brief Queue a state change — does NOT send an immediate toggle.
             * @param target_state Target state (0=OFF, 1=LOW, 2=HIGH)
             */
            void request_state_change(uint8_t target_state);

            BalboaSpa *spa_ = nullptr;
            const char *tag;
            const char *jet_name;

        private:
            ToggleStateMaybe desired_state = ToggleStateMaybe::DONT_KNOW;
            uint8_t toggle_attempts = 0;
            uint8_t max_toggle_attempts = 5;
            bool toggle_inflight_ = false;
            uint32_t request_time_ = 0;
        };

    } // namespace balboa_spa
} // namespace esphome
