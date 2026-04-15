#pragma once

#include "jet_fan_base.h"

namespace esphome
{
    namespace balboa_spa
    {

        class JetFan : public JetFanBase
        {
        public:
            explicit JetFan(uint8_t index);

        protected:
            uint8_t get_jet_state(const SpaState *spaState) override;
            void toggle_jet(uint8_t expected_state, uint8_t max_retries) override;

        private:
            uint8_t index_;
        };

    } // namespace balboa_spa
} // namespace esphome
