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

        void JetToggleComponentBase::request_state_change(uint8_t target_state)
        {
            const SpaState *spaState = spa_->get_current_state();
            if (get_jet_state(spaState) == target_state)
            {
                ESP_LOGD(tag, "Spa/%s: already at target state %d", jet_name, target_state);
                return;
            }
            ESP_LOGD(tag, "Spa/%s: queued state change to %d", jet_name, target_state);
            this->toggle_jet(target_state, 5);
        }

    } // namespace balboa_spa
} // namespace esphome
