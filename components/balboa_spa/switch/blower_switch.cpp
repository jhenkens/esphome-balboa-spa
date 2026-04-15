#include "blower_switch.h"

namespace esphome
{
    namespace balboa_spa
    {
        void BlowerSwitch::toggle_jet(uint8_t expected_state, uint8_t max_retries)
        {
            spa_->toggle_blower(expected_state, max_retries);
        }

    } // namespace balboa_spa
} // namespace esphome
