#pragma once

#include <string>

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "../balboaspa.h"

namespace esphome
{
  namespace balboa_spa
  {

    class BalboaSpaBinarySensors : public binary_sensor::BinarySensor
    {
    public:
      enum class BalboaSpaBinarySensorType : uint8_t
      {
        UNKNOWN = 0,
        BLOWER,
        HIGHRANGE,
        CIRCULATION,
        RESTMODE,
        HEATSTATE,
        CONNECTED,
        FILTER1_WINDOW_ACTIVE,
        FILTER2_WINDOW_ACTIVE,
        CLEANUP_CYCLE,
        TIME_SYNCED
      };

    public:
      BalboaSpaBinarySensors();
      void update();

      void set_parent(BalboaSpa *parent);
      void set_sensor_type(const BalboaSpaBinarySensorType _type) { sensor_type = _type; }

    private:
      BalboaSpaBinarySensorType sensor_type;
      BalboaSpa *spa_;
    };

  } // namespace balboa_spa
} // namespace esphome
