#include "Progress.hpp"

Progress::Progress() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configInput(CLOCK_INPUT, "Clock");
	configInput(RESET_INPUT, "Reset");
	configInput(STEPS_CV_INPUT, "Steps CV");
	for (int i = 0; i < 8; i++) {
		configInput(LENGTH_CV_INPUT + i, string::f("Length CV %d", i + 1));
	}

	configOutput(POLY_OUTPUT, "1V/oct pitch (poly)");
	configOutput(GATE_OUTPUT, "Gate (poly)");
	configOutput(EOC_OUTPUT, "End of cycle");
	for (int i = 0; i < 4; i++) {
		configOutput(OUT_OUTPUT + i, string::f("1V/oct pitch %d", i + 1));
	}

	configParam(STEPS_PARAM, 1.f, 8.f, 8.f, "Steps");
	for (int i = 0; i < 8; i++) {
		configParam(ROOT_PARAM + i, 0.0, 83.0, 36.0, string::f("Step %d Root", i + 1));
		configParam(TYPE_PARAM + i, 0.f, 8.f, 0.f, string::f("Step %d Type", i + 1)); // MAJ, min, MAJ7, 7, min7, dim, aug, sus2, sus4
		configParam(INVERSION_PARAM + i, 0.f, 3.f, 0.f, string::f("Step %d Inversion", i + 1)); // Each of the four notes can be the bass note
		configParam(LENGTH_PARAM + i, 0.f, 16.f, 1.f, string::f("Step %d Length", i + 1)); // 16 steps worth, plus the option to skip.
	}
};

void Progress::setType(int index, int input, int* threeInterval, int* fiveInterval, int* sevenInterval) {
	switch (input) {
		case 0: {
			// Maj
			*threeInterval = 4;
			*fiveInterval = 7;
			*sevenInterval = 0;
			break;
		}
		case 1: {
			// Min
			*threeInterval = 3;
			*fiveInterval = 7;
			*sevenInterval = 0;
			break;
		}
		case 2: {
			// MAJ7
			*threeInterval = 4;
			*fiveInterval = 7;
			*sevenInterval = 11;
			break;
		}
		case 3: {
			// 7
			*threeInterval = 4;
			*fiveInterval = 7;
			*sevenInterval = 10;
			break;
		}
		case 4: {
			// min7
			*threeInterval = 3;
			*fiveInterval = 7;
			*sevenInterval = 10;
			break;
		}
		case 5: {
			// sus4
			*threeInterval = 5;
			*fiveInterval = 7;
			*sevenInterval = 0;
			break;
		}
		case 6: {
			// sus2
			*threeInterval = 2;
			*fiveInterval = 7;
			*sevenInterval = 0;
			break;
		}
		case 7: {
			// Diminished
			*threeInterval = 3;
			*fiveInterval = 6;
			*sevenInterval = 10;
			// *sevenInterval = 0; // Is this better than having a seventh?
			break;
		}
		case 8: {
			// Augmented
			*threeInterval = 4;
			*fiveInterval = 8;
			*sevenInterval = 12;
			// *sevenInterval = 0; // Is this better than having a seventh?
			break;
		}
	}
};

void Progress::setCurrentStep(int step) {
	int numSteps = (int) clamp(roundf(params[STEPS_PARAM].value + inputs[STEPS_CV_INPUT].value), 1.0f, 8.0f);
	int gateLength = (int) clamp(roundf(params[LENGTH_PARAM + this->_currentStep].value + inputs[LENGTH_CV_INPUT + this->_currentStep].value), 0.0f, 16.0f);
	this->_subStep++;
	if (this->_subStep < gateLength) {
		return;
	}

	this->_subStep = 0;
	// Make sure to skip the next step if the gate length is 0.
	this->_prevStep = this->_currentStep;
	this->_currentStep = getNextNonZeroLengthStep(numSteps, step);
};

int Progress::getNextNonZeroLengthStep(int numSteps, int nextStep) {
	for (int i = 0; i < numSteps; i++) {
		if (nextStep >= numSteps) {
			nextStep = 0;
		}

		int gateLength = (int) clamp(roundf(params[LENGTH_PARAM + nextStep].value + inputs[LENGTH_CV_INPUT + nextStep].value), 0.0f, 16.0f);
		if (gateLength == 0) {
			nextStep++;
		} else {
			break;
		}
	}

	if (nextStep >= numSteps) {
		nextStep = 0;
	}
	return nextStep;
};

bool Progress::isChordTypeTriad(int chordType) {
	// Maj (0), min (1), sus4 (5), sus2 (6) are all triads
	return chordType < 2 || chordType == 5 || chordType == 6;
}

bool Progress::isLastSubStep() {
	int gateLength = params[LENGTH_PARAM + this->_currentStep].value;
	return this->_subStep == gateLength - 1;
};

float Progress::frequencyToSemitone(float frequency) {
	return logf(frequency / this->_referenceFrequency) / this->_logTwelfthRootTwo + this->_referenceSemitone;
};

float Progress::semitoneToFrequency(float semitone) {
	return powf(this->_twelfthRootTwo, semitone - this->_referenceSemitone) * this->_referenceFrequency;
};

float Progress::frequencyToCV(float frequency) {
	return log2f(frequency / this->_referenceFrequency);
};

float Progress::cvToFrequency(float cv) {
	return powf(2.0, cv) * this->_referenceFrequency;
};

void Progress::resetSequence() {
	this->_currentStep = 0;
	this->_subStep = 0;
};

void Progress::onSampleRateChange() {
	this->_sampleTime = APP->engine->getSampleTime();
}

void Progress::process(const ProcessArgs& args) {
	bool isClockHigh = false;
	if (inputs[CLOCK_INPUT].isConnected()) {
		isClockHigh = this->_clockTrigger.process(inputs[CLOCK_INPUT].getVoltage());
		if (isClockHigh) {
			if (this->_timeSinceLastClock > 0.0f) {
				this->_clockTime = this->_timeSinceLastClock;
			}
			this->_timeSinceLastClock = 0.0f;
		}
		this->_timeSinceLastClock += this->_sampleTime;
	}

	if (isClockHigh) {
		setCurrentStep(this->_currentStep + 1);

		if (this->_currentStep < this->_prevStep && this->_subStep == 0) {
			this->_eocGenerator.trigger(1e-3f);
		}

		this->_gateGenerator.reset();
		this->_gateGenerator.trigger(this->_gateLength * this->_clockTime);
	}

	// Reset
	if (this->_resetTrigger.process(inputs[RESET_INPUT].getVoltage())) {
		resetSequence();
	}

	int inputRootPitch;
	int inputChordType;
	int inputInversion;
	int pitch;
	int octave;
	float rootCv;
	float thirdCv;
	float fifthCv;
	float seventhCv;
	bool isTriad = false;

	for (int i = 0; i < 8; i++) {
		// Set root pitch
		inputRootPitch = (int) params[ROOT_PARAM + i].value;
		pitch = (int) inputRootPitch % (int) 12;
		octave = int(inputRootPitch / 12);
		this->_roots[i] = pitch;

		// Set chord type
		inputChordType = (int) params[TYPE_PARAM + i].value;
		int threeInterval;
		int fiveInterval;
		int sevenInterval;
		setType(i, inputChordType, &threeInterval, &fiveInterval, &sevenInterval);
		this->_chordTypes[i] = inputChordType;

		// Set inversion
		inputInversion = (int) params[INVERSION_PARAM + i].value;
		switch (inputInversion) {
			case 0: {
				this->_inversions[i] = 0;
				break;
			}
			case 1: {
				this->_inversions[i] = threeInterval;
				break;
			}
			case 2: {
				this->_inversions[i] = fiveInterval;
				break;
			}
			case 3: {
				this->_inversions[i] = sevenInterval;
				break;
			}
		}

		if (i == this->_currentStep) {
			isTriad = isChordTypeTriad(inputChordType);
			int rootOctave = octave;
			if (inputInversion > 0) {
				rootOctave++;
			}
			float rootFrequency = semitoneToFrequency(this->_referenceSemitone + 12 * (rootOctave - this->_referenceOctave) + (pitch - this->_referencePitch));
			rootCv = frequencyToCV(rootFrequency);

			int thirdOctave = octave;
			if (inputInversion > 1) {
				thirdOctave++;
			}
			float thirdFrequency = semitoneToFrequency(this->_referenceSemitone + 12 * (thirdOctave - this->_referenceOctave) + (pitch + threeInterval - this->_referencePitch));
			thirdCv = frequencyToCV(thirdFrequency);

			int fifthOctave = octave;
			if (inputInversion > 2) {
				fifthOctave++;
			}
			float fifthFrequency = semitoneToFrequency(this->_referenceSemitone + 12 * (fifthOctave - this->_referenceOctave) + (pitch + fiveInterval - this->_referencePitch));
			fifthCv = frequencyToCV(fifthFrequency);

			int seventhOctave = octave;
			if (inputInversion > 3 && sevenInterval != 0) {
				seventhOctave++;
			}
			else if (inputInversion > 0 && sevenInterval == 0) {
				seventhOctave++;
			}
			float seventhFrequency = semitoneToFrequency(this->_referenceSemitone + 12 * (seventhOctave - this->_referenceOctave) + (pitch + sevenInterval - this->_referencePitch));
			seventhCv = frequencyToCV(seventhFrequency);
		}
	}

	int gateLength = (int) clamp(roundf(params[LENGTH_PARAM + this->_currentStep].value + inputs[LENGTH_CV_INPUT + this->_currentStep].value), 0.0f, 16.0f);
	if (gateLength != 0) {
		outputs[OUT_OUTPUT + 0].setVoltage(rootCv);
		outputs[OUT_OUTPUT + 1].setVoltage(thirdCv);
		outputs[OUT_OUTPUT + 2].setVoltage(fifthCv);
		outputs[OUT_OUTPUT + 3].setVoltage(seventhCv);

		if (!isTriad) {
			outputs[POLY_OUTPUT].setChannels(4);
			outputs[POLY_OUTPUT].setVoltage(seventhCv, 3);
		} else {
			outputs[POLY_OUTPUT].setChannels(3);
		}
		outputs[POLY_OUTPUT].setVoltage(rootCv, 0);
		outputs[POLY_OUTPUT].setVoltage(thirdCv, 1);
		outputs[POLY_OUTPUT].setVoltage(fifthCv, 2);

		/*
			GATE_OUTPUT is high through every substep,
			and on the last substep we use the gateGenerator to handle the real gate length
		*/
		float gateVoltage = isLastSubStep() ? this->_gateGenerator.process(this->_sampleTime) * 5.0f : 10.f;
		if (!isTriad) {
			outputs[GATE_OUTPUT].setChannels(4);
			outputs[GATE_OUTPUT].setVoltage(gateVoltage, 3);
		} else {
			outputs[GATE_OUTPUT].setChannels(3);
		}
		outputs[GATE_OUTPUT].setVoltage(gateVoltage, 0);
		outputs[GATE_OUTPUT].setVoltage(gateVoltage, 1);
		outputs[GATE_OUTPUT].setVoltage(gateVoltage, 2);
	} else {
		outputs[GATE_OUTPUT].setChannels(4);
		outputs[GATE_OUTPUT].setVoltage(0.f, 0);
		outputs[GATE_OUTPUT].setVoltage(0.f, 1);
		outputs[GATE_OUTPUT].setVoltage(0.f, 2);
		outputs[GATE_OUTPUT].setVoltage(0.f, 3);
	}

	outputs[EOC_OUTPUT].setVoltage(this->_eocGenerator.process(this->_sampleTime) ? 10.f : 0.f);
};

ProgressWidget::ChordDisplayWidget::ChordDisplayWidget(Vec pos, Vec size, Progress* module, int index) {
	box.size = size;
	box.pos = pos.minus(size.div(2));
	this->_module = module;
	this->_index = index;
};

void ProgressWidget::ChordDisplayWidget::chordName() {
	if (this->_module != NULL) {
		static const char * noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
		static const char * chordTypes[] = {
			"",
			"m",
			"MAJ7",
			"7",
			"m7",
			"sus2",
			"sus4",
			"dim",
			"+"
		};

		int root = this->_module->_roots[this->_index];
		int chordType = this->_module->_chordTypes[this->_index];
		int inversion = root + this->_module->_inversions[this->_index];
		if (inversion > 11) {
			inversion -= 12;
		}

		sprintf(this->_inv, "");
		if (inversion != root) {
			sprintf(this->_inv, "/%s", noteNames[inversion]);
		}
		sprintf(this->_text, "%s%s%s", noteNames[root], chordTypes[chordType], this->_inv);
	} else {
		snprintf(this->_text, 9, "         ");
	}
};

void ProgressWidget::ChordDisplayWidget::draw(const DrawArgs &args) {
	nvgBeginPath(args.vg);
	nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
	nvgFillColor(args.vg, this->_backgroundColor);
	nvgFill(args.vg);
	nvgStrokeWidth(args.vg, 1.0);
	nvgStrokeColor(args.vg, this->_borderColor);
	nvgStroke(args.vg);

	rack::Widget::draw(args);
};

void ProgressWidget::ChordDisplayWidget::drawLayer(const DrawArgs &args, int layer) {
	if (layer == 1) {
		nvgFontSize(args.vg, 23);
		std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, this->_fontPath));
		if (font) {
			nvgFontFaceId(args.vg, font->handle);
		}
		nvgTextLetterSpacing(args.vg, -1.5);
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER);

		Vec textPos = Vec(box.size.x/2, 21.0f);
		if (this->_module != NULL && this->_module->_currentStep == this->_index) {
			nvgFillColor(args.vg, this->_highlightColor);
		} else {
			nvgFillColor(args.vg, this->_textColor);
		}
		chordName();
		nvgText(args.vg, textPos.x, textPos.y, this->_text, NULL);
	}

	rack::Widget::drawLayer(args, layer);
}


ProgressWidget::ProgressWidget(Progress* module) {
	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Progress.svg")));

	addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	// SEQUENCER INPUTS
	static const float componentY[8] = {20.32, 33.02, 45.72, 58.42, 71.12, 83.82, 96.52, 109.22};
	for (int i = 0; i < 8; i++) {
		addParam(createParamCentered<DaviesGrayMediumSnap>(mm2px(Vec(68.58, componentY[i])), module, Progress::ROOT_PARAM + i));
		addParam(createParamCentered<DaviesMaroonMediumSnap>(mm2px(Vec(83.82, componentY[i])), module, Progress::TYPE_PARAM + i));
		addParam(createParamCentered<DaviesGrayMediumSnap>(mm2px(Vec(99.06, componentY[i])), module, Progress::INVERSION_PARAM + i));
		addParam(createParamCentered<DaviesMaroonMediumSnap>(mm2px(Vec(114.3, componentY[i])), module, Progress::LENGTH_PARAM + i));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(124.46, componentY[i])), module, Progress::LENGTH_CV_INPUT + i));
	}

	// INPUT SECTION
	addParam(createParamCentered<DaviesGrayMediumSnap>(mm2px(Vec(10.179, 45.72)), module, Progress::STEPS_PARAM));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.339, 45.72)), module, Progress::STEPS_CV_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 25.63)), module, Progress::CLOCK_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.32, 25.63)), module, Progress::RESET_INPUT));

	// OUTPUT SECTION
	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 71.03)), module, Progress::OUT_OUTPUT + 0));
	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.32, 71.03)), module, Progress::OUT_OUTPUT + 1));
	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.117, 83.62)), module, Progress::OUT_OUTPUT + 2));
	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.32, 83.62)), module, Progress::OUT_OUTPUT + 3));
	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 96.33)), module, Progress::POLY_OUTPUT));
	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.277, 96.33)), module, Progress::GATE_OUTPUT));
	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.26, 109.14)), module, Progress::EOC_OUTPUT));

	// SCREENS
	float start = 60;
	float distanceY = 37.5;
	for (int i = 0; i < 8; i++) {
		ChordDisplayWidget* display = new ChordDisplayWidget(Vec(130, start + distanceY * i), Vec(90, 30), module, i);
		addChild(display);
	}
};

Model* modelProgress = createModel<Progress, ProgressWidget>("Progress");
