#pragma once

#include "plugin.hpp"

struct Arp : rack::Module {
	enum ParamIds {
		PATTERN_PARAM,
		GATE_PARAM,
		OCTAVE_PARAM,
		LATCH_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		PATTERN_CV_INPUT,
		GATE_CV_INPUT,
		OCTAVE_CV_INPUT,
		RESET_INPUT,
		CLOCK_INPUT,
		PITCH_INPUT,
		GATE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		GATE_OUTPUT,
		PITCH_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

  rack::dsp::SchmittTrigger _clockTrigger;
	rack::dsp::SchmittTrigger _resetTrigger;
	int _sequencePattern = 1;
	int _octaves = 1;
	bool _isPolyOutput = false;
	bool _shouldLatch = false;
  bool _isNewGate = true;
	std::vector<float> _latchPitches;
	int _step = 0;
	int _maxSteps = 0;
	int _polyChannel = 0;
	float _pitchOut = 0.0f;
	float _gateLength = 0.5f;
	float _sampleTime = 0.001f;
	float _timeSinceLastClock = 0.0f;
	float _clockTime = 0.1f;
	rack::dsp::PulseGenerator _gateGenerator;


	Arp();

  std::vector<float> getOctavePitches(std::vector<float> pitches);
	float getUpPatternPitch(std::vector<float> pitches);
	float getDownPatternPitch(std::vector<float> pitches);
	float getInclusivePatternPitch(std::vector<float> pitches);
	float getExclusivePatternPitch(std::vector<float> pitches);
	float getOutsideInPatternPitch(std::vector<float> pitches);
	float getOrderPatternPitch(std::vector<float> pitches);
	float getRandomPatternPitch(std::vector<float> pitches);
	void setPitchOut(std::vector<float> pitches);
	void setStep(int nextStep, int numberOfPitches);
	void setPolyChannel(int nextPolyChannel);

  void onSampleRateChange() override;
	void process(const ProcessArgs& args) override;
};


struct ArpWidget : rack::ModuleWidget {
	struct PolyOutputModeItem : rack::MenuItem {
		Arp *module;
		void onAction(const rack::event::Action &e) override;
	};

	void appendContextMenu(rack::Menu *menu) override;
	ArpWidget(Arp* module);
};
