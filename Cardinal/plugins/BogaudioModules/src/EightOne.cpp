
#include "EightOne.hpp"

void EightOne::processAlways(const ProcessArgs& args) {
	std::fill(_lightSums, _lightSums + 8, 0.0f);
}

void EightOne::processChannel(const ProcessArgs& args, int c) {
	int step = nextStep(
		c,
		&inputs[RESET_INPUT],
		inputs[CLOCK_INPUT],
		&params[STEPS_PARAM],
		params[DIRECTION_PARAM],
		&params[SELECT_PARAM],
		inputs[SELECT_INPUT]
	);

	Input& in = inputs[IN1_INPUT + step];
	if (_channels > 1) {
		outputs[OUT_OUTPUT].setChannels(_channels);
		outputs[OUT_OUTPUT].setVoltage(in.getPolyVoltage(c), c);
	}
	else {
		outputs[OUT_OUTPUT].setChannels(in.getChannels());
		outputs[OUT_OUTPUT].writeVoltages(in.getVoltages());
	}

	for (int i = 0; i < 8; ++i) {
		_lightSums[i] += step == i;
	}
}

void EightOne::postProcessAlways(const ProcessArgs& args) {
	for (int i = 0; i < 8; ++i) {
		lights[IN1_LIGHT + i].value = _lightSums[i] * _inverseChannels;
	}
}

struct EightOneWidget : AddressableSequenceModuleWidget {
	static constexpr int hp = 6;

	EightOneWidget(EightOne* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);
		setPanel(box.size, "EightOne");
		createScrews();

		// generated by svg_widgets.rb
		auto stepsParamPosition = Vec(58.5, 131.5);
		auto directionParamPosition = Vec(58.5, 167.5);
		auto selectParamPosition = Vec(52.0, 230.0);

		auto in1InputPosition = Vec(11.5, 35.0);
		auto in2InputPosition = Vec(11.5, 76.3);
		auto in3InputPosition = Vec(11.5, 118.6);
		auto in4InputPosition = Vec(11.5, 158.9);
		auto in5InputPosition = Vec(11.5, 200.1);
		auto in6InputPosition = Vec(11.5, 241.4);
		auto in7InputPosition = Vec(11.5, 282.7);
		auto in8InputPosition = Vec(11.5, 324.0);
		auto clockInputPosition = Vec(54.5, 35.0);
		auto resetInputPosition = Vec(54.5, 72.0);
		auto selectInputPosition = Vec(54.5, 270.0);

		auto outOutputPosition = Vec(54.5, 324.0);

		auto in1LightPosition = Vec(23.5, 61.5);
		auto in2LightPosition = Vec(23.5, 102.8);
		auto in3LightPosition = Vec(23.5, 145.1);
		auto in4LightPosition = Vec(23.5, 185.4);
		auto in5LightPosition = Vec(23.5, 226.6);
		auto in6LightPosition = Vec(23.5, 267.9);
		auto in7LightPosition = Vec(23.5, 309.2);
		auto in8LightPosition = Vec(23.5, 350.5);
		// end generated by svg_widgets.rb

		addParam(createParam<Knob16>(stepsParamPosition, module, EightOne::STEPS_PARAM));
		addParam(createParam<SliderSwitch2State14>(directionParamPosition, module, EightOne::DIRECTION_PARAM));
		addParam(createParam<Knob29>(selectParamPosition, module, EightOne::SELECT_PARAM));

		addInput(createInput<Port24>(in1InputPosition, module, EightOne::IN1_INPUT));
		addInput(createInput<Port24>(in2InputPosition, module, EightOne::IN2_INPUT));
		addInput(createInput<Port24>(in3InputPosition, module, EightOne::IN3_INPUT));
		addInput(createInput<Port24>(in4InputPosition, module, EightOne::IN4_INPUT));
		addInput(createInput<Port24>(in5InputPosition, module, EightOne::IN5_INPUT));
		addInput(createInput<Port24>(in6InputPosition, module, EightOne::IN6_INPUT));
		addInput(createInput<Port24>(in7InputPosition, module, EightOne::IN7_INPUT));
		addInput(createInput<Port24>(in8InputPosition, module, EightOne::IN8_INPUT));
		addInput(createInput<Port24>(clockInputPosition, module, EightOne::CLOCK_INPUT));
		addInput(createInput<Port24>(resetInputPosition, module, EightOne::RESET_INPUT));
		addInput(createInput<Port24>(selectInputPosition, module, EightOne::SELECT_INPUT));

		addOutput(createOutput<Port24>(outOutputPosition, module, EightOne::OUT_OUTPUT));

		addChild(createLight<BGSmallLight<GreenLight>>(in1LightPosition, module, EightOne::IN1_LIGHT));
		addChild(createLight<BGSmallLight<GreenLight>>(in2LightPosition, module, EightOne::IN2_LIGHT));
		addChild(createLight<BGSmallLight<GreenLight>>(in3LightPosition, module, EightOne::IN3_LIGHT));
		addChild(createLight<BGSmallLight<GreenLight>>(in4LightPosition, module, EightOne::IN4_LIGHT));
		addChild(createLight<BGSmallLight<GreenLight>>(in5LightPosition, module, EightOne::IN5_LIGHT));
		addChild(createLight<BGSmallLight<GreenLight>>(in6LightPosition, module, EightOne::IN6_LIGHT));
		addChild(createLight<BGSmallLight<GreenLight>>(in7LightPosition, module, EightOne::IN7_LIGHT));
		addChild(createLight<BGSmallLight<GreenLight>>(in8LightPosition, module, EightOne::IN8_LIGHT));
	}
};

Model* modelEightOne = bogaudio::createModel<EightOne, EightOneWidget>("Bogaudio-EightOne", "8:1", "8-channel demultiplexer & sequential switch", "Switch", "Polyphonic");