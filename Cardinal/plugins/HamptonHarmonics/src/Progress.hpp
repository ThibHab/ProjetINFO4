#pragma once

#include "plugin.hpp"

extern rack::Model* modelProgress;

struct Progress : rack::Module {
  enum ParamIds {
		STEPS_PARAM,
		ENUMS(ROOT_PARAM, 8),
		ENUMS(TYPE_PARAM, 8),
		ENUMS(INVERSION_PARAM, 8),
		ENUMS(LENGTH_PARAM, 8),
		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_INPUT,
		RESET_INPUT,
		STEPS_CV_INPUT,
		ENUMS(LENGTH_CV_INPUT, 8),
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(OUT_OUTPUT, 4),
		POLY_OUTPUT,
		GATE_OUTPUT,
		EOC_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	int _roots[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int _chordTypes[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int _inversions[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	rack::dsp::SchmittTrigger _clockTrigger;
	rack::dsp::SchmittTrigger _resetTrigger;
	int _currentStep = 0;
	int _prevStep = -1;
	int _subStep = 0;
	float _gateLength = 0.999f;
	float _sampleTime = 0.001f;
	float _timeSinceLastClock = 0.0f;
	float _clockTime = 0.1f;
	rack::dsp::PulseGenerator _gateGenerator;
	rack::dsp::PulseGenerator _eocGenerator;

	// Pitches
	float _referenceFrequency = 261.626; // C4; frequency at which Rack 1v/octave CVs are zero.
	float _referenceSemitone = 60.0; // C4; value of C4 in semitones is arbitrary here, so have it match midi note numbers when rounded to integer.
	float _twelfthRootTwo = 1.0594630943592953;
	float _logTwelfthRootTwo = logf(1.0594630943592953);
	int _referencePitch = 0;
	int _referenceOctave = 4;

	Progress();

  void setType(int index, int input, int* threeInterval, int* fiveInterval, int* sevenInterval);
  void setCurrentStep(int step);
  int getNextNonZeroLengthStep(int numSteps, int nextStep);
	bool isChordTypeTriad(int chordType);
  bool isLastSubStep();

  float frequencyToSemitone(float frequency);
  float semitoneToFrequency(float semitone);
  float frequencyToCV(float frequency);
  float cvToFrequency(float cv);

  void resetSequence();

	void onSampleRateChange() override;
  void process(const ProcessArgs& args) override;
};

struct ProgressWidget : rack::ModuleWidget {
  ProgressWidget(Progress* module);

  struct ChordDisplayWidget : rack::TransparentWidget {
		Progress* _module;
		int _index;
		std::string _fontPath = "res/PixelOperator.ttf";
		char _text[9];
		char _inv[3];
		NVGcolor _backgroundColor = nvgRGB(0x00, 0x01, 0x0e);
		NVGcolor _borderColor = nvgRGB(0xda, 0xda, 0xda);
		NVGcolor _textColor = nvgRGB(0xff, 0xff, 0x00);
		NVGcolor _highlightColor = nvgRGB(0xff, 0x00, 0x00);

		ChordDisplayWidget(rack::Vec pos, rack::Vec size, Progress* module, int index);

		void chordName();
		void draw(const DrawArgs &args) override;
		void drawLayer(const DrawArgs &args, int layer) override;
  };
};
