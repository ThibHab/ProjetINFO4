/*
 * Carla Native Plugins
 * Copyright (C) 2012-2022 Filipe Coelho <falktx@falktx.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * For a full copy of the GNU General Public License see the doc/GPL.txt file.
 */

// config fix
#include "distrho-3bandeq/DistrhoPluginInfo.h"

#if DISTRHO_PLUGIN_HAS_UI && ! defined(HAVE_DGL)
# undef DISTRHO_PLUGIN_HAS_UI
# define DISTRHO_PLUGIN_HAS_UI 0
#endif

#include "CarlaMathUtils.hpp"
#include "CarlaJuceUtils.hpp"

// Plugin Code
#include "distrho-3bandeq/DistrhoPlugin3BandEQ.cpp"
#if DISTRHO_PLUGIN_HAS_UI
#include "distrho-3bandeq/DistrhoArtwork3BandEQ.cpp"
#include "distrho-3bandeq/DistrhoUI3BandEQ.cpp"
#endif

// DISTRHO Code
#define DISTRHO_PLUGIN_TARGET_CARLA
#include "DistrhoPluginMain.cpp"
#if DISTRHO_PLUGIN_HAS_UI
#include "DistrhoUIMain.cpp"
#endif

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

static const NativePluginDescriptor _3bandeqDesc = {
    /* category  */ NATIVE_PLUGIN_CATEGORY_EQ,
#if DISTRHO_PLUGIN_HAS_UI
    /* hints     */ static_cast<NativePluginHints>(NATIVE_PLUGIN_IS_RTSAFE
                                                  |NATIVE_PLUGIN_HAS_UI
                                                  |NATIVE_PLUGIN_NEEDS_UI_MAIN_THREAD
                                                  |NATIVE_PLUGIN_USES_PARENT_ID),
#else
    /* hints     */ static_cast<NativePluginHints>(NATIVE_PLUGIN_IS_RTSAFE),
#endif
    /* supports  */ NATIVE_PLUGIN_SUPPORTS_NOTHING,
    /* audioIns  */ DISTRHO_PLUGIN_NUM_INPUTS,
    /* audioOuts */ DISTRHO_PLUGIN_NUM_OUTPUTS,
    /* midiIns   */ 0,
    /* midiOuts  */ 0,
    /* paramIns  */ DistrhoPlugin3BandEQ::paramCount,
    /* paramOuts */ 0,
    /* name      */ DISTRHO_PLUGIN_NAME,
    /* label     */ "3bandeq",
    /* maker     */ "falkTX, Michael Gruhn",
    /* copyright */ "LGPL",
    PluginDescriptorFILL(PluginCarla)
};

END_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

CARLA_API_EXPORT
void carla_register_native_plugin_distrho_3bandeq();

CARLA_API_EXPORT
void carla_register_native_plugin_distrho_3bandeq()
{
    USE_NAMESPACE_DISTRHO
    carla_register_native_plugin(&_3bandeqDesc);
}

// -----------------------------------------------------------------------
