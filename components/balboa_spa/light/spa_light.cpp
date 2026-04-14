#include "spa_light.h"

namespace esphome
{
    namespace balboa_spa
    {
        light::LightTraits SpaLight::get_traits()
        {
            auto traits = light::LightTraits();
            traits.set_supported_color_modes({light::ColorMode::ON_OFF});
            return traits;
        }

        void SpaLight::write_state(light::LightState *state)
        {
            bool binary;
            state->current_values_as_binary(&binary);

            SpaState *spaState = spa_->get_current_state();
            if (spaState->lights[index_] != binary)
            {
                spa_->toggle_light(index_ + 1);
            }
        }

        void SpaLight::set_parent(BalboaSpa *parent)
        {
            spa_ = parent;
            parent->register_listener([this]() { this->update(); });
        }

        void SpaLight::update()
        {
            bool light_on = spa_->get_current_state()->lights[index_];
            if (this->last_state_ != light_on)
            {
                this->last_state_ = light_on;
                if (this->state_ != nullptr)
                {
                    auto call = this->state_->make_call();
                    call.set_state(light_on);
                    call.set_save(false);
                    call.perform();
                }
            }
        }

    } // namespace balboa_spa
} // namespace esphome
