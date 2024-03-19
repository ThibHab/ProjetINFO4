#include "MSM.hpp"
#include "MSMComponentLibrary.hpp"

class BitcrusherFX {
	public:
		BitcrusherFX()
		{
			reset();
			quant = powf(2.0f, 32.0f);
			counter = 0.0f;
			_drive = 1.0f;
			input = 0.0f;
		}

		void reset() {
			output = 0.0f;
		}

		float process(float in, long int Bits, float drive, float sample_rate)	{
				// Overdrive
				_drive = drive;
				input = overdrive(in, _drive);
				//Bit-reduction
				quant = std::pow(2.0, Bits);

				counter += sample_rate;
				if (counter >= 1.0f) {
					counter -= 1.0f;
					output = ROUND((input + 1.0) * quant) / quant - 1.0;
				}
		return output;
		}


	private:
		float quant;
		float _drive;
		float counter;
		float input;
		float output;
};

struct Bitcrusher : Module {

	enum ParamIds {
		BITS_PARAM,
		QD_PARAM,
		SR_PARAM,
		NUM_PARAMS
	};

	enum InputIds {
		INPUT,
		CV_BITS,
		NUM_INPUTS
	};

	enum OutputIds {
		OUTPUT,
		NUM_OUTPUTS
	};

	 enum LightIds {
        NUM_LIGHTS
	};

	// Panel Theme
	int Theme = THEME_DEFAULT;

	float out = 0.0f ? 0.0f : 0.0f;

	BitcrusherFX BitC;

	Bitcrusher() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(Bitcrusher::BITS_PARAM, 0.0, 8.0, 8.0, "Bits");
		configParam(Bitcrusher::SR_PARAM, 0.01, 1.0, 1.0, "Sample Rate");
		configParam(Bitcrusher::QD_PARAM, 0.0, 0.95, 0.0, "Overdrive");
		configInput(INPUT, "Master");
		configInput(CV_BITS, "Bit Length");
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
		BitC.reset();
	}

};

void Bitcrusher::process(const ProcessArgs& args) {

	float in = inputs[INPUT].getVoltage() / 5.0f;
	long int bits = params[BITS_PARAM].getValue() + inputs[CV_BITS].getVoltage();
	float qd = params[QD_PARAM].getValue();
	float sr = params[SR_PARAM].getValue();

	out = 5.0f * BitC.process(in, bits, qd, sr);

	outputs[OUTPUT].setVoltage(saturate(out));

};

struct BitCClassicMenu : MenuItem {
	Bitcrusher *bitcrusher;
	void onAction(const event::Action &e) override {
		bitcrusher->Theme = 0;
	}
	void step() override {
		rightText = (bitcrusher->Theme == 0) ? "✔" : "";
		MenuItem::step();
	}
};

struct BitCNightModeMenu : MenuItem {
	Bitcrusher *bitcrusher;
	void onAction(const event::Action &e) override {
		bitcrusher->Theme = 1;
	}
	void step() override {
		rightText = (bitcrusher->Theme == 1) ? "✔" : "";
		MenuItem::step();
	}
};

struct BitcrusherWidget : ModuleWidget {
	// Panel Themes
	SvgPanel *panelClassic;
	SvgPanel *panelNightMode;

#ifndef USING_CARDINAL_NOT_RACK
	void appendContextMenu(Menu *menu) override {
		Bitcrusher *bitcrusher = dynamic_cast<Bitcrusher*>(module);
		assert(bitcrusher);

		menu->addChild(construct<MenuEntry>());
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Theme"));
		menu->addChild(construct<BitCClassicMenu>(&BitCClassicMenu::text, "Classic (default)", &BitCClassicMenu::bitcrusher, bitcrusher));
		menu->addChild(construct<BitCNightModeMenu>(&BitCNightModeMenu::text, "Night Mode", &BitCNightModeMenu::bitcrusher, bitcrusher));
	}
#endif

	BitcrusherWidget(Bitcrusher *module);
	void step() override;
};

BitcrusherWidget::BitcrusherWidget(Bitcrusher *module) {
	setModule(module);
	box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	panelClassic = new SvgPanel();
	panelClassic->box.size = box.size;
	panelClassic->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/Bitcrusher.svg")));
	panelClassic->visible = !THEME_DEFAULT;
	addChild(panelClassic);

	panelNightMode = new SvgPanel();
	panelNightMode->box.size = box.size;
	panelNightMode->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/Bitcrusher-Dark.svg")));
	panelNightMode->visible = !!THEME_DEFAULT;
	addChild(panelNightMode);

	addChild(createWidget<MScrewD>(Vec(0, 0)));
	addChild(createWidget<MScrewA>(Vec(0, 365)));
	addChild(createWidget<MScrewC>(Vec(45, 0)));
	addChild(createWidget<MScrewB>(Vec(45, 365)));

	addParam(createParam<GreenToggleKnobSmall>(Vec(15, 48), module, Bitcrusher::BITS_PARAM));
	addParam(createParam<GreenSmallKnob>(Vec(15, 98), module, Bitcrusher::SR_PARAM));
	addParam(createParam<GreenSmallKnob>(Vec(15, 148), module, Bitcrusher::QD_PARAM));

	addInput(createInput<SilverSixPortA>(Vec(18, 210), module, Bitcrusher::INPUT));
	addInput(createInput<SilverSixPort>(Vec(18, 250), module, Bitcrusher::CV_BITS));

	addOutput(createOutput<SilverSixPortB>(Vec(18, 290), module, Bitcrusher::OUTPUT));

};

void BitcrusherWidget::step() {
#ifdef USING_CARDINAL_NOT_RACK
	panelClassic->visible = !settings::preferDarkPanels;
	panelNightMode->visible = settings::preferDarkPanels;
#else
	if (module) {
		Bitcrusher *bitcrusher = dynamic_cast<Bitcrusher*>(module);
		assert(bitcrusher);
		panelClassic->visible = (bitcrusher->Theme == 0);
		panelNightMode->visible = (bitcrusher->Theme == 1);
	}
#endif
	ModuleWidget::step();
}

Model *modelBitcrusher = createModel<Bitcrusher, BitcrusherWidget>("Bitcrusher");
