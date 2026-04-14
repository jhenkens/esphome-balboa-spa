#pragma once

#include "stdint.h"

#define TEMP_C_TO_F(c) ((c) * 9.0f / 5.0f + 32.0f)
#define TEMP_F_TO_C(f) (((f) - 32.0f) * 5.0f / 9.0f)

namespace esphome
{
    namespace balboa_spa
    {
        // High Range: 80–104 °F / 26.5–40 °C
        static constexpr float HIGHRANGE_MIN_TEMP_F = 80.0f;
        static constexpr float HIGHRANGE_MAX_TEMP_F = 104.0f;
        static constexpr float HIGHRANGE_MIN_TEMP_C = 26.5f;
        static constexpr float HIGHRANGE_MAX_TEMP_C = 40.0f;
        // Low Range: 60–99 °F / 10–37 °C
        static constexpr float LOWRANGE_MIN_TEMP_F = 60.0f;
        static constexpr float LOWRANGE_MAX_TEMP_F = 99.0f;
        static constexpr float LOWRANGE_MIN_TEMP_C = 10.0f;
        static constexpr float LOWRANGE_MAX_TEMP_C = 37.0f;

        // Offset above HIGHRANGE_MAX used as a sanity cap when decoding raw wire temperatures.
        // Values above (HIGHRANGE_MAX + offset) are treated as garbage and discarded.
        static constexpr float TEMP_SANITY_OFFSET_C = 40.0f;  // cap = 80 °C
        static constexpr float TEMP_SANITY_OFFSET_F = 72.0f;  // cap = 176 °F
        struct SpaFaultLog
        {
            uint8_t total_entries : 5;
            uint8_t current_entry : 5;
            uint8_t fault_code : 6;
            const char *fault_message = "";
            uint8_t days_ago : 8;
            uint8_t hour : 5;
            uint8_t minutes : 6;
        };

        struct SpaFilterSettings
        {
            uint8_t filter1_hour : 5;
            uint8_t filter1_minute : 6;
            uint8_t filter1_duration_hour : 5;
            uint8_t filter1_duration_minute : 6;
            uint8_t filter2_enable : 1;
            uint8_t filter2_hour : 5;
            uint8_t filter2_minute : 6;
            uint8_t filter2_duration_hour : 5;
            uint8_t filter2_duration_minute : 6;
        };

        enum class TEMP_SCALE : uint8_t
        {
            UNKNOWN = 254,
            F = 0,
            C = 1
        };

        enum class CLOCK_MODE : uint8_t
        {
            UNKNOWN = 254,
            CLOCK_12HR = 0,
            CLOCK_24HR = 1
        };

        // Reminder/status codes broadcast in StatusMessage payload byte 01.
        // NOTE: This list is incomplete. If you encounter an unknown code, please open a GitHub
        // issue with the code value and the message shown on your spa panel.
        enum class ReminderType : uint8_t
        {
            NONE             = 0x00,
            PRIMING          = 0x01,
            CLEAN_FILTER     = 0x04,
            CHECK_SANITIZER  = 0x09,
            CHECK_PH         = 0x0A,
            FAULT            = 0x1E,
            UNKNOWN          = 0xFF, // sentinel / not-yet-received
        };

        // Reminder/status codes broadcast in StatusMessage payload byte 01.
        // NOTE: This list is incomplete. If you encounter an unknown code, please open a GitHub
        // issue with the code value and the message shown on your spa panel.
        enum class CleanupCycle : uint8_t
        {
            UNKNOWN          = 0x00,
            ACTIVE           = 0x0C,  
            OFF              = 0x04,
        };

        // Heating mode from StatusMessage byte 05, bits[1:0]
        enum class HeatingMode : uint8_t
        {
            READY         = 0,
            REST          = 1,
            READY_IN_REST = 3,
            NOT_YET_RECEIVED = 254,
        };

        enum class ToggleStateMaybe : uint8_t
        {
            OFF = 0,
            ON = 1,
            HIGH_SPEED = 2,
            DONT_KNOW = 255
        };

        static const char *TOGGLE_STATE_MAYBE_STRINGS[] = {
            "OFF",
            "ON",
            "HIGH",
            "DONT_KNOW"
        };
    } // namespace balboa_spa
} // namespace esphome