#include "MSM.hpp"
#include "MSMComponentLibrary.hpp"

struct xseq : Module {

	enum ParamIds {
		BUT1_PARAM,
		BUT2_PARAM,
		BUT3_PARAM,
		BUT4_PARAM,
		BUT5_PARAM,
		BUT6_PARAM,
		BUT7_PARAM,
		BUT8_PARAM,
		BUT9_PARAM,
		NUM_PARAMS
	};

	enum InputIds {
		IN1_INPUT,
		IN2_INPUT,
		IN3_INPUT,

		B1_INPUT,
		B2_INPUT,
		B3_INPUT,
		B4_INPUT,
		B5_INPUT,
		B6_INPUT,
		B7_INPUT,
		B8_INPUT,
		B9_INPUT,

		NUM_INPUTS
	};

	enum OutputIds {
		AND1_OUTPUT,
		AND2_OUTPUT,
		OR1_OUTPUT,
		OR2_OUTPUT,
		NOR1_OUTPUT,
		NOR2_OUTPUT,
		NAND1_OUTPUT,
		NAND2_OUTPUT,
		XOR1_OUTPUT,
		XNOR1_OUTPUT,
		NOT1_OUTPUT,
		NOT2_OUTPUT,
		NOT3_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightIds {
		NOR1_LIGHT,
		NAND1_LIGHT,
		OR1_LIGHT,
		NAND2_LIGHT,
		AND1_LIGHT,
		AND2_LIGHT,
		XOR1_LIGHT,
		NOR2_LIGHT,
		XNOR1_LIGHT,
		NUM_LIGHTS
	};

	bool IN_1 = false;
	bool IN_2 = false;
	bool IN_3 = false;
	dsp::SchmittTrigger trigger1;
	dsp::SchmittTrigger trigger2;
	dsp::SchmittTrigger trigger3;

	int Theme = THEME_DEFAULT;

	xseq() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(xseq::BUT1_PARAM, 0.0, 1.0, 0.0, "Logic Select");
		configParam(xseq::BUT2_PARAM, 0.0, 1.0, 0.0, "Logic Select");
		configParam(xseq::BUT3_PARAM, 0.0, 1.0, 0.0, "Logic Select");
		configParam(xseq::BUT4_PARAM, 0.0, 1.0, 0.0, "Logic Select");
		configParam(xseq::BUT5_PARAM, 0.0, 1.0, 0.0, "Logic Select");
		configParam(xseq::BUT6_PARAM, 0.0, 1.0, 0.0, "Logic Select");
		configParam(xseq::BUT7_PARAM, 0.0, 1.0, 0.0, "Logic Select");
		configParam(xseq::BUT8_PARAM, 0.0, 1.0, 0.0, "Logic Select");
		configParam(xseq::BUT9_PARAM, 0.0, 1.0, 0.0, "Logic Select");
		configInput(IN1_INPUT, "Clock/LFO #1");
		configInput(IN2_INPUT, "Clock/LFO #2");
		configInput(IN3_INPUT, "Clock/LFO #3");
		configOutput(NOR1_OUTPUT, "#1");
		configOutput(NAND1_OUTPUT, "#2");
		configOutput(OR1_OUTPUT, "#3");
		configOutput(NAND2_OUTPUT, "#4");
		configOutput(AND1_OUTPUT, "#5");
		configOutput(AND2_OUTPUT, "#6");
		configOutput(XOR1_OUTPUT, "#7");
		configOutput(NOR2_OUTPUT, "#8");
		configOutput(XNOR1_OUTPUT, "#9");
		configInput(B1_INPUT, "Button #1 Control Voltage");
		configInput(B2_INPUT, "Button #2 Control Voltage");
		configInput(B3_INPUT, "Button #3 Control Voltage");
		configInput(B4_INPUT, "Button #4 Control Voltage");
		configInput(B5_INPUT, "Button #5 Control Voltage");
		configInput(B6_INPUT, "Button #6 Control Voltage");
		configInput(B7_INPUT, "Button #7 Control Voltage");
		configInput(B8_INPUT, "Button #8 Control Voltage");
		configInput(B9_INPUT, "Button #9 Control Voltage");
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

void xseq::process(const ProcessArgs& args) {

	bool NOR1 = true, NOR2 = false, NAND1 = false, NAND2 = false, OR1 = false, XOR1 = false, XNOR1 = true, NOT1 = false, NOT2 = false, NOT3 = false, AND1 = false, AND2 = false;

	char Button1 = 0.0f, Button2 = 0.0f, Button3 = 0.0f, Button4 = 0.0f, Button5 = 0.0f, Button6 = 0.0f, Button7 = 0.0f, Button8 = 0.0f, Button9 = false;

	IN_1 = trigger1.process(inputs[IN1_INPUT].getVoltage() >= 10.0f);
	IN_2 = trigger2.process(inputs[IN2_INPUT].getVoltage() >= 10.0f);
	IN_3 = trigger3.process(inputs[IN3_INPUT].getVoltage() >= 10.0f);

	//////////////////////////////////////////////////////////////////////

	// NOR 1 or AND

	IN_1 = false;
	IN_2 = false;
	//NOR1 = true;


	if(inputs[IN1_INPUT].isConnected()) {
		IN_1 = inputs[IN1_INPUT].getVoltage();
	}

	if(inputs[IN2_INPUT].isConnected()) {
		IN_2 = inputs[IN2_INPUT].getVoltage();
	}


	Button1 = clamp(params[BUT1_PARAM].getValue() + dsp::quadraticBipolar(inputs[B1_INPUT].getVoltage()) / 10.0f, -1.0f, 1.0f);

	switch(Button1) {
		case 1:
			NOR1 = IN_1 && IN_2;		// <- AND
		break;
		case 0:
			NOR1 = !(IN_1 || IN_2);		// <- NOR
		break;
	};

	//Output
	if(outputs[NOR1_OUTPUT].isConnected())
       outputs[NOR1_OUTPUT].setVoltage( 10.0 * NOR1);
   lights[NOR1_LIGHT].setBrightness(10.0 * NOR1);

	//////////////////////////////////////////////////////////////////////

	// NAND 1 or OR

	IN_1 = false;
	IN_2 = false;
	//NAND1 = false;

	if(inputs[IN1_INPUT].isConnected()) {
		IN_1 = inputs[IN1_INPUT].getVoltage();
	}

	if(inputs[IN3_INPUT].isConnected()) {
		IN_2 = inputs[IN3_INPUT].getVoltage();
	}

	Button2 = clamp(params[BUT2_PARAM].getValue() + dsp::quadraticBipolar(inputs[B2_INPUT].getVoltage()) / 10.0f, -1.0f, 1.0f);

	switch(Button2) {
		case 1:
			NAND1 = IN_1 || IN_2;		// <- OR
		break;
		case 0:
			NAND1 = !(IN_1 && IN_2);	// <- NAND
		break;
	}

	//Output
	if(outputs[NAND1_OUTPUT].isConnected())
	outputs[NAND1_OUTPUT].setVoltage( 10.0 * NAND1);
   lights[NAND1_LIGHT].setBrightness(10.0 * NAND1);

	//////////////////////////////////////////////////////////////////////

	// NOT 1

	IN_1 = false;
	IN_2 = false;
	//NOT1 = false;

	if(NOR1) {
		IN_2 = NOR1;
	}

	NOT1 = !(IN_3); // <- NOT

	//Output
	if(outputs[NOT1_OUTPUT].isConnected())
       outputs[NOT1_OUTPUT].setVoltage(10.0 * NOT1);

	//////////////////////////////////////////////////////////////////////

	// OR 1 or AND

	IN_1 = false;
	IN_2 = false;
	//OR1 = false;

	if(NOR1) {
		IN_1 = NOR1;
	}

	if(NOT1) {
		IN_2 = NOT1;
	}


	Button3 = clamp(params[BUT3_PARAM].getValue() + dsp::quadraticBipolar(inputs[B3_INPUT].getVoltage()) / 10.0f, -1.0f, 1.0f);

	switch(Button3) {
		case 1:
			OR1 = !(IN_1 && IN_2);		// <- NAND
		break;
		case 0:
			OR1 = !(IN_1 ^ IN_2);	// <- XNOR
		break;
	}

	//Output
	if(outputs[OR1_OUTPUT].isConnected())
       outputs[OR1_OUTPUT].setVoltage(10.0 * OR1);
   lights[OR1_LIGHT].setBrightness(10.0 * OR1);

   	//////////////////////////////////////////////////////////////////////

	// XOR 2 or NAND

	IN_1 = false;
	IN_2 = false;
	//NAND2 = false;

	if(NOT1) {
		IN_1 = NOT1;
	}

	if(OR1) {
		IN_2 = OR1;
	}

	Button4 = clamp(params[BUT4_PARAM].getValue() + dsp::quadraticBipolar(inputs[B4_INPUT].getVoltage()) / 10.0f, -1.0f, 1.0f);

	switch(Button4) {
		case 1:
		NAND2 = !(IN_1 && IN_2);		// <- NAND
		break;
		case 0:
			NAND2 = !(IN_1 ^ IN_2);		// <- XNOR
		break;

	}

	//Output
	if(outputs[NAND2_OUTPUT].isConnected())
       outputs[NAND2_OUTPUT].setVoltage(10 * NAND2);
   lights[NAND2_LIGHT].setBrightness(10.0 * NAND2);

   	//////////////////////////////////////////////////////////////////////

	// NOT 2

	IN_1 = false;
	IN_2 = false;
	//NOT2 = false;

	if(NAND1) {
		IN_2 = NAND1;
	}

	NOT2 = !(IN_3); // <- NOT

	//Output
	if(outputs[NOT2_OUTPUT].isConnected())
       outputs[NOT2_OUTPUT].setVoltage(10.0 * NOT2);

	//////////////////////////////////////////////////////////////////////

	// AND 1 or NAND

	IN_1 = false;
	IN_2 = true;
	//AND1 = false;

	if(NOT2) {
		IN_1 = NOT2;
	}

	if(inputs[IN2_INPUT].isConnected()) {
		IN_2 = inputs[IN2_INPUT].getVoltage();
	}

	Button5 = clamp(params[BUT5_PARAM].getValue() + dsp::quadraticBipolar(inputs[B5_INPUT].getVoltage()) / 10.0f, -1.0f, 1.0f);

	switch(Button5) {
		case 1:
			AND1 = !(IN_1 && IN_2);			// <- NAND
		break;
		case 0:
			AND1 = IN_1 && IN_2;		// <- AND
		break;
	}

	//Output
	if(outputs[AND1_OUTPUT].isConnected())
		outputs[AND1_OUTPUT].setVoltage(10.0 * AND1);
   lights[AND1_LIGHT].setBrightness(10.0 * AND1);

	//////////////////////////////////////////////////////////////////////

	// NOT 3

	IN_1 = false;
	IN_2 = false;
	//NOT3 = false;

	if(NAND2) {
		IN_2 = NAND2;
	}

	NOT3 = !(IN_3); // <- NOT

	//Output
	if(outputs[NOT3_OUTPUT].isConnected())
       outputs[NOT3_OUTPUT].setVoltage(10.0 * NOT3);

	//////////////////////////////////////////////////////////////////////

	// AND 2

	IN_1 = false;
	IN_2 = true;
	//AND2 = false;

	if(NOT3) {
		IN_1 = NOT3;
	}

	if(inputs[IN2_INPUT].isConnected()) {
		IN_2 = inputs[IN2_INPUT].getVoltage();
	}

	Button6 = clamp(params[BUT6_PARAM].getValue() + dsp::quadraticBipolar(inputs[B6_INPUT].getVoltage()) / 10.0f, -1.0f, 1.0f);

	switch(Button6) {
		case 1:
			AND2 = !(IN_1 && IN_2) && (IN_1 != IN_2);		// <- NAND AND OR
		break;
		case 0:
			AND2 = IN_1 && IN_2;						// <- AND
		break;
	}

	//Output
	if(outputs[AND2_OUTPUT].isConnected())
		outputs[AND2_OUTPUT].setVoltage(10.0 * AND2);
   lights[AND2_LIGHT].setBrightness(10.0 * AND2);

	//////////////////////////////////////////////////////////////////////

	// XOR 1

	IN_1 = false;
	IN_2 = true;
	//XOR1 = false;

	if(AND2) {
		IN_1 = AND2;
	}

	if(inputs[IN3_INPUT].isConnected()) {
		IN_2 = inputs[IN3_INPUT].getVoltage();
	}

	Button7 = clamp(params[BUT7_PARAM].getValue() + dsp::quadraticBipolar(inputs[B7_INPUT].getVoltage()) / 10.0f, -1.0f, 1.0f);

	switch(Button7) {
		case 1:
			XOR1 = !(IN_1 ^ IN_2); // <- XNOR
		break;
		case 0:
			XOR1 = (IN_1 ^ IN_2);  // <- XOR
		break;
	}

	//Output
	if(outputs[XOR1_OUTPUT].isConnected())
		outputs[XOR1_OUTPUT].setVoltage(10.0 * XOR1);
   lights[XOR1_LIGHT].setBrightness(10.0 * XOR1);

	//////////////////////////////////////////////////////////////////////

	// NOR 2

	IN_1 = true;
	IN_2 = false;
	//NOR2 = false;


	if(AND2) {
		IN_1 = AND2;
	}

	if(OR1) {
		IN_2 = OR1;
	}

	Button8 = clamp(params[BUT8_PARAM].getValue() + dsp::quadraticBipolar(inputs[B8_INPUT].getVoltage()) / 10.0f, -1.0f, 1.0f);

	switch(Button8) {
		case 1:
			NOR2 = IN_1 ^ IN_2;								// <- XOR
		break;
		case 0:
			NOR2 = ((IN_1 && IN_2) || (!IN_1 && IN_3)); // <- AND - OR - AND
		break;
	}

	//Output
	if(outputs[NOR2_OUTPUT].isConnected())
       outputs[NOR2_OUTPUT].setVoltage( 10.0 * NOR2);
   lights[NOR2_LIGHT].setBrightness(10.0 * NOR2);

	//////////////////////////////////////////////////////////////////////

	// XNOR 1

	IN_1 = false;
	IN_2 = false;
	//XNOR1 = true;

	if(inputs[IN1_INPUT].isConnected()) {
		IN_1 = inputs[IN1_INPUT].getVoltage();
	}

	if(XOR1) {
		IN_2 = XOR1;
	}

	Button9 = clamp(params[BUT9_PARAM].getValue() + dsp::quadraticBipolar(inputs[B9_INPUT].getVoltage()) / 10.0f, -1.0f, 1.0f);

	switch(Button9) {
		case 1:
			XNOR1 = !(IN_2);								// <- Not In 2
		break;
		case 0:
			XNOR1 = ((IN_1 && IN_2) || (!IN_1 && !IN_3));	// <- AND - OR - AND
		break;
	}

	//Output
	if(outputs[XNOR1_OUTPUT].isConnected())
		outputs[XNOR1_OUTPUT].setVoltage(10.0 * XNOR1);
   lights[XNOR1_LIGHT].setBrightness(10.0 * XNOR1);

	//////////////////////////////////////////////////////////////////////



};

struct xsClassicMenu : MenuItem {
	xseq *xs;
	void onAction(const event::Action &e) override {
		xs->Theme = 0;
	}
	void step() override {
		rightText = (xs->Theme == 0) ? "✔" : "";
		MenuItem::step();
	}
};

struct xsNightModeMenu : MenuItem {
	xseq *xs;
	void onAction(const event::Action &e) override {
		xs->Theme = 1;
	}
	void step() override {
		rightText = (xs->Theme == 1) ? "✔" : "";
		MenuItem::step();
	}
};

struct xseqWidget : ModuleWidget {
	// Panel Themes
	SvgPanel *panelClassic;
	SvgPanel *panelNightMode;

#ifndef USING_CARDINAL_NOT_RACK
	void appendContextMenu(Menu *menu) override {
		xseq *xs = dynamic_cast<xseq*>(module);
		assert(xs);
		menu->addChild(construct<MenuEntry>());
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Theme"));
		menu->addChild(construct<xsClassicMenu>(&xsClassicMenu::text, "Classic (default)", &xsClassicMenu::xs, xs));
		menu->addChild(construct<xsNightModeMenu>(&xsNightModeMenu::text, "Night Mode", &xsNightModeMenu::xs, xs));
	}
#endif

	xseqWidget(xseq *module);
	void step() override;
};

xseqWidget::xseqWidget(xseq *module) {
	setModule(module);
	box.size = Vec(16 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	panelClassic = new SvgPanel();
	panelClassic->box.size = box.size;
	panelClassic->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/XSEQ-Expension.svg")));
	panelClassic->visible = !THEME_DEFAULT;
	addChild(panelClassic);

	panelNightMode = new SvgPanel();
	panelNightMode->box.size = box.size;
	panelNightMode->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/XSEQ-Expension-Dark.svg")));
	panelNightMode->visible = !!THEME_DEFAULT;
	addChild(panelNightMode);

	addChild(createWidget<MScrewA>(Vec(15, 0)));
	addChild(createWidget<MScrewB>(Vec(15, 365)));
	addChild(createWidget<MScrewD>(Vec(box.size.x-90, 0)));
	addChild(createWidget<MScrewC>(Vec(box.size.x-90, 365)));

	addChild(createWidget<MScrewC>(Vec(box.size.x-15, 0)));
	addChild(createWidget<MScrewD>(Vec(box.size.x-60, 365)));

	addParam(createParam<MToggleGrey>(Vec(19,85), module, xseq::BUT1_PARAM));
	addParam(createParam<MToggleGrey>(Vec(77,85), module, xseq::BUT2_PARAM));
	addParam(createParam<MToggleGrey>(Vec(137,85), module, xseq::BUT3_PARAM));

	addParam(createParam<MToggleGrey>(Vec(19,175), module, xseq::BUT4_PARAM));
	addParam(createParam<MToggleGrey>(Vec(77,175), module, xseq::BUT5_PARAM));
	addParam(createParam<MToggleGrey>(Vec(137,175), module, xseq::BUT6_PARAM));

	addParam(createParam<MToggleGrey>(Vec(19,265), module, xseq::BUT7_PARAM));
	addParam(createParam<MToggleGrey>(Vec(77,265), module, xseq::BUT8_PARAM));
	addParam(createParam<MToggleGrey>(Vec(137,265), module, xseq::BUT9_PARAM));

	addInput(createInput<SilverSixPortB>(Vec(20, 45), module, xseq::IN1_INPUT));
	addInput(createInput<SilverSixPortD>(Vec(78, 45), module, xseq::IN2_INPUT));
	addInput(createInput<SilverSixPortA>(Vec(138, 45), module, xseq::IN3_INPUT));

	addOutput(createOutput<SilverSixPort>(Vec(20, 120), module, xseq::NOR1_OUTPUT));
	addOutput(createOutput<SilverSixPortC>(Vec(78, 120), module, xseq::NAND1_OUTPUT));
	addOutput(createOutput<SilverSixPortE>(Vec(138, 120), module, xseq::OR1_OUTPUT));

	addOutput(createOutput<SilverSixPortB>(Vec(20, 210), module, xseq::NAND2_OUTPUT));
	addOutput(createOutput<SilverSixPortA>(Vec(78, 210), module, xseq::AND1_OUTPUT));
	addOutput(createOutput<SilverSixPort>(Vec(138, 210), module, xseq::AND2_OUTPUT));

	addOutput(createOutput<SilverSixPortC>(Vec(20, 300), module, xseq::XOR1_OUTPUT));
	addOutput(createOutput<SilverSixPortA>(Vec(78, 300), module, xseq::NOR2_OUTPUT));
	addOutput(createOutput<SilverSixPortB>(Vec(138, 300), module, xseq::XNOR1_OUTPUT));

	addChild(createLight<MediumLight<GreenLight>>(Vec(28, 150), module, xseq::NOR1_LIGHT));
	addChild(createLight<MediumLight<GreenLight>>(Vec(86, 150), module, xseq::NAND1_LIGHT));
	addChild(createLight<MediumLight<GreenLight>>(Vec(146, 150), module, xseq::OR1_LIGHT));

	addChild(createLight<MediumLight<GreenLight>>(Vec(28, 240), module, xseq::NAND2_LIGHT));
	addChild(createLight<MediumLight<GreenLight>>(Vec(86, 240), module, xseq::AND1_LIGHT));
	addChild(createLight<MediumLight<GreenLight>>(Vec(146, 240), module, xseq::AND2_LIGHT));

	addChild(createLight<MediumLight<GreenLight>>(Vec(28, 330), module, xseq::XOR1_LIGHT));
	addChild(createLight<MediumLight<GreenLight>>(Vec(86, 330), module, xseq::NOR2_LIGHT));
	addChild(createLight<MediumLight<GreenLight>>(Vec(146, 330), module, xseq::XNOR1_LIGHT));

	addInput(createInput<SilverSixPortB>(Vec(197, 20), module, xseq::B1_INPUT));
	addInput(createInput<SilverSixPortC>(Vec(197, 55), module, xseq::B2_INPUT));
	addInput(createInput<SilverSixPort>(Vec(197, 90), module, xseq::B3_INPUT));

	addInput(createInput<SilverSixPortB>(Vec(197, 125), module, xseq::B4_INPUT));
	addInput(createInput<SilverSixPortC>(Vec(197, 160), module, xseq::B5_INPUT));
	addInput(createInput<SilverSixPort>(Vec(197, 195), module, xseq::B6_INPUT));

	addInput(createInput<SilverSixPortB>(Vec(197, 230), module, xseq::B7_INPUT));
	addInput(createInput<SilverSixPortC>(Vec(197, 265), module, xseq::B8_INPUT));
	addInput(createInput<SilverSixPortD>(Vec(197, 300), module, xseq::B9_INPUT));
};

void xseqWidget::step() {
#ifdef USING_CARDINAL_NOT_RACK
	panelClassic->visible = !settings::preferDarkPanels;
	panelNightMode->visible = settings::preferDarkPanels;
#else
	if (module) {
		xseq *xs = dynamic_cast<xseq*>(module);
		assert(xs);
		panelClassic->visible = (xs->Theme == 0);
		panelNightMode->visible = (xs->Theme == 1);
	}
#endif
	ModuleWidget::step();
}

Model *modelxseq = createModel<xseq, xseqWidget>("XSEQ");
