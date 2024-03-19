#include "plugin.hpp"
#include "components/MidiWidget.hpp"

namespace StoermelderPackOne {
namespace MidiStep {

enum MODE {
	BEATSTEP_R1 = 0,
	BEATSTEP_R2 = 1,
	KK_REL = 10,
	XTOUCH_R1 = 20,
	XTOUCH_R2 = 21,
	AKAI_MPD218 = 30
};

struct MidiStepModule : Module {
	static const int PORTS = 8;
	static const int CHANNELS = 16;

	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(OUTPUT_INC, PORTS),
		ENUMS(OUTPUT_DEC, PORTS),
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	/** [Stored to JSON] */
	int panelTheme = 0;
	/** [Stored to JSON] */
	midi::InputQueue midiInput;
	/** [Stored to JSON] */
	MODE mode = MODE::BEATSTEP_R1;
	/** [Stored to JSON] */
	bool polyphonicOutput = false;

	/** [Stored to JSON] */
	int learnedCcs[CHANNELS];
	int learningId;

	int8_t values[128];
	int ccs[128];

	int incPulseCount[CHANNELS];
	dsp::PulseGenerator incPulse[CHANNELS];
	int decPulseCount[CHANNELS];
	dsp::PulseGenerator decPulse[CHANNELS];

	MidiStepModule() {
		panelTheme = pluginSettings.panelThemeDefault;
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for (int i = 0; i < PORTS; i++) {
			configOutput(OUTPUT_DEC + i, string::f("Decrement %i trigger", i + 1));
			configOutput(OUTPUT_INC + i, string::f("Increment %i trigger", i + 1));
		}
		outputInfos[OUTPUT_DEC]->description = outputInfos[OUTPUT_INC]->description = 
			"Outputs polyphonic triggers if enabled on the context menu.";
		onReset();
	}

	void onReset() override {
		for (int i = 0; i < 128; i++) {
			values[i] = 0;
			ccs[i] = -1;
		}
		for (int i = 0; i < CHANNELS; i++) {
			learnedCcs[i] = i;
			ccs[i] = i;
			incPulseCount[i] = 0;
			decPulseCount[i] = 0;
		}
		learningId = -1;
		midiInput.reset();
	}

	void process(const ProcessArgs& args) override {
		midi::Message msg;
		while (midiInput.tryPop(&msg, args.frame)) {
			processMessage(msg);
		}

		for (int i = 0; i < (polyphonicOutput ? CHANNELS : PORTS); i++) {
			if (incPulse[i].process(args.sampleTime)) {
				setOutputVoltage(OUTPUT_INC, i, incPulseCount[i] % 2 == 1 ? 10.f : 0.f);
			}
			else {
				if (incPulseCount[i] > 0) {
					incPulse[i].trigger();
					incPulseCount[i]--;
				}
				setOutputVoltage(OUTPUT_INC, i, 0.f);
			}

			if (decPulse[i].process(args.sampleTime)) {
				setOutputVoltage(OUTPUT_DEC, i, decPulseCount[i] % 2 == 1 ? 10.f : 0.f);
			}
			else {
				if (decPulseCount[i] > 0) {
					decPulse[i].trigger();
					decPulseCount[i]--;
				}
				setOutputVoltage(OUTPUT_DEC, i, 0.f);
			}
		}
		
		outputs[OUTPUT_INC + 0].setChannels(polyphonicOutput ? 16 : 1);
		outputs[OUTPUT_DEC + 0].setChannels(polyphonicOutput ? 16 : 1);
	}

	inline void setOutputVoltage(int out, int idx, float v) {
		if (polyphonicOutput) {
			outputs[out].setVoltage(v, idx);
		}
		else {
			outputs[out + idx].setVoltage(v);
		}
	}

	void processMessage(midi::Message msg) {
		switch (msg.getStatus()) {
			case 0xb: { // cc
				processCC(msg);
				break;
			}
		}
	}

	void processCC(midi::Message msg) {
		uint8_t cc = msg.getNote();

		int8_t value = msg.bytes[2];
		value = clamp(value, 0, 127);
		// Learn
		if (learningId >= 0) {
			learnCC(cc);
			return;
		}

		switch (mode) {
			case MODE::BEATSTEP_R1:
			case MODE::XTOUCH_R2: {
				if (value <= uint8_t(58)) decPulseCount[ccs[cc]] += 6;
				else if (value <= uint8_t(61)) decPulseCount[ccs[cc]] += 4;
				else if (value <= uint8_t(63)) decPulseCount[ccs[cc]] += 2;
				if (value >= uint8_t(70)) incPulseCount[ccs[cc]] += 6;
				else if (value >= uint8_t(67)) incPulseCount[ccs[cc]] += 4;
				else if (value >= uint8_t(65)) incPulseCount[ccs[cc]] += 2;
				break;
			}

			case MODE::BEATSTEP_R2:
			case MODE::KK_REL:
			case MODE::AKAI_MPD218:
			case MODE::XTOUCH_R1: {
				if (value == uint8_t(127)) decPulseCount[ccs[cc]] += 2;
				else if (value == uint8_t(126)) decPulseCount[ccs[cc]] += 4;
				else if (value == uint8_t(125)) decPulseCount[ccs[cc]] += 6;
				if (value == uint8_t(1)) incPulseCount[ccs[cc]] += 2;
				else if (value == uint8_t(2)) incPulseCount[ccs[cc]] += 4;
				else if (value == uint8_t(3)) incPulseCount[ccs[cc]] += 6;
				break;
			}
		}

		values[cc] = value;
	}

	void learnCC(uint8_t cc) {
		if (learningId < 0) {
			return;
		}
		if (learnedCcs[learningId] >= 0) {
			ccs[learnedCcs[learningId]] = -1;
		}
		if (ccs[cc] >= 0) {
			learnedCcs[ccs[cc]] = -1;
		}
		ccs[cc] = learningId;
		learnedCcs[learningId] = cc;
		learningId = -1;
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "panelTheme", json_integer(panelTheme));
		json_object_set_new(rootJ, "mode", json_integer(mode));
		json_object_set_new(rootJ, "polyphonicOutput", json_boolean(polyphonicOutput));

		json_t* ccsJ = json_array();
		for (int i = 0; i < CHANNELS; i++) {
			json_array_append_new(ccsJ, json_integer(learnedCcs[i]));
		}
		json_object_set_new(rootJ, "ccs", ccsJ);

		json_object_set_new(rootJ, "midi", midiInput.toJson());
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		for (int i = 0; i < 128; i++) {
			ccs[i] = -1;
		}

		panelTheme = json_integer_value(json_object_get(rootJ, "panelTheme"));
		mode = (MODE)json_integer_value(json_object_get(rootJ, "mode"));
		polyphonicOutput = json_boolean_value(json_object_get(rootJ, "polyphonicOutput"));

		json_t* ccsJ = json_object_get(rootJ, "ccs");
		if (ccsJ) {
			for (int i = 0; i < CHANNELS; i++) {
				json_t* ccJ = json_array_get(ccsJ, i);
				if (ccJ) {
					learnedCcs[i] = json_integer_value(ccJ);
					ccs[learnedCcs[i]] = i;
				}
			}
		}

		json_t* midiJ = json_object_get(rootJ, "midi");
		if (midiJ) midiInput.fromJson(midiJ);
	}
};


template < int CHANNELS, int PORTS, class COICE >
struct MidiStepLedDisplay : LedDisplay {
	LedDisplaySeparator* hSeparators[CHANNELS / 4];
	LedDisplaySeparator* vSeparators[4];
	COICE* choices[4][CHANNELS / 4];

	void setModule(MidiStepModule* module) {
		Vec pos = Vec(0, 0);

		// Add vSeparators
		for (int x = 1; x < 4; x++) {
			vSeparators[x] = createWidget<LedDisplaySeparator>(pos);
			vSeparators[x]->box.pos.x = box.size.x / 4 * x;
			addChild(vSeparators[x]);
		}
		// Add hSeparators and choice widgets
		for (int y = 0; y < CHANNELS / 4; y++) {
			hSeparators[y] = createWidget<LedDisplaySeparator>(pos);
			hSeparators[y]->box.size.x = box.size.x;
			addChild(hSeparators[y]);
			for (int x = 0; x < 4; x++) {
				choices[x][y] = new COICE;
				choices[x][y]->box.pos = pos;
				choices[x][y]->setId(4 * y + x);
				choices[x][y]->box.size.x = box.size.x / 4;
				choices[x][y]->box.pos.x = box.size.x / 4 * x;
				choices[x][y]->setModule(module);
				addChild(choices[x][y]);
			}
			pos = choices[0][y]->box.getBottomLeft();
		}
		for (int x = 1; x < 4; x++) {
			vSeparators[x]->box.size.y = pos.y - vSeparators[x]->box.pos.y;
		}
	}
};


template < int CHANNELS, int PORTS >
struct MidiStepCcChoice : LedDisplayCenterChoiceEx {
	MidiStepModule* module;
	int id;
	int focusCc;

	void setModule(MidiStepModule* module) {
		this->module = module;
		box.size.y = mm2px(6.666);
		textOffset.y -= 1.4f;
		color = nvgRGB(0xf0, 0xf0, 0xf0);
	}

	void setId(int id) {
		this->id = id;
	}

	void step() override {
		if (!module) {
			text = string::f("%d", id);
			return;
		}
		
		if (module->learningId == id) {
			if (0 <= focusCc)
				text = string::f("%d", focusCc);
			else
				text = "LRN";
			color.a = 0.5f;
		}
		else {
			if (id < PORTS || module->polyphonicOutput) {
				text = module->learnedCcs[id] >= 0 ? string::f("%d", module->learnedCcs[id]) : "OFF";
				color.a = 1.0;
			}
			else {
				text = "-";
				color.a = 0.5f;
			}
			// HACK
			if (APP->event->getSelectedWidget() == this) {
				APP->event->setSelectedWidget(NULL);
			}
		}
	}

	void onButton(const event::Button& e) override {
		e.stopPropagating();
		if (!module) return;

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
			if (id < PORTS || module->polyphonicOutput) {
				e.consume(this);
				module->learningId = id;
				focusCc = -1;
			}
		}
	}

	void onDeselect(const event::Deselect& e) override {
		if (!module) return;
		if (module->learningId == id) {
			if (0 <= focusCc && focusCc < 128) {
				module->learnCC(focusCc);
			}
			module->learningId = -1;
		}
	}

	void onSelectText(const event::SelectText& e) override {
		int c = e.codepoint - '0';
		if (0 <= c && c <= 9) {
			if (focusCc < 0)
				focusCc = 0;
			focusCc = focusCc * 10 + c;
		}
		if (focusCc >= 128)
			focusCc = 0;
		e.consume(this);
	}

	void onSelectKey(const event::SelectKey& e) override {
		if ((e.key == GLFW_KEY_ENTER || e.key == GLFW_KEY_KP_ENTER) && e.action == GLFW_PRESS && (e.mods & RACK_MOD_MASK) == 0) {
			event::Deselect eDeselect;
			onDeselect(eDeselect);
			APP->event->selectedWidget = NULL;
			e.consume(this);
		}
	}
};

struct MidiStepWidget : ThemedModuleWidget<MidiStepModule> {
	typedef MidiStepModule MODULE;
	MidiStepWidget(MODULE* module)
		: ThemedModuleWidget<MODULE>(module, "MidiStep") {
		setModule(module);

		addChild(createWidget<StoermelderBlackScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<StoermelderBlackScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<StoermelderBlackScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<StoermelderBlackScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		MidiWidget<>* midiInputWidget = createWidget<MidiWidget<>>(Vec(10.0f, 36.4f));
		midiInputWidget->box.size = Vec(130.0f, 67.0f);
		midiInputWidget->setMidiPort(module ? &module->midiInput : NULL);
		addChild(midiInputWidget);

		MidiStepLedDisplay<16, 8, MidiStepCcChoice<16, 8>>* midiWidget = createWidget<MidiStepLedDisplay<16, 8, MidiStepCcChoice<16, 8>>>(Vec(10.f, 108.7f));
		midiWidget->box.size = Vec(130.0f, 79.0f);
		midiWidget->setModule(module);
		addChild(midiWidget);

		addOutput(createOutputCentered<StoermelderPort>(Vec(27.9f, 232.7f), module, MODULE::OUTPUT_INC + 0));
		addOutput(createOutputCentered<StoermelderPort>(Vec(56.1f, 232.7f), module, MODULE::OUTPUT_INC + 1));
		addOutput(createOutputCentered<StoermelderPort>(Vec(93.9f, 232.7f), module, MODULE::OUTPUT_INC + 2));
		addOutput(createOutputCentered<StoermelderPort>(Vec(122.1f, 232.7f), module, MODULE::OUTPUT_INC + 3));
		addOutput(createOutputCentered<StoermelderPort>(Vec(27.9f, 261.0f), module, MODULE::OUTPUT_INC + 4));
		addOutput(createOutputCentered<StoermelderPort>(Vec(56.1f, 261.0f), module, MODULE::OUTPUT_INC + 5));
		addOutput(createOutputCentered<StoermelderPort>(Vec(93.9f, 261.0f), module, MODULE::OUTPUT_INC + 6));
		addOutput(createOutputCentered<StoermelderPort>(Vec(122.1f, 261.0f), module, MODULE::OUTPUT_INC + 7));

		addOutput(createOutputCentered<StoermelderPort>(Vec(27.9f, 298.8f), module, MODULE::OUTPUT_DEC + 0));
		addOutput(createOutputCentered<StoermelderPort>(Vec(56.1f, 298.8f), module, MODULE::OUTPUT_DEC + 1));
		addOutput(createOutputCentered<StoermelderPort>(Vec(93.9f, 298.8f), module, MODULE::OUTPUT_DEC + 2));
		addOutput(createOutputCentered<StoermelderPort>(Vec(122.1f, 298.8f), module, MODULE::OUTPUT_DEC + 3));
		addOutput(createOutputCentered<StoermelderPort>(Vec(27.9f, 327.1f), module, MODULE::OUTPUT_DEC + 4));
		addOutput(createOutputCentered<StoermelderPort>(Vec(56.1f, 327.1f), module, MODULE::OUTPUT_DEC + 5));
		addOutput(createOutputCentered<StoermelderPort>(Vec(93.9f, 327.1f), module, MODULE::OUTPUT_DEC + 6));
		addOutput(createOutputCentered<StoermelderPort>(Vec(122.1f, 327.1f), module, MODULE::OUTPUT_DEC + 7));
	}

	void appendContextMenu(Menu* menu) override {
		ThemedModuleWidget<MODULE>::appendContextMenu(menu);
		MODULE* module = dynamic_cast<MODULE*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(StoermelderPackOne::Rack::createMapPtrSubmenuItem<MODE>("Protocol",
			{
				{ MODE::BEATSTEP_R1, "Beatstep Relative #1" },
				{ MODE::BEATSTEP_R2, "Beatstep Relative #2" },
				{ MODE::KK_REL, "NI Komplete Kontrol Relative" },
				{ MODE::XTOUCH_R1, "Behringer X-TOUCH Relative1" },
				{ MODE::XTOUCH_R2, "Behringer X-TOUCH Relative2" },
				{ MODE::AKAI_MPD218, "Akai MPD218 INC/DEC 2" }
			},
			&module->mode,
			false
		));
		menu->addChild(createBoolPtrMenuItem("Polyphonic output", "", &module->polyphonicOutput));
	}
};

} // namespace MidiStep
} // namespace StoermelderPackOne

Model* modelMidiStep = createModel<StoermelderPackOne::MidiStep::MidiStepModule, StoermelderPackOne::MidiStep::MidiStepWidget>("MidiStep");