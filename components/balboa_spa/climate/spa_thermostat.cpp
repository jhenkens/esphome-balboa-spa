#include "esphome.h"
#include "esphome/core/log.h"
#include "spa_thermostat.h"
#include "esphome/components/climate/climate_mode.h"

namespace esphome
{
    namespace balboa_spa
    {
        static const char *TAG = "balboa_spa.climate";

        void BalboaSpaThermostat::set_parent(BalboaSpa *parent)
        {
            spa_temp_init(parent);
        }

        climate::ClimateTraits BalboaSpaThermostat::traits()
        {
            update_traits();
            return traits_;
        }

        void BalboaSpaThermostat::update_traits()
        {
            traits_.set_supported_modes({climate::CLIMATE_MODE_OFF, climate::ClimateMode::CLIMATE_MODE_HEAT});
            traits_.add_feature_flags(climate::CLIMATE_SUPPORTS_ACTION | climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE);
            traits_.set_supported_presets({climate::ClimatePreset::CLIMATE_PRESET_HOME, climate::ClimatePreset::CLIMATE_PRESET_ECO});
            traits_.set_visual_min_temperature(range_min());
            traits_.set_visual_max_temperature(range_max());
            traits_.set_visual_temperature_step(temp_step());
            traits_.set_temperature_unit(is_fahrenheit() ? esphome::TemperatureUnit::FAHRENHEIT : esphome::TemperatureUnit::CELSIUS);
        }

        void BalboaSpaThermostat::control(const climate::ClimateCall &call)
        {
            if (call.get_target_temperature().has_value())
                spa_->set_temp(to_internal(*call.get_target_temperature()));

            if (call.get_preset().has_value())
                spa_->set_highrange(*call.get_preset() == climate::ClimatePreset::CLIMATE_PRESET_HOME);

            if (call.get_mode().has_value())
            {
                auto requested_mode = *call.get_mode();
                bool is_in_rest = spa_->get_restmode();

                if (requested_mode == climate::CLIMATE_MODE_HEAT && is_in_rest)
                {
                    ESP_LOGD(TAG, "Toggle from Rest to Heat (Ready)");
                    spa_->set_rest_mode(false);
                }
                else if (requested_mode == climate::CLIMATE_MODE_OFF && !is_in_rest)
                {
                    ESP_LOGD(TAG, "Toggle from Heat to Rest");
                    spa_->set_rest_mode(true);
                }
            }
        }

        void BalboaSpaThermostat::update()
        {
            const SpaState *s = spa_->get_current_state();
            bool needs_update = false;
            if (!spa_sync_temps(s, this->target_temperature, this->current_temperature, needs_update))
                return;

            auto new_action = spa_->get_heating() ? climate::CLIMATE_ACTION_HEATING : climate::CLIMATE_ACTION_IDLE;
            needs_update |= (new_action != this->action);
            this->action = new_action;

            auto new_mode = spa_->get_restmode() ? climate::CLIMATE_MODE_OFF : climate::CLIMATE_MODE_HEAT;
            needs_update |= (new_mode != this->mode);
            this->mode = new_mode;

            auto new_preset = spa_->get_highrange() ? climate::ClimatePreset::CLIMATE_PRESET_HOME : climate::ClimatePreset::CLIMATE_PRESET_ECO;
            needs_update |= (new_preset != this->preset);
            this->preset = new_preset;

            if (needs_update)
                this->publish_state();
        }

    } // namespace balboa_spa
} // namespace esphome
