#include "esphome/core/log.h"
#include "sensors.h"

namespace esphome
{
    namespace balboa_spa
    {

        static const char *TAG = "BalboaSpa.sensors";

        void BalboaSpaSensors::set_parent(BalboaSpa *parent)
        {
            spa_temp_init(parent, false, this->get_unit_of_measurement_ref().compare("°F") == 0 ? TEMP_SCALE::F : TEMP_SCALE::UNKNOWN);
        }

        void BalboaSpaSensors::update()
        {
            if (sensor_type == BalboaSpaSensorType::TIME_SINCE_LAST_STATUS)
            {
                uint32_t age_ms = spa_->get_time_since_last_status_ms();
                float age_s = age_ms / 1000.0f;
                if (age_s < 120.0f)
                    age_s = 0.0f;
                if (age_s != this->state)
                    this->publish_state(age_s);
                return;
            }

            const SpaState *spaState = spa_->get_current_state();
            if (sensor_type == BalboaSpaSensorType::CURRENT_TEMP || sensor_type == BalboaSpaSensorType::TARGET_TEMP)
            {
                float val_c = sensor_type == BalboaSpaSensorType::CURRENT_TEMP ? spaState->current_temp : spaState->target_temp;
                if (!std::isnan(val_c))
                {
                    float display = to_display(val_c);
                    if (display != this->state)
                        this->publish_state(display);
                }
                return;
            }

            uint8_t sensor_state_value;

            switch (sensor_type)
            {
            case BalboaSpaSensorType::BLOWER:
                sensor_state_value = spaState->blower;
                break;
            case BalboaSpaSensorType::HIGH_RANGE:
                sensor_state_value = spaState->highrange;
                break;
            case BalboaSpaSensorType::CIRCULATION:
                sensor_state_value = spaState->circulation;
                break;
            case BalboaSpaSensorType::REST_MODE:
                if (spaState->rest_mode == HeatingMode::NOT_YET_RECEIVED)
                    return;
                sensor_state_value = (uint8_t)spaState->rest_mode;
                break;
            case BalboaSpaSensorType::HEAT_STATE:
                sensor_state_value = spaState->heat_state;
                if (sensor_state_value == 254)
                    return;
                break;
            case BalboaSpaSensorType::SPA_TEMP_SCALE:
            {
                TEMP_SCALE scale = spa_->get_spa_temp_scale();
                if (scale == TEMP_SCALE::UNKNOWN)
                    return;
                sensor_state_value = static_cast<uint8_t>(scale);
                break;
            }
            default:
                ESP_LOGD(TAG, "Spa/Sensors/UnknownSensorType: SensorType Number: %d", sensor_type);
                return;
            }

            if (this->state != sensor_state_value)
                this->publish_state(sensor_state_value);
        }
    }
}
