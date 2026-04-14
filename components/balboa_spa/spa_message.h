#pragma once

#include <stdint.h>
#include <stddef.h>
#include "spa_types.h"

#ifndef uint8_t
typedef uint8_t uint8_t;
#endif

namespace esphome
{
    namespace balboa_spa
    {
        // Simple time/temp value types used in outgoing message constructors
        struct SpaTime
        {
            uint8_t hour;
            uint8_t minute;
            bool displayAs24Hr;
        };

        struct SpaTemp
        {
            uint8_t temp; // raw spa-scale uint8_t
        };
        // #define MESSAGE_ID(X,Y,Z) ((unsigned long)(X&0xff) | ((unsigned long)(Y&0xff)<<8) | ((unsigned long)(Z&0xff)<<16))

        constexpr uint32_t MESSAGE_ID(uint8_t b1, uint8_t b2, uint8_t b3)
        {
            return ((uint32_t)(b1) | ((uint32_t)(b2) << 8) | ((uint32_t)(b3) << 16));
        }

        //  Message IDs for responses from the hot-tub.
        enum SpaResponseMessageID : uint32_t
        {
            msStatus = MESSAGE_ID(0xff, 0xaf, 0x13),
            msSetTempRange = MESSAGE_ID(0xff, 0xaf, 0x26),
            msConfigResponse = MESSAGE_ID(0xff, 0xbf, 0x94), // actually targeted by client_id, but ignored for matching
            msFilterConfig = MESSAGE_ID(0xff, 0xbf, 0x23), // actually targeted by client_id, but ignored for matching
            msControlConfig = MESSAGE_ID(0xff, 0xbf, 0x24), // actually targeted by client_id, but ignored for matching
            msControlConfig2 = MESSAGE_ID(0xff, 0xbf, 0x2e), // actually targeted by client_id, but ignored for matching
            unknownMessage = MESSAGE_ID(0xff, 0xbf, 0x06), // actually targeted by client_id, but ignored for matching
        };

        //  Message Id's for commands send to the hot-tub
        enum SpaCommandMessageID : uint32_t
        {
            msConfigRequest = MESSAGE_ID(0x0a, 0xbf, 0x04),
            msControlConfigRequest = MESSAGE_ID(0x0a, 0xbf, 0x22),
            msFaultLogRequest = MESSAGE_ID(0x0a, 0xbf, 0x22),      // same type, different payload
            msFilterConfigRequest = MESSAGE_ID(0x0a, 0xbf, 0x22),  // same type, different payload
            msToggleItemRequest = MESSAGE_ID(0x0a, 0xbf, 0x11),
            msSetTempRequest = MESSAGE_ID(0x0a, 0xbf, 0x20),
            msSetTempScaleRequest = MESSAGE_ID(0x0a, 0xbf, 0x27),
            msSetPreferenceRequest = MESSAGE_ID(0x0a, 0xbf, 0x27),  // same type, different payload bytes
            msSetTimeRequest = MESSAGE_ID(0x0a, 0xbf, 0x21),
            msSetFilterConfigRequest = MESSAGE_ID(0x0a, 0xbf, 0x23),
            msNothingToSend = MESSAGE_ID(0x0a, 0xbf, 0x07),
            msIDRequest = MESSAGE_ID(0xfe, 0xbf, 0x01),
            msIDACK = MESSAGE_ID(0x0a, 0xbf, 0x03),
            msSetWiFiSettingsRequest = MESSAGE_ID(0x0a, 0xbf, 0x92) // Not implemented
        };

        struct MessageBase
        {
            uint8_t _length;
            uint32_t _messageType : 24;

            void Dump() const;
            void Dump(size_t) const;
            void SetCRC();
            bool CheckCRC() const;
            uint8_t CalcCRC() const;

            // Replace the client-id byte (b1) in the packed message type at runtime.
            void set_client(uint8_t id) { _messageType = (_messageType & 0xFFFF00u) | id; }

        protected:
            MessageBase(size_t, unsigned long);
            MessageBase() = default;
        } __attribute__((packed));

        //  The last byte of overhead is the CRC check byte.
        struct MessageSuffix
        {
            uint8_t _check;

            MessageSuffix();
        } __attribute__((packed));

        //  Some type safety, make sure the right message ID's are applied to the right
        //  messages.
        struct MessageBaseIncoming : public MessageBase
        {
        };

        struct MessageBaseOutgoing : public MessageBase
        {
        protected:
            MessageBaseOutgoing(size_t size, SpaCommandMessageID id)
                : MessageBase(size, id) {};
        };

        //  Following messages are only received.  They are never instantiated, hence no
        //  constructor.
        //
        //  Using bitfields, we can read the message directly from memory without any
        //  bit-twiddling.
        //
        //  As needed, unknown areas are marked as '_r*'.  Note that some bitfields are
        //  skipped rather than marked.

        struct ConfigResponseMessage : public MessageBaseIncoming
        {
            uint8_t _r[25]; //  Unknown structure
            MessageSuffix _suffix;
        } __attribute__((packed));

        struct StatusMessage : public MessageBaseIncoming
        {                               // uint8_t #
            uint8_t _r1;                   // 00
            ReminderType _reminder;        // 01
            uint8_t _currentTemp;          // 02
            uint8_t _hour;                 // 03
            uint8_t _minute;               // 04
            uint8_t _heatingMode : 2;      // 05
            uint8_t : 0;                   //
            uint8_t _panelMessage : 4;     // 06
            uint8_t : 0;                   //
            uint8_t _r2;                   // 07,
            uint8_t _holdTime;             // 08  if _systemHold is true
            uint8_t _tempScaleCelsius : 1; // 09
            uint8_t _24hrTime : 1;         //
            uint8_t _filter1Running : 1;   //
            uint8_t _filter2Running : 1;   //
            uint8_t : 0;                   //
            uint8_t _r3 : 2;               // 10
            uint8_t _tempRange : 1;        //
            uint8_t _r4 : 1;               //
            uint8_t _heating : 2;          //
            uint8_t : 0;                   //
            uint8_t _pump1 : 2;            // 11
            uint8_t _pump2 : 2;            //
            uint8_t _pump3 : 2;            //
            uint8_t _pump4 : 2;            //
            uint8_t _r5;                   // 12
            uint8_t _r6 : 1;               // 13
            uint8_t _circPump : 1;         //
            uint8_t _blower : 1;           //
            uint8_t : 0;                   //
            uint8_t _light1 : 2;           // 14
            uint8_t _light2 : 2;           //
            uint8_t : 0;                   //
            uint8_t _r7[3];                // 15->17
            uint8_t _r7a : 1;              // both 18 & 19, bit 2 seem related to time not
            uint8_t _timeUnset : 1;        // yet set.
            uint8_t : 0;
            CleanupCycle _cleanupCycle : 4;     // 19
            uint8_t : 0;
            uint8_t _setTemp; // 20
            uint8_t _r8 : 2;  // 21
            uint8_t _systemHold : 1;
            uint8_t : 0;
            uint8_t _r9[2]; // 22, 23

            MessageSuffix _suffix;

        } __attribute__((packed));

        struct FilterStatusMessage : public MessageBaseIncoming
        {                                // uint8_t
            uint8_t filter1StartHour;       // 00
            uint8_t filter1StartMinute;     // 01
            uint8_t filter1DurationHours;   // 02
            uint8_t filter1DurationMinutes; // 03
            uint8_t filter2StartHour : 7;   // 04
            uint8_t filter2enabled : 1;     //
            uint8_t filter2StartMinute;     // 05
            uint8_t filter2DurationHours;   // 06
            uint8_t filter2DurationMinutes; // 07

            MessageSuffix _suffix;
        } __attribute__((packed));

        struct ControlConfigResponse : public MessageBaseIncoming
        {                        // uint8_t
            uint8_t _version[3];    // 00, 01, 02
            uint8_t _r1;            // 03
            uint8_t _name[8];       // 04->11
            uint8_t _currentSetup;  // 12
            uint32_t _signature; // 13->16
            uint8_t _r2[4];         // 17->20

            MessageSuffix _sufffix;
        } __attribute__((packed));

        struct ControlConfig2Response : public MessageBaseIncoming
        {                            // payload byte #
            uint8_t pump1  : 2;     // 00
            uint8_t pump2  : 2;     //
            uint8_t pump3  : 2;     //
            uint8_t pump4  : 2;     //
            uint8_t : 0;            //
            uint8_t pump5  : 2;     // 01
            uint8_t _r1    : 4;     //
            uint8_t pump6  : 2;     //
            uint8_t : 0;            //
            uint8_t light1  : 2;    // 02
            uint8_t light2  : 2;    //
            uint8_t _r2     : 4;    //
            uint8_t : 0;            //
            uint8_t blower  : 2;    // 03
            uint8_t _r3     : 5;    //
            uint8_t circ    : 1;    // bit 7
            uint8_t : 0;            //
            uint8_t aux1    : 1;    // 04
            uint8_t aux2    : 1;    //
            uint8_t _r4     : 2;    //
            uint8_t mister  : 2;    //
            uint8_t _r5     : 2;    //
            uint8_t : 0;            //

            MessageSuffix _suffix;
        } __attribute__((packed));

        //  Following messages are only sent.  Once contructed, they are ready to go.
        struct ConfigRequest : public esphome::balboa_spa::MessageBaseOutgoing
        {
            ConfigRequest();
            //  No content
            MessageSuffix _suffix;
        } __attribute__((packed));

        struct FilterConfigRequest : public esphome::balboa_spa::MessageBaseOutgoing
        {
            FilterConfigRequest();

            uint8_t _payload[3];

            MessageSuffix _suffix;
        } __attribute__((packed));

        struct ControlConfigRequest : public esphome::balboa_spa::MessageBaseOutgoing
        {
            ControlConfigRequest();

            uint8_t _payload[3];

            MessageSuffix _suffix;
        } __attribute__((packed));

        struct FaultLogRequest : public esphome::balboa_spa::MessageBaseOutgoing
        {
            FaultLogRequest();

            uint8_t _payload[3];

            MessageSuffix _suffix;
        } __attribute__((packed));

        struct SetSpaTime : public esphome::balboa_spa::MessageBaseOutgoing
        {
            SetSpaTime(const SpaTime &);

            uint8_t _hour : 7;
            uint8_t _displayAs24Hr : 1;
            uint8_t _minute;

            MessageSuffix _suffix;
        } __attribute__((packed));

        enum ToggleItem
        {
            tiPump1       = 0x04,
            tiPump2       = 0x05,
            tiPump3       = 0x06,
            tiBlower      = 0x0C,
            tiLight1      = 0x11,
            tiHoldMode    = 0x3C,
            tiTempRange   = 0x50,
            tiHeatingMode = 0x51,
        };

        struct ToggleItemMessage : public esphome::balboa_spa::MessageBaseOutgoing
        {
            ToggleItemMessage(ToggleItem Item);

            uint8_t _item;
            uint8_t _r;

            MessageSuffix _suffix;
        } __attribute__((packed));

        struct SetSpaTempMessage : public esphome::balboa_spa::MessageBaseOutgoing
        {
            SetSpaTempMessage(const SpaTemp &);

            uint8_t _temp;

            MessageSuffix _suffix;
        } __attribute__((packed));

        struct SetSpaTempScaleMessage : public esphome::balboa_spa::MessageBaseOutgoing
        {
            SetSpaTempScaleMessage(bool scaleCelsius);

            uint8_t _r1 = 0x01;
            uint8_t _scale;

            MessageSuffix _suffix;
        } __attribute__((packed));

        // Generic 2-byte preference write on message type 0x27.
        struct SetPreferenceMessage : public esphome::balboa_spa::MessageBaseOutgoing
        {
            SetPreferenceMessage(uint8_t code, uint8_t data);

            uint8_t _code;
            uint8_t _data;

            MessageSuffix _suffix;
        } __attribute__((packed));

        // Full filter-cycle configuration write (type 0x23).
        struct SetFilterConfigMessage : public esphome::balboa_spa::MessageBaseOutgoing
        {
            SetFilterConfigMessage(
                uint8_t f1_start_hour, uint8_t f1_start_minute,
                uint8_t f1_dur_hour,   uint8_t f1_dur_minute,
                bool    f2_enable,
                uint8_t f2_start_hour, uint8_t f2_start_minute,
                uint8_t f2_dur_hour,   uint8_t f2_dur_minute);

            uint8_t _f1_start_hour;
            uint8_t _f1_start_minute;
            uint8_t _f1_dur_hour;
            uint8_t _f1_dur_minute;
            uint8_t _f2_start;          // f2_start_hour | 0x80 when enabled
            uint8_t _f2_start_minute;
            uint8_t _f2_dur_hour;
            uint8_t _f2_dur_minute;

            MessageSuffix _suffix;
        } __attribute__((packed));

        // Sent when we have nothing queued for this CTS window.
        struct NothingToSendMessage : public esphome::balboa_spa::MessageBaseOutgoing
        {
            NothingToSendMessage();

            MessageSuffix _suffix;
        } __attribute__((packed));

        // Broadcast new-client request (source = 0xFE, fixed).
        struct IDRequestMessage : public esphome::balboa_spa::MessageBaseOutgoing
        {
            IDRequestMessage();

            uint8_t _payload[2];

            MessageSuffix _suffix;
        } __attribute__((packed));

        // Acknowledge a newly assigned client ID.
        struct IDACKMessage : public esphome::balboa_spa::MessageBaseOutgoing
        {
            IDACKMessage();   // call set_client(id) then SetCRC() before sending

            MessageSuffix _suffix;
        } __attribute__((packed));

    }
}