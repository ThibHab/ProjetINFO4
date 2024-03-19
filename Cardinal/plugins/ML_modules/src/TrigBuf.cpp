#include "ML_modules.hpp"
#include "SettingsHandler.hpp"


struct TrigBuf : Module {

	enum ParamIds {
		ARM1_PARAM,
		ARM2_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ARM1_INPUT,
		ARM2_INPUT,
		GATE1_INPUT,
		GATE2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ARM1_LIGHT,
		ARM2_LIGHT,
		NUM_LIGHTS
	};

	TrigBuf() {

		defaults.setModule("TrigBuf");

		config( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ); 

		configInput(ARM1_INPUT, "Arm" );
		configInput(ARM2_INPUT, "Arm" );
		configInput(GATE1_INPUT, "Trigger" );
		configInput(GATE2_INPUT, "Trigger" );
		configOutput(OUT1_OUTPUT, "Out");
		configOutput(OUT2_OUTPUT, "Out");

    	configButton(TrigBuf::ARM1_PARAM, "Arm");
    	configButton(TrigBuf::ARM2_PARAM, "Arm");

		armOnLoad = defaults.getBool("ArmOnLoad");

		onReset(); 
	};

	SettingsHandler defaults;


	void process(const ProcessArgs &args) override;

	float arm1[PORT_MAX_CHANNELS], arm2[PORT_MAX_CHANNELS];
	float out1[PORT_MAX_CHANNELS], out2[PORT_MAX_CHANNELS];

	bool gate1[PORT_MAX_CHANNELS],    gate2[PORT_MAX_CHANNELS];
	bool delayed1[PORT_MAX_CHANNELS], delayed2[PORT_MAX_CHANNELS];

	bool armOnLoad;

	dsp::SchmittTrigger armTrigger1[PORT_MAX_CHANNELS],  armTrigger2[PORT_MAX_CHANNELS];
	dsp::SchmittTrigger gateTrigger1[PORT_MAX_CHANNELS], gateTrigger2[PORT_MAX_CHANNELS];

	void onReset() override {

		memset(arm1, 0, PORT_MAX_CHANNELS*sizeof(float));
		memset(arm2, 0, PORT_MAX_CHANNELS*sizeof(float));
		memset(out1, 0, PORT_MAX_CHANNELS*sizeof(float));
		memset(out2, 0, PORT_MAX_CHANNELS*sizeof(float));

		memset(gate1, 0, PORT_MAX_CHANNELS*sizeof(bool));
		memset(gate2, 0, PORT_MAX_CHANNELS*sizeof(bool));
		memset(delayed1, 0, PORT_MAX_CHANNELS*sizeof(bool));
		memset(delayed2, 0, PORT_MAX_CHANNELS*sizeof(bool));	

		if(armOnLoad) {
			for (int i=0; i<PORT_MAX_CHANNELS; i++) {
				arm1[i] = 10.0;
				arm2[i] = 10.0;
			};
		};
	};

    json_t *dataToJson() override {
        json_t *rootJ = json_object();

        json_t *armPolyJ = json_array();
            for (int i = 0; i < PORT_MAX_CHANNELS; i++) {
                    json_t *arm1J = json_real( arm1[i]);
                    json_array_append_new(armPolyJ, arm1J);
                    json_t *arm2J = json_real( arm2[i]);
                    json_array_append_new(armPolyJ, arm2J);
            }

		json_object_set_new(rootJ, "armOnLoad", json_integer(armOnLoad));
		json_object_set_new(rootJ, "armPoly", armPolyJ);

        return rootJ;
    }

    void dataFromJson(json_t *rootJ) override {
        json_t *armOnLoadJ = json_object_get(rootJ, "armOnLoad");
        if(armOnLoadJ) armOnLoad = !!json_integer_value(armOnLoadJ);

        json_t *armPolyJ = json_object_get(rootJ, "armPoly");
		if(armPolyJ) {
	        for (int i = 0; i < PORT_MAX_CHANNELS; i++) {
	            json_t *arm1J = json_array_get(armPolyJ, 2*i);
	            arm1[i] = json_real_value(arm1J);
	            json_t *arm2J = json_array_get(armPolyJ, 2*i+1);
	            arm2[i] = json_real_value(arm2J);
	        }
		}

		if(armOnLoad) {
			for (int i=0; i<PORT_MAX_CHANNELS; i++) {
				arm1[i] = 10.0;
				arm2[i] = 10.0;
			};
		};


    }

private:

	bool neg_slope(bool gate, bool last_gate) { return (gate!=last_gate) && last_gate; }

};



void TrigBuf::process(const ProcessArgs &args) {

	bool last_gate1[PORT_MAX_CHANNELS];
	bool last_gate2[PORT_MAX_CHANNELS];

	memcpy(last_gate1, gate1, PORT_MAX_CHANNELS * sizeof(bool));
	memcpy(last_gate2, gate2, PORT_MAX_CHANNELS * sizeof(bool));

	float gate1_in[PORT_MAX_CHANNELS], gate2_in[PORT_MAX_CHANNELS];

	memset(gate1_in, 0, PORT_MAX_CHANNELS*sizeof(float));
	memset(gate2_in, 0, PORT_MAX_CHANNELS*sizeof(float));

	int arm1_channels = inputs[ARM1_INPUT].getChannels(); arm1_channels = MAX(1, arm1_channels);
	int arm2_channels = inputs[ARM2_INPUT].getChannels(); arm2_channels = MAX(1, arm2_channels);

	int gate1_channels = inputs[GATE1_INPUT].getChannels();
	int gate2_channels = inputs[GATE2_INPUT].getChannels();

	int channels_1 = arm1_channels==1?gate1_channels:MIN(arm1_channels, gate1_channels); channels_1 = MAX(1, channels_1);
	int channels_2 = arm2_channels==1?gate2_channels:MIN(arm2_channels, gate2_channels); channels_2 = MAX(1, channels_2);

	memcpy(gate1_in, inputs[GATE1_INPUT].getVoltages(), gate1_channels*sizeof(float) );
	if(inputs[GATE2_INPUT].isConnected()) {
		memcpy(gate2_in, inputs[GATE2_INPUT].getVoltages(), gate2_channels*sizeof(float) );
	} else {
		gate2_channels = gate1_channels;
		memcpy(gate2_in, gate1_in, gate2_channels*sizeof(float) );
	}


	// if gateX_channel == 1: use same gate (clock) for all channels
	// if armX_channels == 1: use same arm for all channels
	// if both > 1 channels_X = MIN(gateX_channels, armX_Channels) (other channels will never be triggered)

	float button1 = params[ARM1_PARAM].getValue();

	for(int c=0; c < channels_1; c++) {

		gateTrigger1[c].process(gate1_in[c]);
		gate1[c] = gateTrigger1[c].isHigh();

		if( armTrigger1[c].process(inputs[ARM1_INPUT].getNormalPolyVoltage(0.0f, c) + button1 ) ) { 
			if (!gate1[c]) {arm1[c] = 10.0;}
			else { delayed1[c] = true;}
		}

	  if(gate1[c]) {
			out1[c] = (arm1[c] > 5.0f) ? 10.0f : 0.0f;
		} 
		else {
			if(out1[c] > 5.0f) {
				arm1[c] = 0.0f;
				out1[c] = 0.0f;
			};
		};

		if( delayed1[c] && neg_slope(gate1[c], last_gate1[c]) ) {
			arm1[c] = 10.0;
			delayed1[c] = false;
		};

	}

	float button2 = params[ARM2_PARAM].getValue();

	for(int c=0; c < channels_2; c++) {

		gateTrigger2[c].process(gate2_in[c]);
		gate2[c] = gateTrigger2[c].isHigh();
	
		if( armTrigger2[c].process(inputs[ARM2_INPUT].getNormalPolyVoltage(inputs[ARM1_INPUT].getNormalPolyVoltage(0.0f, c), c) + button2 ) ) { 
			if (!gate2[c]) {arm2[c] = 10.0f;}
			else {delayed2[c] = true;};
		};
	
		if (gate2[c]) {
			out2[c] = (arm2[c] > 5.0f)? 10.0f : 0.0f;
		} else {
			if(out2[c] > 5.0) {
				arm2[c] = 0.0;
				out2[c] = 0.0;
			};
		};

		if( delayed2[c] && neg_slope(gate2[c], last_gate2[c]) ) {
			arm2[c] = 10.0f;
			delayed2[c] = false;
		};

	}


	outputs[OUT1_OUTPUT].setChannels(channels_1);
	outputs[OUT2_OUTPUT].setChannels(channels_2);

	outputs[OUT1_OUTPUT].writeVoltages(out1);
	outputs[OUT2_OUTPUT].writeVoltages(out2);

	lights[ARM1_LIGHT].setBrightness(arm1[0]);
	lights[ARM2_LIGHT].setBrightness(arm2[0]);
};


struct TrigBufDefaultItem : MenuItem {

        TrigBuf *trigBuf;
    
        void onAction(const event::Action &e) override {
                trigBuf->armOnLoad = !trigBuf->armOnLoad;
				trigBuf->defaults.setBool("ArmOnLoad", trigBuf->armOnLoad);
        };

        void step() override {
                rightText = (trigBuf->armOnLoad == true)? "✔" : "";
        };

};

struct TrigBufWriteDefaultItem : MenuItem {
    TrigBuf *trigBuf;
    
    void onAction(const event::Action &e) override {
        trigBuf->defaults.setBool("ArmOnLoad", trigBuf->armOnLoad);
    };
};


struct TrigBufWidget : ModuleWidget {
	TrigBufWidget(TrigBuf *module);
	void appendContextMenu(Menu *) override;
};

void TrigBufWidget::appendContextMenu(Menu *menu) {

        TrigBuf *trigBuf = dynamic_cast<TrigBuf*>(module);
        assert(trigBuf);

        menu->addChild(construct<MenuLabel>());

        TrigBufDefaultItem *armOnLoad_Item = new TrigBufDefaultItem();
        armOnLoad_Item->text = "Arm on Load";
        armOnLoad_Item->trigBuf = trigBuf;
        menu->addChild(armOnLoad_Item);

        TrigBufWriteDefaultItem *writeDefault_Item = new TrigBufWriteDefaultItem();
        writeDefault_Item->text = "Save Default";
        writeDefault_Item->trigBuf = trigBuf;
        menu->addChild(writeDefault_Item);

};

TrigBufWidget::TrigBufWidget(TrigBuf *module) {
		setModule(module);

	box.size = Vec(15*4, 380);

	{
		SvgPanel *panel = new SvgPanel();
		panel->box.size = box.size;
		panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance,"res/TrigBuf.svg")));
		addChild(panel);
	}

	addChild(createWidget<MLScrew>(Vec(15, 0)));
	addChild(createWidget<MLScrew>(Vec(15, 365)));

	addInput(createInput<MLPort>(Vec(9, 62), module, TrigBuf::ARM1_INPUT));
	addInput(createInput<MLPort>(Vec(9, 105), module, TrigBuf::GATE1_INPUT));
	addOutput(createOutput<MLPortOut>(Vec(9, 150), module, TrigBuf::OUT1_OUTPUT));

	addParam(createParam<ML_SmallLEDButton>(Vec(40,66), module, TrigBuf::ARM1_PARAM));
	addChild(createLight<MLSmallLight<GreenLight>>(Vec(44, 70), module, TrigBuf::ARM1_LIGHT));

	addInput(createInput<MLPort>(Vec(9, 218), module, TrigBuf::ARM2_INPUT));
	addInput(createInput<MLPort>(Vec(9, 263), module, TrigBuf::GATE2_INPUT));
	addOutput(createOutput<MLPortOut>(Vec(9, 305), module, TrigBuf::OUT2_OUTPUT));

	addParam(createParam<ML_SmallLEDButton>(Vec(40,222), module, TrigBuf::ARM2_PARAM));
	addChild(createLight<MLSmallLight<GreenLight>>(Vec(44, 226), module, TrigBuf::ARM2_LIGHT));
}



Model *modelTrigBuf = createModel<TrigBuf, TrigBufWidget>("TrigBuf");
