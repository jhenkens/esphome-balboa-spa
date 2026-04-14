#pragma once

#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"
#include "../balboaspa.h"
#include "../spa_temperature_base.h"

namespace esphome
{
  namespace balboa_spa
  {
    class BalboaSpaThermostat : public climate::Climate, public Component, public SpaTemperatureBase
    {
    public:
      BalboaSpaThermostat() {};

      void update() override;
      void set_parent(BalboaSpa *parent);
      void update_traits() override;

    protected:
      void control(const climate::ClimateCall &call) override;
      climate::ClimateTraits traits() override;
      climate::ClimateTraits traits_ = climate::ClimateTraits();
    };

  } // namespace balboa_spa
} // namespace esphome
