#pragma once
#include "esphome/components/text_sensor/text_sensor.h"
#include "../balboaspa.h"

namespace esphome
{
  namespace balboa_spa
  {

    class ReminderTextSensor : public text_sensor::TextSensor
    {
    public:
      void set_parent(BalboaSpa *parent);
      void update();

    private:
      BalboaSpa *spa_ = nullptr;
      ReminderType last_reminder_ = ReminderType::UNKNOWN;
    };

  } // namespace balboa_spa
} // namespace esphome
