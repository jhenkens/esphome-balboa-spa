#include "reconnect_button.h"

namespace esphome
{
    namespace balboa_spa
    {

        void ReconnectButton::set_parent(BalboaSpa *parent)
        {
            spa_ = parent;
        }

        void ReconnectButton::press_action()
        {
            spa_->reconnect();
        }

    } // namespace balboa_spa
} // namespace esphome
