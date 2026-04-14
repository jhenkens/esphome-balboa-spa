#include "jet_switch.h"
#include <cassert>
#include <iterator>

namespace esphome
{
    namespace balboa_spa
    {
        static constexpr const char *JET_TAGS[] = {"BalboaSpa.JetSwitch1", "BalboaSpa.JetSwitch2", "BalboaSpa.JetSwitch3", "BalboaSpa.JetSwitch4"};
        static constexpr const char *JET_NAMES[] = {"jet1", "jet2", "jet3", "jet4"};
        static_assert(std::size(JET_TAGS) == SpaState::JET_COUNT, "JET_TAGS size must be SpaState::JET_COUNT");
        static_assert(std::size(JET_NAMES) == SpaState::JET_COUNT, "JET_NAMES size must be SpaState::JET_COUNT");

        JetSwitch::JetSwitch(uint8_t index)
            : JetSwitchBase(JET_TAGS[index - 1], JET_NAMES[index - 1]), index_(index)
        {
            assert(index >= 1 && index <= SpaState::JET_COUNT);
        }

        uint8_t JetSwitch::get_jet_state(const SpaState *spaState)
        {
            return spaState->jets[index_ - 1];
        }

        void JetSwitch::toggle_jet(std::function<void()> on_sent)
        {
            spa_->toggle_jet(index_, std::move(on_sent));
        }

    } // namespace balboa_spa
} // namespace esphome
