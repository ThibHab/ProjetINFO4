//
//
// Waveforms
//
//

#pragma once

static float Sine(float pha) {
	static float value;

	value = fastSin(TWO_PI * pha);

	return value;
}
