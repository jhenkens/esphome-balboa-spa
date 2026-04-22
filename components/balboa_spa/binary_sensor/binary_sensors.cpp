#include "esphome/core/log.h"
#include "binary_sensors.h"

namespace esphome
{
    namespace balboa_spa
    {

        static const char *TAG = "BalboaSpa.binary_sensors";

        // Constant for time calculations
        static constexpr int MINUTES_PER_DAY = 24 * 60; // 1440 minutes in a day

        static bool is_filter_window_active(uint8_t current_hour, uint8_t current_minute,
                                            uint8_t start_hour, uint8_t start_minute,
                                            uint8_t duration_hour, uint8_t duration_minute)
        {
            const int current_minutes = current_hour * 60 + current_minute;
            const int start_minutes = start_hour * 60 + start_minute;
            const int end_minutes = start_minutes + duration_hour * 60 + duration_minute;
            if (end_minutes >= MINUTES_PER_DAY)
                return (current_minutes >= start_minutes) || (current_minutes < (end_minutes - MINUTES_PER_DAY));
            return (current_minutes >= start_minutes) && (current_minutes < end_minutes);
        }

        static bool time_is_invalid(uint8_t hour, uint8_t minute)
        {
            // Use the current system time (epoch time converted to local time)
            time_t now_timestamp = time(nullptr);
            struct tm *time_info = localtime(&now_timestamp);

            if (time_info == nullptr || now_timestamp < 1000000000) // Basic validity check
            {
                ESP_LOGD(TAG, "System time not valid yet");
                return true; // Not synced if system time is not valid
            }
            else
            {
                // Consider time synced if spa time matches system time within a 5-minute window
                const int spa_minutes = hour * 60 + minute;
                const int system_minutes = time_info->tm_hour * 60 + time_info->tm_min;
                const int diff = abs(spa_minutes - system_minutes);
                return !((diff <= 5) || (diff >= (MINUTES_PER_DAY - 5))); // Account for wrap-around
            }
        }

        void BalboaSpaBinarySensors::set_parent(BalboaSpa *parent)
        {
            this->spa_ = parent;
            parent->register_listener([this]() { this->update(); });
        }

        void BalboaSpaBinarySensors::update()
        {
            const SpaState *spaState = spa_->get_current_state();
            bool sensor_state_value;
            if (spa_ == nullptr || spaState == nullptr || (!spa_->is_communicating() && sensor_type != BalboaSpaBinarySensorType::CONNECTED))
            {
                this->publish_state(NAN);
                return;
            }

            // Fetch filter settings if needed
            SpaFilterSettings *filterSettings = nullptr;
            if (sensor_type == BalboaSpaBinarySensorType::FILTER1_WINDOW_ACTIVE ||
                sensor_type == BalboaSpaBinarySensorType::FILTER2_WINDOW_ACTIVE)
            {
                filterSettings = spa_->get_current_filter_settings();
                if (filterSettings == nullptr)
                {
                    this->publish_state(NAN);
                    return;
                }
            }

            uint8_t state_value = 0;
            switch (sensor_type)
            {
            case BalboaSpaBinarySensorType::BLOWER:
                sensor_state_value = spaState->blower;
                break;
            case BalboaSpaBinarySensorType::HIGH_RANGE:
                sensor_state_value = spaState->highrange;
                break;
            case BalboaSpaBinarySensorType::CIRCULATION:
                sensor_state_value = spaState->circulation;
                break;
            case BalboaSpaBinarySensorType::REST_MODE:
                if (spaState->rest_mode == HeatingMode::NOT_YET_RECEIVED)
                    return;
                sensor_state_value = (spaState->rest_mode == HeatingMode::REST ||
                                      spaState->rest_mode == HeatingMode::READY_IN_REST);
                break;
            case BalboaSpaBinarySensorType::HEAT_STATE:
                state_value = spaState->heat_state;
                if (state_value != 254)
                {
                    sensor_state_value = state_value;
                }
                break;
            case BalboaSpaBinarySensorType::CONNECTED:
                sensor_state_value = spa_->is_communicating();
                break;
            case BalboaSpaBinarySensorType::FILTER1_WINDOW_ACTIVE:
                sensor_state_value = is_filter_window_active(
                    spaState->hour, spaState->minutes,
                    filterSettings->filter1_hour, filterSettings->filter1_minute,
                    filterSettings->filter1_duration_hour, filterSettings->filter1_duration_minute);
                break;
            case BalboaSpaBinarySensorType::FILTER2_WINDOW_ACTIVE:
                if (filterSettings->filter2_enable)
                    sensor_state_value = is_filter_window_active(
                        spaState->hour, spaState->minutes,
                        filterSettings->filter2_hour, filterSettings->filter2_minute,
                        filterSettings->filter2_duration_hour, filterSettings->filter2_duration_minute);
                else
                    sensor_state_value = false;
                break;
            case BalboaSpaBinarySensorType::CLEANUP_CYCLE:
                sensor_state_value = spaState->cleanup_cycle;
                break;
            case BalboaSpaBinarySensorType::TIME_SYNCED:
                sensor_state_value = time_is_invalid(spaState->hour, spaState->minutes);
                break;
            default:
                ESP_LOGD(TAG, "Spa/BSensors/UnknownSensorType: SensorType Number: %d", sensor_type);
                // Unknown enum value. Ignore
                return;
            }

            if (this->state != sensor_state_value)
            {
                this->publish_state(sensor_state_value);
            }
        }

        BalboaSpaBinarySensors::BalboaSpaBinarySensors()
        {
            spa_ = nullptr;
            sensor_type = BalboaSpaBinarySensorType::UNKNOWN;
        }

    }
}
