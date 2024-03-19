/*
 * DISTRHO Cardinal Plugin
 * Copyright (C) 2021-2022 Filipe Coelho <falktx@falktx.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * For a full copy of the GNU General Public License see the LICENSE file.
 */

#pragma once

#include "rack.hpp"
#include "engine/TerminalModule.hpp"

#ifdef NDEBUG
# undef DEBUG
#endif

using namespace rack;

extern Plugin* pluginInstance;

extern Model* modelAidaX;
extern Model* modelAudioFile;
extern Model* modelAudioToCVPitch;
extern Model* modelCarla;
extern Model* modelCardinalBlank;
extern Model* modelExpanderInputMIDI;
extern Model* modelExpanderOutputMIDI;
extern Model* modelGlBars;
extern Model* modelHostAudio2;
extern Model* modelHostAudio8;
extern Model* modelHostCV;
extern Model* modelHostMIDI;
extern Model* modelHostMIDICC;
extern Model* modelHostMIDIGate;
extern Model* modelHostMIDIMap;
extern Model* modelHostParameters;
extern Model* modelHostParametersMap;
extern Model* modelHostTime;
extern Model* modelIldaeil;
extern Model* modelMPV;
extern Model* modelSassyScope;
extern Model* modelTextEditor;

extern std::vector<Model*> hostTerminalModels;

/*
 * Find the highest absolute and normalized value within a float array.
 */
template<std::size_t size>
static inline
float d_findMaxNormalizedFloat(const float floats[size])
{
    static constexpr const float kEmptyFloats[size] = {};

    if (std::memcmp(floats, kEmptyFloats, sizeof(float)*size) == 0)
        return 0.f;

    float tmp, maxf2 = std::abs(floats[0]);

    for (std::size_t i=1; i<size; ++i)
    {
        if (!std::isfinite(floats[i]))
            __builtin_unreachable();

        tmp = std::abs(floats[i]);

        if (tmp > maxf2)
            maxf2 = tmp;
    }

    if (maxf2 > 1.f)
        maxf2 = 1.f;

    return maxf2;
}

/*
 * Find the highest absolute and normalized value within a float array.
 */
static inline
float d_findMaxNormalizedFloat128(const float floats[128])
{
    static constexpr const float kEmptyFloats[128] = {};

    if (std::memcmp(floats, kEmptyFloats, sizeof(float)*128) == 0)
        return 0.f;

    float tmp, maxf2 = std::abs(floats[0]);

    for (std::size_t i=1; i<128; ++i)
    {
        if (!std::isfinite(floats[i]))
            __builtin_unreachable();

        tmp = std::abs(floats[i]);

        if (tmp > maxf2)
            maxf2 = tmp;
    }

    if (maxf2 > 1.f)
        maxf2 = 1.f;

    return maxf2;
}
