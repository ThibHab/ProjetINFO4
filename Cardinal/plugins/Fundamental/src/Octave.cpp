#include "components.hpp"


struct Octave : Module {
	enum ParamIds {
		OCTAVE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		PITCH_INPUT,
		OCTAVE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		PITCH_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	int lastOctave = 0;

	Octave() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(OCTAVE_PARAM, -4.f, 4.f, 0.f, "Shift", " oct");
		getParamQuantity(OCTAVE_PARAM)->snapEnabled = true;
		configInput(PITCH_INPUT, "1V/octave pitch");
		configInput(OCTAVE_INPUT, "Octave shift CV");
		configOutput(PITCH_OUTPUT, "Pitch");
		configBypass(PITCH_INPUT, PITCH_OUTPUT);
	}

	void process(const ProcessArgs& args) override {
		int channels = std::max(inputs[PITCH_INPUT].getChannels(), 1);
		int octaveParam = std::round(params[OCTAVE_PARAM].getValue());

		for (int c = 0; c < channels; c++) {
			int octave = octaveParam + std::round(inputs[OCTAVE_INPUT].getPolyVoltage(c));
			float pitch = inputs[PITCH_INPUT].getVoltage(c);
			pitch += octave;
			outputs[PITCH_OUTPUT].setVoltage(pitch, c);
			if (c == 0)
				lastOctave = octave;
		}
		outputs[PITCH_OUTPUT].setChannels(channels);
	}

	void dataFromJson(json_t* rootJ) override {
		// In Fundamental 1.1.1 and earlier, the octave param was internal data.
		json_t* octaveJ = json_object_get(rootJ, "octave");
		if (octaveJ) {
			params[OCTAVE_PARAM].setValue(json_integer_value(octaveJ));
		}
	}
};


struct OctaveButton : Widget {
	int octave;

	void drawLayer(const DrawArgs& args, int layer) override {
		if (layer != 1)
			return;

		int activeOctave = 0;
		int lastOctave = 0;

		if (ParamWidget* const paramWidget = getAncestorOfType<ParamWidget>()) {
			if (engine::ParamQuantity* pq = paramWidget->getParamQuantity()) {
				activeOctave = std::round(pq->getValue());
				if (Octave* const module = dynamic_cast<Octave*>(pq->module))
					lastOctave = module->lastOctave;
			}
		}

		nvgBeginPath(args.vg);

		if (octave == 0)
			nvgRect(args.vg, 6.5f, 0.f, box.size.x - 13.f, box.size.y);
		else
			nvgRect(args.vg, 0.f, 0.f, box.size.x, box.size.y);

		if (activeOctave == octave) {
			// Enabled
			if (octave == 0)
				nvgFillColor(args.vg, nvgRGB(0x49, 0x49, 0x49));
			else
				nvgFillColor(args.vg, nvgRGBf(0.76f, 0.11f, 0.22f));
		}
		else if (lastOctave == octave) {
			// Disabled but enabled by CV
			if (octave == 0)
				nvgFillColor(args.vg, color::mult(nvgRGB(0x49, 0x49, 0x49), 0.5));
			else
				nvgFillColor(args.vg, color::mult(nvgRGBf(0.76f, 0.11f, 0.22f), 0.5));
		}
		else {
			// Disabled
			nvgFillColor(args.vg, nvgRGB(0, 0, 0));
		}

		nvgFill(args.vg);
		nvgStrokeColor(args.vg, nvgRGB(0x24, 0x28, 0x28));
		nvgStroke(args.vg);
	}

	void onDragHover(const event::DragHover& e) override {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
			e.consume(this);
		}
		Widget::onDragHover(e);
	}

	void onDragEnter(const event::DragEnter& e) override;
};


struct OctaveParam : ParamWidget {
	static constexpr const int octaves = 9;
	static constexpr const float height = 13.f;
	static constexpr const float margin = 7.0f;

	OctaveParam() {
		box.size = Vec(25.8f, octaves * (height + margin) + margin * 2);

		for (int i = 0; i < octaves; i++) {
			OctaveButton* octaveButton = new OctaveButton();
			octaveButton->box.pos = Vec(0, margin + i * (height + margin));
			octaveButton->box.size = Vec(box.size.x, height);
			octaveButton->octave = 4 - i;
			addChild(octaveButton);
		}
	}
};


inline void OctaveButton::onDragEnter(const event::DragEnter& e) {
	if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
		if (OctaveParam* const origin = dynamic_cast<OctaveParam*>(e.origin)) {
			if (ParamWidget* const paramWidget = getAncestorOfType<ParamWidget>()) {
				if (engine::ParamQuantity* const pq = paramWidget->getParamQuantity()) {
					pq->setValue(octave);
				}
			}
		}
	}
	Widget::onDragEnter(e);
}


struct OctaveWidget : ModuleWidget {
	static constexpr const int kWidth = 3;
	static constexpr const float kHorizontalCenter = kRACK_GRID_WIDTH * kWidth * 0.5f;

	static constexpr const float kVerticalPos1 = kRACK_GRID_HEIGHT - 308.f - kRACK_JACK_HALF_SIZE;
	static constexpr const float kVerticalPos2 = kRACK_GRID_HEIGHT - 268.f - kRACK_JACK_HALF_SIZE;
	static constexpr const float kVerticalPos3 = kRACK_GRID_HEIGHT - 26.f - kRACK_JACK_HALF_SIZE;

	OctaveWidget(Octave* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Octave.svg")));

		addChild(createWidget<ThemedScrew>(Vec(kRACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(kRACK_GRID_WIDTH, kRACK_GRID_HEIGHT - kRACK_GRID_WIDTH)));

		addInput(createInputCentered<FundamentalPort>(Vec(kHorizontalCenter, kVerticalPos1), module, Octave::PITCH_INPUT));
		addInput(createInputCentered<FundamentalPort>(Vec(kHorizontalCenter, kVerticalPos2), module, Octave::OCTAVE_INPUT));

		addChild(createParam<OctaveParam>(Vec(9.6f, kRACK_GRID_HEIGHT - 245.f - OctaveParam::height - OctaveParam::margin), module, Octave::OCTAVE_PARAM));

		addOutput(createOutputCentered<FundamentalPort>(Vec(kHorizontalCenter, kVerticalPos3), module, Octave::PITCH_OUTPUT));
	}
};


Model* modelOctave = createModel<Octave, OctaveWidget>("Octave");
