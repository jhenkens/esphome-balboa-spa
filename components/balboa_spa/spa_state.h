#pragma once

#include <stdint.h>
#include <cmath>
#include "spa_types.h"
#include "spa_message.h"

namespace esphome
{
    namespace balboa_spa
    {
        class SpaState
        {
        public:
            SpaState()
            {
                rest_mode = HeatingMode::NOT_YET_RECEIVED;
                heat_state = 254;
                target_temp = NAN;
                current_temp = NAN;
                reminder = ReminderType::NONE;
                hour = 0;
                minutes = 0;
                jets[0] = jets[1] = jets[2] = jets[3] = 0;
                blower = 0;
                lights[0] = lights[1] = 0;
                highrange = 0;
                circulation = 0;
                cleanup_cycle = 0;
            }

            // Convert raw API byte to a float in the spa's native unit.
            // Celsius values are doubled in the API, so divide by 2.
            static float convert_raw_temp(uint8_t raw, TEMP_SCALE spa_scale)
            {
                if (spa_scale == TEMP_SCALE::C) return raw / 2.0f;
                if (spa_scale == TEMP_SCALE::F) return (float)raw;
                return NAN;
            }

            SpaState(const StatusMessage &msg, TEMP_SCALE spa_scale)
            {
                hour    = msg._hour;
                minutes = msg._minute;
                rest_mode  = static_cast<HeatingMode>(msg._heatingMode);
                heat_state = msg._heating;
                highrange  = msg._tempRange;
                jets[0] = msg._pump1;
                jets[1] = msg._pump2;
                jets[2] = msg._pump3;
                jets[3] = msg._pump4;
                circulation   = msg._circPump;
                blower        = msg._blower;
                lights[0]     = (msg._light1 == 3) ? 1 : 0;
                lights[1]     = msg._light2;
                cleanup_cycle = (msg._cleanupCycle == CleanupCycle::ACTIVE) ? 1 : 0;
                reminder = msg._reminder;

                current_temp = NAN;
                if (msg._currentTemp != 0xFF)
                {
                    float t = convert_raw_temp(msg._currentTemp, spa_scale);
                    if (!std::isnan(t) &&
                        (spa_scale == TEMP_SCALE::C
                            ? t <= HIGHRANGE_MAX_TEMP_C + TEMP_SANITY_OFFSET_C
                            : t <= HIGHRANGE_MAX_TEMP_F + TEMP_SANITY_OFFSET_F))
                        current_temp = t;
                }

                target_temp = NAN;
                if (msg._setTemp != 0xFF)
                {
                    float t = convert_raw_temp(msg._setTemp, spa_scale);
                    if (!std::isnan(t))
                    {
                        bool ok = (spa_scale == TEMP_SCALE::C)
                            ? (t >= LOWRANGE_MIN_TEMP_C && t <= HIGHRANGE_MAX_TEMP_C)
                            : (t >= LOWRANGE_MIN_TEMP_F && t <= HIGHRANGE_MAX_TEMP_F);
                        if (ok) target_temp = t;
                    }
                }
            }

            static constexpr uint8_t JET_COUNT = 4;
            uint8_t jets[JET_COUNT]; // jets[0..3] = jet1..4, values: 0=OFF, 1=LOW, 2=HIGH
            uint8_t blower : 1;
            static constexpr uint8_t LIGHT_COUNT = 2;
            uint8_t lights[LIGHT_COUNT]; // lights[0]=light1, lights[1]=light2, 0=OFF, 1=ON
            uint8_t highrange : 1;
            uint8_t circulation : 1;
            uint8_t cleanup_cycle : 1;
            uint8_t hour : 5;
            uint8_t minutes : 6;
            HeatingMode rest_mode;
            uint8_t heat_state;
            float target_temp;
            float current_temp;
            ReminderType reminder;
        };
    } // namespace balboa_spa
} // namespace esphome