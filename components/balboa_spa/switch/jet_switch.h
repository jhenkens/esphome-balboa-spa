#pragma once

#include "jet_switch_base.h"

namespace esphome
{
  namespace balboa_spa
  {

    class JetSwitch : public JetSwitchBase
    {
    public:
      explicit JetSwitch(uint8_t index);

    protected:
      uint8_t get_jet_state(const SpaState *spaState) override;
      void toggle_jet(std::function<void()> on_sent) override;

    private:
      uint8_t index_;
    };

  } // namespace balboa_spa
} // namespace esphome
