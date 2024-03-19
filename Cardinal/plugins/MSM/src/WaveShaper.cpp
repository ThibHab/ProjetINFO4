#include "MSM.hpp"
#include "Additional/WaveShaper.hpp"
#include "MSMComponentLibrary.hpp"

struct WaveShaper : Module {

	enum ParamIds {
		SHAPE_1_PARAM,
		SHAPE_2_PARAM,
		SHAPE_3_PARAM,
		SHAPE_1_CV_PARAM,
		SHAPE_2_CV_PARAM,
		SHAPE_3_CV_PARAM,
		OUTPUT_GAIN_PARAM,
		MIX_PARAM,
		NUM_PARAMS
	};

	enum InputIds {
		MASTER_INPUT,
		SHAPE_1_CV_INPUT,
		SHAPE_2_CV_INPUT,
		SHAPE_3_CV_INPUT,
		IN_A_INPUT,
		IN_B_INPUT,
		IN_C_INPUT,
		//MIX_CV,
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

	double IN_M = 0.0f, IN_A = 0.0f, IN_B = 0.0f, IN_C = 0.0f;
	double SHAPE_MOD1 = 0.0f, SHAPE_CV1 = 0.0f;
	double SHAPE_MOD2 = 0.0f, SHAPE_CV2 = 0.0f;
	double SHAPE_MOD3 = 0.0f, SHAPE_CV3 = 0.0f;

	double OUT = 0.0f;

	WaveShape<8, 8> waveS;

	WaveShaper() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(WaveShaper::SHAPE_1_PARAM, 1.0, -0.4, 1.0, "Shape A");
		configParam(WaveShaper::SHAPE_1_CV_PARAM, -1.0, 1.0, 0.0, "Shape A CV");
		configParam(WaveShaper::SHAPE_2_PARAM, 0.0, 1.0, 0.0, "Shape B");
		configParam(WaveShaper::SHAPE_2_CV_PARAM, -1.0, 1.0, 0.0, "Shape B CV");
		configParam(WaveShaper::SHAPE_3_PARAM, -1.0, 1.0, -1.0, "Shape C");
		configParam(WaveShaper::SHAPE_3_CV_PARAM, -1.0, 1.0, 0.0, "Shape C CV");
		configParam(WaveShaper::MIX_PARAM, 0.0, 1.0, 0.5, "Dry/Wet Mix");
		configParam(WaveShaper::OUTPUT_GAIN_PARAM, 0.0, 1.0, 0.5, "Wet Level");
		configInput(MASTER_INPUT, "Master");
		configInput(IN_A_INPUT, "A");
		configInput(IN_B_INPUT, "B");
		configInput(IN_C_INPUT, "C");
		configInput(SHAPE_1_CV_INPUT, "(A) Shape Control Voltage");
		configInput(SHAPE_2_CV_INPUT, "(B) Shape Control Voltage");
		configInput(SHAPE_3_CV_INPUT, "(C) Shape Control Voltage");
		configOutput(OUT_OUTPUT, "Master");
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

void WaveShaper::process(const ProcessArgs& args)
{
	IN_M = inputs[MASTER_INPUT].isConnected() ? inputs[MASTER_INPUT].getVoltage() : 0.0f;
	IN_A = inputs[IN_A_INPUT].isConnected() ? inputs[IN_A_INPUT].getVoltage() : 0.0f;
	IN_B = inputs[IN_B_INPUT].isConnected() ? inputs[IN_B_INPUT].getVoltage() : 0.0f;
	IN_C = inputs[IN_C_INPUT].isConnected() ? inputs[IN_C_INPUT].getVoltage() : 0.0f;

	SHAPE_MOD1 = params[SHAPE_1_PARAM].getValue();
	SHAPE_MOD2 = params[SHAPE_2_PARAM].getValue();
	SHAPE_MOD3 = params[SHAPE_3_PARAM].getValue();

	SHAPE_CV1 = (inputs[SHAPE_1_CV_INPUT].getVoltage() * params[SHAPE_1_CV_PARAM].getValue());
	SHAPE_CV2 = (inputs[SHAPE_2_CV_INPUT].getVoltage() * params[SHAPE_2_CV_PARAM].getValue());
	SHAPE_CV3 = (inputs[SHAPE_3_CV_INPUT].getVoltage() * params[SHAPE_3_CV_PARAM].getValue());

	waveS.process();

	waveS.Shape1(IN_M + IN_A,SHAPE_MOD1,SHAPE_CV1, outputs[OUT_OUTPUT].isConnected());
	waveS.Shape2(IN_M + IN_B,SHAPE_MOD2,SHAPE_CV2, outputs[OUT_OUTPUT].isConnected());
	waveS.Shape3(IN_M + IN_C,SHAPE_MOD3,SHAPE_CV3, outputs[OUT_OUTPUT].isConnected());

	double mix = params[MIX_PARAM].getValue();
		OUT = waveS.MasterOutput() *  params[OUTPUT_GAIN_PARAM].getValue();
	double MixOUT = crossfade((IN_M + IN_A + IN_B + IN_C), OUT, mix);

	if(outputs[OUT_OUTPUT].isConnected()) {
		outputs[OUT_OUTPUT].setVoltage(saturate(MixOUT));
	}
	else {
		outputs[OUT_OUTPUT].setVoltage(0.0f);
	}

}

struct WSClassicMenu : MenuItem {
	WaveShaper *waveshaper;
	void onAction(const event::Action &e) override {
		waveshaper->Theme = 0;
	}
	void step() override {
		rightText = (waveshaper->Theme == 0) ? "✔" : "";
		MenuItem::step();
	}
};

struct WSNightModeMenu : MenuItem {
	WaveShaper *waveshaper;
	void onAction(const event::Action &e) override {
		waveshaper->Theme = 1;
	}
	void step() override {
		rightText = (waveshaper->Theme == 1) ? "✔" : "";
		MenuItem::step();
	}
};

struct WaveShaperWidget : ModuleWidget {
	// Panel Themes
	SvgPanel *panelClassic;
	SvgPanel *panelNightMode;

#ifndef USING_CARDINAL_NOT_RACK
	void appendContextMenu(Menu *menu) override {
		WaveShaper *waveshaper = dynamic_cast<WaveShaper*>(module);
		assert(waveshaper);
		menu->addChild(construct<MenuEntry>());
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Theme"));
		menu->addChild(construct<WSClassicMenu>(&WSClassicMenu::text, "Classic (default)", &WSClassicMenu::waveshaper, waveshaper));
		menu->addChild(construct<WSNightModeMenu>(&WSNightModeMenu::text, "Night Mode", &WSNightModeMenu::waveshaper, waveshaper));
	}
#endif

	WaveShaperWidget(WaveShaper *module);
	void step() override;
};

WaveShaperWidget::WaveShaperWidget(WaveShaper *module) {
	setModule(module);
	box.size = Vec(11 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	panelClassic = new SvgPanel();
	panelClassic->box.size = box.size;
	panelClassic->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/WaveShaper.svg")));
	panelClassic->visible = !THEME_DEFAULT;
	addChild(panelClassic);

	panelNightMode = new SvgPanel();
	panelNightMode->box.size = box.size;
	panelNightMode->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/WaveShaper-Dark.svg")));
	panelNightMode->visible = !!THEME_DEFAULT;
	addChild(panelNightMode);

	addChild(createWidget<MScrewA>(Vec(15, 0)));
	addChild(createWidget<MScrewD>(Vec(15, 365)));
	addChild(createWidget<MScrewB>(Vec(135, 0)));
	addChild(createWidget<MScrewA>(Vec(135, 365)));

	addParam(createParam<RedLargeKnob>(Vec(25, 25), module, WaveShaper::SHAPE_1_PARAM));
	addParam(createParam<BlueLargeKnob>(Vec(95, 25), module, WaveShaper::SHAPE_1_CV_PARAM));

	addParam(createParam<RedLargeKnob>(Vec(25, 85), module, WaveShaper::SHAPE_2_PARAM));
	addParam(createParam<BlueLargeKnob>(Vec(95, 85), module, WaveShaper::SHAPE_2_CV_PARAM));

	addParam(createParam<RedLargeKnob>(Vec(25, 145), module, WaveShaper::SHAPE_3_PARAM));
	addParam(createParam<BlueLargeKnob>(Vec(95, 145), module, WaveShaper::SHAPE_3_CV_PARAM));

	addParam(createParam<GreenLargeKnob>(Vec(25, 205), module, WaveShaper::MIX_PARAM));
	//addInput(createInput<SilverSixPortA>(Vec(5, 245), module, WaveShaper::MIX_CV));

	addParam(createParam<GreenLargeKnob>(Vec(95, 205), module, WaveShaper::OUTPUT_GAIN_PARAM));

	addInput(createInput<SilverSixPortA>(Vec(10, 275), module, WaveShaper::MASTER_INPUT));

	addInput(createInput<SilverSixPortC>(Vec(50, 275), module, WaveShaper::IN_A_INPUT));
	addInput(createInput<SilverSixPortD>(Vec(90, 275), module, WaveShaper::IN_B_INPUT));
	addInput(createInput<SilverSixPortA>(Vec(130, 275), module, WaveShaper::IN_C_INPUT));

	addInput(createInput<SilverSixPortD>(Vec(10, 320), module, WaveShaper::SHAPE_1_CV_INPUT));
	addInput(createInput<SilverSixPortA>(Vec(50, 320), module, WaveShaper::SHAPE_2_CV_INPUT));
	addInput(createInput<SilverSixPortC>(Vec(90, 320), module, WaveShaper::SHAPE_3_CV_INPUT));

	addOutput(createOutput<SilverSixPortB>(Vec(130, 320), module, WaveShaper::OUT_OUTPUT));
};

void WaveShaperWidget::step() {
#ifdef USING_CARDINAL_NOT_RACK
	panelClassic->visible = !settings::preferDarkPanels;
	panelNightMode->visible = settings::preferDarkPanels;
#else
	if (module) {
		WaveShaper *waveshaper = dynamic_cast<WaveShaper*>(module);
		assert(waveshaper);
		panelClassic->visible = (waveshaper->Theme == 0);
		panelNightMode->visible = (waveshaper->Theme == 1);
	}
#endif
	ModuleWidget::step();
}

Model *modelWaveShaper = createModel<WaveShaper, WaveShaperWidget>("WaveShaper");
