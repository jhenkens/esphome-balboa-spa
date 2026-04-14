#include "jet_toggle_component_base.h"

namespace esphome
{
    namespace balboa_spa
    {
        void JetToggleComponentBase::set_parent(BalboaSpa *parent)
        {
            spa_ = parent;
        }

        void JetToggleComponentBase::sync_from_spa(
            const SpaState *spaState,
            uint8_t &current_state,
            std::function<void(uint8_t)> publish_callback)
        {
            uint8_t jet_state = this->get_jet_state(spaState);
            if (jet_state != current_state)
            {
                current_state = jet_state;
                publish_callback(jet_state);
                ESP_LOGD(tag, "Spa/%s: state updated to %d", jet_name, jet_state);
            }
        }

        void JetToggleComponentBase::retry_toggle()
        {
            const SpaState *spaState = spa_->get_current_state();
            if (this->desired_state == ToggleStateMaybe::DONT_KNOW) return;

            // Give up if the request is older than 60 seconds
            if ((millis() - this->request_time_) > 60000)
            {
                ESP_LOGW(tag, "Spa/%s: gave up after 60s without reaching target state", jet_name);
                this->desired_state = ToggleStateMaybe::DONT_KNOW;
                this->toggle_attempts = 0;
                this->toggle_inflight_ = false;
                return;
            }

            uint8_t jet_state = this->get_jet_state(spaState);
            uint8_t target = (uint8_t) this->desired_state;

            if (jet_state == target)
            {
                this->desired_state = ToggleStateMaybe::DONT_KNOW;
                this->toggle_attempts = 0;
                this->toggle_inflight_ = false;
                return;
            }

            // Wait for the previous toggle to be transmitted before sending another
            if (this->toggle_inflight_) return;

            if (this->toggle_attempts < this->max_toggle_attempts)
            {
                this->toggle_attempts++;
                this->toggle_inflight_ = true;
                this->toggle_jet([this]{ this->toggle_inflight_ = false; });
                ESP_LOGD(tag, "Spa/%s: toggling (attempt %d/%d) current=%d, target=%d",
                         jet_name, this->toggle_attempts, this->max_toggle_attempts,
                         jet_state, target);
            }
            else
            {
                if (target == 0 && jet_state >= 2)
                {
                    this->toggle_inflight_ = true;
                    this->toggle_jet([this]{ this->toggle_inflight_ = false; });
                    ESP_LOGW(tag, "Spa/%s: failed to turn OFF after %d attempts, trying final toggle from HIGH",
                             jet_name, this->max_toggle_attempts);
                }
                else
                {
                    ESP_LOGW(tag, "Spa/%s: failed to reach target state after %d attempts",
                             jet_name, this->max_toggle_attempts);
                }
                this->desired_state = ToggleStateMaybe::DONT_KNOW;
                this->toggle_attempts = 0;
            }
        }

        void JetToggleComponentBase::request_state_change(uint8_t target_state)
        {
            SpaState *spaState = spa_->get_current_state();
            uint8_t current_jet_state = this->get_jet_state(spaState);
            if (current_jet_state == target_state)
            {
                ESP_LOGD(tag, "Spa/%s: already at target state %d", jet_name, target_state);
                return;
            }
            ToggleStateMaybe target_state_enum = static_cast<ToggleStateMaybe>(target_state);
            if(target_state_enum == ToggleStateMaybe::DONT_KNOW) {
                ESP_LOGW(tag, "Spa/%s: invalid target state %d", jet_name, target_state);
                return;
            }
            this->desired_state = target_state_enum;
            this->toggle_attempts = 0;
            this->toggle_inflight_ = false;
            this->request_time_ = millis();
            this->spa_->request_retry_dispatch();
            ESP_LOGD(tag, "Spa/%s: queued state change from %d to %d", jet_name, current_jet_state, target_state);
        }

    } // namespace balboa_spa
} // namespace esphome
