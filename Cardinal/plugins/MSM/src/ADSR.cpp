#include "MSM.hpp"
#include "MSMComponentLibrary.hpp"
#include "Additional/EnvelopeGenerator.hpp"

struct ADSR : Module {
	enum ParamIds {
		ATTACK_PARAM,
		DECAY_PARAM,
		SUSTAIN_PARAM,
		RELEASE_PARAM,
		ATT_SHAPE,
		DEC_SHAPE,
		REL_SHAPE,
		NUM_PARAMS
	};
	enum InputIds {
		ATTACK_INPUT,
		DECAY_INPUT,
		SUSTAIN_INPUT,
		RELEASE_INPUT,
		GATE_INPUT,
		TRIG_INPUT,
		DEC_SHAPE_CV,
		NUM_INPUTS
	};
	enum OutputIds {
		ENVELOPE_OUTPUT,
		INV_OUTPUT,
		EOA,
		EOD,
		EOS,
		EOR,
		NUM_OUTPUTS
	};
	enum LightIds {
		ATTACK_LIGHT,
		DECAY_LIGHT,
		SUSTAIN_LIGHT,
		RELEASE_LIGHT,
		NUM_LIGHTS
	};

	const int modulationSteps = 100;
	int _modulationStep = 0;
	//char _linearMode = 0;
	dsp::SchmittTrigger _gateTrigger;
	dsp::SchmittTrigger _retrigTrigger;
	EnvelopeGen _envelope;

	dsp::PulseGenerator _triggerAttack;
	dsp::PulseGenerator _triggerDecay;
	dsp::PulseGenerator _triggerSustain;
	dsp::PulseGenerator _triggerRelease;

	float attackshape = 0.0f;
	float decayshape = 0.0f;
	float releaseshape = 0.0f;

	// Panel Theme
	int Theme = THEME_DEFAULT;

	ADSR() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ADSR::ATTACK_PARAM, 0.0f, 1.0f, 0.1f, "Attack");
		configParam(ADSR::DECAY_PARAM, 0.0f, 1.0f, 0.2f, "Decay");
		configParam(ADSR::SUSTAIN_PARAM, 0.0f, 1.0f, 0.5f, "Sustain");
		configParam(ADSR::RELEASE_PARAM, 0.0f, 1.0f, 0.15f, "Release");
		configParam(ADSR::ATT_SHAPE, 0.1f, 4.0f, 1.0f, "Attack Shape");
		configParam(ADSR::DEC_SHAPE, 0.1f, 4.0f, 1.0f, "Decay Shape");
		configParam(ADSR::REL_SHAPE, 0.1f, 4.0f, 1.0f, "Release Shape");
		configInput(ATTACK_INPUT, "Attack Control Voltage");
		configInput(DECAY_INPUT, "Decay Control Voltage");
		configInput(SUSTAIN_INPUT, "Sustain Control Voltage");
		configInput(RELEASE_INPUT, "Release Control Voltage");
		configInput(DEC_SHAPE_CV, "Decay Shape Control Voltage");
		configInput(GATE_INPUT, "Gate");
		configInput(TRIG_INPUT, "Trigger");
		configOutput(ENVELOPE_OUTPUT, "Envelope");
		configOutput(INV_OUTPUT, "Inverted Envelope");
		configOutput(EOA, "Attack Period");
		configOutput(EOD, "Decay Period");
		configOutput(EOS, "Sustain Period");
		configOutput(EOR, "Release Period");
		onReset();
		onSampleRateChange();
	}

	void onReset() override;
	void onSampleRateChange() override;
	void process(const ProcessArgs& args) override;

	//Json for Panel Theme
	json_t *dataToJson() override	{
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Theme", json_integer(Theme));
		//json_object_set_new(rootJ, "Linear", json_integer(_linearMode));
		return rootJ;
	}
	void dataFromJson(json_t *rootJ) override	{
		json_t *ThemeJ = json_object_get(rootJ, "Theme");
		if (ThemeJ)
			Theme = json_integer_value(ThemeJ);

		//json_t *LinJ = json_object_get(rootJ, "Linear");
		//if (LinJ)
		//	_linearMode = json_integer_value(LinJ);
	}

};

void ADSR::onReset() {
	_gateTrigger.reset();
	_envelope.reset();
	//_linearMode = 0;
	_modulationStep = modulationSteps;
};

void ADSR::onSampleRateChange() {
	_envelope.setSampleRate(APP->engine->getSampleRate());
	_modulationStep = modulationSteps;
};

void ADSR::process(const ProcessArgs& args) {

	++_modulationStep;
	if (_modulationStep >= modulationSteps) {
		_modulationStep = 0;

		_envelope.setAttack(params[ATTACK_PARAM].getValue() + clamp(inputs[ATTACK_INPUT].getVoltage() / 10.0f, 0.0f, 1.0f));
		_envelope.setDecay(params[DECAY_PARAM].getValue() + clamp(inputs[DECAY_INPUT].getVoltage() / 10.0f, 0.0f, 1.0f));
		_envelope.setSustain(params[SUSTAIN_PARAM].getValue(), inputs[SUSTAIN_INPUT].getVoltage());
		_envelope.setRelease(params[RELEASE_PARAM].getValue() + clamp(inputs[RELEASE_INPUT].getVoltage() / 10.0f, 0.0f, 1.0f));
		/*
		_envelope.setAttack(powf(params[ATTACK_PARAM].getValue() + clamp(inputs[ATTACK_INPUT].getVoltage() / 10.0f, 0.0f, 1.0f), 2.0f) * 10.f);
		_envelope.setDecay(powf(params[DECAY_PARAM].getValue() + clamp(inputs[DECAY_INPUT].getVoltage() / 10.0f, 0.0f, 1.0f), 2.0f) * 10.f);
		_envelope.setSustain(params[SUSTAIN_PARAM].getValue() + clamp(inputs[SUSTAIN_INPUT].getVoltage() / 10.0f, 0.0f, 1.0f));
		_envelope.setRelease(powf(params[RELEASE_PARAM].getValue() + clamp(inputs[RELEASE_INPUT].getVoltage() / 10.0f, 0.0f, 1.0f), 2.0f) * 10.f);
		*/

		attackshape = clamp(params[ATT_SHAPE].getValue(), 0.1f, 4.0f);
		decayshape = clamp(params[DEC_SHAPE].getValue() + inputs[DEC_SHAPE_CV].getVoltage() / 2.5f, 0.1f, 4.0f);
		releaseshape = clamp(params[REL_SHAPE].getValue(), 0.1f, 4.0f);
		_envelope.setShapes(attackshape, decayshape, releaseshape);
	}

	_gateTrigger.process(inputs[GATE_INPUT].getVoltage());
	_retrigTrigger.process(inputs[TRIG_INPUT].getVoltage());
	_envelope.setGate(_gateTrigger.isHigh(), _retrigTrigger.isHigh());


	outputs[ENVELOPE_OUTPUT].setVoltage(_envelope.next() * 10.0f);
	outputs[INV_OUTPUT].setVoltage(1.0f - (_envelope.next() * 10.0f) + 9.0f);


	lights[ATTACK_LIGHT].setBrightness(_envelope.isStage(EnvelopeGen::ATTACK_STAGE));
	if(!_envelope.isStage(EnvelopeGen::ATTACK_STAGE)) {
		_triggerAttack.trigger(0.01f);
	}
	outputs[EOA].setVoltage(_triggerAttack.process(1.0f / args.sampleRate) ? 10.0 : 0.0);

	lights[DECAY_LIGHT].setBrightness(_envelope.isStage(EnvelopeGen::DECAY_STAGE));
	if(!_envelope.isStage(EnvelopeGen::DECAY_STAGE)) {
		_triggerDecay.trigger(0.01f);
	}
	outputs[EOD].setVoltage(_triggerDecay.process(1.0f / args.sampleRate) ? 10.0 : 0.0);

	lights[SUSTAIN_LIGHT].setBrightness(_envelope.isStage(EnvelopeGen::SUSTAIN_STAGE));
	if(!_envelope.isStage(EnvelopeGen::SUSTAIN_STAGE)) {
		_triggerSustain.trigger(0.01f);
	}
	outputs[EOS].setVoltage(_triggerSustain.process(1.0f / args.sampleRate) ? 10.0 : 0.0);

	lights[RELEASE_LIGHT].setBrightness(_envelope.isStage(EnvelopeGen::RELEASE_STAGE));
	if(!_envelope.isStage(EnvelopeGen::RELEASE_STAGE)) {
		_triggerRelease.trigger(0.01f);
	}
	outputs[EOR].setVoltage(_triggerRelease.process(1.0f / args.sampleRate) ? 10.0 : 0.0);
};

struct ADSRClassicMenu : MenuItem {
	ADSR *adsr;
	void onAction(const event::Action &e) override {
		adsr->Theme = 0;
	}
	void step() override {
		rightText = (adsr->Theme == 0) ? "✔" : "";
		MenuItem::step();
	}
};

struct ADSRNightModeMenu : MenuItem {
	ADSR *adsr;
	void onAction(const event::Action &e) override {
		adsr->Theme = 1;
	}
	void step() override {
		rightText = (adsr->Theme == 1) ? "✔" : "";
		MenuItem::step();
	}
};


struct ADSRWidget : ModuleWidget {
	// Panel Themes
	SvgPanel *panelClassic;
	SvgPanel *panelNightMode;

#ifndef USING_CARDINAL_NOT_RACK
	void appendContextMenu(Menu *menu) override {
		ADSR *adsr = dynamic_cast<ADSR*>(module);
		assert(adsr);
		menu->addChild(construct<MenuEntry>());
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Theme"));
		menu->addChild(construct<ADSRClassicMenu>(&ADSRClassicMenu::text, "Classic (default)", &ADSRClassicMenu::adsr, adsr));
		menu->addChild(construct<ADSRNightModeMenu>(&ADSRNightModeMenu::text, "Night Mode", &ADSRNightModeMenu::adsr, adsr));
		//menu->addChild(construct<MenuEntry>());
		//menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Type"));
		//menu->addChild(construct<ExpoMenu>(&ExpoMenu::text, "Exponential Mode", &ExpoMenu::adsr, adsr));
		//menu->addChild(construct<LogarithmicMenu>(&LogarithmicMenu::text, "Logarithmic Mode", &LogarithmicMenu::adsr, adsr));
		//menu->addChild(construct<LinearMenu>(&LinearMenu::text, "Linear Mode", &LinearMenu::adsr, adsr));
	}
#endif

	ADSRWidget(ADSR *module);
	void step() override;

};

ADSRWidget::ADSRWidget(ADSR *module) {
	setModule(module);
	box.size = Vec(9 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	// Classic Theme
	panelClassic = new SvgPanel();
	panelClassic->box.size = box.size;
	panelClassic->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/ADSR.svg")));
	panelClassic->visible = !THEME_DEFAULT;
	addChild(panelClassic);
	// Night Mode Theme
	panelNightMode = new SvgPanel();
	panelNightMode->box.size = box.size;
	panelNightMode->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/ADSR-Dark.svg")));
	panelNightMode->visible = !!THEME_DEFAULT;
	addChild(panelNightMode);

	addChild(createWidget<MScrewA>(Vec(15, 0)));
	addChild(createWidget<MScrewD>(Vec(box.size.x-30, 0)));
	addChild(createWidget<MScrewC>(Vec(15, 365)));
	addChild(createWidget<MScrewB>(Vec(box.size.x-30, 365)));

	addParam(createParam<MSMSlidePot>(Vec(8, 55), module, ADSR::ATTACK_PARAM));
	addParam(createParam<MSMSlidePot>(Vec(38, 55), module, ADSR::DECAY_PARAM));
	addParam(createParam<MSMSlidePot>(Vec(71, 55), module, ADSR::SUSTAIN_PARAM));
	addParam(createParam<MSMSlidePot>(Vec(101, 55), module, ADSR::RELEASE_PARAM));


	addParam(createParam<GreenTinyKnob>(Vec(9, 205), module, ADSR::ATT_SHAPE));
	addParam(createParam<GreenTinyKnob>(Vec(40, 205), module, ADSR::DEC_SHAPE));
	addInput(createInput<SilverSixPortA>(Vec(71, 205), module, ADSR::DEC_SHAPE_CV));
	addParam(createParam<GreenTinyKnob>(Vec(102, 205), module, ADSR::REL_SHAPE));

	addInput(createInput<SilverSixPortA>(Vec(9, 251.5), module, ADSR::ATTACK_INPUT));
	addInput(createInput<SilverSixPortD>(Vec(40, 251.5), module, ADSR::DECAY_INPUT));
	addInput(createInput<SilverSixPort>(Vec(72, 251.5), module, ADSR::SUSTAIN_INPUT));
	addInput(createInput<SilverSixPortB>(Vec(102, 251.5), module, ADSR::RELEASE_INPUT));

	addOutput(createOutput<SilverSixPortC>(Vec(9, 289.5), module, ADSR::EOA));
	addOutput(createOutput<SilverSixPort>(Vec(40, 289.5), module, ADSR::EOD));
	addOutput(createOutput<SilverSixPortA>(Vec(72, 289.5), module, ADSR::EOS));
	addOutput(createOutput<SilverSixPortB>(Vec(102, 289.5), module, ADSR::EOR));

	addInput(createInput<SilverSixPortA>(Vec(9, 327.5), module, ADSR::GATE_INPUT));
	addInput(createInput<SilverSixPortC>(Vec(40, 327.5), module, ADSR::TRIG_INPUT));
	addOutput(createOutput<SilverSixPort>(Vec(102, 327.5), module, ADSR::ENVELOPE_OUTPUT));
	addOutput(createOutput<SilverSixPortD>(Vec(72, 327.5), module, ADSR::INV_OUTPUT));

	addChild(createLight<SmallLight<BlueLight>>(Vec(19, 46), module, ADSR::ATTACK_LIGHT));
	addChild(createLight<SmallLight<BlueLight>>(Vec(49, 46), module, ADSR::DECAY_LIGHT));
	addChild(createLight<SmallLight<BlueLight>>(Vec(82, 46), module, ADSR::SUSTAIN_LIGHT));
	addChild(createLight<SmallLight<BlueLight>>(Vec(112, 46), module, ADSR::RELEASE_LIGHT));
}

void ADSRWidget::step() {
#ifdef USING_CARDINAL_NOT_RACK
	panelClassic->visible = !settings::preferDarkPanels;
	panelNightMode->visible = settings::preferDarkPanels;
#else
	if (module) {
		ADSR *adsr = dynamic_cast<ADSR*>(module);
		assert(adsr);
		panelClassic->visible = (adsr->Theme == 0);
		panelNightMode->visible = (adsr->Theme == 1);
	}
#endif
	ModuleWidget::step();
}

/*
struct ExpoMenu : MenuItem {
	ADSR *adsr;
	void onAction(EventAction &e) override {
		adsr->_linearMode = 0;
	}
	void step() override {
		rightText = (adsr->_linearMode == 0) ? "✔" : "";
		MenuItem::step();
	}
};

struct LogarithmicMenu : MenuItem {
	ADSR *adsr;
	void onAction(EventAction &e) override {
		adsr->_linearMode = 1;
	}
	void step() override {
		rightText = (adsr->_linearMode == 1) ? "✔" : "";
		MenuItem::step();
	}
};

struct LinearMenu : MenuItem {
	ADSR *adsr;
	void onAction(EventAction &e) override {
		adsr->_linearMode = 2;
	}
	void step() override {
		rightText = (adsr->_linearMode == 2) ? "✔" : "";
		MenuItem::step();
	}
};
*/

Model *modelADSR = createModel<ADSR, ADSRWidget>("ADSR");
