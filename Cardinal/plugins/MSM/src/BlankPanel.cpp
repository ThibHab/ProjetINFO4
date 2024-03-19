#include "MSM.hpp"
#include "MSMComponentLibrary.hpp"

struct BlankPanel : Module {

	enum ParamIds {
		NUM_PARAMS
	};

	enum InputIds {
		NUM_INPUTS
	};

	enum OutputIds {
		NUM_OUTPUTS
	};

	 enum LightIds {
        NUM_LIGHTS
	};

	// Panel Theme
	int Theme = THEME_DEFAULT;

	BlankPanel() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs& args) override {}

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

struct BlankClassicMenu : MenuItem {
	BlankPanel *blankpanel;
	void onAction(const event::Action &e) override {
		blankpanel->Theme = 0;
	}
	void step() override {
		rightText = (blankpanel->Theme == 0) ? "✔" : "";
		MenuItem::step();
	}
};

struct BlankNightModeMenu : MenuItem {
	BlankPanel *blankpanel;
	void onAction(const event::Action &e) override {
		blankpanel->Theme = 1;
	}
	void step() override {
		rightText = (blankpanel->Theme == 1) ? "✔" : "";
		MenuItem::step();
	}
};

struct BlankPanelWidget : ModuleWidget {
	// Panel Themes
	SvgPanel *panelClassic;
	SvgPanel *panelNightMode;

#ifndef USING_CARDINAL_NOT_RACK
	void appendContextMenu(Menu *menu) override {
		BlankPanel *blankpanel = dynamic_cast<BlankPanel*>(module);
		assert(blankpanel);

		menu->addChild(construct<MenuEntry>());
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Theme"));
		menu->addChild(construct<BlankClassicMenu>(&BlankClassicMenu::text, "Classic (default)", &BlankClassicMenu::blankpanel, blankpanel));
		menu->addChild(construct<BlankNightModeMenu>(&BlankNightModeMenu::text, "Night Mode", &BlankNightModeMenu::blankpanel, blankpanel));
	}
#endif

	BlankPanelWidget(BlankPanel *module);
	void step() override;
};

BlankPanelWidget::BlankPanelWidget(BlankPanel *module) {
	setModule(module);
	box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	panelClassic = new SvgPanel();
	panelClassic->box.size = box.size;
	panelClassic->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/BlankPanelLight.svg")));
	panelClassic->visible = !THEME_DEFAULT;
	addChild(panelClassic);

	panelNightMode = new SvgPanel();
	panelNightMode->box.size = box.size;
	panelNightMode->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/BlankPanelDark.svg")));
	panelNightMode->visible = !!THEME_DEFAULT;
	addChild(panelNightMode);

	addChild(createWidget<MScrewD>(Vec(15, 0)));
	addChild(createWidget<MScrewA>(Vec(15, 365)));
	addChild(createWidget<MScrewC>(Vec(90, 0)));
	addChild(createWidget<MScrewB>(Vec(90, 365)));

};

void BlankPanelWidget::step() {
#ifdef USING_CARDINAL_NOT_RACK
	panelClassic->visible = !settings::preferDarkPanels;
	panelNightMode->visible = settings::preferDarkPanels;
#else
	if (module) {
		BlankPanel *blankpanel = dynamic_cast<BlankPanel*>(module);
		assert(blankpanel);
		panelClassic->visible = (blankpanel->Theme == 0);
		panelNightMode->visible = (blankpanel->Theme == 1);
	}
#endif
	ModuleWidget::step();
}

Model *modelBlankPanel = createModel<BlankPanel, BlankPanelWidget>("BlankPanel");
