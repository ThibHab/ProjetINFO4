#include "MSM.hpp"
#include "MSMComponentLibrary.hpp"
#include "Additional/LowFrequencyOscillator.hpp"

struct LFO : Module {

	enum ParamIds {
		SYNCED_PARAM,
		FREQ_PARAM,
		FM_PARAM,
		PW_1_PARAM,
		PWM_1_PARAM,
		OFFSET_1_PARAM,
		INVERT_1_PARAM,
		WAVE_A_PARAM,
		FM_2_PARAM,
		PW_2_PARAM,
		PWM_2_PARAM,
		OFFSET_2_PARAM,
		INVERT_2_PARAM,
		WAVE_B_PARAM,
		MIX_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		FM_1_INPUT,
		PW_1_INPUT,
		RESET_INPUT,
		WAVE_A_INPUT,
		FM_2_INPUT,
		PW_2_INPUT,
		WAVE_B_INPUT,
		CV_MIX_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SIN_OUTPUT,
		TRI_OUTPUT,
		SAW_OUTPUT,
		SQR_OUTPUT,
		SIN_2_OUTPUT,
		TRI_2_OUTPUT,
		SAW_2_OUTPUT,
		SQR_2_OUTPUT,
		LFO_A_OUTPUT,
		LFO_B_OUTPUT,
		OUT_MIX_OUTPUT,
		SH_OUTPUT,
		SH_2_OUTPUT,
		NUM_OUTPUTS
	};

	LowFrequencyOscillator oscillator;
	LowFrequencyOscillator oscillatorB;
	dsp::SchmittTrigger trigger1;
	dsp::SchmittTrigger trigger2;
	float sample1 = 0.0;
	float sample2 = 0.0;

	int Theme = THEME_DEFAULT;

	LFO() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		configParam(LFO::OFFSET_1_PARAM, 0.0, 1.0, 0.0, "Offset");
		configParam(LFO::INVERT_1_PARAM, 0.0, 1.0, 1.0, "Invert");
		configParam(LFO::FREQ_PARAM, -9.0, 7.0, -1.0, "Frequency", "Hz", 2.f, 1.f);
		configParam(LFO::FM_PARAM, 0.0, 1.0, 0.0, "FM CV", "%", 0.0f, 100);
		configParam(LFO::PW_1_PARAM, 0.0, 1.0, 0.5, "Pulse Width", "%", 0.0f, 100);
		configParam(LFO::OFFSET_2_PARAM, 0.0, 1.0, 0.0, "Offset");
		configParam(LFO::INVERT_2_PARAM, 0.0, 1.0, 1.0, "Invert");
		configParam(LFO::FM_2_PARAM, 0.0, 1.0, 0.0, "FM CV", "%", 0.0f, 100);
		configParam(LFO::PW_2_PARAM, 0.0, 1.0, 0.5, "Pulse Width", "%", 0.0f, 100);
		configParam(LFO::WAVE_A_PARAM, 0.0, 4.0, 0.0, "Wave Type");
		configParam(LFO::WAVE_B_PARAM, 0.0, 4.0, 0.0, "Wave Type");
		configParam(LFO::MIX_PARAM, 0.0, 1.0, 0.5, "Crossfeed");
		configInput(FM_1_INPUT, "(A) FM");
		configInput(RESET_INPUT, "Reset");
		configInput(WAVE_A_INPUT, "(A) Wave Type Control Voltage");
		configOutput(LFO_A_OUTPUT, "(A) Master");
		configOutput(SIN_OUTPUT, "(A) Sin");
		configOutput(TRI_OUTPUT, "(A) Triangle");
		configOutput(SH_OUTPUT, "(A) Random Staircase");
		configInput(FM_2_INPUT, "(B) FM");
		configInput(WAVE_B_INPUT, "(B) Wave Type Control Voltage");
		configOutput(LFO_B_OUTPUT, "(B) Master");
		configOutput(SAW_2_OUTPUT, "(B) Sawtooth");
		configOutput(SQR_2_OUTPUT, "(B) Square");
		configInput(CV_MIX_INPUT, "Mix Crossfeed Control Voltage");
		configOutput(OUT_MIX_OUTPUT, "Crossfeed");
		getParamQuantity(INVERT_1_PARAM)->randomizeEnabled = false;
		getParamQuantity(OFFSET_2_PARAM)->randomizeEnabled = false;
		getParamQuantity(INVERT_2_PARAM)->randomizeEnabled = false;
		getParamQuantity(OFFSET_1_PARAM)->randomizeEnabled = false;
	}	void process(const ProcessArgs& args) override;

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

void LFO::process(const ProcessArgs& args)
{
	//Noise
	float noise1 = 2.0f * random::normal();
	float noise2 = 2.0f * random::normal();

	//LFO 1
	oscillator.setPitch(params[FREQ_PARAM].getValue() + inputs[FM_1_INPUT].getVoltage() * params[FM_PARAM].getValue());
	oscillator.setPulseWidth(params[PW_1_PARAM].getValue() + inputs[PW_1_INPUT].getVoltage());
	oscillator.Settings(params[OFFSET_1_PARAM].getValue() > 0.0f, params[INVERT_1_PARAM].getValue() <= 0.0f);
	oscillator.step(1.0f / args.sampleRate);
	oscillator.setReset(inputs[RESET_INPUT].getVoltage());

	outputs[SIN_OUTPUT].setVoltage(5.0f * oscillator.sin());
	outputs[TRI_OUTPUT].setVoltage(5.0f * oscillator.tri());
	outputs[SAW_OUTPUT].setVoltage(5.0f * oscillator.saw());
	outputs[SQR_OUTPUT].setVoltage(5.0f * oscillator.sqr());

	//sample and hold
	 float Switch = params[OFFSET_1_PARAM].getValue();

	if (trigger1.process(outputs[SQR_OUTPUT].getVoltage())) {
		if(Switch > 0.0f) {
		sample1 = noise1 + 5.0f;
		}
		else {
			sample1 = noise1;
		}
	}

	float SHOut1 = sample1 / 5.0f;

	//Output
	outputs[SH_OUTPUT].setVoltage(5.0f * SHOut1);

	float WAVE_A = params[WAVE_A_PARAM].getValue() + inputs[WAVE_A_INPUT].getVoltage();
	WAVE_A = clamp(WAVE_A, 0.0f, 4.0f);
	float WAVE_1_OUT;
	if (WAVE_A < 1.0f)
		WAVE_1_OUT = crossfade(oscillator.sin(), oscillator.tri(), WAVE_A);
	else if (WAVE_A < 2.0f)
		WAVE_1_OUT = crossfade(oscillator.tri(), oscillator.saw(), WAVE_A - 1.0f);
	else if (WAVE_A < 3.0f)
		WAVE_1_OUT = crossfade(oscillator.saw(), oscillator.sqr(), WAVE_A - 2.0f);
	else
		WAVE_1_OUT = crossfade(oscillator.sqr(), SHOut1, WAVE_A - 3.0f);
	outputs[LFO_A_OUTPUT].setVoltage(5.0f * WAVE_1_OUT);


	//LFO 2

	oscillatorB.setPitch(params[FREQ_PARAM].getValue() + inputs[FM_2_INPUT].getVoltage() * params[FM_2_PARAM].getValue());
	oscillatorB.setPulseWidth(params[PW_2_PARAM].getValue() + inputs[PW_2_INPUT].getVoltage());
	oscillatorB.Settings(params[OFFSET_2_PARAM].getValue() > 0.0f, params[INVERT_2_PARAM].getValue() <= 0.0f);
	oscillatorB.step(1.0f / args.sampleRate);
	oscillatorB.setReset(inputs[RESET_INPUT].getVoltage());

	outputs[SIN_2_OUTPUT].setVoltage(5.0f * oscillatorB.sin());
	outputs[TRI_2_OUTPUT].setVoltage(5.0f * oscillatorB.tri());
	outputs[SAW_2_OUTPUT].setVoltage(5.0f * oscillatorB.saw());
	outputs[SQR_2_OUTPUT].setVoltage(5.0f * oscillatorB.sqr());

	//sample and hold
	if (trigger2.process(outputs[SQR_2_OUTPUT].getVoltage())) {
		if(Switch > 0.0f) {
		sample2 = noise1 + 5.0f;
		}
		else {
			sample2 = noise2;
		}
	}

	float SHOut2 = sample2 / 5.0f;

	//Output
	outputs[SH_2_OUTPUT].setVoltage(5.0f * SHOut2);

	float WAVE_B = params[WAVE_B_PARAM].getValue() + inputs[WAVE_B_INPUT].getVoltage();
	WAVE_B = clamp(WAVE_B, 0.0f, 4.0f);
	float WAVE_2_OUT;
	if (WAVE_B < 1.0f)
		WAVE_2_OUT = crossfade(oscillatorB.sin(), oscillatorB.tri(), WAVE_B);
	else if (WAVE_B < 2.0f)
		WAVE_2_OUT = crossfade(oscillatorB.tri(), oscillatorB.saw(), WAVE_B - 1.0f);
	else if (WAVE_B < 3.0f)
		WAVE_2_OUT = crossfade(oscillatorB.saw(), oscillatorB.sqr(), WAVE_B - 2.0f);
	else
		WAVE_2_OUT = crossfade(oscillatorB.sqr(), SHOut2, WAVE_B - 3.0f);
	outputs[LFO_B_OUTPUT].setVoltage(5.0f * WAVE_2_OUT);


	//MIX LFO 1 & LFO 2
	float CrossfadeMix = clamp(params[MIX_PARAM].getValue() + inputs[CV_MIX_INPUT].getVoltage() / 10.f, 0.0f, 1.0f);
	float MIX_IN_1 = outputs[LFO_A_OUTPUT].getVoltage();
	float MIX_IN_2 = outputs[LFO_B_OUTPUT].getVoltage();
	float OutMix;

	OutMix = crossfade(MIX_IN_1, MIX_IN_2, CrossfadeMix);
	outputs[OUT_MIX_OUTPUT].setVoltage(OutMix);
};

struct LClassicMenu : MenuItem {
	LFO *lfo;
	void onAction(const event::Action &e) override {
		lfo->Theme = 0;
	}
	void step() override {
		rightText = (lfo->Theme == 0) ? "✔" : "";
		MenuItem::step();
	}
};

struct LNightModeMenu : MenuItem {
	LFO *lfo;
	void onAction(const event::Action &e) override {
		lfo->Theme = 1;
	}
	void step() override {
		rightText = (lfo->Theme == 1) ? "✔" : "";
		MenuItem::step();
	}
};

struct LFOWidget : ModuleWidget {
	// Panel Themes
	SvgPanel *panelClassic;
	SvgPanel *panelNightMode;

#ifndef USING_CARDINAL_NOT_RACK
	void appendContextMenu(Menu *menu) override {
		LFO *lfo = dynamic_cast<LFO*>(module);
		assert(lfo);
		menu->addChild(construct<MenuEntry>());
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Theme"));
		menu->addChild(construct<LClassicMenu>(&LClassicMenu::text, "Classic (default)", &LClassicMenu::lfo, lfo));
		menu->addChild(construct<LNightModeMenu>(&LNightModeMenu::text, "Night Mode", &LNightModeMenu::lfo, lfo));
	}
#endif

	LFOWidget(LFO *module);
	void step() override;
};

LFOWidget::LFOWidget(LFO *module) {
		setModule(module);
	box.size = Vec(16 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	panelClassic = new SvgPanel();
	panelClassic->box.size = box.size;
	panelClassic->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/MLFO.svg")));
	panelClassic->visible = !THEME_DEFAULT;
	addChild(panelClassic);

	panelNightMode = new SvgPanel();
	panelNightMode->box.size = box.size;
	panelNightMode->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/MLFO-Dark.svg")));
	panelNightMode->visible = !!THEME_DEFAULT;
	addChild(panelNightMode);

	addChild(createWidget<MScrewB>(Vec(15, 0)));
	addChild(createWidget<MScrewC>(Vec(box.size.x-30, 0)));
	addChild(createWidget<MScrewA>(Vec(15, 365)));
	addChild(createWidget<MScrewD>(Vec(box.size.x-30, 365)));

	//LFO 1
	addParam(createParam<VioM2Switch>(Vec(70, 150), module, LFO::OFFSET_1_PARAM));
	addParam(createParam<VioM2Switch>(Vec(70, 215), module, LFO::INVERT_1_PARAM));
	addParam(createParam<RedLargeKnob>(Vec(100, 40), module, LFO::FREQ_PARAM));
	addParam(createParam<BlueSmallKnob>(Vec(11, 142), module, LFO::FM_PARAM));
	addParam(createParam<BlueSmallKnob>(Vec(11, 207), module, LFO::PW_1_PARAM));

	addInput(createInput<SilverSixPortA>(Vec(40, 256), module, LFO::FM_1_INPUT));
	addInput(createInput<SilverSixPort>(Vec(80, 256), module, LFO::RESET_INPUT));
	addInput(createInput<SilverSixPortC>(Vec(11, 298), module, LFO::WAVE_A_INPUT));

	addOutput(createOutput<SilverSixPortD>(Vec(54, 298), module, LFO::LFO_A_OUTPUT));
	addOutput(createOutput<SilverSixPortE>(Vec(35, 338), module, LFO::SIN_OUTPUT));
	addOutput(createOutput<SilverSixPortA>(Vec(69, 338), module, LFO::TRI_OUTPUT));
	addOutput(createOutput<SilverSixPort>(Vec(110, 338), module, LFO::SH_OUTPUT));


	//LFO 2
	addParam(createParam<VioM2Switch>(Vec(162, 150), module, LFO::OFFSET_2_PARAM));
	addParam(createParam<VioM2Switch>(Vec(162, 215), module, LFO::INVERT_2_PARAM));
	addParam(createParam<BlueSmallKnob>(Vec(199, 142), module, LFO::FM_2_PARAM));
	addParam(createParam<BlueSmallKnob>(Vec(199, 207), module, LFO::PW_2_PARAM));

	addInput(createInput<SilverSixPort>(Vec(180, 256), module, LFO::FM_2_INPUT));
	addInput(createInput<SilverSixPortA>(Vec(206, 298), module, LFO::WAVE_B_INPUT));

	addOutput(createOutput<SilverSixPortC>(Vec(162, 298), module, LFO::LFO_B_OUTPUT));
	addOutput(createOutput<SilverSixPortD>(Vec(151, 338), module, LFO::SAW_2_OUTPUT));
	addOutput(createOutput<SilverSixPortA>(Vec(185, 338), module, LFO::SQR_2_OUTPUT));

	//Wave Selectors
	addParam(createParam<GreenSmallKnob>(Vec(40, 50), module, LFO::WAVE_A_PARAM));
	addParam(createParam<GreenSmallKnob>(Vec(175, 50), module, LFO::WAVE_B_PARAM));

	//LFO MIX
	addParam(createParam<GreenSmallKnob>(Vec(108, 105), module, LFO::MIX_PARAM));
	addInput(createInput<SilverSixPort>(Vec(140, 256), module, LFO::CV_MIX_INPUT));
	addOutput(createOutput<SilverSixPortA>(Vec(110, 298), module, LFO::OUT_MIX_OUTPUT));
};

void LFOWidget::step() {
#ifdef USING_CARDINAL_NOT_RACK
	panelClassic->visible = !settings::preferDarkPanels;
	panelNightMode->visible = settings::preferDarkPanels;
#else
	if (module) {
		LFO *lfo = dynamic_cast<LFO*>(module);
		assert(lfo);
		panelClassic->visible = (lfo->Theme == 0);
		panelNightMode->visible = (lfo->Theme == 1);
	}
#endif
	ModuleWidget::step();
}

Model *modelLFO = createModel<LFO, LFOWidget>("MLFO");
