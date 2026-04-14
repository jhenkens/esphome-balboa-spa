#include "sync_time_button.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome
{
    namespace balboa_spa
    {
        static const char *TAG = "balboa_spa.button";

        void SyncTimeButton::set_parent(BalboaSpa *parent)
        {
            spa_ = parent;
        }

        void SyncTimeButton::press_action()
        {
            ESP_LOGD(TAG, "Sync time button pressed");

            // Use the current system time (epoch time converted to local time)
            time_t now_timestamp = time(nullptr);
            struct tm *time_info = localtime(&now_timestamp);

            if (time_info == nullptr || now_timestamp < 1000000000) // Basic validity check
            {
                ESP_LOGW(TAG, "System time not valid yet");
                return;
            }

            // Set the spa time
            spa_->set_time(time_info->tm_hour, time_info->tm_min);
            
            // Request spa settings update to refresh the display
            spa_->request_config_update();
        }

    } // namespace balboa_spa
} // namespace esphome
