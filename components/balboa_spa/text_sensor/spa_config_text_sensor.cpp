#include "spa_config_text_sensor.h"
#include <cstring>

namespace esphome
{
    namespace balboa_spa
    {

        void SpaConfigTextSensor::set_parent(BalboaSpa *parent)
        {
            spa_ = parent;
            parent->register_listener([this]()
                                      { this->update(); });
        }

        void SpaConfigTextSensor::update()
        {
            // Nothing to report until the spa has answered the config request.
            if (!spa_->has_config())
                return;

            ControlConfig2Response config = spa_->get_current_config();
            if (published_ && memcmp(&config, &last_config_, sizeof(config)) == 0)
                return;

            char buf[160];
            snprintf(buf, sizeof(buf),
                     "{\"pump1\":%u,\"pump2\":%u,\"pump3\":%u,\"pump4\":%u,\"pump5\":%u,\"pump6\":%u,"
                     "\"light1\":%u,\"light2\":%u,\"circ\":%u,\"blower\":%u,\"mister\":%u,"
                     "\"aux1\":%u,\"aux2\":%u}",
                     (unsigned)config.pump1, (unsigned)config.pump2, (unsigned)config.pump3,
                     (unsigned)config.pump4, (unsigned)config.pump5, (unsigned)config.pump6,
                     (unsigned)config.light1, (unsigned)config.light2, (unsigned)config.circ,
                     (unsigned)config.blower, (unsigned)config.mister,
                     (unsigned)config.aux1, (unsigned)config.aux2);

            this->publish_state(buf);
            last_config_ = config;
            published_ = true;
        }

    } // namespace balboa_spa
} // namespace esphome
