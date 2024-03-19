#include "MSM.hpp"
#include "Additional/Resources.hpp"
#include "MSMComponentLibrary.hpp"

struct Morpher : Module
{
    enum ParamIds {
		MORPH_PARAM,
		SWITCHCOUNT,
        NUM_PARAMS
    };

    enum InputIds {
        INPUT1,
        INPUT2,
		INPUT3,
		INPUT4,
		INPUT5,
        INPUT6,
		INPUT7,
		INPUT8,
		MORPH_CV,
        NUM_INPUTS
    };

    enum OutputIds {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds {
		LIGHT_1,
		LIGHT_2,
		LIGHT_3,
		LIGHT_4,
		LIGHT_5,
		LIGHT_6,
		LIGHT_7,
		LIGHT_8,
        NUM_LIGHTS
    };

	int Theme = THEME_DEFAULT;

	double Crossfader = 0.0f;
	double In1 = 0.0f;
	double In2 = 0.0f;
	double In3 = 0.0f;
	double In4 = 0.0f;
	double In5 = 0.0f;
	double In6 = 0.0f;
	double In7 = 0.0f;
	double In8 = 0.0f;
	double Out = 0.0f ? 0.0f : 0.0f;

	int switchcount = 0.0f;

  Morpher() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(Morpher::MORPH_PARAM, 0.0, 1.0, 0.0, "Morph Control", "%", 0.0f, 100);
    configParam(Morpher::SWITCHCOUNT, 0.0, 6.0, 0.0, "Input Count", "", 0.0f, 1.0f, 2);
	configInput(INPUT1, "#1");
	configInput(INPUT2, "#2");
	configInput(INPUT3, "#3");
	configInput(INPUT4, "#4");
	configInput(INPUT5, "#5");
	configInput(INPUT6, "#6");
	configInput(INPUT7, "#7");
	configInput(INPUT8, "#8");
	configInput(MORPH_CV, "Morph Control Voltage");
	configOutput(OUTPUT, "Master");
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

	void onReset() override {
		switchcount = 0;
	}

	void onRandomize() override {
		switchcount = random::u32() % 7;
	}

};

void Morpher::process(const ProcessArgs& args) {

	In1 = inputs[INPUT1].isConnected() ? inputs[INPUT1].getVoltage() : 0.0f;
	In2 = inputs[INPUT2].isConnected() ? inputs[INPUT2].getVoltage() : 0.0f;
	In3 = inputs[INPUT3].isConnected() ? inputs[INPUT3].getVoltage() : 0.0f;
	In4 = inputs[INPUT4].isConnected() ? inputs[INPUT4].getVoltage() : 0.0f;
	In5 = inputs[INPUT5].isConnected() ? inputs[INPUT5].getVoltage() : 0.0f;
	In6 = inputs[INPUT6].isConnected() ? inputs[INPUT6].getVoltage() : 0.0f;
	In7 = inputs[INPUT7].isConnected() ? inputs[INPUT7].getVoltage() : 0.0f;
	In8 = inputs[INPUT8].isConnected() ? inputs[INPUT8].getVoltage() : 0.0f;

	switchcount = params[SWITCHCOUNT].getValue();

	switch(switchcount) {
		case 6:
		Crossfader = clamp(params[MORPH_PARAM].getValue() + inputs[MORPH_CV].getVoltage() / 10.0f, 0.0f, 7.0f);
		Crossfader *= 7.0f;

			if(Crossfader < 1.0f)
				Out = crossfade(In1, In2, Crossfader);
			else if(Crossfader < 2.0f)
				Out = crossfade(In2, In3, Crossfader - 1.0f);
			else if(Crossfader < 3.0f)
				Out = crossfade(In3, In4, Crossfader - 2.0f);
			else if(Crossfader < 4.0f)
				Out = crossfade(In4, In5, Crossfader - 3.0f);
			else if(Crossfader < 5.0f)
				Out = crossfade(In5, In6, Crossfader - 4.0f);
			else if(Crossfader < 6.0f)
				Out = crossfade(In6, In7, Crossfader - 5.0f);
			else
				Out = crossfade(In7, In8, Crossfader - 6.0f);
			outputs[OUTPUT].setVoltage(Out);

			lights[LIGHT_1].setBrightness(1.0f);
			lights[LIGHT_2].setBrightness(1.0f);
			lights[LIGHT_3].setBrightness(1.0f);
			lights[LIGHT_4].setBrightness(1.0f);
			lights[LIGHT_5].setBrightness(1.0f);
			lights[LIGHT_6].setBrightness(1.0f);
			lights[LIGHT_7].setBrightness(1.0f);
			lights[LIGHT_8].setBrightness(1.0f);
		break;
		case 5:
		Crossfader = clamp(params[MORPH_PARAM].getValue() + inputs[MORPH_CV].getVoltage() / 10.0f, 0.0f, 6.0f);
		Crossfader *= 6.0f;

			if(Crossfader < 1.0f)
				Out = crossfade(In1, In2, Crossfader);
			else if(Crossfader < 2.0f)
				Out = crossfade(In2, In3, Crossfader - 1.0f);
			else if(Crossfader < 3.0f)
				Out = crossfade(In3, In4, Crossfader - 2.0f);
			else if(Crossfader < 4.0f)
				Out = crossfade(In4, In5, Crossfader - 3.0f);
			else if(Crossfader < 5.0f)
				Out = crossfade(In5, In6, Crossfader - 4.0f);
			else
				Out = crossfade(In6, In7, Crossfader - 5.0f);
			outputs[OUTPUT].setVoltage(Out);

			lights[LIGHT_1].setBrightness(1.0f);
			lights[LIGHT_2].setBrightness(1.0f);
			lights[LIGHT_3].setBrightness(1.0f);
			lights[LIGHT_4].setBrightness(1.0f);
			lights[LIGHT_5].setBrightness(1.0f);
			lights[LIGHT_6].setBrightness(1.0f);
			lights[LIGHT_7].setBrightness(1.0f);
			lights[LIGHT_8].setBrightness(0.0f);
		break;
		case 4:
		Crossfader = clamp(params[MORPH_PARAM].getValue() + inputs[MORPH_CV].getVoltage() / 10.0f, 0.0f, 5.0f);
		Crossfader *= 5.0f;

			if(Crossfader < 1.0f)
				Out = crossfade(In1, In2, Crossfader);
			else if(Crossfader < 2.0f)
				Out = crossfade(In2, In3, Crossfader - 1.0f);
			else if(Crossfader < 3.0f)
				Out = crossfade(In3, In4, Crossfader - 2.0f);
			else if(Crossfader < 4.0f)
				Out = crossfade(In4, In5, Crossfader - 3.0f);
			else
				Out = crossfade(In5, In6, Crossfader - 4.0f);
			outputs[OUTPUT].setVoltage(Out);

      lights[LIGHT_1].setBrightness(1.0f);
			lights[LIGHT_2].setBrightness(1.0f);
			lights[LIGHT_3].setBrightness(1.0f);
			lights[LIGHT_4].setBrightness(1.0f);
			lights[LIGHT_5].setBrightness(1.0f);
			lights[LIGHT_6].setBrightness(1.0f);
			lights[LIGHT_7].setBrightness(0.0f);
			lights[LIGHT_8].setBrightness(0.0f);
		break;;
		case 3:
		Crossfader = clamp(params[MORPH_PARAM].getValue() + inputs[MORPH_CV].getVoltage() / 10.0f, 0.0f, 4.0f);
		Crossfader *= 4.0f;

			if(Crossfader < 1.0f)
				Out = crossfade(In1, In2, Crossfader);
			else if(Crossfader < 2.0f)
				Out = crossfade(In2, In3, Crossfader - 1.0f);
			else if(Crossfader < 3.0f)
				Out = crossfade(In3, In4, Crossfader - 2.0f);
			else
				Out = crossfade(In4, In5, Crossfader - 3.0f);
			outputs[OUTPUT].setVoltage(Out);

			lights[LIGHT_1].setBrightness(1.0f);
			lights[LIGHT_2].setBrightness(1.0f);
			lights[LIGHT_3].setBrightness(1.0f);
			lights[LIGHT_4].setBrightness(1.0f);
			lights[LIGHT_5].setBrightness(1.0f);
			lights[LIGHT_6].setBrightness(0.0f);
			lights[LIGHT_7].setBrightness(0.0f);
			lights[LIGHT_8].setBrightness(0.0f);
		break;
		case 2:
		Crossfader = clamp(params[MORPH_PARAM].getValue() + inputs[MORPH_CV].getVoltage() / 10.0f, 0.0f, 3.0f);
		Crossfader *= 3.0f;

			if(Crossfader < 1.0f)
				Out = crossfade(In1, In2, Crossfader);
			else if(Crossfader < 2.0f)
				Out = crossfade(In2, In3, Crossfader - 1.0f);
			else
				Out = crossfade(In3, In4, Crossfader - 2.0f);
			outputs[OUTPUT].setVoltage(Out);

			lights[LIGHT_1].setBrightness(1.0f);
			lights[LIGHT_2].setBrightness(1.0f);
			lights[LIGHT_3].setBrightness(1.0f);
			lights[LIGHT_4].setBrightness(1.0f);
			lights[LIGHT_5].setBrightness(0.0f);
			lights[LIGHT_6].setBrightness(0.0f);
			lights[LIGHT_7].setBrightness(0.0f);
			lights[LIGHT_8].setBrightness(0.0f);
		break;
		case 1:
		Crossfader = clamp(params[MORPH_PARAM].getValue() + inputs[MORPH_CV].getVoltage() / 10.0f, 0.0f, 2.0f);
		Crossfader *= 2.0f;

			if(Crossfader < 1.0f)
				Out = crossfade(In1, In2, Crossfader);
			else
				Out = crossfade(In2, In3, Crossfader - 1.0f);
			outputs[OUTPUT].setVoltage(Out);

			lights[LIGHT_1].setBrightness(1.0f);
			lights[LIGHT_2].setBrightness(1.0f);
			lights[LIGHT_3].setBrightness(1.0f);
			lights[LIGHT_4].setBrightness(0.0f);
			lights[LIGHT_5].setBrightness(0.0f);
			lights[LIGHT_6].setBrightness(0.0f);
			lights[LIGHT_7].setBrightness(0.0f);
			lights[LIGHT_8].setBrightness(0.0f);
		break;
		case 0:
		Crossfader = clamp(params[MORPH_PARAM].getValue() + inputs[MORPH_CV].getVoltage() / 10.0f, 0.0f, 1.0f);
		Crossfader *= 1.0f;
			Out = crossfade(In1, In2, Crossfader);
			outputs[OUTPUT].setVoltage(Out);

			lights[LIGHT_1].setBrightness(1.0f);
			lights[LIGHT_2].setBrightness(1.0f);
			lights[LIGHT_3].setBrightness(0.0f);
			lights[LIGHT_4].setBrightness(0.0f);
			lights[LIGHT_5].setBrightness(0.0f);
			lights[LIGHT_6].setBrightness(0.0f);
			lights[LIGHT_7].setBrightness(0.0f);
			lights[LIGHT_8].setBrightness(0.0f);
		break;
	}



};

struct MorpherWidget : ModuleWidget {
	// Panel Themes
	SvgPanel *panelClassic;
	SvgPanel *panelNightMode;

#ifndef USING_CARDINAL_NOT_RACK
  void appendContextMenu(Menu *menu) override {
  	Morpher *morpher = dynamic_cast<Morpher*>(module);
  	assert(morpher);

    struct morphClassicMenu : MenuItem {
    	Morpher *morpher;
    	void onAction(const event::Action &e) override {
    		morpher->Theme = 0;
    	}
    	void step() override {
    		rightText = (morpher->Theme == 0) ? "✔" : "";
    		MenuItem::step();
    	}
    };

    struct morphNightModeMenu : MenuItem {
    	Morpher *morpher;
    	void onAction(const event::Action &e) override {
    		morpher->Theme = 1;
    	}
    	void step() override {
    		rightText = (morpher->Theme == 1) ? "✔" : "";
    		MenuItem::step();
    	}
    };

    menu->addChild(construct<MenuEntry>());
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Theme"));
    menu->addChild(construct<morphClassicMenu>(&morphClassicMenu::text, "Classic (default)", &morphClassicMenu::morpher, morpher));
    menu->addChild(construct<morphNightModeMenu>(&morphNightModeMenu::text, "Night Mode", &morphNightModeMenu::morpher, morpher));
  }
#endif

	MorpherWidget(Morpher *module);
	void step() override;
};

MorpherWidget::MorpherWidget(Morpher *module) {
		setModule(module);
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	panelClassic = new SvgPanel();
	panelClassic->box.size = box.size;
	panelClassic->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/Morpher.svg")));
  panelClassic->visible = !THEME_DEFAULT;
	addChild(panelClassic);

	panelNightMode = new SvgPanel();
	panelNightMode->box.size = box.size;
	panelNightMode->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/Morpher-Dark.svg")));
  panelNightMode->visible = !!THEME_DEFAULT;
	addChild(panelNightMode);

  addChild(createWidget<MScrewB>(Vec(0, 0)));
  addChild(createWidget<MScrewA>(Vec(box.size.x - 15, 0)));
  addChild(createWidget<MScrewC>(Vec(0, 365)));
  addChild(createWidget<MScrewD>(Vec(box.size.x - 15, 365)));

	addParam(createParam<GreenSmallKnob>(Vec(30, 75), module, Morpher::MORPH_PARAM));

	addParam(createParam<GreenToggleKnobSmall>(Vec(31, 35), module, Morpher::SWITCHCOUNT));

  addInput(createInput<SilverSixPortA>(Vec(14, 128), module, Morpher::INPUT1));
  addInput(createInput<SilverSixPort>(Vec(52, 128), module, Morpher::INPUT2));
	addInput(createInput<SilverSixPortB>(Vec(14, 178), module, Morpher::INPUT3));
  addInput(createInput<SilverSixPortD>(Vec(52, 178), module, Morpher::INPUT4));
	addInput(createInput<SilverSixPortA>(Vec(14, 228), module, Morpher::INPUT5));
  addInput(createInput<SilverSixPort>(Vec(52, 228), module, Morpher::INPUT6));
	addInput(createInput<SilverSixPortB>(Vec(14, 278), module, Morpher::INPUT7));
  addInput(createInput<SilverSixPortD>(Vec(52, 278), module, Morpher::INPUT8));

	addInput(createInput<SilverSixPort>(Vec(14, 328), module, Morpher::MORPH_CV));

  addOutput(createOutput<SilverSixPortE>(Vec(52, 328), module, Morpher::OUTPUT));

	addChild(createLight<SmallLight<BlueLight>>(Vec(24, 120), module, Morpher::LIGHT_1));
	addChild(createLight<SmallLight<BlueLight>>(Vec(62, 120), module, Morpher::LIGHT_2));
	addChild(createLight<SmallLight<BlueLight>>(Vec(24, 170), module, Morpher::LIGHT_3));
	addChild(createLight<SmallLight<BlueLight>>(Vec(62, 170), module, Morpher::LIGHT_4));
	addChild(createLight<SmallLight<BlueLight>>(Vec(24, 220), module, Morpher::LIGHT_5));
	addChild(createLight<SmallLight<BlueLight>>(Vec(62, 220), module, Morpher::LIGHT_6));
	addChild(createLight<SmallLight<BlueLight>>(Vec(24, 270), module, Morpher::LIGHT_7));
	addChild(createLight<SmallLight<BlueLight>>(Vec(62, 270), module, Morpher::LIGHT_8));
};

void MorpherWidget::step() {
#ifdef USING_CARDINAL_NOT_RACK
	panelClassic->visible = !settings::preferDarkPanels;
	panelNightMode->visible = settings::preferDarkPanels;
#else
  if (module) {
  	Morpher *morpher = dynamic_cast<Morpher*>(module);
  	assert(morpher);
  	panelClassic->visible = (morpher->Theme == 0);
  	panelNightMode->visible = (morpher->Theme == 1);
  }
#endif
	ModuleWidget::step();
}


Model *modelMorpher = createModel<Morpher, MorpherWidget>("Morph");
