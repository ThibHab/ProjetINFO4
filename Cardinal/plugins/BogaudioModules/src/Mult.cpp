
#include "Mult.hpp"

void Mult::processAll(const ProcessArgs& args) {
	int n = inputs[INA_INPUT].getChannels();
	float* out = inputs[INA_INPUT].getVoltages();
	outputs[OUTA1_OUTPUT].setChannels(n);
	outputs[OUTA1_OUTPUT].writeVoltages(out);
	outputs[OUTA2_OUTPUT].setChannels(n);
	outputs[OUTA2_OUTPUT].writeVoltages(out);
	outputs[OUTA3_OUTPUT].setChannels(n);
	outputs[OUTA3_OUTPUT].writeVoltages(out);

	if (inputs[INB_INPUT].isConnected()) {
		n = inputs[INB_INPUT].getChannels();
		out = inputs[INB_INPUT].getVoltages();
	}
	outputs[OUTB1_OUTPUT].setChannels(n);
	outputs[OUTB1_OUTPUT].writeVoltages(out);
	outputs[OUTB2_OUTPUT].setChannels(n);
	outputs[OUTB2_OUTPUT].writeVoltages(out);
	outputs[OUTB3_OUTPUT].setChannels(n);
	outputs[OUTB3_OUTPUT].writeVoltages(out);
}

struct MultWidget : BGModuleWidget {
	static constexpr int hp = 3;

	MultWidget(Mult* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);
		setPanel(box.size, "Mult");
		createScrews();

		// generated by svg_widgets.rb
		auto inaInputPosition = Vec(10.5, 21.0);
		auto inbInputPosition = Vec(10.5, 181.0);

		auto outa1OutputPosition = Vec(10.5, 62.0);
		auto outa2OutputPosition = Vec(10.5, 92.0);
		auto outa3OutputPosition = Vec(10.5, 122.0);
		auto outb1OutputPosition = Vec(10.5, 222.0);
		auto outb2OutputPosition = Vec(10.5, 252.0);
		auto outb3OutputPosition = Vec(10.5, 282.0);
		// end generated by svg_widgets.rb

		addInput(createInput<Port24>(inaInputPosition, module, Mult::INA_INPUT));
		addInput(createInput<Port24>(inbInputPosition, module, Mult::INB_INPUT));

		addOutput(createOutput<Port24>(outa1OutputPosition, module, Mult::OUTA1_OUTPUT));
		addOutput(createOutput<Port24>(outa2OutputPosition, module, Mult::OUTA2_OUTPUT));
		addOutput(createOutput<Port24>(outa3OutputPosition, module, Mult::OUTA3_OUTPUT));
		addOutput(createOutput<Port24>(outb1OutputPosition, module, Mult::OUTB1_OUTPUT));
		addOutput(createOutput<Port24>(outb2OutputPosition, module, Mult::OUTB2_OUTPUT));
		addOutput(createOutput<Port24>(outb3OutputPosition, module, Mult::OUTB3_OUTPUT));
	}
};

Model* modelMult = bogaudio::createModel<Mult, MultWidget>("Bogaudio-Mult", "MULT", "1:6 or dual 1:3 multiple/splitter", "Multiple", "Polyphonic");
