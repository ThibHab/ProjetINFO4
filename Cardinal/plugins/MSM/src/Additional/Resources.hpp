#pragma once

// 2 PI
#define TWO_PI (6.28318530717958647692528676655900576)

// Round
#define ROUND(f) ((float) ((f > 0.0) ? floor(f+0.5) : ceil(f-0.5)))

// Buffer sizes
#define HISTORY_SIZE_A (1<<21)
#define HISTORY_SIZE_B (1<<21)

// Divisions
#define DIVISIONS 25

// SampleRate
#define SR (APP->engine->getSampleRate())

// Saturate X
#define _limitX 15.0f

// Saturate
#define _limit 10.0f

// Saturate2
#define _limit2 5.0f

// Threshold for Softclip
#define softClipThreshold (2.0f / 3.0f)
