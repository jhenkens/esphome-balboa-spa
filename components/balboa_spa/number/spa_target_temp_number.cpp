#include "esphome/core/log.h"
#include "spa_target_temp_number.h"

namespace esphome
{
    namespace balboa_spa
    {

        static const char *TAG = "BalboaSpa.number";

        void SpaTargetTempNumber::set_parent(BalboaSpa *parent)
        {
            bool fahrenheit = this->get_unit_of_measurement_ref().compare("°F") == 0;
            spa_temp_init(parent, fahrenheit);
            update_traits();
        }

        void SpaTargetTempNumber::update_traits()
        {
            this->traits.set_min_value(range_min());
            this->traits.set_max_value(range_max());
            this->traits.set_step(temp_step());
        }

        void SpaTargetTempNumber::update()
        {
            float newState;
            if (sync_temp(spa_->get_current_state()->target_temp, newState))
                this->publish_state(newState);
        }

        void SpaTargetTempNumber::control(float value)
        {
            ESP_LOGD(TAG, "Setting target temperature to %.1f", value);
            spa_->set_temp(to_internal(value));
        }

    } // namespace balboa_spa
} // namespace esphome
