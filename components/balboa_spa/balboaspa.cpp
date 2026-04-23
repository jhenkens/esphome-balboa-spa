#include "balboaspa.h"
#include "spa_message.h"
#include "esphome/core/helpers.h"
#include <cmath>

#define MAX_CLIENT_ID 0x2F

// Print a 24-bit packed message type in wire byte order (b1 b2 b3)
#define MSGTYPE_FMT "%02X %02X %02X"
#define MSGTYPE_ARGS(t) (unsigned)((t) & 0xFF), (unsigned)(((t) >> 8) & 0xFF), (unsigned)(((t) >> 16) & 0xFF)


namespace esphome
{
    namespace balboa_spa
    {

        static const char *TAG = "BalboaSpa.component";
        static const char *CRC_TAG = "BalboaSpa.CRC";

        template <typename T>
        static bool check_msg_length(const uint8_t *buf, const char *name)
        {
            if (buf[0] < sizeof(T))
            {
                ESP_LOGW(TAG, "Message %s too short: got %d bytes, need %d", name, buf[0], (int)sizeof(T));
                return false;
            }
            return true;
        }

        // Protocol byte indices for status update (0x13) message
        void BalboaSpa::setup()
        {
            input_started = false;
            filtersettings_update_timer = 0;
            setup_time_ = millis();

            if (remember_client_id_)
            {
                client_id_pref_ = global_preferences->make_preference<uint8_t>(fnv1_hash("balboa_spa_client_id"));
                uint8_t saved_id = 0;
                if (client_id_pref_.load(&saved_id) && saved_id >= 1 && saved_id <= MAX_CLIENT_ID)
                {
                    remembered_client_id_ = saved_id;
                    use_remembered_client_id_ = true;
                    ESP_LOGD(TAG, "Restored client ID 0x%02X from preferences", saved_id);
                }
            }
        }

        void BalboaSpa::update()
        {
            uint32_t now = millis();
            if(now - setup_time_ < startup_delay_ms_) {
                last_received_time = now;
                return;
            }
            if (last_received_time + 10000 < now)
            {
                if (now - last_dead_log_time >= 60000)
                {
                    ESP_LOGW(TAG, "No new message since %d Seconds! Mark as dead!", (now - last_received_time) / 1000);
                    last_dead_log_time = now;
                }
                status_set_error(LOG_STR("No Communication with Balboa Mainboard!"));
                if (client_id != 0)
                {
                    client_id = 0;
                    for (const auto &listener : this->client_id_listeners_)
                        listener();
                }
            }
            else if (status_has_error())
            {
                status_clear_error();
            } else{
                last_dead_log_time = 0;
            }

            // Filter settings periodic update timer (every 5 minutes)
            if (filtersettings_request_status == 2)
            {
                filtersettings_update_timer++;
                if (filtersettings_update_timer >= 6000)
                {                                      // 6000 * 50ms = 5 minutes
                    filtersettings_request_status = 0; // Reset to request again
                    filtersettings_update_timer = 0;
                    ESP_LOGD(TAG, "Spa/debug/filtersettings_request_status: %s", "resetting for periodic update");
                }
            }

            ESP_LOGV(TAG, "Checking serial for incoming data...");
            while (true)
            {
                if(!read_serial()){
                    break;
                }
            }
            ESP_LOGV(TAG, "Finished processing serial data. Updating sensors if needed...");

            uint32_t now_dispatch = millis();
            bool keepalive = (now_dispatch - last_listener_dispatch_time) >= listener_keepalive_ms_;

            if(highrange_dirty_ || keepalive)
            {
                highrange_dirty_ = false;
                for (const auto &listener : this->highrange_listeners_)
                {
                    listener();
                }
            }

            if (state_dirty_ || keepalive)
            {
                for (const auto &listener : this->listeners_)
                {
                    listener();
                }
                state_dirty_ = false;
                last_listener_dispatch_time = now_dispatch;
            }
        }

        float BalboaSpa::get_setup_priority() const { return esphome::setup_priority::LATE; }

        ControlConfig2Response BalboaSpa::get_current_config() { return spaConfig; }
        SpaState *BalboaSpa::get_current_state() { return &spaState; }
        SpaFilterSettings *BalboaSpa::get_current_filter_settings() { return &spaFilterSettings; }
        SpaFaultLog *BalboaSpa::get_current_fault_log() { return &spaFaultLog; }

        ExpectedField BalboaSpa::toggle_code_to_field(uint8_t code)
        {
            switch (code)
            {
            case 0x04: return ExpectedField::JET_0;
            case 0x05: return ExpectedField::JET_1;
            case 0x06: return ExpectedField::JET_2;
            case 0x07: return ExpectedField::JET_3;
            case tiBlower:      return ExpectedField::BLOWER;
            case 0x11:          return ExpectedField::LIGHT_0;
            case 0x12:          return ExpectedField::LIGHT_1;
            case tiTempRange:   return ExpectedField::HIGH_RANGE;
            case tiHeatingMode: return ExpectedField::REST_MODE;
            default:            return ExpectedField::NONE;
            }
        }

        bool BalboaSpa::is_satisfied(const PendingCmd &cmd) const
        {
            if (cmd.type == CmdType::SET_FILTER)
                return filter_cmd_satisfied(cmd);
            if (cmd.type == CmdType::SET_TEMP)
                return (spa_temp_scale != TEMP_SCALE::C && spa_temp_scale != TEMP_SCALE::F)
                    || (!std::isnan(spaState.target_temp) && spaState.target_temp == cmd.target_temperature);
            if (cmd.type == CmdType::SET_TIME)
                return spaState.hour == cmd.time.hour && spaState.minutes == cmd.time.minute;
            switch (cmd.target_field)
            {
            case ExpectedField::JET_0:    return spaState.jets[0] == cmd.expected_toggle_value;
            case ExpectedField::JET_1:    return spaState.jets[1] == cmd.expected_toggle_value;
            case ExpectedField::JET_2:    return spaState.jets[2] == cmd.expected_toggle_value;
            case ExpectedField::JET_3:    return spaState.jets[3] == cmd.expected_toggle_value;
            case ExpectedField::BLOWER:   return spaState.blower == cmd.expected_toggle_value;
            case ExpectedField::LIGHT_0:  return spaState.lights[0] == cmd.expected_toggle_value;
            case ExpectedField::LIGHT_1:  return spaState.lights[1] == cmd.expected_toggle_value;
            case ExpectedField::HIGH_RANGE: return (spaState.highrange & 0x1) == cmd.expected_toggle_value;
            case ExpectedField::REST_MODE: return (uint8_t)spaState.rest_mode == cmd.expected_toggle_value;
            case ExpectedField::TARGET_TEMP:
                return !std::isnan(spaState.target_temp) && spaState.target_temp == cmd.target_temperature;
            case ExpectedField::CLOCK_24H:
                return (uint8_t)clock_mode_24hr == cmd.pref.data;
            case ExpectedField::NONE:
            case ExpectedField::FILTER:
            default:
                return false;
            }
        }

        void BalboaSpa::remove_at(uint8_t i)
        {
            for (uint8_t j = i; j + 1 < cmd_count_; j++)
                cmd_queue_[j] = cmd_queue_[j + 1];
            cmd_count_--;
        }

        bool BalboaSpa::filter_cmd_satisfied(const PendingCmd &c) const
        {
            return c.target_field == ExpectedField::FILTER &&
                spaFilterSettings.filter1_hour            == c.filter.f1_start_hour   &&
                spaFilterSettings.filter1_minute          == c.filter.f1_start_minute &&
                spaFilterSettings.filter1_duration_hour   == c.filter.f1_dur_hour     &&
                spaFilterSettings.filter1_duration_minute == c.filter.f1_dur_minute   &&
                spaFilterSettings.filter2_enable          == c.filter.f2_enable       &&
                spaFilterSettings.filter2_hour            == c.filter.f2_start_hour   &&
                spaFilterSettings.filter2_minute          == c.filter.f2_start_minute &&
                spaFilterSettings.filter2_duration_hour   == c.filter.f2_dur_hour     &&
                spaFilterSettings.filter2_duration_minute == c.filter.f2_dur_minute;
        }

        void BalboaSpa::rebuild_pending_msg()
        {
            if (cmd_count_ == 0 || client_id == 0) {
                pending_msg_len_ = 0;
                return;
            }

            const PendingCmd &head = cmd_queue_[0];

            // Skip rebuild if the head command hasn't changed
            if (pending_msg_len_ > 0 &&
                memcmp(&head, &pending_cmd_, sizeof(PendingCmd)) == 0)
                return;

            switch (head.type)
            {
            case CmdType::TOGGLE: {
                ToggleItemMessage msg(static_cast<ToggleItem>(head.toggle_code));
                msg.set_client(client_id);
                msg.SetCRC();
                memcpy(pending_msg_buf_, &msg, msg._length);
                pending_msg_len_ = msg._length;
                break;
            }
            case CmdType::SET_TEMP: {
                uint8_t raw = (spa_temp_scale == TEMP_SCALE::C)
                    ? (uint8_t)roundf(head.target_temperature * 2.0f)
                    : (uint8_t)roundf(head.target_temperature);
                SetSpaTempMessage msg({raw});
                msg.set_client(client_id);
                msg.SetCRC();
                memcpy(pending_msg_buf_, &msg, msg._length);
                pending_msg_len_ = msg._length;
                break;
            }
            case CmdType::SET_TIME: {
                SetSpaTime msg({head.time.hour, head.time.minute, false});
                msg.set_client(client_id);
                msg.SetCRC();
                memcpy(pending_msg_buf_, &msg, msg._length);
                pending_msg_len_ = msg._length;
                break;
            }
            case CmdType::SET_PREF: {
                SetPreferenceMessage msg(head.pref.code, head.pref.data);
                msg.set_client(client_id);
                msg.SetCRC();
                memcpy(pending_msg_buf_, &msg, msg._length);
                pending_msg_len_ = msg._length;
                break;
            }
            case CmdType::SET_FILTER: {
                SetFilterConfigMessage msg(
                    head.filter.f1_start_hour, head.filter.f1_start_minute,
                    head.filter.f1_dur_hour,   head.filter.f1_dur_minute,
                    head.filter.f2_enable != 0,
                    head.filter.f2_start_hour, head.filter.f2_start_minute,
                    head.filter.f2_dur_hour,   head.filter.f2_dur_minute);
                msg.set_client(client_id);
                msg.SetCRC();
                memcpy(pending_msg_buf_, &msg, msg._length);
                pending_msg_len_ = msg._length;
                break;
            }
            }

            pending_cmd_ = head;
        }

        void BalboaSpa::prune_and_rebuild()
        {
            uint8_t i = 0;
            while (i < cmd_count_)
            {
                const PendingCmd &c = cmd_queue_[i];
                if (is_satisfied(c))
                {
                    ESP_LOGD(TAG, "Command field=%d satisfied, removing", (int)c.target_field);
                    remove_at(i);
                }
                else
                {
                    i++;
                }
            }
            rebuild_pending_msg();
        }

        void BalboaSpa::insert_cmd(PendingCmd cmd)
        {
            if (is_satisfied(cmd)) {
                ESP_LOGD(TAG, "Command field=%d already satisfied, not adding to queue", (int)cmd.target_field);
                return;
            }
            uint32_t now = millis();
            if (cmd.queued_at == 0)
                cmd.queued_at = now;
            if (cmd.available_at == 0)
                cmd.available_at = now;

            // Remove any existing entry with the same target_field (dedup)
            if (cmd.target_field != ExpectedField::NONE)
            {
                for (uint8_t i = 0; i < cmd_count_; i++)
                {
                    if (cmd_queue_[i].target_field == cmd.target_field)
                    {
                        remove_at(i);
                        break;
                    }
                }
            }

            if (cmd_count_ >= CMD_QUEUE_SIZE)
            {
                ESP_LOGW(TAG, "Command queue full, dropping command field=%d type=%d", (int)cmd.target_field, (int)cmd.type);
                return;
            }

            // Insertion-sort by available_at (ascending) so earliest-available is at head
            uint8_t pos = cmd_count_;
            while (pos > 0 && cmd_queue_[pos - 1].available_at > cmd.available_at)
            {
                cmd_queue_[pos] = cmd_queue_[pos - 1];
                pos--;
            }
            cmd_queue_[pos] = cmd;
            cmd_count_++;
            prune_and_rebuild();
        }


        void BalboaSpa::enqueue_toggle(uint8_t code, ExpectedField field, uint8_t expected_toggle_value, uint8_t max_retries)
        {
            PendingCmd cmd;
            cmd.type = CmdType::TOGGLE;
            cmd.toggle_code = code;
            cmd.target_field = field;
            cmd.expected_toggle_value = expected_toggle_value;
            cmd.max_retries = max_retries;
            insert_cmd(cmd);
        }

        void BalboaSpa::enqueue_current_filter()
        {
            PendingCmd cmd;
            cmd.type = CmdType::SET_FILTER;
            cmd.target_field = ExpectedField::FILTER;
            cmd.max_retries = 5;
            cmd.filter.f1_start_hour   = target_filter1_start_hour;
            cmd.filter.f1_start_minute = target_filter1_start_minute;
            cmd.filter.f1_dur_hour     = target_filter1_duration_hour;
            cmd.filter.f1_dur_minute   = target_filter1_duration_minute;
            cmd.filter.f2_enable       = target_filter2_enable ? 1 : 0;
            cmd.filter.f2_start_hour   = target_filter2_start_hour;
            cmd.filter.f2_start_minute = target_filter2_start_minute;
            cmd.filter.f2_dur_hour     = target_filter2_duration_hour;
            cmd.filter.f2_dur_minute   = target_filter2_duration_minute;
            insert_cmd(cmd);
        }

        void BalboaSpa::set_temp(float temp)
        {
            if (spa_temp_scale != TEMP_SCALE::C && spa_temp_scale != TEMP_SCALE::F)
            {
                ESP_LOGW(TAG, "set_temp(%f): spa_temp_scale not set. Ignoring", temp);
                return;
            }

            // temp is in the spa's native unit — validate range
            bool valid = (spa_temp_scale == TEMP_SCALE::C)
                ? (temp >= LOWRANGE_MIN_TEMP_C && temp <= HIGHRANGE_MAX_TEMP_C)
                : (temp >= LOWRANGE_MIN_TEMP_F && temp <= HIGHRANGE_MAX_TEMP_F);

            if (!valid)
            {
                ESP_LOGW(TAG, "set_temp(%f): out of range for spa scale %d", temp, spa_temp_scale);
                return;
            }

            PendingCmd temp_cmd;
            temp_cmd.type = CmdType::SET_TEMP;
            temp_cmd.target_temperature = temp;  // raw conversion happens at send time
            temp_cmd.target_field = ExpectedField::TARGET_TEMP;
            temp_cmd.max_retries = 5;
            insert_cmd(temp_cmd);
        }

        void BalboaSpa::set_highrange(bool high)
        {
            ESP_LOGD(TAG, "highrange=%d to %d requested", get_highrange(), high);
            if (high != get_highrange())
            {
                enqueue_toggle(tiTempRange, ExpectedField::HIGH_RANGE, high ? 1 : 0, 3);
            }
        }

        bool BalboaSpa::get_heating()
        {
            return spaState.heat_state == 1;
        }

        bool BalboaSpa::get_restmode()
        {
            return spaState.rest_mode == HeatingMode::REST;
        }

        bool BalboaSpa::get_highrange()
        {
            return spaState.highrange & 0x1 == 1;
        }

        void BalboaSpa::toggle_heat()
        {
        }

        void BalboaSpa::set_rest_mode(bool rest)
        {
            ESP_LOGD(TAG, "rest_mode=%d to %d requested", (int)spaState.rest_mode, rest ? 1 : 0);
            if (get_restmode() != rest)
            {
                ESP_LOGD(TAG, "Send 0x51 to toggle heat/rest");
                enqueue_toggle(tiHeatingMode, ExpectedField::REST_MODE,
                               rest ? (uint8_t)HeatingMode::REST : (uint8_t)HeatingMode::READY, 3);
            }
        }

        void BalboaSpa::request_config_update()
        {
            ESP_LOGD(TAG, "Requesting spa config update");
            config_request_status = 0; // Reset to request config again
        }

        void BalboaSpa::request_filter_settings_update()
        {
            ESP_LOGD(TAG, "Requesting spa filter settings update");
            filtersettings_request_status = 0; // Reset to request filter settings again
        }

        void BalboaSpa::request_fault_log_update()
        {
            ESP_LOGD(TAG, "Requesting spa fault log update");
            faultlog_request_status = 0; // Reset to request fault log again
        }

        void BalboaSpa::set_time(int hour, int minute)
        {
            if (hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59)
            {
                target_hour = hour;
                target_minute = minute;
                PendingCmd time_cmd;
                time_cmd.type = CmdType::SET_TIME;
                time_cmd.time.hour = target_hour;
                time_cmd.time.minute = target_minute;
                time_cmd.target_field = ExpectedField::NONE;
                time_cmd.max_retries = 0;
                insert_cmd(time_cmd);
                ESP_LOGI(TAG, "Spa time set to: %02d:%02d", hour, minute);
            }
        }

        void BalboaSpa::set_timescale(bool is_24h)
        {
            PendingCmd pref_cmd;
            pref_cmd.type = CmdType::SET_PREF;
            pref_cmd.pref.code = 0x02;
            pref_cmd.pref.data = (uint8_t)(is_24h ? 0x01 : 0x00);
            pref_cmd.target_field = ExpectedField::CLOCK_24H;
            pref_cmd.max_retries = 3;
            insert_cmd(pref_cmd);
        }

        void BalboaSpa::set_filter1_config(uint8_t start_hour, uint8_t start_minute, uint8_t duration_hour, uint8_t duration_minute)
        {
            if (start_hour < 24 && start_minute < 60 && duration_hour < 24 && duration_minute < 60)
            {
                target_filter1_start_hour = start_hour;
                target_filter1_start_minute = start_minute;
                target_filter1_duration_hour = duration_hour;
                target_filter1_duration_minute = duration_minute;
                enqueue_current_filter();
            }
        }

        void BalboaSpa::set_filter2_config(uint8_t start_hour, uint8_t start_minute, uint8_t duration_hour, uint8_t duration_minute)
        {
            if (start_hour < 24 && start_minute < 60 && duration_hour < 24 && duration_minute < 60)
            {
                target_filter2_start_hour = start_hour;
                target_filter2_start_minute = start_minute;
                target_filter2_duration_hour = duration_hour;
                target_filter2_duration_minute = duration_minute;
                target_filter2_enable = true;
                enqueue_current_filter();
            }
        }

        void BalboaSpa::disable_filter2()
        {
            target_filter2_enable = false;
            enqueue_current_filter();
        }

        void BalboaSpa::set_filter1_start_time(uint8_t hour, uint8_t minute)
        {
            if (hour < 24 && minute < 60)
            {
                target_filter1_start_hour = hour;
                target_filter1_start_minute = minute;
                enqueue_current_filter();
                ESP_LOGI(TAG, "Filter 1 start time set to %02d:%02d", hour, minute);
            }
        }

        void BalboaSpa::set_filter1_duration(uint8_t hour, uint8_t minute)
        {
            if (hour < 24 && minute < 60)
            {
                target_filter1_duration_hour = hour;
                target_filter1_duration_minute = minute;
                enqueue_current_filter();
                ESP_LOGI(TAG, "Filter 1 duration set to %02d:%02d", hour, minute);
            }
        }

        void BalboaSpa::set_filter2_start_time(uint8_t hour, uint8_t minute)
        {
            if (hour < 24 && minute < 60)
            {
                target_filter2_start_hour = hour;
                target_filter2_start_minute = minute;
                target_filter2_enable = true;
                enqueue_current_filter();
                ESP_LOGI(TAG, "Filter 2 start time set to %02d:%02d", hour, minute);
            }
        }

        void BalboaSpa::set_filter2_duration(uint8_t hour, uint8_t minute)
        {
            if (hour < 24 && minute < 60)
            {
                target_filter2_duration_hour = hour;
                target_filter2_duration_minute = minute;
                target_filter2_enable = true;
                enqueue_current_filter();
                ESP_LOGI(TAG, "Filter 2 duration set to %02d:%02d", hour, minute);
            }
        }

        void BalboaSpa::toggle_light(uint8_t index)
        {
            uint8_t code = (uint8_t)(0x10 + index);
            ExpectedField field = (index == 1) ? ExpectedField::LIGHT_0 : ExpectedField::LIGHT_1;
            uint8_t current = (index == 1) ? spaState.lights[0] : spaState.lights[1];
            uint8_t expected = current ? 0 : 1;
            enqueue_toggle(code, field, expected, 3);
        }

        void BalboaSpa::toggle_jet(uint8_t index, uint8_t expected_state, uint8_t max_retries)
        {
            uint8_t code = (uint8_t)(tiPump1 - 1 + index); // tiPump1=0x04, tiPump2=0x05, etc.
            ExpectedField field = toggle_code_to_field(code);
            enqueue_toggle(code, field, expected_state, max_retries);
        }

        void BalboaSpa::toggle_blower(uint8_t expected_state, uint8_t max_retries)
        {
            enqueue_toggle(tiBlower, ExpectedField::BLOWER, expected_state, max_retries);
        }

        void BalboaSpa::clear_reminder()
        {
            ESP_LOGI(TAG, "Clearing spa reminder");
            enqueue_toggle(0x03, ExpectedField::NONE, 0, 0);
        }

        bool BalboaSpa::read_serial()
        {
            while(input_failed && available() >= 2){
                read_byte(&received_byte);
                if(received_byte == 0x7E){
                    if(peek_equals(0x7E)){
                        input_failed = false;
                        input_started = false;
                    }
                }
            }

            if(input_failed){
                return false;
            }

            // Waiting for SOF
            if(!input_started){
                if(available() < 2){
                    // Need at least 2 to check for duplicate SOF or new SOF
                    return false;
                }
                read_byte(&received_byte);
                if(received_byte == 0x7E){
                    if(peek_equals(0x7E)){
                        ESP_LOGV(TAG, "Skipping duplicate SOF byte");
                        // Don't do anything, the next read will handle it.
                        return true;
                    }

                    // read length byte immediately after SOF to start the packet
                    read_byte(&input_buffer[0]);
                    size_t received_length = (size_t)(input_buffer[0]);
                    // Sanity check: total packet size
                    if (received_length > BUFFER_LENGTH - 1)
                    {
                        ESP_LOGV(TAG, "Length byte %d would exceed buffer capacity — discarding", received_length);
                        input_failed = true;
                        return true;
                    }
                    if (received_length < 2)
                    {
                        ESP_LOGV(TAG, "Invalid length byte %d — discarding", received_length);
                        input_failed = true;
                        return true;
                    }
                    input_started = true;
                } else{
                    return true; // Not the start of a packet, but we consumed a byte, so return true to indicate progress.
                }
            }

            size_t length = (size_t)(input_buffer[0]);

            // length byte includes itself, but we also need the closing message, so read until length (which is length byte + payload) + 1 (closing byte)
            if(available() < length){
                // Not enough bytes available yet, wait for the next update to read more.
                return false;
            }

            if(!read_array(&input_buffer[1], length - 1)){
                // Failed to read the expected number of bytes, mark as failed and wait for the next packet.
                ESP_LOGV(TAG, "Failed to read expected bytes. Expected: %d", length - 1);
                input_failed = true;
                return true;
            }

            if(!peek_equals(0x7E)){
                // Failed to peek the closing byte, mark as failed and wait for the next packet.
                ESP_LOGV(TAG, "Failed to peek closing byte.");
                input_failed = true;
                return true;
            }

            const MessageBase *msg_base = reinterpret_cast<const MessageBase *>(input_buffer);
            if (!msg_base->CheckCRC())
            {
                uint8_t calculated_crc = msg_base->CalcCRC();
                uint8_t packet_crc = input_buffer[length - 1];
                ESP_LOGV(CRC_TAG, "CRC mismatch: calculated=0x%02X packet=0x%02X msg=[%s]",
                         calculated_crc, packet_crc,
                         format_hex(input_buffer, length).c_str());
                input_failed = true;
                return true;
            }

            read_byte(&received_byte); // consume the closing byte. Leave it here so we can get the double E7
            last_received_time = millis();
            // Set it to false here so we know it is always cleared.
            input_started = false;
            process_message();
            return true;
        }

        void BalboaSpa::establish_id()
        {
            uint8_t found_msg_type = input_buffer[3];
            ESP_LOGD(TAG, "Spa/node/id: %s", "Unregistered");
            // FE BF 02:got new client ID
            if (found_msg_type == 0x02)
            {
                if (use_client_id_override)
                {
                    client_id = client_id_override;
                    ESP_LOGI(TAG, "Spa/node/id: Using override ID: 0x%02X, acknowledging", client_id);
                }
                else if (use_remembered_client_id_)
                {
                    client_id = remembered_client_id_;
                    ESP_LOGI(TAG, "Spa/node/id: Using remembered ID: 0x%02X, acknowledging", client_id);
                }
                else
                {
                    client_id = input_buffer[4];
                    if (client_id > MAX_CLIENT_ID)
                        client_id = MAX_CLIENT_ID;
                    ESP_LOGI(TAG, "Spa/node/id: Got ID: 0x%02X, acknowledging", client_id);
                    if (remember_client_id_)
                    {
                        client_id_pref_.save(&client_id);
                        ESP_LOGI(TAG, "Saved client ID 0x%02X to preferences", client_id);
                    }
                }
                ID_ack();
                ESP_LOGD(TAG, "Spa/node/id: 0x%02X", client_id);
                for (const auto &listener : this->client_id_listeners_)
                    listener();
            }

            // FE BF 00:Any new clients?
            if (found_msg_type == 0x00)
            {
                ESP_LOGD(TAG, "Spa/node/id: %s", "Requesting ID");
                ID_request();
            }
        }

        void BalboaSpa::process_message(){
            size_t length = (size_t)(input_buffer[0]);
            uint8_t found_client_id = input_buffer[1];
            uint8_t found_msg_type = input_buffer[3];
            uint8_t found_crc = input_buffer[length - 1];
            
            const MessageBase *base = reinterpret_cast<const MessageBase *>(input_buffer);
            if(found_client_id == client_id){
                input_buffer[1] = 0xFF;
                switch (base->_messageType)
                {
                    case msConfigResponse: {
                        if (!check_msg_length<ConfigResponseMessage>(input_buffer, "ConfigResponseMessage")) break;
                        const ConfigResponseMessage *msg = reinterpret_cast<const ConfigResponseMessage *>(input_buffer);
                        (void)msg;
                        ESP_LOGV(TAG, "ConfigResponseMessage");
                        break;
                    }
                    case msFilterConfig: {
                        if(last_filter_crc != found_crc){
                        if (!check_msg_length<FilterStatusMessage>(input_buffer, "FilterStatusMessage")) break;
                            const FilterStatusMessage *msg = reinterpret_cast<const FilterStatusMessage *>(input_buffer);
                            ESP_LOGV(TAG, "FilterStatusMessage");
                            decodeFilterSettings(msg);
                        }
                        break;
                    }
                    case msControlConfig2: {
                        if (!check_msg_length<ControlConfig2Response>(input_buffer, "ControlConfig2Response")) break;
                        ESP_LOGV(TAG, "ControlConfig2Response");
                        if(last_settings_crc != found_crc){
                            const ControlConfig2Response *msg = reinterpret_cast<const ControlConfig2Response *>(input_buffer);
                            decodeSettings(msg);
                        }
                        break;
                    }
                    default:
                        if(found_msg_type == 0x06){ 
                            send_message();
                        }
                        else if (found_msg_type == 0x28 && last_fault_crc != found_crc)
                        {
                            decodeFault();
                        } else{
                            ESP_LOGD(TAG, "Unhandled client_id targeted message type: " MSGTYPE_FMT, MSGTYPE_ARGS(base->_messageType));
                        }
                        break;
                }
            } else if(found_client_id == 0xFF){
                switch (base->_messageType)
                {
                    case msStatus: {
                        if (!check_msg_length<StatusMessage>(input_buffer, "StatusMessage")) break;
                        const StatusMessage *msg = reinterpret_cast<const StatusMessage *>(input_buffer);
                        if(last_state_crc != msg->_suffix._check){
                            ESP_LOGV(TAG, "StatusMessage: currentTemp=%d setTemp=%d", msg->_currentTemp, msg->_setTemp);
                            decodeState(msg);
                        }
                        break;
                    }
                    case msSetTempRange: {
                        ESP_LOGV(TAG, "SetTempRangeMessage");
                        break;
                    }
                    default:
                        ESP_LOGD(TAG, "Unhandled broadcast message type: " MSGTYPE_FMT, MSGTYPE_ARGS(base->_messageType));
                        break;
                }
            } else if(found_client_id == 0xFE){
                if(client_id == 0){
                    establish_id(); 
                }
            }
        }


        template <typename T>
        void BalboaSpa::send_typed(T &msg)
        {
            msg.set_client(client_id);
            msg.SetCRC();
            rs485_send(msg);
        }

        void BalboaSpa::send_message()
        {
            uint32_t now_cts = millis();
            if (last_cts_time > 0)
                ESP_LOGV(TAG, "CTS interval: %u ms", now_cts - last_cts_time);
            last_cts_time = now_cts;

            if (cmd_count_ > 0 && pending_msg_len_ > 0 && cmd_queue_[0].available_at <= millis())
            {
                PendingCmd sent = cmd_queue_[0];

                if (millis() - sent.queued_at > 30000)
                {
                    ESP_LOGW(TAG, "Dropping expired command field=%d type=%d (age %u ms)",
                             (int)sent.target_field, (int)sent.type,
                             (unsigned)(millis() - sent.queued_at));
                }
                else
                {
                    write_byte(0x7E);
                    write_array(pending_msg_buf_, pending_msg_len_);
                    write_byte(0x7E);
                    flush();

                    switch (sent.type)
                    {
                    case CmdType::SET_TIME:
                        ESP_LOGI(TAG, "Sending SET_TIME %02d:%02d", sent.time.hour, sent.time.minute);
                        break;
                    case CmdType::SET_TEMP:
                        ESP_LOGI(TAG, "Sending SET_TEMP %.1f (attempt %d/%d)",
                                 sent.target_temperature, sent.retry_count + 1, sent.max_retries + 1);
                        break;
                    case CmdType::SET_PREF:
                        ESP_LOGI(TAG, "Sending SET_PREF code=0x%02X data=0x%02X (attempt %d/%d)",
                                 sent.pref.code, sent.pref.data,
                                 sent.retry_count + 1, sent.max_retries + 1);
                        break;
                    case CmdType::SET_FILTER:
                        ESP_LOGI(TAG, "Sending SET_FILTER f1=%02d:%02d dur=%02d:%02d f2=%s %02d:%02d dur=%02d:%02d (attempt %d/%d)",
                                 sent.filter.f1_start_hour, sent.filter.f1_start_minute,
                                 sent.filter.f1_dur_hour, sent.filter.f1_dur_minute,
                                 sent.filter.f2_enable ? "on" : "off",
                                 sent.filter.f2_start_hour, sent.filter.f2_start_minute,
                                 sent.filter.f2_dur_hour, sent.filter.f2_dur_minute,
                                 sent.retry_count + 1, sent.max_retries + 1);
                        break;
                    case CmdType::TOGGLE:
                    default:
                        ESP_LOGI(TAG, "Sending TOGGLE 0x%02X field=%d expected=%d (attempt %d/%d)",
                                 sent.toggle_code, (int)sent.target_field, sent.expected_toggle_value,
                                 sent.retry_count + 1, sent.max_retries + 1);
                        break;
                    }

                    remove_at(0);
                    if (sent.target_field != ExpectedField::NONE && sent.retry_count < sent.max_retries)
                    {
                        sent.retry_count++;
                        sent.available_at = millis() + RETRY_BACKOFF_MS;
                        insert_cmd(sent);  // rebuilds automatically
                    }
                    else
                    {
                        prune_and_rebuild();
                    }
                }
                return;
            }

            if (config_request_status == 0)
            {
                ControlConfigRequest msg;
                send_typed(msg);
                ESP_LOGD(TAG, "Spa/config/status: %s", "Getting config");
                config_request_status = 1;
            }
            else if (faultlog_request_status == 0)
            {
                FaultLogRequest msg;
                send_typed(msg);
                faultlog_request_status = 1;
                ESP_LOGD(TAG, "Spa/debug/faultlog_request_status: %s", "requesting fault log, #1");
            }
            else if (filtersettings_request_status == 0 && faultlog_request_status == 2)
            {
                FilterConfigRequest msg;
                send_typed(msg);
                ESP_LOGD(TAG, "Spa/debug/filtersettings_request_status: %s", "requesting filter settings");
                filtersettings_request_status = 1;
            }
            else
            {
                NothingToSendMessage msg;
                send_typed(msg);
            }
        }

        void BalboaSpa::ID_request()
        {
            IDRequestMessage msg;
            msg.SetCRC();
            rs485_send(msg);
        }

        void BalboaSpa::ID_ack()
        {
            IDACKMessage msg;
            send_typed(msg);
        }

        void BalboaSpa::rs485_send(const MessageBaseOutgoing &msg)
        {
            const uint8_t *data = reinterpret_cast<const uint8_t *>(&msg);
            write_byte(0x7E); // SOF
            write_array(data, msg._length);
            write_byte(0x7E); // EOF
            flush();
        }


        void BalboaSpa::decodeSettings(const ControlConfig2Response *msg)
        {
            memcpy(&spaConfig, msg, sizeof(ControlConfig2Response));
            ESP_LOGD(TAG, "Spa/config: pumps=%d/%d/%d/%d/%d/%d lights=%d/%d circ=%d blower=%d mister=%d aux=%d/%d",
                     spaConfig.pump1, spaConfig.pump2, spaConfig.pump3,
                     spaConfig.pump4, spaConfig.pump5, spaConfig.pump6,
                     spaConfig.light1, spaConfig.light2,
                     spaConfig.circ, spaConfig.blower, spaConfig.mister,
                     spaConfig.aux1, spaConfig.aux2);
            config_request_status = 2;
            last_settings_crc = msg->_suffix._check;
        }

        void BalboaSpa::decodeState(const StatusMessage *msg)
        {
            TEMP_SCALE new_temp_scale = static_cast<TEMP_SCALE>(msg->_tempScaleCelsius);
            CLOCK_MODE new_clock_mode_24hr = static_cast<CLOCK_MODE>(msg->_24hrTime);
            if(new_temp_scale != spa_temp_scale || new_clock_mode_24hr != clock_mode_24hr){
                ESP_LOGD(TAG, "Spa/config changed: temperature_scale %d->%d clock_mode_24hr %d->%d (from status)",
                         spa_temp_scale, new_temp_scale, clock_mode_24hr, new_clock_mode_24hr);
                spa_temp_scale = new_temp_scale;
                clock_mode_24hr = new_clock_mode_24hr;
            }
            SpaState newState(*msg, spa_temp_scale);
            
            ESP_LOGI(TAG, "Spa/status received: current_temp=%f set_temp=%f rest_mode=%d highrange=%d",
                     newState.current_temp, newState.target_temp, (int)newState.rest_mode, newState.highrange & 0x1);


            if (std::isnan(newState.target_temp))
                ESP_LOGW(TAG, "Spa/temperature/target INVALID (raw %d)", msg->_setTemp);
            if (std::isnan(newState.current_temp) && msg->_currentTemp != 0xFF)
                ESP_LOGW(TAG, "Spa/temperature/current INVALID (raw %d)", msg->_currentTemp);

            state_dirty_ = true;
            highrange_dirty_ |= (newState.highrange & 0x1) != (spaState.highrange & 0x1);

            spaState = newState;
            prune_and_rebuild();

            last_state_crc = msg->_suffix._check;
            last_status_received_ms_ = millis();
        }

        void BalboaSpa::decodeFilterSettings(const FilterStatusMessage *msg)
        {
            spaFilterSettings.filter1_hour = input_buffer[4];
            spaFilterSettings.filter1_minute = input_buffer[5];
            spaFilterSettings.filter1_duration_hour = input_buffer[6];
            spaFilterSettings.filter1_duration_minute = input_buffer[7];
            spaFilterSettings.filter2_enable = bitRead(input_buffer[8], 7);                             // check
            spaFilterSettings.filter2_hour = input_buffer[8] ^ (spaFilterSettings.filter2_enable << 7); // check
            spaFilterSettings.filter2_minute = input_buffer[9];
            spaFilterSettings.filter2_duration_hour = input_buffer[10];
            spaFilterSettings.filter2_duration_minute = input_buffer[11];

            // Filter 1 time conversion
            static PROGMEM const char *format_string = R"({"start":"%.2i:%.2i","duration":"%.2i:%.2i"} )";
            const auto payload_length = std::snprintf(nullptr, 0, format_string, spaFilterSettings.filter1_hour, spaFilterSettings.filter1_minute, spaFilterSettings.filter1_duration_hour, spaFilterSettings.filter1_duration_minute);

            char filter_payload[payload_length + 1];
            std::memset(filter_payload, 0, payload_length + 1);
            std::snprintf(filter_payload, payload_length + 1, format_string, spaFilterSettings.filter1_hour, spaFilterSettings.filter1_minute, spaFilterSettings.filter1_duration_hour, spaFilterSettings.filter1_duration_minute);
            ESP_LOGD(TAG, "Spa/filter1/state: %s", filter_payload);

            // Filter 2 time conversion
            ESP_LOGD(TAG, "Spa/filter2_enabled/state: %s", spaFilterSettings.filter2_enable == 1 ? STRON : STROFF);
            std::snprintf(filter_payload, payload_length + 1, format_string, spaFilterSettings.filter2_hour, spaFilterSettings.filter2_minute, spaFilterSettings.filter2_duration_hour, spaFilterSettings.filter2_duration_minute);
            ESP_LOGD(TAG, "Spa/filter2/state: %s", filter_payload);

            filtersettings_request_status = 2;
            filtersettings_update_timer = 0; // Reset timer after successful decode
            prune_and_rebuild();

            for (const auto &filter_listener : this->filter_listeners_)
            {
                filter_listener(&spaFilterSettings);
            }

            last_filter_crc = input_buffer[input_buffer[0] - 1];
        }

        void BalboaSpa::decodeFault()
        {
            // Accesses up to input_buffer[9] — require at least 10 bytes.
            if (input_buffer[0] < 10)
            {
                ESP_LOGW(TAG, "FaultLog message too short: got %d bytes, need 10", input_buffer[0]);
                return;
            }
            spaFaultLog.total_entries = input_buffer[4];
            spaFaultLog.current_entry = input_buffer[5];
            spaFaultLog.fault_code = input_buffer[6];
            switch (spaFaultLog.fault_code)
            { // this is a inelegant way to do it, a lookup table would be better
            case 15:
                spaFaultLog.fault_message = "Sensors are out of sync";
                break;
            case 16:
                spaFaultLog.fault_message = "The water flow is low";
                break;
            case 17:
                spaFaultLog.fault_message = "The water flow has failed";
                break;
            case 18:
                spaFaultLog.fault_message = "The settings have been reset";
                break;
            case 19:
                spaFaultLog.fault_message = "Priming Mode";
                break;
            case 20:
                spaFaultLog.fault_message = "The clock has failed";
                break;
            case 21:
                spaFaultLog.fault_message = "The settings have been reset";
                break;
            case 22:
                spaFaultLog.fault_message = "Program memory failure";
                break;
            case 26:
                spaFaultLog.fault_message = "Sensors are out of sync -- Call for service";
                break;
            case 27:
                spaFaultLog.fault_message = "The heater is dry";
                break;
            case 28:
                spaFaultLog.fault_message = "The heater may be dry";
                break;
            case 29:
                spaFaultLog.fault_message = "The water is too hot";
                break;
            case 30:
                spaFaultLog.fault_message = "The heater is too hot";
                break;
            case 31:
                spaFaultLog.fault_message = "Sensor A Fault";
                break;
            case 32:
                spaFaultLog.fault_message = "Sensor B Fault";
                break;
            case 34:
                spaFaultLog.fault_message = "A pump may be stuck on";
                break;
            case 35:
                spaFaultLog.fault_message = "Hot fault";
                break;
            case 36:
                spaFaultLog.fault_message = "The GFCI test failed";
                break;
            case 37:
                spaFaultLog.fault_message = "Standby Mode (Hold Mode)";
                break;
            default:
                spaFaultLog.fault_message = "Unknown error";
                break;
            }
            spaFaultLog.days_ago = input_buffer[7];
            spaFaultLog.hour = input_buffer[8];
            spaFaultLog.minutes = input_buffer[9];
            ESP_LOGD(TAG, "Spa/fault/Entries: %d", spaFaultLog.total_entries);
            ESP_LOGD(TAG, "Spa/fault/Entry: %d", spaFaultLog.current_entry);
            ESP_LOGD(TAG, "Spa/fault/Code: %d", spaFaultLog.fault_code);
            ESP_LOGD(TAG, "Spa/fault/Message: %s", spaFaultLog.fault_message);
            ESP_LOGD(TAG, "Spa/fault/DaysAgo: %d", spaFaultLog.days_ago);
            ESP_LOGD(TAG, "Spa/fault/Hours: %d", spaFaultLog.hour);
            ESP_LOGD(TAG, "Spa/fault/Minutes: %d", spaFaultLog.minutes);
            faultlog_request_status = 2;
            // ESP_LOGD(TAG, "Spa/debug/faultlog_request_status: have the faultlog, #2");

            // Notify fault log listeners
            for (const auto &listener : this->fault_log_listeners_)
            {
                listener(&spaFaultLog);
            }

            last_fault_crc = input_buffer[input_buffer[0] - 1];
        }

        bool BalboaSpa::is_communicating()
        {
            return client_id != 0;
        }

        void BalboaSpa::reconnect()
        {
            ESP_LOGD(TAG, "Reconnect requested, clearing client ID");
            if (use_remembered_client_id_)
            {
                use_remembered_client_id_ = false;
                remembered_client_id_ = 0;
                if (remember_client_id_)
                {
                    uint8_t zero = 0;
                    client_id_pref_.save(&zero);
                    ESP_LOGI(TAG, "Cleared remembered client ID from preferences");
                }
            }
            client_id = 0;
            for (const auto &listener : this->client_id_listeners_)
                listener();
        }

        void BalboaSpa::set_client_id(uint8_t id)
        {
            if (id >= 1 && id <= 0x2F)
            {
                client_id_override = id;
                use_client_id_override = true;
                ESP_LOGD(TAG, "Client ID override set to %d", id);
            }
            else
            {
                ESP_LOGW(TAG, "Invalid client ID override %d, must be between 1 and 47", id);
            }
        }

    } // namespace balboa_spa
} // namespace esphome
