#include "ML_modules.hpp"


struct TrigSwitch : Module {
	enum ParamIds {
		STEP_PARAM,
		NUM_PARAMS = STEP_PARAM + 9
	};
	enum InputIds {
		TRIG_INPUT,
		CV_INPUT = TRIG_INPUT + 8,
		NUM_INPUTS = CV_INPUT + 8
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		STEP_LIGHT,
		NUM_LIGHTS = STEP_LIGHT+8
	};

	TrigSwitch() {
		config( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS );
        for (int i=0; i<8; i++) {
			configInput(TRIG_INPUT+i, "set #"+std::to_string(i+1));
			configInput(CV_INPUT+i,   "CV #"      +std::to_string(i+1));
			configButton(TrigSwitch::STEP_PARAM + i, "set #"+std::to_string(i+1));
		}
		configOutput(OUT_OUTPUT, "CV");
		onReset(); 
	};

	void process(const ProcessArgs &args) override;

	int position=0;

    const float in_min[4] = {0.0, 0.0, 0.0, -5.0};
    const float in_max[4] = {8.0, 6.0, 10.0, 5.0};


	dsp::SchmittTrigger stepTriggers[8];


	void onReset() override {

		position = 0;
		for(int i=0; i<8; i++) lights[i].value = 0.0;
	};


	json_t *dataToJson() override {

		json_t *rootJ = json_object();
	
		json_object_set_new(rootJ, "position", json_integer(position));
	
		return rootJ;
	};
	
	void dataFromJson(json_t *rootJ) override {

		json_t *positionJ = json_object_get(rootJ, "position");
		if(positionJ) position = json_integer_value(positionJ);

	};

};


void TrigSwitch::process(const ProcessArgs &args) {

	float values[PORT_MAX_CHANNELS];

	memset(values, 0, 16*sizeof(float));

	for(int i=0; i<8; i++) {
		if( stepTriggers[i].process( inputs[TRIG_INPUT+i].getNormalVoltage(0.0))  + params[STEP_PARAM+i].getValue() ) position = i;
		lights[i].value = (i==position)?1.0:0.0;
	};

	if(inputs[CV_INPUT+position].isConnected()) inputs[CV_INPUT+position].readVoltages(values);
	outputs[OUT_OUTPUT].setChannels( inputs[CV_INPUT+position].getChannels() );
	outputs[OUT_OUTPUT].writeVoltages(values);
};



struct TrigSwitchWidget : ModuleWidget {
	TrigSwitchWidget(TrigSwitch *module);
};

TrigSwitchWidget::TrigSwitchWidget(TrigSwitch *module) {
		
	setModule(module);

	box.size = Vec(15*8, 380);

	{
		SvgPanel *panel = new SvgPanel();
		panel->box.size = box.size;
		panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance,"res/TrigSwitch.svg")));
		addChild(panel);
	}

	addChild(createWidget<MLScrew>(Vec(15, 0)));
	addChild(createWidget<MLScrew>(Vec(box.size.x-30, 0)));
	addChild(createWidget<MLScrew>(Vec(15, 365)));
	addChild(createWidget<MLScrew>(Vec(box.size.x-30, 365)));


	const float offset_y = 60, delta_y = 32, row1=14, row2 = 50, row3 = 79;

	for (int i=0; i<8; i++) {

		addInput(createInput<MLPort>(             Vec(row1, offset_y + i*delta_y), module, TrigSwitch::TRIG_INPUT + i));

		addParam(createParam<ML_MediumLEDButton>(Vec(row2 , offset_y + i*delta_y +3 ), module, TrigSwitch::STEP_PARAM + i)); 
		addChild(createLight<MLMediumLight<GreenLight>>( Vec(row2 + 4, offset_y + i*delta_y + 7), module, TrigSwitch::STEP_LIGHT+i));
		
		addInput(createInput<MLPort>(             Vec(row3, offset_y + i*delta_y), module, TrigSwitch::CV_INPUT + i));

	}
	addOutput(createOutput<MLPortOut>(Vec(row3, 320), module, TrigSwitch::OUT_OUTPUT));

}

Model *modelTrigSwitch = createModel<TrigSwitch, TrigSwitchWidget>("TrigSwitch");
