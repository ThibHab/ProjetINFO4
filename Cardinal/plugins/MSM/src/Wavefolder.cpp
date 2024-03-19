#include "MSM.hpp"
#include "Additional/Wavefolder.hpp"
#include "MSMComponentLibrary.hpp"

struct Wavefolder : Module {

	enum ParamIds {
		SHAPE_PARAM,
		SHAPE_CV_PARAM,
		UP_PARAM,
		DOWN_PARAM,
		GAIN_PARAM,
		SYM_PARAM,
		TYPESWITCH,
		RANGE,
		GAIN_CV_PARAM,
		NUM_PARAMS
	};

	enum InputIds {
		IN_INPUT,
		SHAPE_CV_INPUT,
		UP_INPUT,
		DOWN_INPUT,
		GAIN_INPUT,
		NUM_INPUTS
	};

	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightIds {
		NUM_LIGHTS
	};

	int Theme = THEME_DEFAULT;

	double IN_1 = 0.0f;

	double DOWN = 0.0f;
	double UP = 0.0f;

	double SHAPE_CV = 0.0f;

	char Type = 0.0f;

	double SHAPER = 0.0f;
	double SHAPE_SYM = 0.0f;
	double SHAPE_MOD = 0.0f;
	double FOLD = 0.0f;

	bool type = false;

	Wavefold wavefold;

	Wavefolder() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(Wavefolder::TYPESWITCH, 0.0, 1.0, 0.0, "Fold Type");
		configParam(Wavefolder::RANGE, 0.0, 5.0, 2.5, "Fold Range");
		configParam(Wavefolder::SHAPE_PARAM, 0.0, 1.5, 0.0, "Fold Shape");
		configParam(Wavefolder::SHAPE_CV_PARAM, -1.0, 1.0, 0.0, "Fold Shape CV");
		configParam(Wavefolder::SYM_PARAM, -2.0, 2.0, 0.0, "Simmetry");
		configParam(Wavefolder::UP_PARAM, 0.0, 1.0, 0.0, "Upper Shape");
		configParam(Wavefolder::DOWN_PARAM, 0.0, 1.0, 0.0, "Lower Shape");
		configParam(Wavefolder::GAIN_PARAM, 0.0, 1.0, 0.0, "Fold Gain");
		configParam(Wavefolder::GAIN_CV_PARAM, -1.0, 1.0, 0.0, "Fold Gain CV");
		configInput(UP_INPUT, "Asymmetric Up Folding Control Voltage");
		configInput(DOWN_INPUT, "Asymmetric Down Folding Control Voltage");
		configInput(GAIN_INPUT, "Fold Gain Control Voltage");
		configInput(SHAPE_CV_INPUT, "Fold Shape Control Voltage");
		configInput(IN_INPUT, "Master");
		configOutput(OUT_OUTPUT, "Master");
		getParamQuantity(TYPESWITCH)->randomizeEnabled = false;
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

void Wavefolder::process(const ProcessArgs& args) {

	type = params[TYPESWITCH].getValue() > 0;

	IN_1 = inputs[IN_INPUT].isConnected() ? inputs[IN_INPUT].getVoltage() : 0.0f;
	UP = clamp(params[UP_PARAM].getValue() + inputs[UP_INPUT].getVoltage() / 2.5f, 0.0f, 4.0f);
	DOWN = clamp(params[DOWN_PARAM].getValue() + inputs[DOWN_INPUT].getVoltage() / 2.5f, 0.0f, 4.0f);

		SHAPE_MOD = params[SHAPE_PARAM].getValue();

	SHAPE_CV = (inputs[SHAPE_CV_INPUT].getVoltage() * params[SHAPE_CV_PARAM].getValue()) / 5.0f;
	SHAPE_SYM = clamp(params[SYM_PARAM].getValue(), -10.0f, 10.0f);

	wavefold.Shape(IN_1, SHAPE_MOD, SHAPE_CV, SHAPE_SYM, params[RANGE].getValue(), UP, DOWN, outputs[OUT_OUTPUT].isConnected());

	if (type)
			wavefold.processB();
	else
			wavefold.processA();

	FOLD = saturate(wavefold.Output());

	double mix = params[GAIN_PARAM].getValue() + clamp(params[GAIN_CV_PARAM].getValue() * inputs[GAIN_INPUT].getVoltage() / 5.0f,0.0f,1.0f);
	//if (inputs[GAIN_INPUT].isConnected()) {
	//	mix *= clamp(params[GAIN_CV_PARAM].getValue() + inputs[GAIN_INPUT].getVoltage() / 5.0f, 0.0f, 1.0f);
	//}
	double OUT = crossfade(IN_1, FOLD, mix);

	if(outputs[OUT_OUTPUT].isConnected()) {
		outputs[OUT_OUTPUT].setVoltage(OUT);
	}
};

struct WFClassicMenu : MenuItem {
	Wavefolder *wavefolder;
	void onAction(const event::Action &e) override {
		wavefolder->Theme = 0;
	}
	void step() override {
		rightText = (wavefolder->Theme == 0) ? "✔" : "";
		MenuItem::step();
	}
};

struct WFNightModeMenu : MenuItem {
	Wavefolder *wavefolder;
	void onAction(const event::Action &e) override {
		wavefolder->Theme = 1;
	}
	void step() override {
		rightText = (wavefolder->Theme == 1) ? "✔" : "";
		MenuItem::step();
	}
};

struct WavefolderWidget : ModuleWidget {
	// Panel Themes
	SvgPanel *panelClassic;
	SvgPanel *panelNightMode;

#ifndef USING_CARDINAL_NOT_RACK
	void appendContextMenu(Menu *menu) override {
		Wavefolder *wavefolder = dynamic_cast<Wavefolder*>(module);
		assert(wavefolder);
		menu->addChild(construct<MenuEntry>());
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Theme"));
		menu->addChild(construct<WFClassicMenu>(&WFClassicMenu::text, "Classic (default)", &WFClassicMenu::wavefolder, wavefolder));
		menu->addChild(construct<WFNightModeMenu>(&WFNightModeMenu::text, "Night Mode", &WFNightModeMenu::wavefolder, wavefolder));
	}
#endif

	WavefolderWidget(Wavefolder *module);
	void step() override;
};

WavefolderWidget::WavefolderWidget(Wavefolder *module) {
	setModule(module);
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	panelClassic = new SvgPanel();
	panelClassic->box.size = box.size;
	panelClassic->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/Wavefolder.svg")));
	panelClassic->visible = !THEME_DEFAULT;
	addChild(panelClassic);

	panelNightMode = new SvgPanel();
	panelNightMode->box.size = box.size;
	panelNightMode->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/Wavefolder-Dark.svg")));
	panelNightMode->visible = !!THEME_DEFAULT;
	addChild(panelNightMode);

	addChild(createWidget<MScrewA>(Vec(0, 0)));
	addChild(createWidget<MScrewC>(Vec(box.size.x-15, 0)));
	addChild(createWidget<MScrewD>(Vec(0, 365)));
	addChild(createWidget<MScrewA>(Vec(box.size.x-15, 365)));

	addParam(createParam<VioM2Switch>(Vec(5, 55), module, Wavefolder::TYPESWITCH));

	addParam(createParam<YellowSmallKnob>(Vec(50, 105), module, Wavefolder::RANGE));

	addParam(createParam<RedLargeKnob>(Vec(22, 40), module, Wavefolder::SHAPE_PARAM));
	addParam(createParam<GreenSmallKnob>(Vec(10, 105), module, Wavefolder::SHAPE_CV_PARAM));

	addParam(createParam<BlueSmallKnob>(Vec(10, 205), module, Wavefolder::SYM_PARAM));

	addParam(createParam<RedSmallKnob>(Vec(10, 155), module, Wavefolder::UP_PARAM));
	addParam(createParam<RedSmallKnob>(Vec(50, 155), module, Wavefolder::DOWN_PARAM));

	addParam(createParam<GreenSmallKnob>(Vec(50, 205), module, Wavefolder::GAIN_PARAM));
	addParam(createParam<GreenSmallKnob>(Vec(30, 250), module, Wavefolder::GAIN_CV_PARAM));

	addInput(createInput<SilverSixPort>(Vec(5, 295), module, Wavefolder::UP_INPUT));
	addInput(createInput<SilverSixPortB>(Vec(33, 295), module, Wavefolder::DOWN_INPUT));
	addInput(createInput<SilverSixPortE>(Vec(60, 295), module, Wavefolder::GAIN_INPUT));

	addInput(createInput<SilverSixPortC>(Vec(33, 330), module, Wavefolder::SHAPE_CV_INPUT));
	addInput(createInput<SilverSixPortA>(Vec(5, 330), module, Wavefolder::IN_INPUT));
	addOutput(createOutput<SilverSixPort>(Vec(60, 330), module, Wavefolder::OUT_OUTPUT));
};

void WavefolderWidget::step() {
#ifdef USING_CARDINAL_NOT_RACK
	panelClassic->visible = !settings::preferDarkPanels;
	panelNightMode->visible = settings::preferDarkPanels;
#else
	if (module) {
		Wavefolder *wavefolder = dynamic_cast<Wavefolder*>(module);
		assert(wavefolder);
		panelClassic->visible = (wavefolder->Theme == 0);
		panelNightMode->visible = (wavefolder->Theme == 1);
	}
#endif
	ModuleWidget::step();
}

Model *modelWavefolder = createModel<Wavefolder, WavefolderWidget>("Wavefolder");
