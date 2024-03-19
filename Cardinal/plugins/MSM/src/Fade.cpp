#include "MSM.hpp"
#include "Additional/Resources.hpp"
#include "MSMComponentLibrary.hpp"

struct Fade : Module {

	enum ParamIds {
		CF_A_PARAM,
		CF_B_PARAM,
		CF_AB_PARAM,
		NUM_PARAMS
	};

	enum InputIds {
		IN_1_INPUT,
		IN_2_INPUT,
		IN_3_INPUT,
		IN_4_INPUT,
		CVA_INPUT,
		CVB_INPUT,
		CVAB_INPUT,
		NUM_INPUTS
	};

	enum OutputIds {
		OUT_A_OUTPUT,
		OUT_B_OUTPUT,
		OUT_AB_OUTPUT,
		NUM_OUTPUTS
	};

	 enum LightIds {
        NUM_LIGHTS
	};

	int Theme = THEME_DEFAULT;

	Fade() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(Fade::CF_A_PARAM, 0.0, 1.0, 0.5, "A");
		configParam(Fade::CF_B_PARAM, 0.0, 1.0, 0.5, "B");
		configParam(Fade::CF_AB_PARAM, 0.0, 1.0, 0.5, "AB");
		configInput(IN_1_INPUT, "(A) Fader #1");
		configInput(IN_2_INPUT, "(A) Fader #2");
		configInput(IN_3_INPUT, "(B) Fader #3");
		configInput(IN_4_INPUT, "(B) Fader #4");
		configInput(CVA_INPUT, "(A) Fader Control Voltage");
		configInput(CVB_INPUT, "(B) Fader Control Voltage");
		configInput(CVAB_INPUT, "(AB) Fader Control Voltage");
		configOutput(OUT_A_OUTPUT, "(A) Fader");
		configOutput(OUT_B_OUTPUT, "(B) Fader");
		configOutput(OUT_AB_OUTPUT, "(AB) Fader");
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

void Fade::process(const ProcessArgs& args)
{
	double CrossfadeA = clamp(params[CF_A_PARAM].getValue() + inputs[CVA_INPUT].getVoltage() / 10.f, 0.0f, 1.0f);
	double IN_1 = inputs[IN_1_INPUT].getVoltage();
	double IN_2 = inputs[IN_2_INPUT].getVoltage();
	double OutA;

	OutA = crossfade(IN_1, IN_2, CrossfadeA);
	outputs[OUT_A_OUTPUT].setVoltage(OutA);


	double CrossfadeB = clamp(params[CF_B_PARAM].getValue() + inputs[CVB_INPUT].getVoltage() / 10.f, 0.0f, 1.0f);
	double IN_3 = inputs[IN_3_INPUT].getVoltage();
	double IN_4 = inputs[IN_4_INPUT].getVoltage();
	double OutB;

	OutB = crossfade(IN_3, IN_4, CrossfadeB);
	outputs[OUT_B_OUTPUT].setVoltage(OutB);


	double CrossfadeAB = clamp(params[CF_AB_PARAM].getValue() + inputs[CVAB_INPUT].getVoltage() / 10.f, 0.0f, 1.0f);
	double IN_A = OutA;
	double IN_B = OutB;
	double OutAB;

	OutAB = crossfade(IN_A, IN_B, CrossfadeAB);
	outputs[OUT_AB_OUTPUT].setVoltage(OutAB);


};

struct FadeClassicMenu : MenuItem {
	Fade *fade;
	void onAction(const event::Action &e) override {
		fade->Theme = 0;
	}
	void step() override {
		rightText = (fade->Theme == 0) ? "✔" : "";
		MenuItem::step();
	}
};

struct FadekNightModeMenu : MenuItem {
	Fade *fade;
	void onAction(const event::Action &e) override {
		fade->Theme = 1;
	}
	void step() override {
		rightText = (fade->Theme == 1) ? "✔" : "";
		MenuItem::step();
	}
};

struct FadeWidget : ModuleWidget {
	// Panel Themes
	SvgPanel *panelClassic;
	SvgPanel *panelNightMode;

#ifndef USING_CARDINAL_NOT_RACK
	void appendContextMenu(Menu *menu) override {
		Fade *fade = dynamic_cast<Fade*>(module);
		assert(fade);

		menu->addChild(construct<MenuEntry>());
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Theme"));
		menu->addChild(construct<FadeClassicMenu>(&FadeClassicMenu::text, "Classic (default)", &FadeClassicMenu::fade, fade));
		menu->addChild(construct<FadekNightModeMenu>(&FadekNightModeMenu::text, "Night Mode", &FadekNightModeMenu::fade, fade));
	}
#endif

	FadeWidget(Fade *module);
	void step() override;
};

FadeWidget::FadeWidget(Fade *module) {
	setModule(module);
	box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	panelClassic = new SvgPanel();
	panelClassic->box.size = box.size;
	panelClassic->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/Fade.svg")));
	panelClassic->visible = !THEME_DEFAULT;
	addChild(panelClassic);

	panelNightMode = new SvgPanel();
	panelNightMode->box.size = box.size;
	panelNightMode->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/Fade-Dark.svg")));
	panelNightMode->visible = !!THEME_DEFAULT;
	addChild(panelNightMode);

	addChild(createWidget<MScrewB>(Vec(15, 0)));
	addChild(createWidget<MScrewA>(Vec(15, 365)));
	addChild(createWidget<MScrewC>(Vec(90, 0)));
	addChild(createWidget<MScrewD>(Vec(90, 365)));

	//Params
	addParam(createParam<GreenLargeKnob>(Vec(10, 65), module, Fade::CF_A_PARAM));
	addParam(createParam<GreenLargeKnob>(Vec(64, 65), module, Fade::CF_B_PARAM));
	addParam(createParam<GreenLargeKnob>(Vec(38, 110), module, Fade::CF_AB_PARAM));

	//Inputs
	addInput(createInput<SilverSixPortA>(Vec(10, 240), module, Fade::IN_1_INPUT));
	addInput(createInput<SilverSixPort>(Vec(49, 240), module, Fade::IN_2_INPUT));
	addInput(createInput<SilverSixPortB>(Vec(10, 280), module, Fade::IN_3_INPUT));
	addInput(createInput<SilverSixPortC>(Vec(49, 280), module, Fade::IN_4_INPUT));

	addInput(createInput<SilverSixPortD>(Vec(10, 320), module, Fade::CVA_INPUT));
	addInput(createInput<SilverSixPortE>(Vec(49, 320), module, Fade::CVB_INPUT));
	addInput(createInput<SilverSixPort>(Vec(49, 200), module, Fade::CVAB_INPUT));

	//Outputs
	addOutput(createOutput<SilverSixPortA>(Vec(88, 240), module, Fade::OUT_A_OUTPUT));
	addOutput(createOutput<SilverSixPort>(Vec(88, 280), module, Fade::OUT_B_OUTPUT));
	addOutput(createOutput<SilverSixPortC>(Vec(88, 320), module, Fade::OUT_AB_OUTPUT));
};

void FadeWidget::step() {
#ifdef USING_CARDINAL_NOT_RACK
	panelClassic->visible = !settings::preferDarkPanels;
	panelNightMode->visible = settings::preferDarkPanels;
#else
	if (module) {
		Fade *fade = dynamic_cast<Fade*>(module);
		assert(fade);
		panelClassic->visible = (fade->Theme == 0);
		panelNightMode->visible = (fade->Theme == 1);
	}
#endif
	ModuleWidget::step();
}

Model *modelFade = createModel<Fade, FadeWidget>("Fade");
