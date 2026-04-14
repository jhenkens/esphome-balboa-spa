#include "spa_message.h"
#include "esphome/core/log.h"

namespace esphome
{
    namespace balboa_spa
    {

        static const char *MSG_TAG = "BalboaSpa.Message";

        // Balboa CRC-8: processes 'length' bytes starting at 'data'
        static uint8_t crc8_balboa(const uint8_t *data, size_t length)
        {
            unsigned long crc = 0x02;
            for (size_t i = 0; i < length; i++)
            {
                crc ^= data[i];
                for (int bit = 0; bit < 8; bit++)
                {
                    if (crc & 0x80)
                        crc = (crc << 1) ^ 0x07;
                    else
                        crc <<= 1;
                }
            }
            return (uint8_t)(crc ^ 0x02);
        }

        MessageBase::MessageBase(size_t size, unsigned long messageType)
        {
            _length = size;
            _messageType = messageType & 0x00ffffff;
        }

        void MessageBase::Dump() const
        {
            Dump(_length);
        }

        void MessageBase::Dump(size_t size) const
        {
            const uint8_t *p = reinterpret_cast<const uint8_t *>(this);
            std::string out;
            char buf[4];
            for (size_t i = 0; i < size; i++)
            {
                snprintf(buf, sizeof(buf), "%02X ", p[i]);
                out += buf;
            }
            ESP_LOGD(MSG_TAG, "Dump (%d bytes): %s", size, out.c_str());
        }

        void MessageBase::SetCRC()
        {
            *(reinterpret_cast<uint8_t *>(this) + _length - 1) = CalcCRC();
        }

        bool MessageBase::CheckCRC() const
        {
            return (*(reinterpret_cast<const uint8_t *>(this) + _length - 1) == CalcCRC());
        }

        uint8_t MessageBase::CalcCRC() const
        {
            return crc8_balboa(&_length, _length - 1);
        }

        MessageSuffix::MessageSuffix()
        {
            _check = 0xff;
        }

        ConfigRequest::ConfigRequest()
            : MessageBaseOutgoing(sizeof(*this), msConfigRequest)
        {}

        FilterConfigRequest::FilterConfigRequest()
            : MessageBaseOutgoing(sizeof(*this), msFilterConfigRequest),
              _payload{0x01, 0x00, 0x00}
        {}

        ControlConfigRequest::ControlConfigRequest()
            : MessageBaseOutgoing(sizeof(*this), msControlConfigRequest),
            _payload{0x00, 0x00, 0x01}
        {
        }

        FaultLogRequest::FaultLogRequest()
            : MessageBaseOutgoing(sizeof(*this), msFaultLogRequest),
              _payload{0x20, 0xFF, 0x00}
        {}

        SetSpaTime::SetSpaTime(const SpaTime &time)
            : MessageBaseOutgoing(sizeof(*this), msSetTimeRequest),
              _hour(time.hour), _displayAs24Hr(time.displayAs24Hr), _minute(time.minute)
        {}

        ToggleItemMessage::ToggleItemMessage(ToggleItem item)
            : MessageBaseOutgoing(sizeof(*this), msToggleItemRequest),
              _item(item), _r(0)
        {}

        SetSpaTempMessage::SetSpaTempMessage(const SpaTemp &temp)
            : MessageBaseOutgoing(sizeof(*this), msSetTempRequest),
              _temp(temp.temp)
        {}

        SetSpaTempScaleMessage::SetSpaTempScaleMessage(bool scaleCelsius)
            : MessageBaseOutgoing(sizeof(*this), msSetTempScaleRequest),
              _scale(scaleCelsius)
        {}

        SetPreferenceMessage::SetPreferenceMessage(uint8_t code, uint8_t data)
            : MessageBaseOutgoing(sizeof(*this), msSetPreferenceRequest),
              _code(code), _data(data)
        {}

        SetFilterConfigMessage::SetFilterConfigMessage(
            uint8_t f1_start_hour, uint8_t f1_start_minute,
            uint8_t f1_dur_hour,   uint8_t f1_dur_minute,
            bool    f2_enable,
            uint8_t f2_start_hour, uint8_t f2_start_minute,
            uint8_t f2_dur_hour,   uint8_t f2_dur_minute)
            : MessageBaseOutgoing(sizeof(*this), msSetFilterConfigRequest),
              _f1_start_hour(f1_start_hour), _f1_start_minute(f1_start_minute),
              _f1_dur_hour(f1_dur_hour),     _f1_dur_minute(f1_dur_minute),
              _f2_start(f2_enable ? (f2_start_hour | 0x80) : 0x00),
              _f2_start_minute(f2_start_minute),
              _f2_dur_hour(f2_dur_hour),     _f2_dur_minute(f2_dur_minute)
        {}

        NothingToSendMessage::NothingToSendMessage()
            : MessageBaseOutgoing(sizeof(*this), msNothingToSend)
        {}

        IDRequestMessage::IDRequestMessage()
            : MessageBaseOutgoing(sizeof(*this), msIDRequest),
              _payload{0x02, 0xF1}
        {}

        IDACKMessage::IDACKMessage()
            : MessageBaseOutgoing(sizeof(*this), msIDACK)
        {}

    } // namespace balboa_spa
} // namespace esphome
