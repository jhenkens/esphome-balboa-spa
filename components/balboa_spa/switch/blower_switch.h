#pragma once

#include "jet_switch_base.h"

namespace esphome
{
  namespace balboa_spa
  {

    class BlowerSwitch : public JetSwitchBase
    {
    public:
      BlowerSwitch() : JetSwitchBase("BalboaSpa.BlowerSwitch", "blower") {};

    protected:
      uint8_t get_jet_state(const SpaState *spaState) override { return spaState->blower; }
      void toggle_jet(uint8_t expected_state, uint8_t max_retries) override;
    };

  } // namespace balboa_spa
} // namespace esphome
