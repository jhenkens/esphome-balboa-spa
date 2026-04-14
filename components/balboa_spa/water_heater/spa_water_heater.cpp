#include "esphome.h"
#include "esphome/core/log.h"
#include "spa_water_heater.h"

namespace esphome
{
    namespace balboa_spa
    {

        static const char *TAG = "balboa_spa.water_heater";

        void BalboaSpaWaterHeater::set_parent(BalboaSpa *parent)
        {
            bool fahrenheit = false;
#ifdef USE_WATER_HEATER_TEMPERATURE_UNIT
            fahrenheit = this->temperature_unit_override_ == water_heater::WaterHeaterTemperatureUnit::WATER_HEATER_TEMPERATURE_UNIT_FAHRENHEIT;
#endif
            spa_temp_init(parent, fahrenheit);
        }

        water_heater::WaterHeaterTraits BalboaSpaWaterHeater::traits()
        {
            update_traits();
            return traits_;
        }

        void BalboaSpaWaterHeater::update_traits()
        {
            traits_.set_supported_modes({
                water_heater::WATER_HEATER_MODE_OFF,
                water_heater::WATER_HEATER_MODE_ECO,
                water_heater::WATER_HEATER_MODE_ELECTRIC    ,
            });
            traits_.set_supports_current_temperature(true);
            traits_.set_min_temperature(range_min());
            traits_.set_max_temperature(range_max());
            traits_.set_target_temperature_step(temp_step());
        }

        void BalboaSpaWaterHeater::update()
        {
            const SpaState *s = spa_->get_current_state();
            bool needs_update = false;
            if (!spa_sync_temps(s, this->target_temperature_, this->current_temperature_, needs_update))
                return;

            if (s->rest_mode != HeatingMode::NOT_YET_RECEIVED)
            {
                water_heater::WaterHeaterMode new_mode;
                if (spa_->get_restmode())
                    new_mode = water_heater::WATER_HEATER_MODE_OFF;
                else if (spa_->get_highrange())
                    new_mode = water_heater::WATER_HEATER_MODE_ELECTRIC;
                else
                    new_mode = water_heater::WATER_HEATER_MODE_ECO;

                needs_update |= (new_mode != this->mode_);
                this->mode_ = new_mode;
            }

            uint32_t new_state = spa_->get_heating() ? water_heater::WATER_HEATER_STATE_ON : 0;
            needs_update |= (new_state != this->state_);
            this->state_ = new_state;

            if (needs_update)
                this->publish_state();
        }

        void BalboaSpaWaterHeater::control(const water_heater::WaterHeaterCall &call)
        {
            float target_temp = call.get_target_temperature();
            if (!std::isnan(target_temp))
                spa_->set_temp(to_internal(target_temp));

            if (call.get_mode().has_value())
            {
                auto requested_mode = *call.get_mode();
                if (requested_mode == water_heater::WATER_HEATER_MODE_OFF)
                {
                    spa_->set_rest_mode(true);
                }
                else
                {
                    bool want_high = (requested_mode == water_heater::WATER_HEATER_MODE_ELECTRIC);
                    spa_->set_highrange(want_high);
                    spa_->set_rest_mode(false);
                }
            }
        }

    } // namespace balboa_spa
} // namespace esphome
