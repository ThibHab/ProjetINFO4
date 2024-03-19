#include "MSM.hpp"
#include "MSMComponentLibrary.hpp"
#include "Additional/VRand.hpp"

struct Noise : Module {

	enum ParamIds {
		LP_PARAM,
		HP_PARAM,
		MIX_PARAM,
		TYPE,
		NUM_PARAMS
	};

	enum InputIds {
		CV_INPUT,
		NUM_INPUTS
	};

	enum OutputIds {
		WNOISE_OUTPUT,
		CNOISE_OUTPUT,
		NUM_OUTPUTS
	};

	 enum LightIds {
        NUM_LIGHTS
	};

	// Panel Theme
	int Theme = THEME_DEFAULT;

	VRand *VR = new VRand();

	dsp::RCFilter filterL;
	dsp::RCFilter filterH;

	float NoiSetyPe = 0.0f;

	float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f, b3 = 0.0f, b4 = 0.0f, b5 = 0.0f, b6 = 0.0f, pink = 0.0f;

	Noise() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(Noise::TYPE, 0.0, 2.0, 0.0, "Noise type");
		configParam(Noise::LP_PARAM, 0.0, 1.0, 1.0, "Cutoff", "Hz", 5.0f, 1e4, 0.0f);
		configParam(Noise::HP_PARAM, 0.0, 1.0, 0.0, "Cutoff", "Hz", 5.0f, 500.0f, 0.0f);
		configParam(Noise::MIX_PARAM, 0.0, 1.0, 0.5, "Mix");
		configInput(CV_INPUT, "Noise Amplitude Control Voltage");
		configOutput(WNOISE_OUTPUT, "Noise");
		configOutput(CNOISE_OUTPUT, "Colored Noise");
	}

	~Noise() override {
		delete VR;
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

void Noise::process(const ProcessArgs& args) {

	float WNoise = VR->white();
	float BNoise = VR->brown();
	float CNoise = WNoise;

		  b0 = 0.99886 * b0 + WNoise * 0.0555179;
		  b1 = 0.99332 * b1 + WNoise * 0.0750759;
		  b2 = 0.96900 * b2 + WNoise * 0.1538520;
		  b3 = 0.86650 * b3 + WNoise * 0.3104856;
		  b4 = 0.55000 * b4 + WNoise * 0.5329522;
		  b5 = -0.7616 * b5 - WNoise * 0.0168980;
		  pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + WNoise * 0.5362;
		  b6 = WNoise * 0.115926;

	char Type = params[TYPE].getValue();
	switch(Type) {
		case 2:
		NoiSetyPe = 8.0f * BNoise;
		break;
		case 1:
		NoiSetyPe = pink / 5.0f;
		break;
		case 0:
		NoiSetyPe = 1.0f * WNoise;
		break;
	}

	// Filter
	float LP = params[LP_PARAM].getValue();
	float lowpassFreq = 10000.0f * powf(5.0f, clamp(LP, 0.0f, 1.0f));
	filterL.setCutoff(lowpassFreq / args.sampleRate);
	filterL.process(CNoise);
	CNoise = filterL.lowpass();

	float HP = params[HP_PARAM].getValue();
	float highpassFreq = 500.0f * powf(5.0f, clamp(HP, 0.0f, 1.0f));
	filterH.setCutoff(highpassFreq / args.sampleRate);
	filterH.process(CNoise);
	CNoise = filterH.highpass();

	float Fast_RandFloat = 1.5f * CNoise;

	float mixcontrol = params[MIX_PARAM].getValue();

	float mix = crossfade(WNoise, Fast_RandFloat, mixcontrol);

	// Noise
	if(inputs[CV_INPUT].isConnected()) {
		NoiSetyPe *= clamp(inputs[CV_INPUT].getVoltage() / 10.0f, 0.0f, 1.0f);
		outputs[WNOISE_OUTPUT].setVoltage(saturate2(NoiSetyPe));
	}
	else {
		outputs[WNOISE_OUTPUT].setVoltage(saturate2(NoiSetyPe));
	}
	// Colored Noise
	if(inputs[CV_INPUT].isConnected()) {
		mix *= clamp(inputs[CV_INPUT].getVoltage() / 10.0f, 0.0f, 1.0f);
		outputs[CNOISE_OUTPUT].setVoltage(saturate2(mix));
	}
	else {
		outputs[CNOISE_OUTPUT].setVoltage(saturate2(mix));
	}
};

struct NClassicMenu : MenuItem {
	Noise *noise;
	void onAction(const event::Action &e) override {
		noise->Theme = 0;
	}
	void step() override {
		rightText = (noise->Theme == 0) ? "✔" : "";
		MenuItem::step();
	}
};

struct NNightModeMenu : MenuItem {
	Noise *noise;
	void onAction(const event::Action &e) override {
		noise->Theme = 1;
	}
	void step() override {
		rightText = (noise->Theme == 1) ? "✔" : "";
		MenuItem::step();
	}
};

struct NoiseWidget : ModuleWidget {
	// Panel Themes
	SvgPanel *panelClassic;
	SvgPanel *panelNightMode;

#ifndef USING_CARDINAL_NOT_RACK
	void appendContextMenu(Menu* menu) override {
		Noise *noise = dynamic_cast<Noise*>(module);
		assert(noise);
		menu->addChild(construct<MenuEntry>());
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Theme"));
		menu->addChild(construct<NClassicMenu>(&NClassicMenu::text, "Classic (default)", &NClassicMenu::noise, noise));
		menu->addChild(construct<NNightModeMenu>(&NNightModeMenu::text, "Night Mode", &NNightModeMenu::noise, noise));
	}
#endif

	NoiseWidget(Noise *module);
	void step() override;
};


NoiseWidget::NoiseWidget(Noise *module) {
		setModule(module);

	box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	panelClassic = new SvgPanel();
	panelClassic->box.size = box.size;
	panelClassic->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/Noise.svg")));
	panelClassic->visible = !THEME_DEFAULT;
	addChild(panelClassic);

	panelNightMode = new SvgPanel();
	panelNightMode->box.size = box.size;
	panelNightMode->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/Noise-Dark.svg")));
	panelNightMode->visible = !!THEME_DEFAULT;
	addChild(panelNightMode);

	addChild(createWidget<MScrewD>(Vec(0, 0)));
	addChild(createWidget<MScrewA>(Vec(0, 365)));
	addChild(createWidget<MScrewC>(Vec(45, 0)));
	addChild(createWidget<MScrewB>(Vec(45, 365)));

	addParam(createParam<MThree>(Vec(12, 28), module, Noise::TYPE));

	addParam(createParam<GreenSmallKnob>(Vec(15, 48), module, Noise::LP_PARAM));
	addParam(createParam<GreenSmallKnob>(Vec(15, 96), module, Noise::HP_PARAM));
	addParam(createParam<GreenSmallKnob>(Vec(15, 154), module, Noise::MIX_PARAM));

	addInput(createInput<SilverSixPortA>(Vec(18, 210), module, Noise::CV_INPUT));

	addOutput(createOutput<SilverSixPortB>(Vec(18, 250), module, Noise::WNOISE_OUTPUT));
	addOutput(createOutput<SilverSixPort>(Vec(18, 300), module, Noise::CNOISE_OUTPUT));

};

void NoiseWidget::step() {
#ifdef USING_CARDINAL_NOT_RACK
	panelClassic->visible = !settings::preferDarkPanels;
	panelNightMode->visible = settings::preferDarkPanels;
#else
	if (module) {
		Noise *noise = dynamic_cast<Noise*>(module);
		assert(noise);
		panelClassic->visible = (noise->Theme == 0);
		panelNightMode->visible = (noise->Theme == 1);
	}
#endif
	ModuleWidget::step();
}


Model *modelNoise = createModel<Noise, NoiseWidget>("Noise");
