#pragma once
#include "Resources.hpp"

//----------------------------------------------------------------------------

inline double fastSin(float x)	{
	if(M_PI < x) {
		x = x-static_cast<int>((x+M_PI)/(TWO_PI))*TWO_PI;
	}
	else if(x < -M_PI) {
		x = x-static_cast<int>((x-M_PI)/(TWO_PI))*TWO_PI;
	}
 
	return x*(1 - x*x*(0.16666667f - x*x*(0.00833333f - x*x*(0.0001984f - x*x*0.0000027f))));
}

inline float fastSin2(float x)	{
	if(M_PI < x) {
		x = x-static_cast<int>((x+M_PI)/(TWO_PI))*TWO_PI;
	}
	else if(x < -M_PI) {
		x = x-static_cast<int>((x-M_PI)/(TWO_PI))*TWO_PI;
	}

	return x*(1 - x*x*(0.16666667f - x*x*(0.00833333f - x*x*(0.0001984f - x*x*0.0000027f))));
}

inline float tanh_noclip(float x) { // no clip
  return x * ( 27 + x * x ) / ( 27 + 9 * x * x );
}

// different clipping algos 
static inline float saturateX( float input ) {
	float x1 = fabsf( input + _limitX );
	float x2 = fabsf( input - _limitX );
	return 0.5f * (x1 - x2);
}

static inline float saturate( float input ) {
	float x1 = fabsf( input + _limit );
	float x2 = fabsf( input - _limit );
	return 0.5f * (x1 - x2);
}

static inline float saturate2( float input ) {
    float x1 = fabsf( input + _limit2 );
    float x2 = fabsf( input - _limit2 );
    return 0.5f * (x1 - x2);
}

inline float fastatan( float x )
{
    return (x / (0.05 + 0.2 * (x * x)));
}

inline float overdrive(float inSample, float dr) {
	const float k = 2.f * dr / (1.f - dr);
	return (1.f + k) * inSample / (1.f + k * fabs(inSample));
}

static inline float softClip(float sample) {
    if (sample < -1.f) {
        return -softClipThreshold;
    }
    else if (sample > 1.f) {
        return softClipThreshold;
    }
    else {
        return sample - ((sample * sample * sample) / 3.f);
    }
}

static inline float arctangent(float sample, float drive)
{
    return (2.f / M_PI)* atan(drive * sample);
}

static inline float hardClip(float sample) {
    if (sample < -5.f) {
        return -5.f;
    }
    else if (sample > 5.f) {
        return 5.f;
    }
    else {
        return sample;
    }
}

static inline float cubicShaper(float sample)
{
    return 1.5f * sample - 0.5f * sample * sample * sample;
}

static inline float foldback(float sample, float threshold) { // threshold -> 0-1
    if (sample > threshold || sample < -threshold) {
        sample = fabs(fabs(fmod(sample - threshold, threshold * 4)) - threshold * 2) - threshold;
    }
    return sample;
}

//----------------------------------------------------------------------------


static inline float Limit(float input, float min, float max) {
	int above = (input > min);
	int notAbove = above != 1;
	input = above * input + notAbove * min;

	int below = (input < max);
	int notBelow = below != 1;
	input = below * input + notBelow * max;

	return input;
}

static inline void ZeroBuffer(float* buffer, int len) {
	for (int i = 0; i < len; i++)
		buffer[i] = 0.0f;
}