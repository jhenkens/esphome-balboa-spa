#pragma once

#include "esphome/core/component.h"
#include "balboaspa.h"
#include <functional>

namespace esphome
{
    namespace balboa_spa
    {
        /**
         * @brief Base class for components that need toggle logic for jets/blower.
         *
         * Provides sync_from_spa() for reflecting spa state changes, and
         * request_state_change() to enqueue a toggle with expected state.
         * Retry is handled automatically by the command queue.
         */
        class JetToggleComponentBase
        {
        public:
            JetToggleComponentBase(const char *tag, const char *jet_name)
                : tag(tag), jet_name(jet_name) {};

            void set_parent(BalboaSpa *parent);

        protected:
            /**
             * @brief Get the current jet state from spa state
             * @return Current jet state (0=OFF, 1=LOW, 2=HIGH)
             */
            virtual uint8_t get_jet_state(const SpaState *spaState) = 0;

            /**
             * @brief Trigger a toggle command for this jet with expected final state.
             */
            virtual void toggle_jet(uint8_t expected_state, uint8_t max_retries) = 0;

            /**
             * @brief Sync ESPHome component state from the spa — no toggling.
             *        Call from register_listener on every state update.
             */
            void sync_from_spa(
                const SpaState *spaState,
                uint8_t &current_state,
                std::function<void(uint8_t)> publish_callback);

            /**
             * @brief Queue a state change toward target_state.
             * @param target_state Target state (0=OFF, 1=LOW, 2=HIGH)
             */
            void request_state_change(uint8_t target_state);

            BalboaSpa *spa_ = nullptr;
            const char *tag;
            const char *jet_name;
        };

    } // namespace balboa_spa
} // namespace esphome
