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
            void toggle_jet(uint8_t index, std::function<void()> on_sent = nullptr);   // index 1-4
            void toggle_blower(std::function<void()> on_sent = nullptr);
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
            void register_retry_listener(const std::function<void()> &func) { this->retry_listeners_.push_back(func); }
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
            void request_retry_dispatch(){
                this->should_run_update_loop_ = true;
            }

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
            // Command queue — replaces single send_command byte
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
                uint32_t queued_at = 0;
                std::function<void()> on_sent;
                union
                {
                    uint8_t toggle_code;
                    uint8_t temp_raw;
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
            PendingCmd cmd_queue_[CMD_QUEUE_SIZE];
            uint8_t cmd_head_ = 0;
            uint8_t cmd_tail_ = 0;
            uint8_t cmd_count_ = 0;
            bool enqueue_cmd(const PendingCmd &cmd);
            bool dequeue_cmd(PendingCmd &out);
            void enqueue_toggle(uint8_t code);
            void enqueue_current_filter();

            // Staging fields for commands that are built incrementally
            uint8_t target_temperature = 0x00;
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
            uint32_t last_retry_dispatch_time_ = 0;
            static constexpr uint32_t RETRY_LISTENER_INTERVAL_MS = 1000;
            uint32_t startup_delay_ms_ = 10000;
            uint32_t setup_time_ = 0;
            bool live_range_refresh_ = false;
            bool highrange_dirty_ = false;
            bool state_dirty_ = false;
            bool should_run_update_loop_ = false;

            TEMP_SCALE spa_temp_scale = TEMP_SCALE::UNKNOWN;
            CLOCK_MODE clock_mode_24hr = CLOCK_MODE::UNKNOWN;

            std::vector<std::function<void()>> listeners_;
            std::vector<std::function<void()>> retry_listeners_;
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
