# Architecture & What Changed

This component began as a fork of [Dakoriki/ESPHome-Balboa-Spa](https://github.com/Dakoriki/ESPHome-Balboa-Spa), which itself built on [MHotchin/BalBoaSpa](https://github.com/MHotchin/BalBoaSpa) and [ccutrer/balboa_worldwide_app](https://github.com/ccutrer/balboa_worldwide_app). It is not a minor tweak on top of that lineage — the codebase was substantially restructured and the command delivery model was replaced entirely.

This was AI-assisted, not "vibe coded": development leaned heavily on Claude, but was directed by an author who is a software engineer by trade and had prior experience writing ESPHome custom components. Every change below was a deliberate design decision, not something generated and accepted blind.

## Structural cleanup

- Unified per-jet and per-light component classes into single parameterised classes — one `JetSwitch(index)` instead of `JetSwitch1`, `JetSwitch2`, etc., eliminating a large amount of boilerplate
- Extracted a typed message layer for all wire protocol messages, replacing scattered raw byte construction. This layer was built on the protocol groundwork in [MHotchin/BalBoaSpa's `BalBoaMessages.h`](https://github.com/MHotchin/BalBoaSpa/blob/0e8edb21273203d5013aded61c0e72d4247d6a88/src/BalBoaMessages.h), combined with the protocol documentation in [ccutrer/balboa_worldwide_app](https://github.com/ccutrer/balboa_worldwide_app/blob/main/doc/protocol.md)
- Shared temperature base class between the `climate` and `water_heater` platforms
- Added a `number` platform for target temperature and a `rest_mode` switch
- Removed the `CircularBuffer` dependency in favour of a flat array

## Reliable command delivery

The biggest functional change: commands are now queued with an expected outcome rather than fire-and-forget. Each toggle or set-temp command carries the spa state field it expects to change. After every status update from the spa, satisfied commands are pruned from the queue — unsatisfied ones are retried with a 5-second backoff.

Properties of this approach:
- The queue is insertion-sorted so the earliest-available command is always at the head
- Per-component retry logic that previously lived in each switch/fan class is gone — the queue handles it centrally
- Wire bytes for the head command are pre-serialised whenever the queue changes, so CTS response is a buffer write with no message construction on the hot path
- Commands targeting a state that is already at the desired value are discarded immediately

The retry behaviour has drastically improved jet on/off reliability. It still takes a few seconds sometimes, but almost always completes within 15 seconds.

## Native Fahrenheit support

This component supports `unit_of_measurement: °F` on the `climate` platform and temperature sensors, bypassing ESPHome/Home Assistant conversion so temperature steps are exact (no rounding artifacts from Celsius conversion). This requires ESPHome 2026.08 or newer, and likely 2026.09 for Home Assistant.

- **Home Assistant** — [home-assistant/core#168747](https://github.com/home-assistant/core/pull/168747): adds native `unit_of_measurement` support to the ESPHome climate and water heater integrations, eliminating floating-point errors from unit conversions. Currently open, awaiting code owner approval.

## Related projects

- [Dakoriki/ESPHome-Balboa-Spa](https://github.com/Dakoriki/ESPHome-Balboa-Spa) — the original fork base
- [MHotchin/BalBoaSpa](https://github.com/MHotchin/BalBoaSpa) — source of the original protocol message definitions
- [ccutrer/balboa_worldwide_app](https://github.com/ccutrer/balboa_worldwide_app) — protocol documentation
- [brianfeucht/esphome-balboa-spa](https://github.com/brianfeucht/esphome-balboa-spa) — an actively maintained sibling project using simple on/off `switch` jet control; see the [hardware guide](HARDWARE.md) for a build based on it that uses the `fan` platform instead for more stable jet behavior
