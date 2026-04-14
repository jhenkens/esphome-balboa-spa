#include "rest_mode_switch.h"
#include "esphome/core/log.h"

namespace esphome
{
    namespace balboa_spa
    {
        static const char *TAG = "balboa_spa.switch.rest_mode";

        void RestModeSwitch::update()
        {
            const SpaState *spaState = spa_->get_current_state();
            bool in_rest = (spaState->rest_mode == HeatingMode::REST);
            if (this->state != in_rest)
            {
                this->publish_state(in_rest);
            }
        }

        void RestModeSwitch::set_parent(BalboaSpa *parent)
        {
            spa_ = parent;
            parent->register_listener([this]() { this->update(); });
        }

        void RestModeSwitch::write_state(bool state)
        {
            spa_->set_rest_mode(state);
        }

    } // namespace balboa_spa
} // namespace esphome
