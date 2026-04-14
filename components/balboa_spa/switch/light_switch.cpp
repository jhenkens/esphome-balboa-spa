#include "light_switch.h"
#include <cassert>

namespace esphome
{
    namespace balboa_spa
    {
        LightSwitch::LightSwitch(uint8_t index) : index_(index)
        {
            assert(index >= 1 && index <= SpaState::LIGHT_COUNT);
        }

        void LightSwitch::update()
        {
            bool new_state = spa_->get_current_state()->lights[index_ - 1];
            if (this->state != new_state)
                this->publish_state(new_state);
        }

        void LightSwitch::set_parent(BalboaSpa *parent)
        {
            spa_ = parent;
            parent->register_listener([this]() { this->update(); });
        }

        void LightSwitch::write_state(bool state)
        {
            if (spa_->get_current_state()->lights[index_ - 1] != state)
                spa_->toggle_light(index_);
        }

    } // namespace balboa_spa
} // namespace esphome
