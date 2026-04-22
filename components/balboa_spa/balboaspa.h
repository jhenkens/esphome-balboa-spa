#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/log.h"

#define BUFFER_LENGTH 100

// Not defined in recent framework libs so stealing from
// https://github.com/espressif/arduino-esp32/blob/496b8411773243e1ad88a68652d6982ba2366d6b/cores/esp32/Arduino.h#L99
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)

#include "spa_types.h"
#include "spa_state.h"
#include "spa_message.h"
#include <string>
#include <functional>

namespace esphome
{
    namespace balboa_spa
    {
        static const float ESPHOME_BALBOASPA_POLLING_INTERVAL = 50; // frequency to poll uart device

        static constexpr const char *STRON = "ON";
        static constexpr const char *STROFF = "OFF";

        // Identifies which spa state field a queued command is expected to change.
        // Used as the dedup key and for pruning satisfied commands after state updates.
        enum class ExpectedField : uint8_t
        {
            NONE        = 0,  // fire-and-forget, never retried
            JET_0       = 1,
            JET_1       = 2,
            JET_2       = 3,
            JET_3       = 4,
            BLOWER      = 5,
            LIGHT_0     = 6,
            LIGHT_1     = 7,
            HIGH_RANGE   = 8,
            REST_MODE   = 9,
            TARGET_TEMP = 10,
            CLOCK_24H   = 11,
            FILTER      = 12,
        };

        class BalboaSpa : public uart::UARTDevice, public PollingComponent
        {
        public:
            BalboaSpa() : PollingComponent(ESPHOME_BALBOASPA_POLLING_INTERVAL) {}
            void setup() override;
            void update() override;
            float get_setup_priority() const override;

            ControlConfig2Response get_current_config();
            SpaState *get_current_state();
            SpaFilterSettings *get_current_filter_settings();
            SpaFaultLog *get_current_fault_log();

            void set_temp(float temp);
            void set_time(int hour, int minute);
            void set_timescale(bool is_24h);
            void set_filter1_config(uint8_t start_hour, uint8_t start_minute, uint8_t duration_hour, uint8_t duration_minute);
            void set_filter2_config(uint8_t start_hour, uint8_t start_minute, uint8_t duration_hour, uint8_t duration_minute);
            void set_filter1_start_time(uint8_t hour, uint8_t minute);
            void set_filter1_duration(uint8_t hour, uint8_t minute);
            void set_filter2_start_time(uint8_t hour, uint8_t minute);
            void set_filter2_duration(uint8_t hour, uint8_t minute);
            void disable_filter2();
            void toggle_light(uint8_t index); // index 1-2
            void toggle_jet(uint8_t index, uint8_t expected_state, uint8_t max_retries = 5);  // index 1-4
            void toggle_blower(uint8_t expected_state, uint8_t max_retries = 5);
            void set_highrange(bool high);
            void clear_reminder();

            void set_live_range_refresh(bool v) { live_range_refresh_ = v; }
            bool get_live_range_refresh() const { return live_range_refresh_; }

            TEMP_SCALE get_spa_temp_scale() { return spa_temp_scale; }
            uint32_t get_time_since_last_status_ms() { return millis() - last_status_received_ms_; }
            void set_client_id(uint8_t id);
            void set_listener_keepalive(uint32_t ms) { listener_keepalive_ms_ = ms; }
            void set_startup_delay(uint32_t ms) { startup_delay_ms_ = ms; }

            bool is_communicating();

            void register_listener(const std::function<void()> &func) { this->listeners_.push_back(func); }
            void register_filter_listener(const std::function<void(SpaFilterSettings *)> &func) { this->filter_listeners_.push_back(func); }
            void register_fault_log_listener(const std::function<void(SpaFaultLog *)> &func) { this->fault_log_listeners_.push_back(func); }
            void register_highrange_listener(const std::function<void()> &func) { this->highrange_listeners_.push_back(func); }

            bool get_heating();
            bool get_restmode();
            bool get_highrange();
            void toggle_heat();
            void set_rest_mode(bool rest);
            void request_config_update();
            void request_filter_settings_update();
            void request_fault_log_update();

        private:
            uint8_t input_buffer[BUFFER_LENGTH];
            bool input_started = false;
            bool input_failed = true;
            bool peek_equals(uint8_t val)
            {
                if (!available())
                    return false;
                uint8_t byte;
                peek_byte(&byte);
                return byte == val;
            }
            uint8_t output_buffer[BUFFER_LENGTH];
            uint8_t received_byte, loop_index, temp_index;
            uint8_t last_state_crc = 0x00;
            uint8_t last_settings_crc = 0x00;
            uint8_t last_filter_crc = 0x00;
            uint8_t last_fault_crc = 0x00;
            // Command queue — sorted linear array, insertion-sorted by available_at
            enum class CmdType : uint8_t
            {
                TOGGLE,
                SET_TEMP,
                SET_TIME,
                SET_PREF,
                SET_FILTER
            };
            struct PendingCmd
            {
                CmdType type;
                ExpectedField target_field = ExpectedField::NONE; // dedup key + expected-state selector
                uint8_t expected_toggle_value = 0;   // value expected in spa state after command succeeds
                uint8_t retry_count = 0;      // number of times sent so far
                uint8_t max_retries = 0;      // 0 = fire-and-forget; give up when retry_count >= max_retries
                uint32_t queued_at = 0;       // when first created
                uint32_t available_at = 0;    // earliest millis() at which this may be sent
                union
                {
                    uint8_t toggle_code;
                    float target_temperature;
                    struct
                    {
                        uint8_t hour;
                        uint8_t minute;
                    } time;
                    struct
                    {
                        uint8_t code;
                        uint8_t data;
                    } pref;
                    struct
                    {
                        uint8_t f1_start_hour, f1_start_minute;
                        uint8_t f1_dur_hour, f1_dur_minute;
                        uint8_t f2_enable;
                        uint8_t f2_start_hour, f2_start_minute;
                        uint8_t f2_dur_hour, f2_dur_minute;
                    } filter;
                };
            };
            static const uint8_t CMD_QUEUE_SIZE = 8;
            static constexpr uint32_t RETRY_BACKOFF_MS = 5000;
            static constexpr uint8_t PENDING_MSG_BUF_SIZE = 24;
            PendingCmd cmd_queue_[CMD_QUEUE_SIZE];
            uint8_t cmd_count_ = 0;
            uint8_t pending_msg_buf_[PENDING_MSG_BUF_SIZE];
            uint8_t pending_msg_len_ = 0;
            PendingCmd pending_cmd_ = {};      // command used to build pending_msg_buf_
            void insert_cmd(PendingCmd cmd);
            void remove_at(uint8_t i);
            void rebuild_pending_msg();
            void enqueue_toggle(uint8_t code, ExpectedField field, uint8_t expected_toggle_value, uint8_t max_retries);
            void enqueue_current_filter();
            static ExpectedField toggle_code_to_field(uint8_t code);
            bool is_expected_satisfied(const PendingCmd &cmd) const;
            bool filter_cmd_satisfied(const PendingCmd &cmd) const;
            void prune_satisfied();

            // Staging fields for commands that are built incrementally
            uint8_t target_hour = 0x00;
            uint8_t target_minute = 0x00;
            uint8_t target_filter1_start_hour = 0x00;
            uint8_t target_filter1_start_minute = 0x00;
            uint8_t target_filter1_duration_hour = 0x00;
            uint8_t target_filter1_duration_minute = 0x00;
            uint8_t target_filter2_start_hour = 0x00;
            uint8_t target_filter2_start_minute = 0x00;
            uint8_t target_filter2_duration_hour = 0x00;
            uint8_t target_filter2_duration_minute = 0x00;
            bool target_filter2_enable = false;
            uint8_t client_id = 0x00;
            uint8_t client_id_override = 0x00;
            bool use_client_id_override = false;
            uint32_t last_received_time = 0;
            uint32_t last_status_received_ms_ = 0;
            uint32_t last_dead_log_time = 0;
            uint32_t last_cts_time = 0;
            uint32_t last_listener_dispatch_time = 0;
            uint32_t listener_keepalive_ms_ = 300000;
            uint32_t startup_delay_ms_ = 10000;
            uint32_t setup_time_ = 0;
            bool live_range_refresh_ = false;
            bool highrange_dirty_ = false;
            bool state_dirty_ = false;

            TEMP_SCALE spa_temp_scale = TEMP_SCALE::UNKNOWN;
            CLOCK_MODE clock_mode_24hr = CLOCK_MODE::UNKNOWN;

            std::vector<std::function<void()>> listeners_;
            std::vector<std::function<void(SpaFilterSettings *)>> filter_listeners_;
            std::vector<std::function<void(SpaFaultLog *)>> fault_log_listeners_;
            std::vector<std::function<void()>> highrange_listeners_;

            char config_request_status = 0;           // stages: 0-> want it; 1-> requested it; 2-> got it; 3-> further processed it
            char faultlog_request_status = 0;         // stages: 0-> want it; 1-> requested it; 2-> got it; 3-> further processed it
            char filtersettings_request_status = 0;   // stages: 0-> want it; 1-> requested it; 2-> got it; 3-> further processed it
            char faultlog_update_timer = 0;           // temp logic so we only get the fault log once per 5 minutes
            uint16_t filtersettings_update_timer = 0; // timer for periodic filter settings requests (every 5 minutes)

            ControlConfig2Response spaConfig;
            SpaState spaState;
            SpaFaultLog spaFaultLog;
            SpaFilterSettings spaFilterSettings;

            bool read_serial();
            void process_message();
            void send_message();
            void update_sensors();

            void establish_id();
            void ID_request();
            void ID_ack();
            template <typename T>
            void send_typed(T &msg);
            void rs485_send(const MessageBaseOutgoing &msg);
            void decodeSettings(const ControlConfig2Response *msg);
            void decodeState(const StatusMessage *msg);
            void decodeFilterSettings(const FilterStatusMessage *msg);
            void decodeFault();
        };

    } // namespace balboa_spa
} // namespace esphome
