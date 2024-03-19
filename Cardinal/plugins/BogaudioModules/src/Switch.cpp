
#include "Switch.hpp"

void bogaudio::Switch::reset() {
	for (int i = 0; i < _channels; ++i) {
		_trigger[i].reset();
		_latchedHigh[i] = false;
	}
}

int bogaudio::Switch::channels() {
	return inputs[GATE_INPUT].getChannels();
}

void bogaudio::Switch::channelsChanged(int before, int after) {
	for (; before < after; ++before) {
		_trigger[before].reset();
	}
}

void bogaudio::Switch::modulate() {
	_latch = params[LATCH_PARAM].getValue() > 0.5f;
}

void bogaudio::Switch::processAlways(const ProcessArgs& args) {
	_high1LightSum = 0;
	_low1LightSum = 0;
	_high2LightSum = 0;
	_low2LightSum = 0;
}

void bogaudio::Switch::processChannel(const ProcessArgs& args, int c) {
	bool triggered = _trigger[c].process(10.0f*params[GATE_PARAM].getValue() + inputs[GATE_INPUT].getVoltage(c));
	if (_latch) {
		if (triggered) {
			_latchedHigh[c] = !_latchedHigh[c];
		}
	}
	else {
		_latchedHigh[c] = false;
	}

	if (_latchedHigh[c] || (!_latch && _trigger[c].isHigh())) {
		++_high1LightSum;
		++_high2LightSum;

		if (_channels == 1) {
			outputs[OUT1_OUTPUT].setChannels(inputs[HIGH1_INPUT].getChannels());
			outputs[OUT1_OUTPUT].writeVoltages(inputs[HIGH1_INPUT].getVoltages());

			outputs[OUT2_OUTPUT].setChannels(inputs[HIGH2_INPUT].getChannels());
			outputs[OUT2_OUTPUT].writeVoltages(inputs[HIGH2_INPUT].getVoltages());
		}
		else {
			outputs[OUT1_OUTPUT].setChannels(_channels);
			outputs[OUT1_OUTPUT].setVoltage(inputs[HIGH1_INPUT].getPolyVoltage(c), c);

			outputs[OUT2_OUTPUT].setChannels(_channels);
			outputs[OUT2_OUTPUT].setVoltage(inputs[HIGH2_INPUT].getPolyVoltage(c), c);
		}
	}
	else {
		++_low1LightSum;
		++_low2LightSum;

		if (_channels == 1) {
			outputs[OUT1_OUTPUT].setChannels(inputs[LOW1_INPUT].getChannels());
			outputs[OUT1_OUTPUT].writeVoltages(inputs[LOW1_INPUT].getVoltages());

			outputs[OUT2_OUTPUT].setChannels(inputs[LOW2_INPUT].getChannels());
			outputs[OUT2_OUTPUT].writeVoltages(inputs[LOW2_INPUT].getVoltages());
		}
		else {
			outputs[OUT1_OUTPUT].setChannels(_channels);
			outputs[OUT1_OUTPUT].setVoltage(inputs[LOW1_INPUT].getPolyVoltage(c), c);

			outputs[OUT2_OUTPUT].setChannels(_channels);
			outputs[OUT2_OUTPUT].setVoltage(inputs[LOW2_INPUT].getPolyVoltage(c), c);
		}
	}
}

void bogaudio::Switch::postProcessAlways(const ProcessArgs& args) {
	lights[HIGH1_LIGHT].value = _high1LightSum * _inverseChannels;
	lights[LOW1_LIGHT].value = _low1LightSum * _inverseChannels;
	lights[HIGH2_LIGHT].value = _high2LightSum * _inverseChannels;
	lights[LOW2_LIGHT].value = _low2LightSum * _inverseChannels;
}

struct SwitchWidget : SaveLatchToPatchModuleWidget {
	static constexpr int hp = 3;

	SwitchWidget(bogaudio::Switch* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);
		setPanel(box.size, "Switch");
		createScrews();

		// generated by svg_widgets.rb
		auto gateParamPosition = Vec(13.5, 22.0);
		auto latchParamPosition = Vec(31.5, 84.0);

		auto gateInputPosition = Vec(10.5, 44.0);
		auto high1InputPosition = Vec(10.5, 100.0);
		auto low1InputPosition = Vec(10.5, 136.0);
		auto high2InputPosition = Vec(10.5, 217.0);
		auto low2InputPosition = Vec(10.5, 253.0);

		auto out1OutputPosition = Vec(10.5, 174.0);
		auto out2OutputPosition = Vec(10.5, 291.0);

		auto high1LightPosition = Vec(7.5, 126.3);
		auto low1LightPosition = Vec(7.5, 162.3);
		auto high2LightPosition = Vec(7.5, 243.3);
		auto low2LightPosition = Vec(7.5, 279.3);
		// end generated by svg_widgets.rb

		addParam(createParam<Button18>(gateParamPosition, module, bogaudio::Switch::GATE_PARAM));
		addParam(createParam<IndicatorButtonGreen9>(latchParamPosition, module, bogaudio::Switch::LATCH_PARAM));

		addInput(createInput<Port24>(gateInputPosition, module, bogaudio::Switch::GATE_INPUT));
		addInput(createInput<Port24>(high1InputPosition, module, bogaudio::Switch::HIGH1_INPUT));
		addInput(createInput<Port24>(low1InputPosition, module, bogaudio::Switch::LOW1_INPUT));
		addInput(createInput<Port24>(high2InputPosition, module, bogaudio::Switch::HIGH2_INPUT));
		addInput(createInput<Port24>(low2InputPosition, module, bogaudio::Switch::LOW2_INPUT));

		addOutput(createOutput<Port24>(out1OutputPosition, module, bogaudio::Switch::OUT1_OUTPUT));
		addOutput(createOutput<Port24>(out2OutputPosition, module, bogaudio::Switch::OUT2_OUTPUT));

		addChild(createLight<BGSmallLight<GreenLight>>(high1LightPosition, module, bogaudio::Switch::HIGH1_LIGHT));
		addChild(createLight<BGSmallLight<GreenLight>>(low1LightPosition, module, bogaudio::Switch::LOW1_LIGHT));
		addChild(createLight<BGSmallLight<GreenLight>>(high2LightPosition, module, bogaudio::Switch::HIGH2_LIGHT));
		addChild(createLight<BGSmallLight<GreenLight>>(low2LightPosition, module, bogaudio::Switch::LOW2_LIGHT));
	}
};

Model* modelSwitch = bogaudio::createModel<bogaudio::Switch, SwitchWidget>("Bogaudio-Switch", "SWITCH", "2-way signal router", "Switch", "Polyphonic");
