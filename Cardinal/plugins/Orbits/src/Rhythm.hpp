#pragma once

#include <cstdint>
#include <bitset>

namespace rhythm {

static const auto max_length = 32u;

typedef std::bitset<max_length> Rhythm;

Rhythm rhythm(uint32_t length, uint32_t on_beats);
Rhythm rhythm(uint32_t length, uint32_t on_beats, uint32_t shift);
bool beat(uint32_t length, uint32_t on_beats, uint32_t shift, uint32_t num, bool invert=false);
Rhythm rotate(Rhythm val, uint32_t size, uint32_t amount);

uint32_t numNearEvenRhythms(uint32_t length, uint32_t density);
Rhythm nearEvenRhythm(uint32_t length, uint32_t density, uint32_t variation);
bool nearEvenRhythmBeat(uint32_t length, uint32_t on_beats, uint32_t variation, uint32_t shift, uint32_t num);

}
