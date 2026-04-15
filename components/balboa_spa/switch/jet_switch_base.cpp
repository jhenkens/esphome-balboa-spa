#include "jet_switch_base.h"

namespace esphome
{
    namespace balboa_spa
    {
        void JetSwitchBase::update()
        {
            const SpaState *spaState = spa_->get_current_state();
            this->sync_from_spa(
                spaState,
                current_switch_state_,
                [this](uint8_t new_state)
                {
                    bool new_switch_state = (new_state > 0);
                    if (this->state != new_switch_state)
                    {
                        this->publish_state(new_switch_state);
                    }
                });
        }

        void JetSwitchBase::set_parent(BalboaSpa *parent)
        {
            JetToggleComponentBase::set_parent(parent);
            parent->register_listener([this]() { this->update(); });
        }

        void JetSwitchBase::write_state(bool state)
        {
            // For switch: OFF = 0, ON = 1 (LOW speed)
            uint8_t target_state = state ? on_level_ : 0;
            this->request_state_change(target_state);
        }

    } // namespace balboa_spa
} // namespace esphome
