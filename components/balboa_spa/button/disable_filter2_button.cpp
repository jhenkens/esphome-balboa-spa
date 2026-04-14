#include "disable_filter2_button.h"
#include "esphome/core/log.h"

namespace esphome
{
    namespace balboa_spa
    {
        static const char *TAG = "balboa_spa.button";

        void DisableFilter2Button::set_parent(BalboaSpa *parent)
        {
            spa_ = parent;
        }

        void DisableFilter2Button::press_action()
        {
            ESP_LOGD(TAG, "Disable Filter 2 button pressed");
            spa_->disable_filter2();
            ESP_LOGI(TAG, "Filter 2 disabled");
            // Request fresh filter settings from spa after update
            spa_->request_filter_settings_update();
        }

    } // namespace balboa_spa
} // namespace esphome