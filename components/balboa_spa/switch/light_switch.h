#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "../balboaspa.h"

namespace esphome
{
  namespace balboa_spa
  {

    class LightSwitch : public switch_::Switch
    {
    public:
      explicit LightSwitch(uint8_t index);
      void update();
      void set_parent(BalboaSpa *parent);

    protected:
      void write_state(bool state) override;

    private:
      BalboaSpa *spa_;
      uint8_t index_;
    };

  } // namespace balboa_spa
} // namespace esphome
