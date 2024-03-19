#include "CatroModulo.hpp"
//#include "dsp/minblep.hpp" //got to figure this out...

//Catro-Module 8xlfo

struct LowFrequencyOscillator {
	
	float phase = 0.0f;
	float pshift = 0.0f;
	float typemix = 0.0f;
	float pw = 0.5f;
	float freq = 1.0f;
	bool invert = false;
	bool paused = false;
	dsp::SchmittTrigger resetTrigger;

	LowFrequencyOscillator() {}
	void setPitch(float pitch) {
			//pitch = fminf(pitch, 13.0f);
			freq = powf(2.0f, pitch);
	}

	void setPitch(int multi, float base) {
		switch (multi){
			case -8 : freq = base / 7680 ; break;
			case -7 : freq = base / 3840 ; break;
			case -6 : freq = base / 1920 ; break;
			case -5 : freq = base / 1440 ; break;
			case -4 : freq = base / 960  ; break;
			case -3 : freq = base / 720  ; break;
			case -2 : freq = base / 480  ; break;
			case -1 : freq = base / 360  ; break;
			case 0  : freq = base / 240  ; break;
			case 1  : freq = base / 120  ; break;
			case 2  : freq = base / 60   ; break;
			case 3  : freq = base / 30   ; break;
			case 4  : freq = base / 15   ; break; 
			case 5  : freq = base / 10   ; break;
			case 6  : freq = base / 7.5  ; break;
			case 7  : freq = base / 5    ; break;
			case 8  : freq = base / 2.5  ; break;

			default : freq = 0; break;
		}
	}
	void setPitchU(float multi, float base) {
		freq = powf(multi, 3.0f) * (base / 240);
	}

	void setPulseWidth(float pw_) {
		const float pwMin = 0.01f;
		pw = clamp(pw_, pwMin, 1.0f - pwMin);
	}
	void setShift(float ps){
		pshift = ps;
	}
	void setMix(float sm){
		typemix = sm;
	}
	void setReset(float reset) {
		if (resetTrigger.process(reset * 100)) {
			phase = 0.0f;
		}
	}

	void pause(bool p) {
		paused = p;
	}

	void step(float dt) {
		float deltaPhase = fminf(freq * dt, 0.5f);
		phase += paused ? 0.0f : deltaPhase;
		if (phase >= 1.0f)
			phase -= 1.0f;

		pshift += phase;
		if(pshift >= 1.0f){
			pshift -= 1.0f;	
		}		
	}

	float sin() {
			return sinf(2*M_PI * pshift) * (invert ? -1.0f : 1.0f);
	}

	float tri(float x) {
		return 4.0f * fabsf(x - roundf(x));
	}
	float tri() {
			return -1.0f + tri(invert ? pshift - 0.25f : pshift - 0.75f);
	}
	float saw(float x) {
		return 2.0f * (x - roundf(x));
	}
	float saw() {
			return saw(pshift) * (invert ? -1.0f : 1.0f);
	}
	float sqr() {
		float sqr = (pshift < pw) ^ invert ? 1.0f : -1.0f;
		return sqr;
	}

	//mixing the types
	float tmix() {
		float tmr = 0.0f;
		if (typemix == 0.0f){
			tmr = sin();
		}else if (typemix < 1.0f){
			tmr = (1.0f - typemix) * sin() + typemix * tri();
		}else if (typemix < 2.0f){
			typemix -= 1.0f;
			tmr = (1.0f - typemix) * tri() + typemix * saw();
		}else if (typemix < 3.0f){
			typemix -= 2.0f;
			tmr = (1.0f - typemix) * saw() + typemix * sqr();
		}else{
			tmr = sqr();
		}
		return tmr;
	}

	// float light() {
	// 	return sinf(2*M_PI * pshift);
	// }
};



struct CM1Module : Module {
	enum ParamIds {
		ENUMS(PARAM_TYPE, 8),
		ENUMS(PARAM_RATE, 8),
		ENUMS(PARAM_PW, 8),
		ENUMS(PARAM_PHASE, 8),
		PARAM_RESET,
		PARAM_OFFSET,
		PARAM_LOCK,
		PARAM_PAUSE,
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(INPUT_TYPE, 8),
		ENUMS(INPUT_RATE, 8),
		ENUMS(INPUT_PW, 8),
		ENUMS(INPUT_PHASE, 8),
		INPUT_RESET,
		INPUT_BPM,
		INPUT_PAUSE,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(OUTPUT_LFO, 8),
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS = 0
	};

	//generate 8 lfos
	LowFrequencyOscillator lfo[8];

	//bpm clock for synced lfo
	CM_BpmClock lfoclock;
	bool synced = false;

	CM1Module() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		int i = -1;
		while(++i < 8){
			configParam(CM1Module::PARAM_TYPE + i , 0.0f, 3.0f, 0.0f, "shape", "", 0.0f, 1.0f, 1.0f);
			configParam(CM1Module::PARAM_RATE + i , 0.0f, 1.0f, 0.5f, "rate", "", 0.0f, 16.0f, -8.0f);
			configParam(CM1Module::PARAM_PW + i ,0.001f, 1.0f, 0.5f, "pulse width", "%", 0.0f, 100.0f);
			configParam(CM1Module::PARAM_PHASE + i , 0.0f , 1.0f, 0.5f, "phase", "°", 0.0f, 360.0f, -180.0f);
		}
		configParam(CM1Module::PARAM_RESET, 0.0f, 1.0f, 0.0f, "reset");
		configParam(CM1Module::PARAM_OFFSET, 0.0f, 1.0f, 0.0f, "offset", "V", 0.0f, 5.0f);
		configParam(CM1Module::PARAM_LOCK, 0.0f, 1.0f, 0.0f, "lock", "", 0.0f, -1.0f, 1.0f);
		configParam(CM1Module::PARAM_PAUSE, 0.0f, 1.0f, 0.0f, "pause");

	}
	void process(const ProcessArgs &args) override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - dataToJson, dataFromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

//Tarzan - this is for TJ :)

void CM1Module::process(const ProcessArgs &args) {
	float offset = 5.0f * params[PARAM_OFFSET].getValue();
	float reset = (inputs[INPUT_RESET].getVoltage() || params[PARAM_RESET].getValue());
	bool paused = (inputs[INPUT_PAUSE].getVoltage() || params[PARAM_PAUSE].getValue());

	if (inputs[INPUT_BPM].isConnected()){
		lfoclock.setcv(inputs[INPUT_BPM].getVoltage());
		synced = true;
	}else{
		synced = false;
	}

	for (int i = 0; i < 8; i++) {
		float out;
		if (outputs[OUTPUT_LFO + i].isConnected() == true){
			//merge modulations
			float mod_one = clamp((inputs[INPUT_TYPE + i].isConnected()) ? params[PARAM_TYPE + i].getValue() * inputs[INPUT_TYPE + i].getVoltage() * 0.1f : params[PARAM_TYPE + i].getValue(), 0.0f, 3.0f); //TYPE
			float mod_two;
			if (synced == true){
				if (params[PARAM_LOCK].getValue() == 0){
					mod_two = roundf(clamp((inputs[INPUT_RATE + i].isConnected()) ? (params[PARAM_RATE + i].getValue() * 16.0f - 8.0f) + (inputs[INPUT_RATE + i].getVoltage() * 0.8f) : params[PARAM_RATE + i].getValue() * 16.0f - 8.0f , -8.0f, 8.0f)); //RATE;
				}else{
					mod_two = clamp((inputs[INPUT_RATE + i].isConnected()) ? (params[PARAM_RATE + i].getValue() * 2.0f) + (inputs[INPUT_RATE + i].getVoltage() * 0.2f) : params[PARAM_RATE + i].getValue() * 2.0f , 0.0f, 4.0f); //RATE
				}
			}else{
				mod_two = clamp((inputs[INPUT_RATE + i].isConnected()) ? (params[PARAM_RATE + i].getValue() * 21.0 - 8.0) + (inputs[INPUT_RATE + i].getVoltage() * 2.1) : params[PARAM_RATE + i].getValue() * 21.0 - 8.0 , -8.0f, 13.0f); //RATE
			}
			float mod_thr = clamp((inputs[INPUT_PW + i].isConnected()) ? params[PARAM_PW + i].getValue() * (1.0 + inputs[INPUT_PW + i].getVoltage()) * 0.05f : params[PARAM_PW + i].getValue(), 0.0f, 1.0f); //PW
			float mod_fou = clamp((inputs[INPUT_PHASE + i].isConnected()) ? params[PARAM_PHASE + i].getValue() + inputs[INPUT_PHASE + i].getVoltage() * 0.1f : params[PARAM_PHASE + i].getValue(), 0.0f, 1.0f); //PHASE
			

			//set lfo mods
			lfo[i].pause(paused);
			lfo[i].setMix(mod_one);
			if (synced == true){
				if (params[PARAM_LOCK].getValue()){
					lfo[i].setPitchU(mod_two, lfoclock.getbpm());
				}else{
					lfo[i].setPitch(mod_two, lfoclock.getbpm());
				}
			}else{
				lfo[i].setPitch(mod_two);
			}
			lfo[i].setPulseWidth(mod_thr);
			lfo[i].setShift(mod_fou);

			//run lfo
			lfo[i].step(args.sampleTime);
			lfo[i].setReset(reset);

			out = 5.0f * lfo[i].tmix() + offset;
			outputs[OUTPUT_LFO + i].setVoltage(clamp(out, -10.0f, 10.0f) );
		}else{
			out = 0.0f;
			outputs[OUTPUT_LFO + i].setVoltage(0);
		}
		
	}
}


struct CM1ModuleWidget : ModuleWidget {
	CM1ModuleWidget(CM1Module *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CM-1.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		//GRID
		const float gridrowjacks[8] = {35.5, 74.3, 113.1, 151.9, 190.7, 229.5, 268.2, 307};
		const float gridcoljacks[10] = {3.7, 29.9, 64.8, 94.1, 126.0, 155.1, 186.4, 215.8, 249.2, 278.5};
		
		int i = -1;

		
		while(++i < 8){
		//INPUTS
			addInput(createInput<CM_Input_def>(Vec(gridcoljacks[0], gridrowjacks[i]), module, CM1Module::INPUT_TYPE + i));
			addInput(createInput<CM_Input_def>(Vec(gridcoljacks[2], gridrowjacks[i]), module, CM1Module::INPUT_RATE + i));
			addInput(createInput<CM_Input_def>(Vec(gridcoljacks[4], gridrowjacks[i]), module, CM1Module::INPUT_PW + i));
			addInput(createInput<CM_Input_def>(Vec(gridcoljacks[6], gridrowjacks[i]), module, CM1Module::INPUT_PHASE + i));
		//POTS
			addChild(createWidget<CM_Pot1_small>(Vec(gridcoljacks[1], gridrowjacks[i] - 15.0)));
			addChild(createWidget<CM_Pot1_small>(Vec(gridcoljacks[3], gridrowjacks[i] + 7.0)));
			addChild(createWidget<CM_Pot1_small>(Vec(gridcoljacks[5], gridrowjacks[i] - 15.0)));
			addChild(createWidget<CM_Pot1_small>(Vec(gridcoljacks[7], gridrowjacks[i] - 15.0)));
		//KNOBS
			addParam(createParam<CM_Knob_small_def_half>(Vec(gridcoljacks[1], gridrowjacks[i] - 15.0), module, CM1Module::PARAM_TYPE + i ));
			addParam(createParam<CM_Knob_small_def>(Vec(gridcoljacks[3], gridrowjacks[i] + 7.0), module, CM1Module::PARAM_RATE + i ));
			addParam(createParam<CM_Knob_small_def>(Vec(gridcoljacks[5], gridrowjacks[i] - 15.0), module, CM1Module::PARAM_PW + i ));
			addParam(createParam<CM_Knob_small_def>(Vec(gridcoljacks[7], gridrowjacks[i] - 15.0), module, CM1Module::PARAM_PHASE + i ));
		//OUTPUTS
			addOutput(createOutput<CM_Output_def>(Vec(gridcoljacks[8], gridrowjacks[i]), module, CM1Module::OUTPUT_LFO + i));
		}
		
		//RESET
		addParam(createParam<CM_I_def_tinybuttonL>(Vec(8.5, 339.2), module, CM1Module::PARAM_RESET));
		addInput(createInput<CM_Input_def>(Vec(17.4, 339.2), module, CM1Module::INPUT_RESET));

		//RESET
		addParam(createParam<CM_I_def_tinybuttonL>(Vec(214.3, 329.3), module, CM1Module::PARAM_PAUSE));
		addInput(createInput<CM_Input_def>(Vec(223.1, 329.3), module, CM1Module::INPUT_PAUSE));
		
		//OFFSET (+5V)
		addParam(createParam<CM_Switch_small>(Vec(48.3, 338.7), module, CM1Module::PARAM_OFFSET));

		//sync (bpm)
		addInput(createInput<CM_Input_bpm>(Vec(70.1, 339.2), module, CM1Module::INPUT_BPM));

		//synclock
		addParam(createParam<CM_Switch_smallh>(Vec(100.5, 343.2), module, CM1Module::PARAM_LOCK));


	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per pluginInstance, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelCM1Module = createModel<CM1Module, CM1ModuleWidget>("CatroModulo_CM-1");
