#pragma once

#include <cmath>
#include "balboaspa.h"

namespace esphome
{
    namespace balboa_spa
    {
        // Mixin base class for spa temperature components.
        // Provides unit-of-measurement aware temperature conversion and
        // high/low range min/max tracking.
        // SpaState always stores temperatures in Celsius internally.
        class SpaTemperatureBase
        {
        public:
            virtual void update() = 0;
            virtual void update_traits() = 0;

        protected:
            // Call in set_parent(), passing the component's unit_of_measurement.
            // Stores the spa pointer, sets the display unit, and registers the
            // highrange and state listeners via virtual dispatch.
            // fahrenheit = true  → display °F
            // fahrenheit = false → display °C
            void spa_temp_init(BalboaSpa *spa, bool fahrenheit, bool includeHighrangeListener = true)
            {
                spa_ = spa;
                temp_scale_ = fahrenheit ? TEMP_SCALE::F : TEMP_SCALE::C;
                if (spa_->get_live_range_refresh() && includeHighrangeListener)
                    spa_->register_highrange_listener([this]() { update_traits(); });
                spa_->register_listener([this]() { update(); });
            }

            bool is_fahrenheit() const { return temp_scale_ == TEMP_SCALE::F; }

            // Min/max for the display unit.
            // When live_range_refresh is disabled (default), always returns the full spectrum
            // (low-range min to high-range max) so the entity's range never changes.
            // When live_range_refresh is enabled, tracks the spa's current high/low range.
            float range_min() const
            {
                if (!this->spa_->get_live_range_refresh())
                    return is_fahrenheit() ? LOWRANGE_MIN_TEMP_F : LOWRANGE_MIN_TEMP_C;
                bool highrange = this->spa_->get_highrange();
                if (is_fahrenheit())
                    return highrange ? HIGHRANGE_MIN_TEMP_F : LOWRANGE_MIN_TEMP_F;
                return highrange ? HIGHRANGE_MIN_TEMP_C : LOWRANGE_MIN_TEMP_C;
            }

            float range_max() const
            {
                if (!this->spa_->get_live_range_refresh())
                    return is_fahrenheit() ? HIGHRANGE_MAX_TEMP_F : HIGHRANGE_MAX_TEMP_C;
                bool highrange = this->spa_->get_highrange();
                if (is_fahrenheit())
                    return highrange ? HIGHRANGE_MAX_TEMP_F : LOWRANGE_MAX_TEMP_F;
                return highrange ? HIGHRANGE_MAX_TEMP_C : LOWRANGE_MAX_TEMP_C;
            }

            float temp_step() const { return is_fahrenheit() ? 1.0f : 0.5f; }

            // Convert a value in the spa's native unit (from SpaState) to the display unit + round.
            // Compares spa_->get_spa_temp_scale() against temp_scale_ and converts only if they differ.
            float to_display(float val_spa) const
            {
                TEMP_SCALE spa_scale = this->spa_->get_spa_temp_scale();
                float converted;
                if (spa_scale == temp_scale_)
                    converted = val_spa;
                else if (spa_scale == TEMP_SCALE::C){
                    converted = TEMP_C_TO_F(val_spa);
                }
                else{
                    converted = TEMP_F_TO_C(val_spa);
                }
                return converted;
            }

            // Convert a value in the display unit back to the spa's native unit for set_temp().
            float to_internal(float display_val) const
            {
                TEMP_SCALE spa_scale = spa_->get_spa_temp_scale();
                if (temp_scale_ == spa_scale)
                    return display_val;
                if (temp_scale_ == TEMP_SCALE::F)
                    return TEMP_F_TO_C(display_val);
                return TEMP_C_TO_F(display_val);
            }

            // Sync current and target temps from spaState into the component's member fields.
            // If the spa is not communicating, sets both to NAN and returns false (caller should
            // return early without publishing). Otherwise ORs any change into needs_update and
            // returns true.
            bool spa_sync_temps(const SpaState *s, float &target, float &current, bool &needs_update)
            {
                if (!spa_->is_communicating())
                {
                    target = NAN;
                    current = NAN;
                    return false;
                }
                needs_update |= sync_temp(s->target_temp, target);
                needs_update |= sync_temp(s->current_temp, current);
                return true;
            }

            // Sync a display-unit value from a SpaState value in the spa's native unit.
            // Updates stored and returns true if the value changed.
            bool sync_temp(float val_spa, float &stored) const
            {
                if (std::isnan(val_spa))
                    return false;
                float display = to_display(val_spa);
                if (display == stored)
                    return false;
                stored = display;
                return true;
            }

            BalboaSpa *spa_ = nullptr;
            TEMP_SCALE temp_scale_ = TEMP_SCALE::C;
        };

    } // namespace balboa_spa
} // namespace esphome
