#include "blower_switch.h"

namespace esphome
{
    namespace balboa_spa
    {
        void BlowerSwitch::toggle_jet(std::function<void()> on_sent)
        {
            spa_->toggle_blower(std::move(on_sent));
        }

    } // namespace balboa_spa
} // namespace esphome
