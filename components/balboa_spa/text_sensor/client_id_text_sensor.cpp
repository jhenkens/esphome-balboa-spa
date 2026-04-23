#include "client_id_text_sensor.h"

namespace esphome
{
  namespace balboa_spa
  {

    void ClientIdTextSensor::set_parent(BalboaSpa *parent)
    {
      spa_ = parent;
      parent->register_client_id_listener([this]() { this->update(); });
    }

    void ClientIdTextSensor::update()
    {
      uint8_t id = spa_->get_client_id();
      if (id == 0)
      {
        this->publish_state("None");
      }
      else
      {
        char buf[5];
        snprintf(buf, sizeof(buf), "0x%02X", id);
        this->publish_state(buf);
      }
    }

  } // namespace balboa_spa
} // namespace esphome
