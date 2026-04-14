#include "highrange_switch.h"

namespace esphome
{
    namespace balboa_spa
    {
        void HighrangeSwitch::update()
        {
            bool high = spa_->get_current_state()->highrange;
            if (this->state != high)
            {
                this->publish_state(high);
            }
        }

        void HighrangeSwitch::set_parent(BalboaSpa *parent)
        {
            spa_ = parent;
            parent->register_listener([this]() { this->update(); });
        }

        void HighrangeSwitch::write_state(bool state)
        {
            spa_->set_highrange(state);
        }

    } // namespace balboa_spa
} // namespace esphome
