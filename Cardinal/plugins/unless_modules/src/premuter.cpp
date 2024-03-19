#include "unless.hpp"
#include "utils.hpp"
#include "../art/components.hpp"

#define MODULE_WIDTH 2

struct Premuter : Module {
  enum ParamIds {
    DURATION_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    LEFT_INPUT,
    RIGHT_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    LEFT_OUTPUT,
    RIGHT_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  int sample = 0;
  int sample_rate = -1;
  
  void (Premuter::*step_fun)();

  void forward_inputs(){
    outputs[LEFT_OUTPUT].setVoltage(inputs[LEFT_INPUT].getVoltage());
    outputs[RIGHT_OUTPUT].setVoltage(inputs[RIGHT_INPUT].getVoltage());
  }

  void mute_step() {
    int mute_duration = sample_rate * params[DURATION_PARAM].getValue();
    if(sample < mute_duration){
      // SILENCE!
      sample++;
    }else{
      if(sample - mute_duration < sample_rate){
        // ANTI-POPPIN
        sample++;
        float t = (float)(sample - mute_duration) / (float)sample_rate;
        t = t * t;
        outputs[LEFT_OUTPUT].setVoltage(inputs[LEFT_INPUT].getVoltage() * t);
        outputs[RIGHT_OUTPUT].setVoltage(inputs[RIGHT_INPUT].getVoltage() * t);
      }else{
        // SWITCH TO FORWARDING INPUTS
        step_fun = &Premuter::forward_inputs;
      }
    }
  }

  void process(const ProcessArgs &args) override {
    (this->*step_fun)();
  }

  Premuter(){
    sample_rate = APP->engine->getSampleRate();
    step_fun = &Premuter::mute_step;
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(DURATION_PARAM, 0, 15, 1, "duration of pre-muting", " seconds (+1s fade in)");
    configInput(LEFT_INPUT, "left");
    configInput(RIGHT_INPUT, "right");
    configOutput(LEFT_OUTPUT, "left");
    configOutput(RIGHT_OUTPUT, "right");
    onReset();
  }
};

struct PremuterPanel : _less::Widget {
  Premuter* module;
  PremuterPanel(){
  }
  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
  }
  void render() override{
    fill(theme.bg);
    rect(0,0,box.size.x, box.size.y);
    fill(theme.fg);
    float fs = 12.0f;
    text("p", center.x, RACK_GRID_WIDTH, fs);
    text("m", center.x, RACK_GRID_WIDTH * 2, fs);
    text("u", center.x, box.size.y - RACK_GRID_WIDTH * 3, fs);
    text("g", center.x, box.size.y - RACK_GRID_WIDTH * 2, fs);
    text("~", center.x, box.size.y - RACK_GRID_WIDTH * 0.6f, fs);
  }
};

struct PremuterWidget : ModuleWidget {
  PremuterWidget(Premuter *module){
    setModule(module);
    box.size = Vec(MODULE_WIDTH * RACK_GRID_WIDTH, 380);

    _less::Panel *panelWidget = new _less::Panel(box.size, nvgHSLA(0, 0, 0, 0));
    PremuterPanel *panel = new PremuterPanel();
    panel->module = module;
    panel->set_box(0.0f,0.0f,box.size.x, box.size.y);

    panelWidget->fb->addChild(panel);
    addChild(panelWidget);

    float x = box.size.x * 0.5f;
    float y = 60.0f;
    float h = 30.0f;

    _less::Knob<unless::InputKnob> *k = new _less::Knob<unless::InputKnob>(
      Vec(x, y),
      Vec(20.0f, 20.0f),
      module, 
      Premuter::DURATION_PARAM,
      true
    );
    k->sw->thickness = 1.0f;
    addParam(k);

    addInput(
      new _less::Port<unless::RingHoleInput>(
        Vec(x, y + h),
        module, 
        Premuter::LEFT_INPUT
    ));

    addInput(
      new _less::Port<unless::RingHoleInput>(
        Vec(x, y + h + h),
        module, 
        Premuter::RIGHT_INPUT
    ));

    y = RACK_GRID_HEIGHT - 80.0f - h;
    addOutput(
      new _less::Port<unless::RingHoleOutput>(
        Vec(x, y),
        module, 
        Premuter::LEFT_OUTPUT
    ));

    addOutput(
      new _less::Port<unless::RingHoleOutput>(
        Vec(x, y + h),
        module, 
        Premuter::RIGHT_OUTPUT
    ));

  }
};

Model *modelPremuter = createModel<Premuter, PremuterWidget>("premuter");