
#include "EQS.hpp"

bool EQS::active() {
	return outputs[LEFT_OUTPUT].isConnected() || outputs[RIGHT_OUTPUT].isConnected();
}

int EQS::channels() {
	return inputs[LEFT_INPUT].getChannels();
}

void EQS::addChannel(int c) {
	_engines[c] = new Engine();
}

void EQS::removeChannel(int c) {
	delete _engines[c];
	_engines[c] = NULL;
}

float EQS::knobToDb(Param& p) {
	float v = clamp(p.getValue(), -1.0f, 1.0f);
	if (v < 0.0f) {
		return -v * bogaudio::dsp::Equalizer::cutDb;
	}
	return v * bogaudio::dsp::Equalizer::gainDb;
}

void EQS::modulate() {
	_lowDb = knobToDb(params[LOW_PARAM]);
	_midDb = knobToDb(params[MID_PARAM]);
	_highDb = knobToDb(params[HIGH_PARAM]);
}

void EQS::modulateChannel(int c) {
	float sr = APP->engine->getSampleRate();
	_engines[c]->_left.setParams(
		sr,
		_lowDb,
		_midDb,
		_highDb
	);
	_engines[c]->_right.setParams(
		sr,
		_lowDb,
		_midDb,
		_highDb
	);
}

void EQS::processAll(const ProcessArgs& args) {
	outputs[LEFT_OUTPUT].setChannels(_channels);
	outputs[RIGHT_OUTPUT].setChannels(_channels);
}

void EQS::processChannel(const ProcessArgs& args, int c) {
	outputs[LEFT_OUTPUT].setVoltage(_engines[c]->_left.next(inputs[LEFT_INPUT].getVoltage(c)), c);
	outputs[RIGHT_OUTPUT].setVoltage(_engines[c]->_right.next(inputs[RIGHT_INPUT].getVoltage(c)), c);
}

struct EQSWidget : BGModuleWidget {
	static constexpr int hp = 6;

	EQSWidget(EQS* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);
		setPanel(box.size, "EQS");
		createScrews();

		// generated by svg_widgets.rb
		auto lowParamPosition = Vec(26.0, 53.0);
		auto midParamPosition = Vec(26.0, 135.0);
		auto highParamPosition = Vec(26.0, 217.0);

		auto leftInputPosition = Vec(16.0, 279.0);
		auto rightInputPosition = Vec(50.0, 279.0);

		auto leftOutputPosition = Vec(16.0, 320.0);
		auto rightOutputPosition = Vec(50.0, 320.0);
		// end generated by svg_widgets.rb

		addParam(createParam<Knob38>(lowParamPosition, module, EQS::LOW_PARAM));
		addParam(createParam<Knob38>(midParamPosition, module, EQS::MID_PARAM));
		addParam(createParam<Knob38>(highParamPosition, module, EQS::HIGH_PARAM));

		addInput(createInput<Port24>(leftInputPosition, module, EQS::LEFT_INPUT));
		addInput(createInput<Port24>(rightInputPosition, module, EQS::RIGHT_INPUT));

		addOutput(createOutput<Port24>(leftOutputPosition, module, EQS::LEFT_OUTPUT));
		addOutput(createOutput<Port24>(rightOutputPosition, module, EQS::RIGHT_OUTPUT));
	}
};

Model* modelEQS = createModel<EQS, EQSWidget>("Bogaudio-EQS", "EQS", "Stereo 3-band equalizer", "Equalizer", "Polyphonic");