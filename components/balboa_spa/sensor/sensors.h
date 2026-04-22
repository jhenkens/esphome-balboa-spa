#pragma once

#include <string>

#include "esphome/components/sensor/sensor.h"
#include "../balboaspa.h"
#include "../spa_temperature_base.h"

namespace esphome
{
  namespace balboa_spa
  {

    class BalboaSpaSensors : public sensor::Sensor, public SpaTemperatureBase
    {
    public:
      enum class BalboaSpaSensorType : uint8_t
      {
        BLOWER = 1,
        HIGH_RANGE = 2,
        CIRCULATION = 3,
        REST_MODE = 4,
        HEAT_STATE = 5,
        CURRENT_TEMP = 6,
        TARGET_TEMP = 7,
        SPA_TEMP_SCALE = 8,
        TIME_SINCE_LAST_STATUS = 9,
      };

    public:
      BalboaSpaSensors() {};
      void update() override;
      void update_traits() override {};

      void set_parent(BalboaSpa *parent);
      void set_sensor_type(BalboaSpaSensorType _type) { sensor_type = _type; }

    private:
      BalboaSpaSensorType sensor_type;
    };

  } // namespace balboa_spa
} // namespace esphome
