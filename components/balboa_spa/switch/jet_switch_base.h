#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "../jet_toggle_component_base.h"

namespace esphome
{
    namespace balboa_spa
    {

        class JetSwitchBase : public switch_::Switch, public JetToggleComponentBase
        {
        public:
            JetSwitchBase(const char *tag, const char *jet_name)
                : JetToggleComponentBase(tag, jet_name) {};

            void update();  // sync ESPHome ← spa
            void set_parent(BalboaSpa *parent);
            void set_max_toggle_attempts(uint8_t value) { JetToggleComponentBase::set_max_toggle_attempts(value); }
            void set_on_level(uint8_t value) { on_level_ = value; }

        protected:
            void write_state(bool state) override;
            virtual uint8_t get_jet_state(const SpaState *spaState) = 0;
            virtual void toggle_jet(std::function<void()> on_sent) = 0;

        private:
            uint8_t current_switch_state_ = 0; // 0=OFF, >0=ON for switch
            uint8_t on_level_ = 1;         // spa state value that means ON (1=low, 2=high)
        };

    } // namespace balboa_spa
} // namespace esphome
