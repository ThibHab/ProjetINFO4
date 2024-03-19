#include "MSM.hpp"
#include "MSMComponentLibrary.hpp"

struct SimpleSlider : Module {

	enum ParamIds {
		SLIDER_PARAM,
		TYPE_PARAM,
		NUM_PARAMS
	};

	enum InputIds {
		IN1_INPUT,
		IN2_INPUT,
		CV_INPUT,
		NUM_INPUTS
	};

	enum OutputIds {
		MAIN_OUTPUT,
		NUM_OUTPUTS
	};

	int Theme = THEME_DEFAULT;

	SimpleSlider() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		configParam(SimpleSlider::SLIDER_PARAM, 0.0, 1.0, 0.5, "Slider");
		configParam(SimpleSlider::TYPE_PARAM, 0.0, 1.0, 0.0, "Type");
		configInput(IN1_INPUT, "#1");
		configInput(IN2_INPUT, "#2");
		configInput(CV_INPUT, "Slider Position Control Voltage");
		configOutput(MAIN_OUTPUT, "Master");
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

void SimpleSlider::process(const ProcessArgs& args) {

	double SLIDER = clamp(params[SLIDER_PARAM].getValue() + inputs[CV_INPUT].getVoltage() / 10.f, 0.0f, 1.0f);
	double IN1 = inputs[IN1_INPUT].getVoltage();
	double IN2 = inputs[IN2_INPUT].getVoltage();
	double TYPE = params[TYPE_PARAM].getValue();
	double OUT = outputs[MAIN_OUTPUT].getVoltage();

	if(TYPE == 0.0f) {
		OUT = crossfade(IN1, IN2, SLIDER);
	}
	else {
		OUT = (IN1 + IN2) * SLIDER;
	}
	outputs[MAIN_OUTPUT].setVoltage(OUT);


};

struct SimpleClassicMenu : MenuItem {
	SimpleSlider *simpleslider;
	void onAction(const event::Action &e) override {
		simpleslider->Theme = 0;
	}
	void step() override {
		rightText = (simpleslider->Theme == 0) ? "✔" : "";
		MenuItem::step();
	}
};

struct SimpleNightModeMenu : MenuItem {
	SimpleSlider *simpleslider;
	void onAction(const event::Action &e) override {
		simpleslider->Theme = 1;
	}
	void step() override {
		rightText = (simpleslider->Theme == 1) ? "✔" : "";
		MenuItem::step();
	}
};



struct SimpleSliderWidget : ModuleWidget {
	// Panel Themes
	SvgPanel *panelClassic;
	SvgPanel *panelNightMode;

#ifndef USING_CARDINAL_NOT_RACK
	void appendContextMenu(Menu *menu) override {
		SimpleSlider *simpleslider = dynamic_cast<SimpleSlider*>(module);
		assert(simpleslider);
		menu->addChild(construct<MenuEntry>());
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Theme"));
		menu->addChild(construct<SimpleClassicMenu>(&SimpleClassicMenu::text, "Classic (default)", &SimpleClassicMenu::simpleslider, simpleslider));
		menu->addChild(construct<SimpleNightModeMenu>(&SimpleNightModeMenu::text, "Night Mode", &SimpleNightModeMenu::simpleslider, simpleslider));
	}
#endif

	SimpleSliderWidget(SimpleSlider *module);
	void step() override;
};

SimpleSliderWidget::SimpleSliderWidget(SimpleSlider *module) {
		setModule(module);
	box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	panelClassic = new SvgPanel();
	panelClassic->box.size = box.size;
	panelClassic->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/SimpleSlider.svg")));
	panelClassic->visible = !THEME_DEFAULT;
	addChild(panelClassic);

	panelNightMode = new SvgPanel();
	panelNightMode->box.size = box.size;
	panelNightMode->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/SimpleSlider-Dark.svg")));
	panelNightMode->visible = !!THEME_DEFAULT;
	addChild(panelNightMode);


	addChild(createWidget<MScrewA>(Vec(15, 0)));
	addChild(createWidget<MScrewB>(Vec(15, 365)));
	addChild(createWidget<MScrewC>(Vec(90, 0)));
	addChild(createWidget<MScrewA>(Vec(90, 365)));

	addParam(createParam<MSMSlider>(Vec(32, 90), module, SimpleSlider::SLIDER_PARAM));

	addParam(createParam<VioMSwitch>(Vec(54.5, 40), module, SimpleSlider::TYPE_PARAM));

	addInput(createInput<SilverSixPortA>(Vec(8, 338), module, SimpleSlider::IN1_INPUT));
	addInput(createInput<SilverSixPortE>(Vec(48, 338), module, SimpleSlider::IN2_INPUT));

	addInput(createInput<SilverSixPortC>(Vec(88, 338), module, SimpleSlider::CV_INPUT));

	addOutput(createOutput<SilverSixPortD>(Vec(47, 299), module, SimpleSlider::MAIN_OUTPUT));
};

void SimpleSliderWidget::step() {
#ifdef USING_CARDINAL_NOT_RACK
	panelClassic->visible = !settings::preferDarkPanels;
	panelNightMode->visible = settings::preferDarkPanels;
#else
	if (module) {
		SimpleSlider *simpleslider = dynamic_cast<SimpleSlider*>(module);
		assert(simpleslider);
		panelClassic->visible = (simpleslider->Theme == 0);
		panelNightMode->visible = (simpleslider->Theme == 1);
	}
#endif
	ModuleWidget::step();
}

Model *modelSimpleSlider = createModel<SimpleSlider, SimpleSliderWidget>("SimpleSlider");
