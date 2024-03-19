#include "MSM.hpp"
#include "MSMComponentLibrary.hpp"

struct RandomSource : Module {
		enum ParamIds {
		RANGE_PARAM,
		RANGE_CV_PARAM,
		SWITCH_PARAM,
		SLEW_PARAM,
		SHAPE_PARAM,
		NUM_PARAMS
	};

		enum InputIds {
		TRIG_INPUT,
		SH_INPUT,
		SLEW_CV,
		RANGE_CV_INPUT,
		NUM_INPUTS
	};

		enum OutputIds {
		SH_OUTPUT,
		SLEWED_OUT,
		NUM_OUTPUTS
	};

		dsp::SchmittTrigger trigger;
		double sample = 0.0;
		double out = 0.0;

		int Theme = THEME_DEFAULT;

		RandomSource() {
			config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
			configParam(RandomSource::RANGE_PARAM, 0.0, 1.0, 0.0, "Sample Range");
			configParam(RandomSource::SLEW_PARAM, 0.0, 1.0, 0.5, "Slew");
			configParam(RandomSource::SHAPE_PARAM, 0.0, 1.0, 0.5, "Shape");
			configParam(RandomSource::RANGE_CV_PARAM, 0.0, 1.0, 0.0, "Sample Range CV");
			configParam(RandomSource::SWITCH_PARAM, 0.0, 1.0, 0.0, "Uni/Bi");
			configInput(SH_INPUT, "Sample");
			configInput(TRIG_INPUT, "Trigger");
			configInput(SLEW_CV, "Slew Magnitude Control Voltage");
			configInput(RANGE_CV_INPUT, "Sample Input Attenuator Control Voltage");
			configOutput(SH_OUTPUT, "Sample & Hold");
			configOutput(SLEWED_OUT, "Amplitude Slewed");
		}

		void process(const ProcessArgs& args) override;

	//Json for Panel Theme
	json_t *dataToJson() override	{
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Theme", json_integer(Theme));
		return rootJ;
	}
	void dataFromJson(json_t *rootJ) override	{
		json_t *ThemeJ = json_object_get(rootJ, "Theme");
		if (ThemeJ)
			Theme = json_integer_value(ThemeJ);
	}
};

void RandomSource::process(const ProcessArgs& args) {

	//sample and hold
	double range = params[RANGE_PARAM].getValue() + params[RANGE_CV_PARAM].getValue() * (inputs[RANGE_CV_INPUT].getVoltage() / 5.0f);
 	double Switch = params[SWITCH_PARAM].getValue();

	if (trigger.process(inputs[TRIG_INPUT].getVoltage())) {
		if(Switch > 0.0) {
		sample = inputs[SH_INPUT].getNormalVoltage(range) + 5.0;
		}
		else {
			sample = inputs[SH_INPUT].getNormalVoltage(range);
		}
	}

	double SHOut = (sample ? sample : 0.0f) * range;

	// S&H Output
	outputs[SH_OUTPUT].setVoltage(saturate(SHOut));

	//slew limiter
	double in = outputs[SH_OUTPUT].getVoltage();
	double shape = params[SHAPE_PARAM].getValue();

	const double slewMin = 0.1;
	const double slewMax = 10000.0;
	const double shapeScale = 1/10.0;

	// Rise
	if (in > out) {
		double rise = inputs[SLEW_CV].getVoltage() / 10.0 + params[SLEW_PARAM].getValue();
		double slew = slewMax * powf(slewMin / slewMax, rise);
		out += slew * crossfade(1.0f, shapeScale * (in - out), shape) * args.sampleTime;
		if (out > in)
			out = in;
	}
	// Fall
	else if (in < out) {
		double fall = inputs[SLEW_CV].getVoltage() / 10.0 + params[SLEW_PARAM].getValue();
		double slew = slewMax * powf(slewMin / slewMax, fall);
		out -= slew * crossfade(1.0f, shapeScale * (out - in), shape) * args.sampleTime;
		if (out < in)
			out = in;
	}

	outputs[SLEWED_OUT].setVoltage(saturate(out));
};

struct RandomSClassicMenu : MenuItem {
	RandomSource *randomsource;
	void onAction(const event::Action &e) override {
		randomsource->Theme = 0;
	}
	void step() override {
		rightText = (randomsource->Theme == 0) ? "✔" : "";
		MenuItem::step();
	}
};

struct RandomSNightModeMenu : MenuItem {
	RandomSource *randomsource;
	void onAction(const event::Action &e) override {
		randomsource->Theme = 1;
	}
	void step() override {
		rightText = (randomsource->Theme == 1) ? "✔" : "";
		MenuItem::step();
	}
};

struct RandomSourceWidget : ModuleWidget {
	// Panel Themes
	SvgPanel *panelClassic;
	SvgPanel *panelNightMode;

#ifndef USING_CARDINAL_NOT_RACK
	void appendContextMenu(Menu *menu) override {
		RandomSource *randomsource = dynamic_cast<RandomSource*>(module);
		assert(randomsource);
		menu->addChild(construct<MenuEntry>());
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Theme"));
		menu->addChild(construct<RandomSClassicMenu>(&RandomSClassicMenu::text, "Classic (default)", &RandomSClassicMenu::randomsource, randomsource));
		menu->addChild(construct<RandomSNightModeMenu>(&RandomSNightModeMenu::text, "Night Mode", &RandomSNightModeMenu::randomsource, randomsource));
	}
#endif

	RandomSourceWidget(RandomSource *module);
	void step() override;
};

RandomSourceWidget::RandomSourceWidget(RandomSource *module) {
	setModule(module);
	box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	panelClassic = new SvgPanel();
	panelClassic->box.size = box.size;
	panelClassic->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/RandomSource.svg")));
	panelClassic->visible = !THEME_DEFAULT;
	addChild(panelClassic);

	panelNightMode = new SvgPanel();
	panelNightMode->box.size = box.size;
	panelNightMode->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/RandomSource-Dark.svg")));
	panelNightMode->visible = !!THEME_DEFAULT;
	addChild(panelNightMode);

	//Screw
	addChild(createWidget<MScrewB>(Vec(15, 0)));
	addChild(createWidget<MScrewC>(Vec(15, 365)));
	addChild(createWidget<MScrewD>(Vec(box.size.x - 30, 0)));
	addChild(createWidget<MScrewA>(Vec(box.size.x - 30, 365)));

	//Param
	addParam(createParam<GreenLargeKnob>(Vec(7, 75), module, RandomSource::RANGE_PARAM));
	addParam(createParam<GreenLargeKnob>(Vec(65, 115), module, RandomSource::SLEW_PARAM));
	addParam(createParam<GreenLargeKnob>(Vec(7, 165), module, RandomSource::SHAPE_PARAM));

	addParam(createParam<GreenSmallKnob>(Vec(73, 189), module, RandomSource::RANGE_CV_PARAM));

	addParam(createParam<VioMSwitch>(Vec(82.5, 65), module, RandomSource::SWITCH_PARAM));

	//Inputs
	addInput(createInput<SilverSixPortA>(Vec(18, 289.5), module, RandomSource::SH_INPUT));
	addInput(createInput<SilverSixPortD>(Vec(18, 327.5), module, RandomSource::TRIG_INPUT));
	addInput(createInput<SilverSixPortC>(Vec(18, 251.5), module, RandomSource::SLEW_CV));
	addInput(createInput<SilverSixPortC>(Vec(80, 251.5), module, RandomSource::RANGE_CV_INPUT));

	//Outputs
	addOutput(createOutput<SilverSixPortB>(Vec(80, 289.5), module, RandomSource::SH_OUTPUT));
	addOutput(createOutput<SilverSixPort>(Vec(80, 327.5), module, RandomSource::SLEWED_OUT));

};

void RandomSourceWidget::step() {
#ifdef USING_CARDINAL_NOT_RACK
	panelClassic->visible = !settings::preferDarkPanels;
	panelNightMode->visible = settings::preferDarkPanels;
#else
	if (module) {
		RandomSource *randomsource = dynamic_cast<RandomSource*>(module);
		assert(randomsource);
		panelClassic->visible = (randomsource->Theme == 0);
		panelNightMode->visible = (randomsource->Theme == 1);
	}
#endif
	ModuleWidget::step();
}

Model *modelRandomSource = createModel<RandomSource, RandomSourceWidget>("RandomSource");
