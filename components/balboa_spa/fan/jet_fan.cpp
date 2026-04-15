#include "jet_fan.h"
#include <cassert>
#include <iterator>

namespace esphome
{
    namespace balboa_spa
    {
        static constexpr const char *JET_TAGS[] = {"BalboaSpa.JetFan1", "BalboaSpa.JetFan2", "BalboaSpa.JetFan3", "BalboaSpa.JetFan4"};
        static constexpr const char *JET_NAMES[] = {"jet1", "jet2", "jet3", "jet4"};
        static_assert(std::size(JET_TAGS) == SpaState::JET_COUNT, "JET_TAGS size must be SpaState::JET_COUNT");
        static_assert(std::size(JET_NAMES) == SpaState::JET_COUNT, "JET_NAMES size must be SpaState::JET_COUNT");

        JetFan::JetFan(uint8_t index)
            : JetFanBase(JET_TAGS[index - 1], JET_NAMES[index - 1]), index_(index)
        {
            assert(index >= 1 && index <= SpaState::JET_COUNT);
        }

        uint8_t JetFan::get_jet_state(const SpaState *spaState)
        {
            return spaState->jets[index_ - 1];
        }

        void JetFan::toggle_jet(uint8_t expected_state, uint8_t max_retries)
        {
            spa_->toggle_jet(index_, expected_state, max_retries);
        }

    } // namespace balboa_spa
} // namespace esphome
