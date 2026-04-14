#include "clear_reminder_button.h"

namespace esphome
{
    namespace balboa_spa
    {

        void ClearReminderButton::set_parent(BalboaSpa *parent)
        {
            spa_ = parent;
        }

        void ClearReminderButton::press_action()
        {
            spa_->clear_reminder();
        }

    } // namespace balboa_spa
} // namespace esphome
