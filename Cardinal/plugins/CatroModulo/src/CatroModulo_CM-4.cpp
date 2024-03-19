#include "CatroModulo.hpp"


//Catro-Module CM-4: vcClk

struct CM4Module : Module {

	enum ParamIds {
		PARAM_BPM,
		PARAM_RST,
		PARAM_SNAP,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_EXT,
		INPUT_BPM,
		INPUT_RST,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_BPM1,
		OUTPUT_BPM2,
		OUTPUT_D2,
		OUTPUT_X2,
		OUTPUT_CLKD2,
		OUTPUT_CLK,
		OUTPUT_CLKX2,
		OUTPUT_RST,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	float bpm_display = 0.0;
    //create objects
    CM_BpmClock bpmclock;
	bool isreset = false;
	
	CM4Module() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(CM4Module::PARAM_BPM, 0.0f, 5.0, 0.0f, "tempo", " BPM", 0.0f, 100.0f);
		configParam(CM4Module::PARAM_SNAP, 0.0f, 2.0f, 1.0f, "snap");
		configParam(CM4Module::PARAM_RST, 0.0f, 1.0f, 0.0f, "reset");

			//initialize objects
	}
	void process(const ProcessArgs &args) override;
};

void CM4Module::process(const ProcessArgs &args) {
	if (params[PARAM_SNAP].getValue() == 0){
		bpmclock.setbpm(int( (params[PARAM_BPM].getValue() * 100.0) * 100) / 100.0f );
	}else if (params[PARAM_SNAP].getValue() == 1){
		bpmclock.setbpm(int(params[PARAM_BPM].getValue() * 100.0));
	}else if (params[PARAM_SNAP].getValue() == 2){
		bpmclock.setbpm(int( (params[PARAM_BPM].getValue() * 100.0) * 0.2) * 5.0f );
	}
	
	outputs[OUTPUT_RST].setVoltage((inputs[INPUT_RST].getVoltage() || params[PARAM_RST].getValue()) * 10.0);
	bpmclock.setReset(outputs[OUTPUT_RST].getVoltage());
	
	//external clock
	if (inputs[INPUT_EXT].isConnected()){
		float extcv = bpmclock.exttrack(inputs[INPUT_EXT].getVoltage(), args.sampleTime);
		//extcv = (extcv > 0) ? (1.0f / extcv) * (60000.0f / args.sampleRate ) : 0.0f;//* 1.211387038158690f : 0.0f;
		if (params[PARAM_SNAP].getValue() == 0){
		bpmclock.addcv(bpmclock.bpmtocv(0.01) * roundf(extcv * 10000.0f));
		}else if (params[PARAM_SNAP].getValue() == 1){
		bpmclock.addcv(bpmclock.bpmtocv(1) * roundf(extcv * 100.0f));
		}else if (params[PARAM_SNAP].getValue() == 2){
		bpmclock.addcv(bpmclock.bpmtocv(5) * roundf(extcv * 20.0f));
		}
		isreset = false;
	}else if (isreset == false){
		bpmclock.extreset(true);
		isreset = true;
	}
	outputs[OUTPUT_BPM1].setVoltage(bpmclock.getcv());

	//add bpm cv
	outputs[OUTPUT_BPM2].setVoltage(bpmclock.addcv((inputs[INPUT_BPM].isConnected()) ? inputs[INPUT_BPM].getVoltage() : 0.0f));

	outputs[OUTPUT_D2].setVoltage(bpmclock.getcv() * 0.5);
	outputs[OUTPUT_X2].setVoltage(bpmclock.getcv() * 2.0);
	bpm_display = bpmclock.getbpm();

	bpmclock.step(args.sampleTime);

	outputs[OUTPUT_CLK].setVoltage(bpmclock.track(1) * 10.0);
	outputs[OUTPUT_CLKD2].setVoltage(bpmclock.track(2) * 10.0);
	outputs[OUTPUT_CLKX2].setVoltage(bpmclock.track(0) * 10.0);
}

struct CM4ModuleWidget : ModuleWidget {

	CM4ModuleWidget(CM4Module *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CM-4.svg")));

		//addChild(createWidget<ScrewSilver>(Vec(30, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 16, 0)));
		addChild(createWidget<ScrewSilver>(Vec(5, 365)));
		// addChild(createWidget<ScrewSilver>(Vec(box.size.x - 60, 365)));

		//UNIQUE ELEMENTS
		addParam(createParam<CM_Knob_huge_red_os>(Vec(3.6 , 56.0), module, CM4Module::PARAM_BPM));
		addParam(createParam<CM_Switch_small_3>(Vec(7.0, 43.0), module, CM4Module::PARAM_SNAP));

		addInput(createInput<CM_Input_ext>(Vec(0.0 , 126.3), module, CM4Module::INPUT_EXT));
		addInput(createInput<CM_Input_bpm>(Vec(7.0 , 169.1), module, CM4Module::INPUT_BPM));

		addOutput(createOutput<CM_Output_bpm>(Vec(44.4 , 126.3), module, CM4Module::OUTPUT_BPM1));
		addOutput(createOutput<CM_Output_bpm>(Vec(44.4 , 169.1), module, CM4Module::OUTPUT_BPM2));

		addOutput(createOutput<CM_Output_bpm>(Vec(7.0 , 212.0), module, CM4Module::OUTPUT_D2));
		addOutput(createOutput<CM_Output_bpm>(Vec(44.4 , 212.0), module, CM4Module::OUTPUT_X2));

		addOutput(createOutput<CM_Output_def>(Vec(26.1 , 293.9), module, CM4Module::OUTPUT_CLK));
		addOutput(createOutput<CM_Output_def>(Vec(3.5 , 326.5), module, CM4Module::OUTPUT_CLKD2));
		addOutput(createOutput<CM_Output_def>(Vec(48.1 , 326.5), module, CM4Module::OUTPUT_CLKX2));

		addInput(createInput<CM_Input_small>(Vec(6.2 , 251.8), module, CM4Module::INPUT_RST));
		addParam(createParam<CM_Button_small_red>(Vec(29.4 , 251.8), module, CM4Module::PARAM_RST));
		addOutput(createOutput<CM_Output_small>(Vec(52.4 , 251.8), module, CM4Module::OUTPUT_RST));

		if (module)
		{
			//LCD display
			NumDisplayWidget *display = new NumDisplayWidget();
			display->box.pos = Vec(7.0, 21.0);
			display->box.size = Vec(61.1, 20.4);
			display->value = &module->bpm_display;
			addChild(display);
		}
	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per pluginInstance, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelCM4Module = createModel<CM4Module, CM4ModuleWidget>("CatroModulo_CM-4");
