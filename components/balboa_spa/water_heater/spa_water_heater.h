#pragma once

#include "esphome/components/water_heater/water_heater.h"
#include "../balboaspa.h"
#include "../spa_temperature_base.h"

namespace esphome
{
  namespace balboa_spa
  {

    // Modes:
    //   OFF         → rest_mode=REST (sleep/rest, energy-saving standby)
    //   ECO         → rest_mode=READY, highrange=0 (ready, standard temp range)
    //   ELECTRIC → rest_mode=READY, highrange=1 (ready, high temp range)

    class BalboaSpaWaterHeater : public water_heater::WaterHeater, public SpaTemperatureBase
    {
    public:
      BalboaSpaWaterHeater() {};

      void update() override;
      void set_parent(BalboaSpa *parent);
      void update_traits() override;

      water_heater::WaterHeaterCallInternal make_call() override
      {
        return water_heater::WaterHeaterCallInternal(this);
      }

    protected:
      void control(const water_heater::WaterHeaterCall &call) override;
      water_heater::WaterHeaterTraits traits() override;
      water_heater::WaterHeaterTraits traits_ = water_heater::WaterHeaterTraits();
    };

  } // namespace balboa_spa
} // namespace esphome
