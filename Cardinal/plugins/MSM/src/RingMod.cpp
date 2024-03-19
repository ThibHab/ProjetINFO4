#include "MSM.hpp"
#include "MSMComponentLibrary.hpp"

struct RingMod : Module
{
    enum ParamIds {
		CARRIER_A_PARAM,
		MODULATOR_A_PARAM,
		MIXA,
		MIXB,
        NUM_PARAMS
    };

    enum InputIds {
        CARRIER_A,
        MODULATOR_A,
		MIXA_CV,
		CARRIER_B,
		MODULATOR_B,
		MIXB_CV,
        NUM_INPUTS
    };

    enum OutputIds {
        OUTPUT_A,
		OUTPUT_B,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

	int Theme = THEME_DEFAULT;

  RingMod() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(RingMod::MIXA, 0.0f, 1.0f, 0.5f, "Modulation", "%", 0.f, 100);
    configParam(RingMod::MIXB, 0.0f, 1.0f, 0.5f, "Modulation", "%", 0.f, 100);
	configInput(MIXA_CV, "(A) Modulation Level Control Voltage");
	configInput(MODULATOR_A, "(A) Modulator");
	configInput(CARRIER_A, "(A) Carrier");
	configOutput(OUTPUT_A, "(A) Master");
	configInput(MIXB_CV, "(B) Modulation Level Control Voltage");
	configInput(MODULATOR_B, "(B) Modulator");
	configInput(CARRIER_B, "(B) Carrier");
	configOutput(OUTPUT_B, "(B) Master");
  }

  void process(const ProcessArgs& args) override;

	float carA = 0.0f;
	float modA = 0.0f;
	float carB = 0.0f;
	float modB  = 0.0f;

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

void RingMod::process(const ProcessArgs& args)
{
	carA = inputs[CARRIER_A].getVoltage() / 5.0;
	modA = inputs[MODULATOR_A].getVoltage() / 5.0;

	float waveA = clamp(params[MIXA].getValue() + inputs[MIXA_CV].getVoltage() / 10.0f, 0.0f, 1.0f);
	float ringoutA = carA * modA * 5.0;
	float outA = crossfade(inputs[CARRIER_A].getVoltage(), ringoutA, waveA);

	outputs[OUTPUT_A].setVoltage(outA);

	carB = inputs[CARRIER_B].getVoltage() / 5.0;
	modB = inputs[MODULATOR_B].getVoltage() / 5.0;

	float waveB = clamp(params[MIXB].getValue() + inputs[MIXB_CV].getVoltage() / 10.0f, 0.0f, 1.0f);
	float ringoutB = carB * modB * 5.0;
	float outB = crossfade(inputs[CARRIER_B].getVoltage(), ringoutB, waveB);

	outputs[OUTPUT_B].setVoltage(outB);
};

struct RMClassicMenu : MenuItem {
	RingMod *ringmod;
	void onAction(const event::Action &e) override {
		ringmod->Theme = 0;
	}
	void step() override {
		rightText = (ringmod->Theme == 0) ? "✔" : "";
		MenuItem::step();
	}
};

struct RMNightModeMenu : MenuItem {
	RingMod *ringmod;
	void onAction(const event::Action &e) override {
		ringmod->Theme = 1;
	}
	void step() override {
		rightText = (ringmod->Theme == 1) ? "✔" : "";
		MenuItem::step();
	}
};

struct RingModWidget : ModuleWidget {
	// Panel Themes
	SvgPanel *panelClassic;
	SvgPanel *panelNightMode;

#ifndef USING_CARDINAL_NOT_RACK
  void appendContextMenu(Menu *menu) override {
  	RingMod *ringmod = dynamic_cast<RingMod*>(module);
  	assert(ringmod);
  	menu->addChild(construct<MenuEntry>());
  	menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Theme"));
  	menu->addChild(construct<RMClassicMenu>(&RMClassicMenu::text, "Classic (default)", &RMClassicMenu::ringmod, ringmod));
  	menu->addChild(construct<RMNightModeMenu>(&RMNightModeMenu::text, "Night Mode", &RMNightModeMenu::ringmod, ringmod));
  }
#endif

	RingModWidget(RingMod *module);
	void step() override;
};

RingModWidget::RingModWidget(RingMod *module) {
	setModule(module);
	box.size = Vec(5 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	panelClassic = new SvgPanel();
	panelClassic->box.size = box.size;
	panelClassic->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/RingMod.svg")));
  panelClassic->visible = !THEME_DEFAULT;
	addChild(panelClassic);

	panelNightMode = new SvgPanel();
	panelNightMode->box.size = box.size;
	panelNightMode->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/RingMod-Dark.svg")));
  panelNightMode->visible = !!THEME_DEFAULT;
	addChild(panelNightMode);

  addChild(createWidget<MScrewB>(Vec(0, 0)));
  addChild(createWidget<MScrewA>(Vec(box.size.x - 15, 0)));
  addChild(createWidget<MScrewD>(Vec(0, 365)));
  addChild(createWidget<MScrewC>(Vec(box.size.x - 15, 365)));

	addParam(createParam<GreenSmallKnob>(Vec(22, 98), module, RingMod::MIXA));
	addInput(createInput<SilverSixPortD>(Vec(5, 144), module, RingMod::MIXA_CV));
  addInput(createInput<SilverSixPortA>(Vec(45, 67), module, RingMod::MODULATOR_A));
  addInput(createInput<SilverSixPort>(Vec(5, 67), module, RingMod::CARRIER_A));
  addOutput(createOutput<SilverSixPortC>(Vec(45, 144), module, RingMod::OUTPUT_A));

	addParam(createParam<GreenSmallKnob>(Vec(22, 242), module, RingMod::MIXB));
	addInput(createInput<SilverSixPortC>(Vec(5, 288), module, RingMod::MIXB_CV));
	addInput(createInput<SilverSixPortE>(Vec(45, 211), module, RingMod::MODULATOR_B));
  addInput(createInput<SilverSixPortB>(Vec(5, 211), module, RingMod::CARRIER_B));
  addOutput(createOutput<SilverSixPort>(Vec(45, 288), module, RingMod::OUTPUT_B));

};

void RingModWidget::step() {
#ifdef USING_CARDINAL_NOT_RACK
	panelClassic->visible = !settings::preferDarkPanels;
	panelNightMode->visible = settings::preferDarkPanels;
#else
  if (module) {
  	RingMod *ringmod = dynamic_cast<RingMod*>(module);
  	assert(ringmod);
  	panelClassic->visible = (ringmod->Theme == 0);
  	panelNightMode->visible = (ringmod->Theme == 1);
  }
#endif
	ModuleWidget::step();
}

Model *modelRingMod = createModel<RingMod, RingModWidget>("Ring");
