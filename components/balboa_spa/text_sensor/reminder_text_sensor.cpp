#include "reminder_text_sensor.h"

namespace esphome
{
    namespace balboa_spa
    {
        static const char *TAG = "balboa_spa.text_sensor.reminder";

        void ReminderTextSensor::set_parent(BalboaSpa *parent)
        {
            spa_ = parent;
            parent->register_listener([this]() { this->update(); });
        }

        void ReminderTextSensor::update()
        {
            const SpaState *spaState = spa_->get_current_state();
            // Check if the reminder has changed
            if (spaState->reminder != last_reminder_)
            {
                // NOTE: This list of reminder codes is incomplete. If you encounter an unknown
                // code, please open a GitHub issue with the code value and the reminder message
                // displayed on your spa control panel so we can expand this mapping.
                std::string reminder_message;
                switch (spaState->reminder)
                {
                    case ReminderType::NONE:
                        reminder_message = "None";
                        break;
                    case ReminderType::PRIMING:
                        reminder_message = "Priming";
                        break;
                    case ReminderType::CLEAN_FILTER:
                        reminder_message = "Clean Filter";
                        break;
                    case ReminderType::CHECK_SANITIZER:
                        reminder_message = "Check Sanitizer";
                        break;
                    case ReminderType::CHECK_PH:
                        reminder_message = "Check pH";
                        break;
                    case ReminderType::FAULT:
                        reminder_message = "Fault";
                        break;
                    default:
                        // Format unknown reminder code in hex
                        char hex_str[8];
                        snprintf(hex_str, sizeof(hex_str), "0x%02X", static_cast<uint8_t>(spaState->reminder));
                        reminder_message = std::string("Unknown (") + hex_str + ")";
                        break;
                }
                
                ESP_LOGD(TAG, "Reminder update: %s (0x%02X)",
                         reminder_message.c_str(), static_cast<uint8_t>(spaState->reminder));
                this->publish_state(reminder_message);
                last_reminder_ = spaState->reminder;
            }
        }

    } // namespace balboa_spa
} // namespace esphome
