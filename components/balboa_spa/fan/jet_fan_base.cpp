#include "jet_fan_base.h"

namespace esphome
{
    namespace balboa_spa
    {
        fan::FanTraits JetFanBase::get_traits()
        {
            return fan::FanTraits(false, true, false, 2);
        }

        void JetFanBase::update()
        {
            const SpaState *spaState = spa->get_current_state();
            this->sync_from_spa(
                spaState,
                current_fan_state,
                [this](uint8_t new_state)
                {
                    if (new_state == 0)
                    {
                        if (this->state != false)
                        {
                            this->state = false;
                            this->speed = 0;
                            this->publish_state();
                        }
                    }
                    else if (new_state == 1)
                    {
                        if (this->state != true || this->speed != 1)
                        {
                            this->state = true;
                            this->speed = 1;
                            this->publish_state();
                        }
                    }
                    else if (new_state == 2)
                    {
                        if (this->state != true || this->speed != 2)
                        {
                            this->state = true;
                            this->speed = 2;
                            this->publish_state();
                        }
                    }
                });
        }

        void JetFanBase::set_parent(BalboaSpa *parent)
        {
            JetToggleComponentBase::set_parent(parent);
            parent->register_listener([this]() { this->update(); });
            parent->register_retry_listener([this]() { this->retry_toggle(); });
        }

        void JetFanBase::control(const fan::FanCall &call)
        {
            // Handle state change
            if (call.get_state().has_value())
            {
                bool new_state = *call.get_state();
                if (new_state)
                {
                    // Turning ON - check if speed is specified
                    uint8_t target_speed = 1; // Default to LOW
                    if (call.get_speed().has_value())
                    {
                        uint8_t speed_val = *call.get_speed();
                        target_speed = (speed_val >= 2) ? 2 : 1;
                    }
                    else if (this->speed > 0)
                    {
                        // Use current speed if already set
                        target_speed = this->speed;
                    }

                    ESP_LOGD(tag, "Spa/%s/fan: turning ON at speed %d", jet_name, target_speed);
                    this->request_state_change(target_speed);
                }
                else
                {
                    // Turning OFF
                    ESP_LOGD(tag, "Spa/%s/fan: turning OFF", jet_name);
                    this->request_state_change(0);
                }
            }
            // Handle speed change while already ON
            else if (call.get_speed().has_value() && this->state)
            {
                uint8_t new_speed = *call.get_speed();
                uint8_t target_speed = (new_speed >= 2) ? 2 : 1;

                ESP_LOGD(tag, "Spa/%s/fan: changing speed to %d", jet_name, target_speed);
                this->request_state_change(target_speed);
            }
        }
    } // namespace balboa_spa
} // namespace esphome
