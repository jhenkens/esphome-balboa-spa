#pragma once

#include "esphome/components/number/number.h"
#include "esphome/core/component.h"
#include "../balboaspa.h"
#include "../spa_temperature_base.h"

namespace esphome
{
    namespace balboa_spa
    {

        class SpaTargetTempNumber : public number::Number, public Component, public SpaTemperatureBase
        {
        public:
            void set_parent(BalboaSpa *parent);
            void update() override;
            void update_traits() override;

        protected:
            void control(float value) override;
        };

    } // namespace balboa_spa
} // namespace esphome
