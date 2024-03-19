#include "plugin.hpp"


namespace rack {
namespace core {


struct GateMidiOutput : midi::Output {
	uint8_t vels[128];
	bool lastGates[128];
	int64_t frame = -1;

	GateMidiOutput() {
		reset();
	}

	void reset() {
		for (uint8_t note = 0; note < 128; note++) {
			vels[note] = 100;
			lastGates[note] = false;
		}
		Output::reset();
	}

	void panic() {
		// Send all note off commands
		for (uint8_t note = 0; note < 128; note++) {
			// Note off
			midi::Message m;
			m.setStatus(0x8);
			m.setNote(note);
			m.setValue(0);
			m.setFrame(frame);
			sendMessage(m);
			lastGates[note] = false;
		}
	}

	void setVelocity(uint8_t note, uint8_t vel) {
		vels[note] = vel;
	}

	void setGate(uint8_t note, bool gate) {
		if (gate && !lastGates[note]) {
			// Note on
			midi::Message m;
			m.setStatus(0x9);
			m.setNote(note);
			m.setValue(vels[note]);
			m.setFrame(frame);
			sendMessage(m);
		}
		else if (!gate && lastGates[note]) {
			// Note off
			midi::Message m;
			m.setStatus(0x8);
			m.setNote(note);
			m.setValue(vels[note]);
			m.setFrame(frame);
			sendMessage(m);
		}
		lastGates[note] = gate;
	}

	void setFrame(int64_t frame) {
		this->frame = frame;
	}
};


struct Gate_MIDI : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(GATE_INPUTS, 16),
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	GateMidiOutput midiOutput;
	bool velocityMode = false;
	int learningId = -1;
	int8_t learnedNotes[16] = {};
	dsp::SchmittTrigger cellTriggers[16];

	Gate_MIDI() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for (int id = 0; id < 16; id++)
			configInput(GATE_INPUTS + id, string::f("Cell %d", id + 1));
		onReset();
	}

	void onReset() override {
		for (int y = 0; y < 4; y++) {
			for (int x = 0; x < 4; x++) {
				learnedNotes[4 * y + x] = 36 + 4 * (3 - y) + x;
			}
		}
		learningId = -1;
		midiOutput.reset();
		midiOutput.midi::Output::reset();
	}

	void process(const ProcessArgs& args) override {
		midiOutput.setFrame(args.frame);

		for (int id = 0; id < 16; id++) {
			int8_t note = learnedNotes[id];
			if (note < 0)
				continue;

			if (velocityMode) {
				uint8_t vel = (uint8_t) clamp(std::round(inputs[GATE_INPUTS + id].getVoltage() / 10.f * 127), 0.f, 127.f);
				midiOutput.setVelocity(note, vel);
				midiOutput.setGate(note, vel > 0);
			}
			else {
				cellTriggers[id].process(inputs[GATE_INPUTS + id].getVoltage(), 0.1f, 2.f);
				midiOutput.setVelocity(note, 100);
				midiOutput.setGate(note, cellTriggers[id].isHigh());
			}
		}
	}

	void setLearnedNote(int id, int8_t note) {
		// Unset IDs of similar note
		if (note >= 0) {
			for (int id = 0; id < 16; id++) {
				if (learnedNotes[id] == note)
					learnedNotes[id] = -1;
			}
		}
		learnedNotes[id] = note;
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();

		json_t* notesJ = json_array();
		for (int id = 0; id < 16; id++) {
			json_t* noteJ = json_integer(learnedNotes[id]);
			json_array_append_new(notesJ, noteJ);
		}
		json_object_set_new(rootJ, "notes", notesJ);

		json_object_set_new(rootJ, "velocity", json_boolean(velocityMode));

		json_object_set_new(rootJ, "midi", midiOutput.toJson());
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* notesJ = json_object_get(rootJ, "notes");
		if (notesJ) {
			for (int id = 0; id < 16; id++) {
				json_t* noteJ = json_array_get(notesJ, id);
				if (noteJ)
					setLearnedNote(id, json_integer_value(noteJ));
			}
		}

		json_t* velocityJ = json_object_get(rootJ, "velocity");
		if (velocityJ)
			velocityMode = json_boolean_value(velocityJ);

		json_t* midiJ = json_object_get(rootJ, "midi");
		if (midiJ)
			midiOutput.fromJson(midiJ);
	}
};


struct Gate_MIDIWidget : ModuleWidget {
	Gate_MIDIWidget(Gate_MIDI* module) {
		setModule(module);
		setPanel(createPanel(asset::system("res/Core/Gate_MIDI.svg"), asset::system("res/Core/Gate_MIDI-dark.svg")));

		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(8.189, 78.431)), module, Gate_MIDI::GATE_INPUTS + 0));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(19.739, 78.431)), module, Gate_MIDI::GATE_INPUTS + 1));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(31.289, 78.431)), module, Gate_MIDI::GATE_INPUTS + 2));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(42.838, 78.431)), module, Gate_MIDI::GATE_INPUTS + 3));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(8.189, 89.946)), module, Gate_MIDI::GATE_INPUTS + 4));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(19.739, 89.946)), module, Gate_MIDI::GATE_INPUTS + 5));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(31.289, 89.946)), module, Gate_MIDI::GATE_INPUTS + 6));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(42.838, 89.946)), module, Gate_MIDI::GATE_INPUTS + 7));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(8.189, 101.466)), module, Gate_MIDI::GATE_INPUTS + 8));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(19.739, 101.466)), module, Gate_MIDI::GATE_INPUTS + 9));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(31.289, 101.466)), module, Gate_MIDI::GATE_INPUTS + 10));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(42.838, 101.466)), module, Gate_MIDI::GATE_INPUTS + 11));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(8.189, 112.998)), module, Gate_MIDI::GATE_INPUTS + 12));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(19.739, 112.984)), module, Gate_MIDI::GATE_INPUTS + 13));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(31.289, 112.984)), module, Gate_MIDI::GATE_INPUTS + 14));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(42.838, 112.984)), module, Gate_MIDI::GATE_INPUTS + 15));

		typedef Grid16MidiDisplay<NoteChoice<Gate_MIDI>> TMidiDisplay;
		TMidiDisplay* display = createWidget<TMidiDisplay>(mm2px(Vec(0.0, 13.039)));
		display->box.size = mm2px(Vec(50.8, 55.88));
		display->setMidiPort(module ? &module->midiOutput : NULL);
		display->setModule(module);
		addChild(display);
	}

	void appendContextMenu(Menu* menu) override {
		Gate_MIDI* module = dynamic_cast<Gate_MIDI*>(this->module);

		menu->addChild(new MenuSeparator);

		menu->addChild(createBoolPtrMenuItem("Velocity mode", "", &module->velocityMode));

		menu->addChild(createMenuItem("Panic", "",
			[=]() {module->midiOutput.panic();}
		));
	}
};


Model* modelGate_MIDI = createModel<Gate_MIDI, Gate_MIDIWidget>("CV-Gate");


} // namespace core
} // namespace rack
