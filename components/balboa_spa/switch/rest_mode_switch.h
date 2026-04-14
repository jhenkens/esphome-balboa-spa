#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "../balboaspa.h"

namespace esphome
{
  namespace balboa_spa
  {

    class RestModeSwitch : public switch_::Switch
    {
    public:
      RestModeSwitch() {};
      void update();
      void set_parent(BalboaSpa *parent);

    protected:
      void write_state(bool state) override;

    private:
      BalboaSpa *spa_;
    };

  } // namespace balboa_spa
} // namespace esphome
