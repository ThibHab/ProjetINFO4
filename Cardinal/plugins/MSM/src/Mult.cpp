#include "MSM.hpp"
#include "MSMComponentLibrary.hpp"

struct Mult : Module
{
		enum ParamIds {
			MODE1_PARAM,
			MODE2_PARAM,
			MODE3_PARAM,
			NUM_PARAMS
		};

		enum InputIds {
			IN_1_INPUT,
			IN_2_INPUT,
			IN_3_INPUT,
			IN_4_INPUT,
			IN_5_INPUT,
			IN_6_INPUT,
			NUM_INPUTS
		};

		enum OutputIds {
			OUT_11_OUTPUT,
			OUT_12_OUTPUT,
			OUT_13_OUTPUT,
			OUT_14_OUTPUT,
			OUT_15_OUTPUT,
			OUT_16_OUTPUT,
			OUT_21_OUTPUT,
			OUT_22_OUTPUT,
			OUT_23_OUTPUT,
			OUT_24_OUTPUT,
			OUT_25_OUTPUT,
			OUT_26_OUTPUT,
			OUT_31_OUTPUT,
			OUT_32_OUTPUT,
			OUT_33_OUTPUT,
			OUT_34_OUTPUT,
			OUT_35_OUTPUT,
			OUT_36_OUTPUT,
			NUM_OUTPUTS

		};

		int Theme = THEME_DEFAULT;

		Mult() {
			config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
			configParam(Mult::MODE1_PARAM, 0.0, 1.0, 1.0, "Add/Sub");
			configParam(Mult::MODE2_PARAM, 0.0, 1.0, 1.0, "Add/Sub");
			configParam(Mult::MODE3_PARAM, 0.0, 1.0, 1.0, "Add/Sub");
			configInput(IN_1_INPUT, "A1");
			configInput(IN_2_INPUT, "A2");
			configInput(IN_3_INPUT, "B1");
			configInput(IN_4_INPUT, "B2");
			configInput(IN_5_INPUT, "C1");
			configInput(IN_6_INPUT, "C2");
			configOutput(OUT_11_OUTPUT, "A1");
			configOutput(OUT_12_OUTPUT, "A2");
			configOutput(OUT_13_OUTPUT, "A3");
			configOutput(OUT_14_OUTPUT, "A4");
			configOutput(OUT_15_OUTPUT, "A5");
			configOutput(OUT_16_OUTPUT, "A6");
			configOutput(OUT_21_OUTPUT, "B1");
			configOutput(OUT_22_OUTPUT, "B2");
			configOutput(OUT_23_OUTPUT, "B3");
			configOutput(OUT_24_OUTPUT, "B4");
			configOutput(OUT_25_OUTPUT, "B5");
			configOutput(OUT_26_OUTPUT, "B6");
			configOutput(OUT_31_OUTPUT, "C1");
			configOutput(OUT_32_OUTPUT, "C2");
			configOutput(OUT_33_OUTPUT, "C3");
			configOutput(OUT_34_OUTPUT, "C4");
			configOutput(OUT_35_OUTPUT, "C5");
			configOutput(OUT_36_OUTPUT, "C6");
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

void Mult::process(const ProcessArgs& args) {

	double IN_1, IN_2, IN_3;

	char IN_1Mode = params[MODE1_PARAM].getValue() > 0.0;
	char IN_2Mode = params[MODE2_PARAM].getValue() > 0.0;
	char IN_3Mode = params[MODE3_PARAM].getValue() > 0.0;

	switch(IN_1Mode) {
		case 1:
		IN_1 = inputs[IN_1_INPUT].getVoltage() + inputs[IN_2_INPUT].getVoltage();
		break;
		default:
		IN_1 = inputs[IN_1_INPUT].getVoltage() - inputs[IN_2_INPUT].getVoltage();
		break;
	}

	switch(IN_2Mode) {
		case 1:
		IN_2 = inputs[IN_3_INPUT].getVoltage() + inputs[IN_4_INPUT].getVoltage();
		break;
		default:
		IN_2 = inputs[IN_3_INPUT].getVoltage() - inputs[IN_4_INPUT].getVoltage();
		break;
	}
	switch(IN_3Mode) {
		case 1:
		IN_3 = inputs[IN_5_INPUT].getVoltage() + inputs[IN_6_INPUT].getVoltage();
		break;
		default:
		IN_3 = inputs[IN_5_INPUT].getVoltage() - inputs[IN_6_INPUT].getVoltage();
		break;
	}



	//Mult1
	outputs[OUT_11_OUTPUT].setVoltage(IN_1);
	outputs[OUT_12_OUTPUT].setVoltage(IN_1);
	outputs[OUT_13_OUTPUT].setVoltage(IN_1);
	outputs[OUT_14_OUTPUT].setVoltage(IN_1);
	outputs[OUT_15_OUTPUT].setVoltage(IN_1);
	outputs[OUT_16_OUTPUT].setVoltage(IN_1);

	//Mult2
	outputs[OUT_21_OUTPUT].setVoltage(IN_2);
	outputs[OUT_22_OUTPUT].setVoltage(IN_2);
	outputs[OUT_23_OUTPUT].setVoltage(IN_2);
	outputs[OUT_24_OUTPUT].setVoltage(IN_2);
	outputs[OUT_25_OUTPUT].setVoltage(IN_2);
	outputs[OUT_26_OUTPUT].setVoltage(IN_2);

	//Mult3
	outputs[OUT_31_OUTPUT].setVoltage(IN_3);
	outputs[OUT_32_OUTPUT].setVoltage(IN_3);
	outputs[OUT_33_OUTPUT].setVoltage(IN_3);
	outputs[OUT_34_OUTPUT].setVoltage(IN_3);
	outputs[OUT_35_OUTPUT].setVoltage(IN_3);
	outputs[OUT_36_OUTPUT].setVoltage(IN_3);
};

struct MultClassicMenu : MenuItem {
	Mult *mult;
	void onAction(const event::Action &e) override {
		mult->Theme = 0;
	}
	void step() override {
		rightText = (mult->Theme == 0) ? "✔" : "";
		MenuItem::step();
	}
};

struct MultNightModeMenu : MenuItem {
	Mult *mult;
	void onAction(const event::Action &e) override {
		mult->Theme = 1;
	}
	void step() override {
		rightText = (mult->Theme == 1) ? "✔" : "";
		MenuItem::step();
	}
};

struct MultWidget : ModuleWidget {
	// Panel Themes
	SvgPanel *panelClassic;
	SvgPanel *panelNightMode;

#ifndef USING_CARDINAL_NOT_RACK
	void appendContextMenu(Menu *menu) override {
		Mult *mult = dynamic_cast<Mult*>(module);
		assert(mult);

		menu->addChild(construct<MenuEntry>());
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Theme"));
		menu->addChild(construct<MultClassicMenu>(&MultClassicMenu::text, "Classic (default)", &MultClassicMenu::mult, mult));
		menu->addChild(construct<MultNightModeMenu>(&MultNightModeMenu::text, "Night Mode", &MultNightModeMenu::mult, mult));
	}
#endif

	MultWidget(Mult *module);
	void step() override;
};

MultWidget::MultWidget(Mult *module) {
		setModule(module);
	box.size = Vec(9 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	panelClassic = new SvgPanel();
	panelClassic->box.size = box.size;
	panelClassic->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/Mult.svg")));
	panelClassic->visible = !THEME_DEFAULT;
	addChild(panelClassic);

	panelNightMode = new SvgPanel();
	panelNightMode->box.size = box.size;
	panelNightMode->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/Mult-Dark.svg")));
	panelNightMode->visible = !!THEME_DEFAULT;
	addChild(panelNightMode);

  addChild(createWidget<MScrewA>(Vec(15, 0)));
  addChild(createWidget<MScrewA>(Vec(box.size.x - 30, 0)));
  addChild(createWidget<MScrewB>(Vec(15, 365)));
  addChild(createWidget<MScrewD>(Vec(box.size.x - 30, 365)));

	addInput(createInput<SilverSixPortC>(Vec(15, 30), module, Mult::IN_1_INPUT));
	addInput(createInput<SilverSixPortD>(Vec(15, 60), module, Mult::IN_2_INPUT));
	addInput(createInput<SilverSixPortE>(Vec(55, 30), module, Mult::IN_3_INPUT));
	addInput(createInput<SilverSixPortA>(Vec(55, 60), module, Mult::IN_4_INPUT));
	addInput(createInput<SilverSixPort>(Vec(95, 30), module, Mult::IN_5_INPUT));
	addInput(createInput<SilverSixPortB>(Vec(95, 60), module, Mult::IN_6_INPUT));

	addParam(createParam<VioMSwitch>(Vec(20, 94), module, Mult::MODE1_PARAM));
	addParam(createParam<VioMSwitch>(Vec(60, 94), module, Mult::MODE2_PARAM));
	addParam(createParam<VioMSwitch>(Vec(100, 94), module, Mult::MODE3_PARAM));

	addOutput(createOutput<SilverSixPortA>(Vec(15, 120), module, Mult::OUT_11_OUTPUT));
	addOutput(createOutput<SilverSixPort>(Vec(15, 160), module, Mult::OUT_12_OUTPUT));
	addOutput(createOutput<SilverSixPortC>(Vec(15, 200), module, Mult::OUT_13_OUTPUT));
	addOutput(createOutput<SilverSixPortB>(Vec(15, 240), module, Mult::OUT_14_OUTPUT));
	addOutput(createOutput<SilverSixPortA>(Vec(15, 280), module, Mult::OUT_15_OUTPUT));
	addOutput(createOutput<SilverSixPort>(Vec(15, 320), module, Mult::OUT_16_OUTPUT));

	addOutput(createOutput<SilverSixPortE>(Vec(55, 120), module, Mult::OUT_21_OUTPUT));
	addOutput(createOutput<SilverSixPortD>(Vec(55, 160), module, Mult::OUT_22_OUTPUT));
	addOutput(createOutput<SilverSixPort>(Vec(55, 200), module, Mult::OUT_23_OUTPUT));
	addOutput(createOutput<SilverSixPortB>(Vec(55, 240), module, Mult::OUT_24_OUTPUT));
	addOutput(createOutput<SilverSixPortE>(Vec(55, 280), module, Mult::OUT_25_OUTPUT));
	addOutput(createOutput<SilverSixPort>(Vec(55, 320), module, Mult::OUT_26_OUTPUT));

	addOutput(createOutput<SilverSixPort>(Vec(95, 120), module, Mult::OUT_31_OUTPUT));
	addOutput(createOutput<SilverSixPortA>(Vec(95, 160), module, Mult::OUT_32_OUTPUT));
	addOutput(createOutput<SilverSixPortE>(Vec(95, 200), module, Mult::OUT_33_OUTPUT));
	addOutput(createOutput<SilverSixPort>(Vec(95, 240), module, Mult::OUT_34_OUTPUT));
	addOutput(createOutput<SilverSixPortC>(Vec(95, 280), module, Mult::OUT_35_OUTPUT));
	addOutput(createOutput<SilverSixPort>(Vec(95, 320), module, Mult::OUT_36_OUTPUT));
};

void MultWidget::step() {
#ifdef USING_CARDINAL_NOT_RACK
	panelClassic->visible = !settings::preferDarkPanels;
	panelNightMode->visible = settings::preferDarkPanels;
#else
	if (module) {
		Mult *mult = dynamic_cast<Mult*>(module);
		assert(mult);
		panelClassic->visible = (mult->Theme == 0);
		panelNightMode->visible = (mult->Theme == 1);
	}
#endif
	ModuleWidget::step();
}

Model *modelMult = createModel<Mult, MultWidget>("Mult");
