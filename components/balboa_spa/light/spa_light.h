#pragma once

#include "esphome/components/light/light_output.h"
#include "../balboaspa.h"

namespace esphome
{
  namespace balboa_spa
  {

    class SpaLight : public light::LightOutput
    {
    public:
      light::LightTraits get_traits() override;
      void setup_state(light::LightState *state) override { state_ = state; }
      void write_state(light::LightState *state) override;
      void set_parent(BalboaSpa *parent);
      void set_index(uint8_t index) { index_ = index; }
      void update();

    private:
      BalboaSpa *spa_;
      light::LightState *state_{nullptr};
      bool last_state_{false};
      uint8_t index_{0};
    };

  } // namespace balboa_spa
} // namespace esphome
