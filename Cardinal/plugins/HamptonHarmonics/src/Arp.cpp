#include "Arp.hpp"

Arp::Arp() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configInput(PITCH_INPUT, "1V/oct pitch (poly)");
	configInput(GATE_INPUT, "Gate (poly)");
	configInput(CLOCK_INPUT, "Clock");
	configInput(RESET_INPUT, "Reset");
	configInput(GATE_CV_INPUT, "Gate CV");
	configInput(OCTAVE_CV_INPUT, "Octave CV");
	configInput(PATTERN_CV_INPUT, "Pattern CV");

	configOutput(PITCH_OUTPUT, "1V/oct pitch");
	configOutput(GATE_OUTPUT, "Gate");

	configParam(PATTERN_PARAM, 1.f, 7.f, 1.f, "Pattern"); // 7 options, initialize at 1
	configParam(GATE_PARAM, 0.f, 1.f, 0.5f, "Gate length");
	configParam(OCTAVE_PARAM, 1.f, 4.f, 1.f, "Octave");
	configSwitch(LATCH_PARAM, 0, 1, 0, "Latch/Hold", {"Off", "On"});

	configBypass(PITCH_INPUT, PITCH_OUTPUT);
	configBypass(GATE_INPUT, GATE_OUTPUT);
};

float Arp::getUpPatternPitch(std::vector<float> pitches) {
	std::sort(pitches.begin(), pitches.end());
	return pitches.at(this->_step);
}

float Arp::getDownPatternPitch(std::vector<float> pitches) {
	std::sort(pitches.begin(), pitches.end(), std::greater<float>());
	return pitches.at(this->_step);
}

float Arp::getInclusivePatternPitch(std::vector<float> pitches) {
	std::vector<float> downPitches = pitches;
	std::sort(pitches.begin(), pitches.end());
	std::sort(downPitches.begin(), downPitches.end(), std::greater<float>());
	std::vector<float> combinedPitches;
	combinedPitches.reserve(pitches.size() + downPitches.size());
	combinedPitches.insert(combinedPitches.end(), pitches.begin(), pitches.end());
	combinedPitches.insert(combinedPitches.end(), downPitches.begin(), downPitches.end());
	return combinedPitches.at(this->_step);
}

float Arp::getExclusivePatternPitch(std::vector<float> pitches) {
	std::vector<float> downPitches = pitches;
	std::sort(pitches.begin(), pitches.end());
	std::sort(downPitches.begin(), downPitches.end(), std::greater<float>());
	std::vector<float> combinedPitches;
	combinedPitches.reserve(pitches.size() + downPitches.size());
	combinedPitches.insert(combinedPitches.end(), pitches.begin(), pitches.end() - 1);
	combinedPitches.insert(combinedPitches.end(), downPitches.begin(), downPitches.end());
	return combinedPitches.at(this->_step);
}

float Arp::getOutsideInPatternPitch(std::vector<float> pitches) {
	std::vector<float> downPitches = pitches;
	std::sort(pitches.begin(), pitches.end());
	std::sort(downPitches.begin(), downPitches.end(), std::greater<float>());
	std::vector<float> outsideInPitches;
	int length = pitches.size() / 2 + 1; // the plus one helps odd numbers, but also isn't reached in even numbers
	for (int i = 0; i < length; i++) {
		outsideInPitches.push_back(pitches.at(i));
		outsideInPitches.push_back(downPitches.at(i));
	}
	return outsideInPitches.at(this->_step);
}

float Arp::getOrderPatternPitch(std::vector<float> pitches) {
	return pitches.at(this->_step);
}

float Arp::getRandomPatternPitch(std::vector<float> pitches) {
	int randomStep = random::u32() % this->_maxSteps;
	return pitches.at(randomStep);
}

void Arp::setPitchOut(std::vector<float> pitches) {
	switch (this->_sequencePattern) {
		case 1: {
			this->_pitchOut = getUpPatternPitch(pitches);
			break;
		}
		case 2: {
			this->_pitchOut = getDownPatternPitch(pitches);
			break;
		}
		case 3: {
			this->_pitchOut = getInclusivePatternPitch(pitches);
			break;
		}
		case 4: {
			this->_pitchOut = getExclusivePatternPitch(pitches);
			break;
		}
		case 5: {
			this->_pitchOut = getOutsideInPatternPitch(pitches);
			break;
		}
		case 6: {
			this->_pitchOut = getOrderPatternPitch(pitches);
			break;
		}
		case 7: {
			this->_pitchOut = getRandomPatternPitch(pitches);
			break;
		}
	}
}

std::vector<float> Arp::getOctavePitches(std::vector<float> pitches) {
	std::vector<float> result;
	int numberOfPitches = pitches.size();
	for (int i = 0; i < this->_octaves; i++) {
		for (int j = 0; j < numberOfPitches; j++) {
			result.push_back(pitches.at(j) + i);
		}
	}
	return result;
}

void Arp::setStep(int nextStep, int numberOfPitches) {
	this->_step = nextStep;
	this->_maxSteps = numberOfPitches;

	// Change maxSteps based on octaves
	this->_maxSteps *= this->_octaves;

	// Change maxSteps based on sequence pattern
	if (this->_sequencePattern == 3) { // inclusive
		this->_maxSteps *= 2; // double the max steps since we're going up and back down
	}
	else if (this->_sequencePattern == 4) { // exclusive
		this->_maxSteps += this->_maxSteps - 2; // double the max steps, but subtract 2 since we're not doubling the top and bottom
	}

	if (this->_step >= this->_maxSteps) {
		this->_step = 0;
	}
}

void Arp::setPolyChannel(int nextPolyChannel) {
	this->_polyChannel = nextPolyChannel;

	if (this->_polyChannel >= 16) {
		this->_polyChannel = 0;
	}
}

void Arp::onSampleRateChange() {
	this->_sampleTime = APP->engine->getSampleTime();
}

void Arp::process(const ProcessArgs& args) {
	// Set params
	this->_sequencePattern = (int) clamp(roundf(params[PATTERN_PARAM].getValue() + inputs[PATTERN_CV_INPUT].value), 1.0f, 7.0f);
	this->_octaves = (int) clamp(roundf(params[OCTAVE_PARAM].getValue() + inputs[OCTAVE_CV_INPUT].value), 1.0f, 4.0f);
	this->_shouldLatch = params[LATCH_PARAM].getValue() > 0.5f;
	this->_gateLength = clamp(params[GATE_PARAM].getValue() + inputs[GATE_CV_INPUT].value, 0.0f, 1.0f);

	// Get pitches from input
	std::vector<float> pitches;
	int numberOfPitches = 0;
	bool hasHighGate = false;
	if (inputs[PITCH_INPUT].isConnected()) {
		int numberOfChannels = inputs[PITCH_INPUT].getChannels();
		for (int c = 0; c < numberOfChannels; c++) {
			if (inputs[GATE_INPUT].getPolyVoltage(c) > 0.0f) {
				pitches.push_back(inputs[PITCH_INPUT].getPolyVoltage(c));
				hasHighGate = true;
			}
		}
		numberOfPitches = pitches.size();
	}

	// Handle latching
	if (this->_shouldLatch) {
		if (this->_isNewGate && hasHighGate) {
			this->_latchPitches.clear();
		}
		int numberOfLatchedPitches = this->_latchPitches.size();
		// Save pitches when we are receiving new pitches
		if (numberOfLatchedPitches < numberOfPitches) {
			this->_latchPitches = pitches;
		}
		// Use saved pitches if the saved pitches have more notes than what is currently played
		else if (numberOfLatchedPitches > numberOfPitches) {
			pitches = this->_latchPitches;
			numberOfPitches = pitches.size();
		}
	}

	// Reset isNewGate
	this->_isNewGate = false;
	if (!hasHighGate) {
		this->_isNewGate = true;
	}

	// Keep track of the time between each clock trigger so we can use it for gate length
	bool isClockHigh = false;
	if (inputs[CLOCK_INPUT].isConnected()) {
		isClockHigh = _clockTrigger.process(inputs[CLOCK_INPUT].getVoltage());
		if (isClockHigh) {
			if (this->_timeSinceLastClock > 0.0f) {
				this->_clockTime = this->_timeSinceLastClock;
			}
			this->_timeSinceLastClock = 0.0f;
		}
		this->_timeSinceLastClock += this->_sampleTime;
	}

	// Step if clock is high and we have pitches to step through
	if (isClockHigh) {
		setStep(this->_step + 1, numberOfPitches);
		setPolyChannel(this->_polyChannel + 1);

		// handle gate length
		_gateGenerator.reset();
		float gl = this->_gateLength * this->_clockTime;
		_gateGenerator.trigger(std::max(0.001f, gl));	
	}

	// Reset
	if (this->_resetTrigger.process(inputs[RESET_INPUT].getVoltage())) {
		setStep(0, numberOfPitches);
		setPolyChannel(0);
	}

	// Handle octaves
	pitches = getOctavePitches(pitches);

	// Set the pitch out
	if (isClockHigh && numberOfPitches > 0) {
		setPitchOut(pitches);
	}

	if (this->_isPolyOutput) {
		outputs[PITCH_OUTPUT].setChannels(16);
		outputs[GATE_OUTPUT].setChannels(16);

		outputs[PITCH_OUTPUT].setVoltage(this->_pitchOut, this->_polyChannel);
		outputs[GATE_OUTPUT].setVoltage(hasHighGate || this->_shouldLatch ? this->_gateGenerator.process(this->_sampleTime) * 5.0f : 0.0f, this->_polyChannel);
	} else {
		outputs[PITCH_OUTPUT].setChannels(1);
		outputs[GATE_OUTPUT].setChannels(1);
		outputs[PITCH_OUTPUT].setVoltage(this->_pitchOut);
		outputs[GATE_OUTPUT].setVoltage(hasHighGate || this->_shouldLatch ? this->_gateGenerator.process(this->_sampleTime) * 5.0f : 0.0f);
	}


}

void ArpWidget::PolyOutputModeItem::onAction(const rack::event::Action &e) {
	module->_isPolyOutput = !module->_isPolyOutput;
}

void ArpWidget::appendContextMenu(rack::Menu *menu) {
	Arp *module = dynamic_cast<Arp*>(this->module);
	assert(module);

	menu->addChild(new rack::MenuSeparator);
	rack::MenuLabel *outputModeLabel = new rack::MenuLabel;
	outputModeLabel->text = "Output Mode";
	menu->addChild(outputModeLabel);

	PolyOutputModeItem *polyOutputModeItem = createMenuItem<PolyOutputModeItem>("Polyphonic output", CHECKMARK(module->_isPolyOutput));
	polyOutputModeItem->module = module;
	menu->addChild(polyOutputModeItem);
}

ArpWidget::ArpWidget(Arp* module) {
	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Arp.svg")));

	addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	addParam(createParamCentered<DaviesMaroonLargeSnap>(mm2px(Vec(25.371, 26.239)), module, Arp::PATTERN_PARAM));
	addParam(createParamCentered<DaviesGrayMedium>(mm2px(Vec(10.177, 58.39)), module, Arp::GATE_PARAM));
	addParam(createParamCentered<DaviesGrayMediumSnap>(mm2px(Vec(40.64, 58.39)), module, Arp::OCTAVE_PARAM));
	addParam(createParam<CKSS>(mm2px(Vec(27.57, 72.88)), module, Arp::LATCH_PARAM));

	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.371, 39.67)), module, Arp::PATTERN_CV_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.371, 52.01)), module, Arp::GATE_CV_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.371, 64.70)), module, Arp::OCTAVE_CV_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.351, 96)), module, Arp::RESET_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.772, 96)), module, Arp::CLOCK_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.772, 111)), module, Arp::PITCH_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.351, 111)), module, Arp::GATE_INPUT));

	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(39.398, 96)), module, Arp::GATE_OUTPUT));
	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(39.398, 111)), module, Arp::PITCH_OUTPUT));
};

Model* modelArp = createModel<Arp, ArpWidget>("Arp");
