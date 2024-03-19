// INCLUDES
#include "rack.hpp"
#include "asset.hpp"

#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <cstdint>
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include <random>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

#include "Additional/DSP.hpp"
#include "Additional/Linear_Interpolator.hpp"
#include "Additional/samplerate.h"

using namespace rack;

extern Plugin *pluginInstance;

extern Model *modelVCO;
extern Model *modelBVCO;
extern Model *modelExperimentalVCO;
extern Model *modelNoise;

extern Model *modelLFO;
extern Model *modelVCA;
extern Model *modelADSR;
extern Model *modelDelay;
extern Model *modelWaveShaper;
extern Model *modelWavefolder;
extern Model *modelBitcrusher;
extern Model *modelPhaserModule;
extern Model *modelMorpher;
extern Model *modelRingMod;
extern Model *modelRandomSource;
extern Model *modelMult;
extern Model *modelCrazyMult;
extern Model *modelFade;
extern Model *modelSimpleSlider;
extern Model *modelxseq;
extern Model *modelBlankPanel;

#ifdef USING_CARDINAL_NOT_RACK
#define THEME_DEFAULT (settings::preferDarkPanels ? 1 : 0)
#else
#define THEME_DEFAULT 0
#endif
