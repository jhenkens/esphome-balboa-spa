#include "request_filter_settings_button.h"
#include "esphome/core/log.h"

namespace esphome
{
    namespace balboa_spa
    {
        static const char *TAG = "balboa_spa.button";

        void RequestFilterSettingsButton::set_parent(BalboaSpa *parent)
        {
            spa_ = parent;
        }

        void RequestFilterSettingsButton::press_action()
        {
            ESP_LOGD(TAG, "Request filter settings button pressed");
            spa_->request_filter_settings_update();
            ESP_LOGI(TAG, "Filter settings update requested");
        }

    } // namespace balboa_spa
} // namespace esphome
