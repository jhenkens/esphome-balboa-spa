#pragma once
#include "esphome/components/text_sensor/text_sensor.h"
#include "../balboaspa.h"

namespace esphome
{
    namespace balboa_spa
    {

        // Publishes the hardware configuration the spa reports about itself
        // (pumps, lights, circulation pump, blower, mister, aux outputs) as JSON.
        // The values are the raw fields from the configuration response.
        class SpaConfigTextSensor : public text_sensor::TextSensor
        {
        public:
            void set_parent(BalboaSpa *parent);
            void update();

        private:
            BalboaSpa *spa_ = nullptr;
            ControlConfig2Response last_config_;
            bool published_ = false;
        };

    } // namespace balboa_spa
} // namespace esphome
