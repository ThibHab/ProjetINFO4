#include "MSM.hpp"
#include "MSMComponentLibrary.hpp"
#include "Additional/ExperimentalVCO.hpp"

struct ExperimentalVCO : Module {

	enum ParamIds {
		LFOMODE,
		FREQ_OCT_PARAM,
		FREQ_PARAM,
		FINE_PARAM,
		FM_PARAM,
		TYPE,
		WINDOW,
		MOD1,
		MOD2,
		MOD3,
		MOD1_CV_PARAM,
		MOD2_CV_PARAM,
		MOD3_CV_PARAM,
		NUM_PARAMS
	};

	enum InputIds {
		VOCT_INPUT,
		RESET_INPUT,
		TYPE_INPUT,
		WINDOW_CV,
		MOD1_CV,
		MOD2_CV,
		MOD3_CV,
		LIN_INPUT,
		NUM_INPUTS
	};

	enum OutputIds {
		OUTPUT,
		NUM_OUTPUTS
	};

	MSVCO<8, 8> oscillator;

	int Oct = 0;

	// Panel Theme
	int Theme = THEME_DEFAULT;

	ExperimentalVCO() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		configParam(ExperimentalVCO::LFOMODE, 0.0, 1.0, 1.0, "LFO/VCO");
		configParam(ExperimentalVCO::FREQ_OCT_PARAM, 0.0, 6.0, 3.0, "Octave", "", 0.0f, 1.0f, 1.0f);
		configParam(ExperimentalVCO::FREQ_PARAM, 0.0, 12.0, 0.0, "Note");
		configParam(ExperimentalVCO::FINE_PARAM, -1.0, 1.0, 0.0, "Fine");
		configParam(ExperimentalVCO::MOD1, 0.01f, 1.0, 1.0, "Mod 1");
		configParam(ExperimentalVCO::MOD2, 0.01f, 1.0, 1.0, "Mod 2");
		configParam(ExperimentalVCO::MOD3, 0.01f, 1.0, 1.0, "Mod 3");
		configParam(ExperimentalVCO::MOD1_CV_PARAM, 1.0f, -1.0, 0.0, "Mod 1 CV", "%", 0.0f, 100);
		configParam(ExperimentalVCO::MOD2_CV_PARAM, 1.0f, -1.0, 0.0, "Mod 2 CV", "%", 0.0f, 100);
		configParam(ExperimentalVCO::MOD3_CV_PARAM, 1.0f, -1.0, 0.0, "Mod 3 CV", "%", 0.0f, 100);
		configParam(ExperimentalVCO::TYPE, 0.0, 13.0, 0.0, "Type", "", 0.0f, 1.0f, 1.0f);
		configParam(ExperimentalVCO::WINDOW, 512.0, 2047.0, 1024.0, "Window Size");
		configParam(ExperimentalVCO::FM_PARAM, -1.0, 1.0, 0.0, "FM CV", "%", 0.0f, 100);
		configInput(WINDOW_CV, "Window Size Control Voltage");
		configInput(TYPE_INPUT, "Waveform Base Type Control Voltage");
		configInput(RESET_INPUT, "Reset");
		configInput(LIN_INPUT, "FM");
		configInput(VOCT_INPUT, "Volt/Octave");
		configInput(MOD1_CV, "Modulation 1 Control Voltage");
		configInput(MOD2_CV, "Modulation 2 Control Voltage");
		configInput(MOD3_CV, "Modulation 3 Control Voltage");
		configOutput(OUTPUT, "Master");
		getParamQuantity(LFOMODE)->randomizeEnabled = false;
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

void ExperimentalVCO::process(const ProcessArgs& args) {

	char type = 0;
	// Pitch
	//float pitchCv = 0.0f;
	float FreqFine = params[FREQ_PARAM].getValue();
	float Fine = params[FINE_PARAM].getValue();
	//float window = 0.0f;
	float mod1, mod2, mod3;

	Oct = clamp(params[FREQ_OCT_PARAM].getValue(), 0.0, 12.0) * 12;

	bool lfomode = params[LFOMODE].getValue();

	float pitchCv = 12.0f * inputs[VOCT_INPUT].getVoltage();

	pitchCv += (dsp::quadraticBipolar(params[FM_PARAM].getValue()) * 12.0f * inputs[LIN_INPUT].getVoltage());

	oscillator.setPitch(Oct, pitchCv + FreqFine + Fine, lfomode);
	oscillator.process(1.0f / args.sampleRate, inputs[RESET_INPUT].getVoltage(), inputs[RESET_INPUT].isConnected());

	float window = clamp(params[WINDOW].getValue() + (120.0f * inputs[WINDOW_CV].getVoltage()), 512.0f, 2047.0f);

	if(inputs[MOD1_CV].isConnected()) {
		mod1 = clamp(params[MOD1].getValue() + (dsp::quadraticBipolar(params[MOD1_CV_PARAM].getValue()) * 12.0f * inputs[MOD1_CV].getVoltage()) / 10.0f, 0.01f, 1.0f);
	}
	else {
		mod1 = clamp(params[MOD1].getValue(), 0.01f, 1.0f);
	}
	if(inputs[MOD2_CV].isConnected()) {
		mod2 = clamp(params[MOD2].getValue() + (dsp::quadraticBipolar(params[MOD2_CV_PARAM].getValue()) * 12.0f * inputs[MOD2_CV].getVoltage()) / 10.0f, 0.01f, 1.0f);
	}
	else {
		mod2 = clamp(params[MOD2].getValue(), 0.01f, 1.0f);
	}
	if(inputs[MOD3_CV].isConnected()) {
		mod3 = clamp(params[MOD3].getValue() + (dsp::quadraticBipolar(params[MOD3_CV_PARAM].getValue()) * 12.0f * inputs[MOD3_CV].getVoltage()) / 10.0f, 0.01f, 1.0f);
	}
	else {
		mod3 = clamp(params[MOD3].getValue(), 0.01f, 1.0f);
	}
	if(inputs[TYPE_INPUT].isConnected()) {
		type = clamp(params[TYPE].getValue() + 3 * inputs[TYPE_INPUT].getVoltage(), 0.0, 13.0);
	}
	else {
		type = clamp(params[TYPE].getValue(), 0.0, 13.0);
	}

	oscillator.TYPE(type);
	oscillator.Window(window);
	oscillator.Mods(mod1, mod2, mod3);

	oscillator.outEnabled(outputs[OUTPUT].isConnected());

	if(outputs[OUTPUT].isConnected()) {
		outputs[OUTPUT].setVoltage(saturate2(7.0f * oscillator.getOutput()));
	}

};

struct ExperimentalVCOClassicMenu : MenuItem {
	ExperimentalVCO *experimentalvco;
	void onAction(const event::Action &e) override {
		experimentalvco->Theme = 0;
	}
	void step() override {
		rightText = (experimentalvco->Theme == 0) ? "✔" : "";
		MenuItem::step();
	}
};

struct ExperimentalVCONightModeMenu : MenuItem {
	ExperimentalVCO *experimentalvco;
	void onAction(const event::Action &e) override {
		experimentalvco->Theme = 1;
	}
	void step() override {
		rightText = (experimentalvco->Theme == 1) ? "✔" : "";
		MenuItem::step();
	}
};

struct ExperimentalVCOEspenMenu : MenuItem {
	ExperimentalVCO *experimentalvco;
	void onAction(const event::Action &e) override {
		experimentalvco->Theme = 2;
	}
	void step() override {
		rightText = (experimentalvco->Theme == 2) ? "✔" : "";
		MenuItem::step();
	}
};

struct ExperimentalVCOOmriMenu : MenuItem {
	ExperimentalVCO *experimentalvco;
	void onAction(const event::Action &e) override {
		experimentalvco->Theme = 3;
	}
	void step() override {
		rightText = (experimentalvco->Theme == 3) ? "✔" : "";
		MenuItem::step();
	}
};

struct ExperimentalVCOWidget : ModuleWidget {
	// Panel Themes
	SvgPanel *pClassic;
	SvgPanel *pNightMode;

	SvgPanel *pEspenFront;
	MSMPanel *pEspenBack;

	MSMPanel *pOmriBack;
	SvgPanel *pOmriFront;

#ifndef USING_CARDINAL_NOT_RACK
	void appendContextMenu(Menu *menu) override {
		ExperimentalVCO *experimentalvco = dynamic_cast<ExperimentalVCO*>(module);
		assert(experimentalvco);
		menu->addChild(construct<MenuEntry>());
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Theme"));
		menu->addChild(construct<ExperimentalVCOClassicMenu>(&ExperimentalVCOClassicMenu::text, "Classic (default)", &ExperimentalVCOClassicMenu::experimentalvco, experimentalvco));
		menu->addChild(construct<ExperimentalVCONightModeMenu>(&ExperimentalVCONightModeMenu::text, "Night Mode", &ExperimentalVCONightModeMenu::experimentalvco, experimentalvco));
		menu->addChild(construct<ExperimentalVCOEspenMenu>(&ExperimentalVCOEspenMenu::text, "Espen's Treasure | Jedi", &ExperimentalVCOEspenMenu::experimentalvco, experimentalvco));
		menu->addChild(construct<ExperimentalVCOOmriMenu>(&ExperimentalVCOOmriMenu::text, "Omri's Treasure | Mushroom", &ExperimentalVCOOmriMenu::experimentalvco, experimentalvco));
	}
#endif

	ExperimentalVCOWidget(ExperimentalVCO *module);
	void step() override;
};

ExperimentalVCOWidget::ExperimentalVCOWidget(ExperimentalVCO *module) {
	setModule(module);
	box.size = Vec(14 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	pClassic = new SvgPanel();
	pClassic->box.size = box.size;
	pClassic->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/MSVCO.svg")));
	pClassic->visible = !THEME_DEFAULT;
	addChild(pClassic);

	pNightMode = new SvgPanel();
	pNightMode->box.size = box.size;
	pNightMode->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/MSVCO-Dark.svg")));
	pNightMode->visible = !!THEME_DEFAULT;
	addChild(pNightMode);

	pEspenBack = new MSMLightPanel();
	pEspenBack->box.size = box.size;
	pEspenBack->imagePath = asset::plugin(pluginInstance, "res/Panels/MSVCO-Espen-Back.png");
	pEspenBack->visible = false;
	addChild(pEspenBack);

	pEspenFront = new SvgPanel();
	pEspenFront->box.size = box.size;
	pEspenFront->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/MSVCO-Espen-Front.svg")));
	pEspenFront->visible = false;
	addChild(pEspenFront);

	pOmriBack = new MSMLightPanel();
	pOmriBack->box.size = box.size;
	pOmriBack->imagePath = asset::plugin(pluginInstance, "res/Panels/MSVCO-Omri-Back.png");
	pOmriBack->visible = false;
	addChild(pOmriBack);

	pOmriFront = new SvgPanel();
	pOmriFront->box.size = box.size;
	pOmriFront->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/MSVCO-Omri-Front.svg")));
	pOmriFront->visible = false;
	addChild(pOmriFront);


	addChild(createWidget<MScrewA>(Vec(15, 0)));
	addChild(createWidget<MScrewC>(Vec(box.size.x-30, 0)));
	addChild(createWidget<MScrewD>(Vec(15, 365)));
	addChild(createWidget<MScrewB>(Vec(box.size.x-30, 365)));

	// Params
	addParam(createParam<VioM2Switch>(Vec(98, 293), module, ExperimentalVCO::LFOMODE));

	addParam(createParam<RedLargeToggleKnob>(Vec(81.5, 26), module, ExperimentalVCO::FREQ_OCT_PARAM));
	addParam(createParam<RedSmallToggleKnob>(Vec(29.5, 34.5), module, ExperimentalVCO::FREQ_PARAM));
	addParam(createParam<RedSmallKnob>(Vec(149.5, 34.5), module, ExperimentalVCO::FINE_PARAM));

	addParam(createParam<GreenSmallKnob>(Vec(29.5, 99.5), module, ExperimentalVCO::MOD1));
	addParam(createParam<GreenSmallKnob>(Vec(90, 99.5), module, ExperimentalVCO::MOD2));
	addParam(createParam<GreenSmallKnob>(Vec(149.5, 99.5), module, ExperimentalVCO::MOD3));

	addParam(createParam<BlueSmallKnob>(Vec(29.5, 164.5), module, ExperimentalVCO::MOD1_CV_PARAM));
	addParam(createParam<BlueSmallKnob>(Vec(90, 164.5), module, ExperimentalVCO::MOD2_CV_PARAM));
	addParam(createParam<BlueSmallKnob>(Vec(149.5, 164.5), module, ExperimentalVCO::MOD3_CV_PARAM));

	addParam(createParam<GreenToggleKnobSmall>(Vec(90, 229.5), module, ExperimentalVCO::TYPE));
	addParam(createParam<YellowSmallKnob>(Vec(29.5, 229.5), module, ExperimentalVCO::WINDOW));
	addParam(createParam<RedSmallKnob>(Vec(149.5, 229.5), module, ExperimentalVCO::FM_PARAM));

	// Ports
	addInput(createInput<SilverSixPortB>(Vec(12.5, 287.5), module, ExperimentalVCO::WINDOW_CV));
	addInput(createInput<SilverSixPortB>(Vec(57, 287.5), module, ExperimentalVCO::TYPE_INPUT));
	addInput(createInput<SilverSixPortB>(Vec(127.5, 287.5), module, ExperimentalVCO::RESET_INPUT));
	addInput(createInput<SilverSixPortB>(Vec(173, 287.5), module, ExperimentalVCO::LIN_INPUT));

	addInput(createInput<SilverSixPortA>(Vec(12.5, 328), module, ExperimentalVCO::VOCT_INPUT));
	addInput(createInput<SilverSixPortC>(Vec(57, 328), module, ExperimentalVCO::MOD1_CV));
	addInput(createInput<SilverSixPortD>(Vec(92.5, 328), module, ExperimentalVCO::MOD2_CV));
	addInput(createInput<SilverSixPortE>(Vec(127.5, 328), module, ExperimentalVCO::MOD3_CV));
	addOutput(createOutput<SilverSixPort>(Vec(173, 328), module, ExperimentalVCO::OUTPUT));

};

void ExperimentalVCOWidget::step() {
#ifdef USING_CARDINAL_NOT_RACK
	pClassic->visible = !settings::preferDarkPanels;
	pNightMode->visible = settings::preferDarkPanels;
#else
	if (module) {
		ExperimentalVCO *experimentalvco = dynamic_cast<ExperimentalVCO*>(module);
		assert(experimentalvco);
		pClassic->visible = (experimentalvco->Theme == 0);
		pNightMode->visible = (experimentalvco->Theme == 1);
		pEspenBack->visible = (experimentalvco->Theme == 2);
		pEspenFront->visible = (experimentalvco->Theme == 2);

		pOmriBack->visible = (experimentalvco->Theme == 3);
		pOmriFront->visible = (experimentalvco->Theme == 3);
	}
#endif
	ModuleWidget::step();
}

Model *modelExperimentalVCO = createModel<ExperimentalVCO, ExperimentalVCOWidget>("TreasureVCO");
