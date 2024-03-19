#include "components.hpp"


static inline
float findMaxNormalizedFloat10(const float floats[], const std::size_t count)
{
	static constexpr const float kEmptyFloats[128] = {};

	if (std::memcmp(floats, kEmptyFloats, count) == 0)
		return 0.f;

	float tmp, maxf2 = std::abs(floats[0]);

	for (std::size_t i=1; i<count; ++i)
	{
		tmp = std::abs(*floats++);

		if (tmp > maxf2)
			maxf2 = tmp;
	}

	if (maxf2 > 10.f)
		maxf2 = 10.f;

	return maxf2;
}


struct Sum : Module {
	enum ParamIds {
		LEVEL_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		POLY_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		MONO_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(VU_LIGHTS, 6),
		NUM_LIGHTS
	};

	int lastChannels = 0;

	uint32_t internalDataFrame = 0;
	float internalDataBuffer[128];
	volatile bool resetMeters = true;
	float levelMeter = 0.0f;

	Sum() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(LEVEL_PARAM, 0.f, 1.f, 1.f, "Level", "%", 0.f, 100.f);
		configInput(POLY_INPUT, "Polyphonic");
		configOutput(MONO_OUTPUT, "Monophonic");
	}

	void process(const ProcessArgs& args) override {
		float sum = inputs[POLY_INPUT].getVoltageSum();
		sum *= params[LEVEL_PARAM].getValue();
		outputs[MONO_OUTPUT].setVoltage(sum);
		lastChannels = inputs[POLY_INPUT].getChannels();

		const uint32_t j = internalDataFrame++;
		internalDataBuffer[j] = sum;

		if (internalDataFrame == 128)
		{
			internalDataFrame = 0;

			if (resetMeters)
				levelMeter = 0.0f;

			levelMeter = std::max(levelMeter, findMaxNormalizedFloat10(internalDataBuffer, 128));
			resetMeters = false;
		}
	}

	void onReset() override
	{
		resetMeters = true;
	}

	void onSampleRateChange(const SampleRateChangeEvent& e) override
	{
		resetMeters = true;
	}
};


struct FundamentalNanoMeter : Widget {
	Sum* module;
	float levelMeter = 0.0f;

	void updateMeters()
	{
		if (module == nullptr || module->resetMeters)
			return;

		// Only fetch new values once DSP side is updated
		levelMeter = module->levelMeter;
		module->resetMeters = true;
	}
	void drawLayer(const DrawArgs& args, int layer) override
	{
		if (layer != 1)
			return;

		const float usableHeight = box.size.y;

		nvgBeginPath(args.vg);
		nvgRect(args.vg,
				0,
				0,
				box.size.x,
				usableHeight);
		nvgFillColor(args.vg, nvgRGB(26, 26, 26));
		nvgFill(args.vg);

		nvgFillColor(args.vg, nvgRGBAf(0.76f, 0.11f, 0.22f, 0.5f));
		nvgStrokeColor(args.vg, nvgRGBf(0.76f, 0.11f, 0.22f));

		updateMeters();

		const float height = 1.0f + std::sqrt(levelMeter * 0.1f) * (usableHeight - 1.0f);
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 1.0f, usableHeight - height, box.size.x - 2.0f, height);
		nvgFill(args.vg);
		nvgStroke(args.vg);
	}
};


struct SumChannelDisplay : Widget {
	std::string fontPath = asset::system("res/fonts/DSEG7ClassicMini-BoldItalic.ttf");
	std::string text = "00";
	Sum* module;

	void drawLayer(const DrawArgs& args, int layer) override {
		if (layer == 1) {
			std::shared_ptr<Font> font = APP->window->loadFont(fontPath);

			nvgRect(args.vg, 0.f, 0.f, box.size.x, box.size.y);
			nvgFontFaceId(args.vg, font->handle);
			nvgFontSize(args.vg, 16);
			nvgTextLetterSpacing(args.vg, 0.0);
			nvgTextAlign(args.vg, NVG_ALIGN_RIGHT);
			nvgFillColor(args.vg, nvgRGBf(0.76f, 0.11f, 0.22f));
			nvgText(args.vg, box.size.x*0.8666f, 34.f, text.c_str(), nullptr);
		}

		Widget::drawLayer(args, layer);
	}

	void step() override {
		const int channels = module != nullptr ? module->lastChannels : 16;
		text = string::f("%02d", channels);
	}
};


struct SumWidget : ModuleWidget {
	typedef FundamentalBlackKnob<30> Knob;

	static constexpr const int kWidth = 3;
	static constexpr const float kHorizontalCenter = kRACK_GRID_WIDTH * kWidth * 0.5f;

	static constexpr const float kVerticalPos1 = kRACK_GRID_HEIGHT - 308.f - kRACK_JACK_HALF_SIZE;
	static constexpr const float kVerticalPos2 = kRACK_GRID_HEIGHT - 75.f - Knob::kHalfSize;
	static constexpr const float kVerticalPos3 = kRACK_GRID_HEIGHT - 25.f - kRACK_JACK_HALF_SIZE;

	SumWidget(Sum* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Sum.svg")));

		addChild(createWidget<ThemedScrew>(Vec(kRACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(kRACK_GRID_WIDTH, kRACK_GRID_HEIGHT - kRACK_GRID_WIDTH)));

		addInput(createInputCentered<FundamentalPort>(Vec(kHorizontalCenter, kVerticalPos1), module, Sum::POLY_INPUT));
		addParam(createParamCentered<Knob>(Vec(kHorizontalCenter, kVerticalPos2), module, Sum::LEVEL_PARAM));
		addOutput(createOutputCentered<FundamentalPort>(Vec(kHorizontalCenter, kVerticalPos3), module, Sum::MONO_OUTPUT));

		FundamentalNanoMeter* display = createWidget<FundamentalNanoMeter>(Vec(9.f, kRACK_GRID_HEIGHT - 124.f - 130.f));
		display->box.size = Vec(27.f, 130.f);
		display->module = module;
		addChild(display);

		SumChannelDisplay* channelDisplay = createWidget<SumChannelDisplay>(Vec(4.5f, kRACK_GRID_HEIGHT - 261.f - 40.f));
		channelDisplay->box.size = Vec(36.f, 130.f);
		channelDisplay->module = module;
		addChild(channelDisplay);
	}
};


Model* modelSum = createModel<Sum, SumWidget>("Sum");
