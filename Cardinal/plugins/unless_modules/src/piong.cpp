#include "unless.hpp"
#include "widgets.hpp"
#include "dsp/digital.hpp"
#include <math.h> 


struct Piong : Module {
  enum ParamIds {
    P1_PARAM,
    P2_PARAM,
    S1_PARAM,
    S2_PARAM,
    SPEED_PARAM,
    AUTOBALL_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    P1_INPUT,
    P2_INPUT,
    S1_INPUT,
    S2_INPUT,
    ANGLE_INPUT,
    Y_INPUT,
    BALL_INPUT,
    CLK_INPUT,
    SPEED_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    HIT1_OUTPUT,
    HIT2_OUTPUT,
    LEFT_OUTPUT,
    RIGHT_OUTPUT,
    WALLT_OUTPUT,
    WALLB_OUTPUT,
    X_OUTPUT,
    Y_OUTPUT,
    CENTER_OUTPUT,
    VCCENTER_OUTPUT,
    VCHIT1_OUTPUT,
    VCHIT2_OUTPUT,
    VCLEFT_OUTPUT,
    VCRIGHT_OUTPUT,
    VCWALLT_OUTPUT,
    VCWALLB_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  rack::dsp::SchmittTrigger gateTrigger;

  rack::dsp::PulseGenerator pulses[9];

  Vec dir = Vec(0.0f, 0.0f);

  Vec pos = Vec(0.5f,0.5f);
  float p1 = 0.0f;
  float p2 = 0.0f;
  float s1 = 0.0f;
  float s2 = 0.0f;
  float p1last = 0.0f;
  float p2last = 0.0f;
  float speed = 0.0f;
  float PI = 3.141592f;
  int score1 = 0;
  int score2 = 0;
  float aspectRatio = 180.f / 135.0f;
  bool crossedCenter = false;
  bool served = false;
  int lastHit = 0;

  bool ball = false;
  bool updated = false;
  float ballWidth = 0.02f;
  float playerWidth = 0.04f;

  Piong(){
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configSwitch(AUTOBALL_PARAM, 0, 1, 1, "spawn new balls", {"manual", "auto"});
    configParam(P1_PARAM, 1.0, 0.0, 0.5, "player 1 position");
    configParam(S1_PARAM, 0.0, 1.0, 0.15, "player 1 size");
    configParam(S2_PARAM, 0.0, 1.0, 0.15, "player 2 size");
    configParam(P2_PARAM, 1.0, 0.0, 0.5, "player 2 position");

    configInput(P1_INPUT, "player 1 position");
    configInput(P2_INPUT, "player 2 position");
    configInput(S1_INPUT, "player 1 size");
    configInput(S2_INPUT, "player 2 size");
    configInput(ANGLE_INPUT, "angle");
    configInput(Y_INPUT, "ball y");
    configInput(BALL_INPUT, "spawn ball trigger");
    configInput(CLK_INPUT, "clock");
    configInput(SPEED_INPUT, "speed");

    configOutput(HIT1_OUTPUT, "player 1 hit trigger");
    configOutput(HIT2_OUTPUT, "player 2 hit trigger");
    
    configOutput(LEFT_OUTPUT, "left trigger");
    configOutput(RIGHT_OUTPUT, "right trigger");
    configOutput(WALLT_OUTPUT, "top wall trigger");
    configOutput(WALLB_OUTPUT, "bottom wall trigger");

    configOutput(CENTER_OUTPUT, "center trigger");

    configOutput(X_OUTPUT, "ball x");
    configOutput(Y_OUTPUT, "ball y");
    configOutput(VCCENTER_OUTPUT, "center cv");
    configOutput(VCHIT1_OUTPUT, "player 1 hit cv");
    configOutput(VCHIT2_OUTPUT, "player 2 hit cv");
    configOutput(VCLEFT_OUTPUT, "left cv");
    configOutput(VCRIGHT_OUTPUT, "right cv");
    configOutput(VCWALLT_OUTPUT, "top wall cv");
    configOutput(VCWALLB_OUTPUT, "top wall cv");

    onReset();
  }
  // void onReset() override {
  //   pos.x = 0.5f;
  //   pos.y = 0.5f;
  //   ball = false;

  // }
  int vcToDir(float v){
    return (int) floor(v * 0.1f * 8);
  }
  void normalize(Vec v){
    float mag = sqrt(v.x * v.x + v.y * v.y);
    v.x /= mag;
    v.y /= mag;
  }
  void spawnBall(){
    pos.x = 0.5f;
    pos.y = inputs[Y_INPUT].active
      ? inputs[Y_INPUT].value * 0.1f
      : random::uniform();
    float angle = inputs[ANGLE_INPUT].active
      ? inputs[ANGLE_INPUT].value * 0.1f * 2.0f * PI
      // : (2 * PI * (0.25f + (random::uniform() > 0.5f ?  0.30f : 0.0f)) + PI * random::uniform() * 0.20f) + (random::uniform() > 0.5f ? PI : 0.0f);
      : PI * 0.5f + (random::uniform() > 0.5f ? -1.f : 1.f) * (0.2f + random::uniform() * 1.2f) + floor(random::uniform() * 2.f) * PI;

    dir.x = sin(angle);
    dir.y = cos(angle);

    ball = true;
    crossedCenter = false;
    served = false;
  }
  void ballOut(int side, int vcside){
    // outputs[side].value = 10.0f;
    pulses[side].trigger();
    ball = false;
    outputs[vcside].value = 10.0f * clamp(pos.y, 0.0f, 1.0f);
    if(params[AUTOBALL_PARAM].value == 1) 
      spawnBall();
  }
  void hitPlayer(int vcout, int hitout, float p, float s, float v, float x){
    outputs[vcout].value = 10.0f * ((pos.y - p) / s);
    // outputs[hitout].value = 10.0f;
    pulses[hitout].trigger();
    pos.x = x; 
    dir.x *= -1.0f;
    if(v != 0.0f){
      dir.y *= (v > 0.0f) == (dir.y > 0.0f) ? 1.0f : -1.0f;
      dir.y += v * 1000.0f;
    }
    served = true;
    crossedCenter = false;
  }
  void hitWall(int wall, int vcwall, float y){
    outputs[vcwall].value = 10.0f * pos.x;
    pulses[wall].trigger();
    pos.y = y; 
    dir.y *= -1.0f;
  }

  void process(const ProcessArgs &args) override {
    if(gateTrigger.process(inputs[BALL_INPUT].value / 2.f) || (!ball && params[AUTOBALL_PARAM].value == 1))
      spawnBall();

    speed = inputs[SPEED_INPUT].active ? inputs[SPEED_INPUT].value * 0.1f : 0.01f;

    s1 =  0.04f + 0.96f * (inputs[S1_INPUT].active ? (inputs[S1_INPUT].value / 10.f) : params[S1_PARAM].value);
    s2 =  0.04f + 0.96f * (inputs[S2_INPUT].active ? (inputs[S2_INPUT].value / 10.f) : params[S2_PARAM].value);

    p1 = (1.0f - s1) * (inputs[P1_INPUT].active ? inputs[P1_INPUT].value / 10.f : params[P1_PARAM].value);
    p2 = (1.0f - s2) * (inputs[P2_INPUT].active ? inputs[P2_INPUT].value / 10.f : params[P2_PARAM].value);

    float vel1 = p1 - p1last;
    float vel2 = p2 - p2last;

    p1last = p1;
    p2last = p2;


    bool update = false;
    if(inputs[CLK_INPUT].value > 0.0f){
      if(!updated){
        update = true;
        updated = true;
      }
    }else
      updated = false;

    outputs[HIT1_OUTPUT].value = outputs[HIT1_OUTPUT].isConnected() ? (pulses[HIT1_OUTPUT].process(args.sampleTime) ? 10.0f : 0.0) : 0.0f;
    outputs[HIT2_OUTPUT].value = outputs[HIT2_OUTPUT].isConnected() ? (pulses[HIT2_OUTPUT].process(args.sampleTime) ? 10.0f : 0.0) : 0.0f;
    outputs[WALLT_OUTPUT].value = outputs[WALLT_OUTPUT].isConnected() ? (pulses[WALLT_OUTPUT].process(args.sampleTime) ? 10.0f : 0.0) : 0.0f;
    outputs[WALLB_OUTPUT].value = outputs[WALLB_OUTPUT].isConnected() ? (pulses[WALLB_OUTPUT].process(args.sampleTime) ? 10.0f : 0.0) : 0.0f;
    outputs[CENTER_OUTPUT].value = outputs[CENTER_OUTPUT].isConnected() ? (pulses[CENTER_OUTPUT].process(args.sampleTime) ? 10.0f : 0.0) : 0.0f;
    outputs[LEFT_OUTPUT].value = outputs[LEFT_OUTPUT].isConnected() ? (pulses[LEFT_OUTPUT].process(args.sampleTime) ? 10.0f : 0.0) : 0.0f;
    outputs[RIGHT_OUTPUT].value = outputs[RIGHT_OUTPUT].isConnected() ? (pulses[RIGHT_OUTPUT].process(args.sampleTime) ? 10.0f : 0.0) : 0.0f;
    
    if(update && ball){
      pos.x += dir.x * speed;
      pos.y += dir.y * speed * aspectRatio;

      // players, sides
      if(dir.x < 0.0f){

        if(pos.x - ballWidth * 0.5 < playerWidth){ 
          if(pos.y > p1 && p1 + s1 > pos.y)
            hitPlayer(VCHIT1_OUTPUT, HIT1_OUTPUT, p1, s1, vel1, playerWidth + (ballWidth * 0.5));
          else if(pos.x <= 0.0f)
            ballOut(LEFT_OUTPUT, VCLEFT_OUTPUT);
        }
      }
      else{
        if(pos.x + ballWidth * 0.5f >= 1.0f - playerWidth){ 
          if(pos.y > p2 && p2 + s2 > pos.y)
            hitPlayer(VCHIT2_OUTPUT, HIT2_OUTPUT, p2, s2, vel2, 1.0f - playerWidth - (ballWidth * 0.5));
          else if(pos.x >= 1.0f)
            ballOut(RIGHT_OUTPUT, VCRIGHT_OUTPUT);
        }
      }
      // top & bottom walls
      if(dir.y < 0.0f){
        if(pos.y - ballWidth * 0.5f < 0.0f)
          hitWall(WALLT_OUTPUT, VCWALLT_OUTPUT, ballWidth * 0.5f);
      }
      else{
        if(pos.y + ballWidth * 0.5f > 1.0f)
          hitWall(WALLB_OUTPUT, VCWALLB_OUTPUT, 1.0f - ballWidth * 0.5f);
      }

      // center
      if(served && !crossedCenter){
        crossedCenter = (dir.x > 0.0f && pos.x > 0.5f) || (dir.x < 0.0f && pos.x < 0.5f);
        if(crossedCenter){
          pulses[CENTER_OUTPUT].trigger();
          // outputs[CENTER_OUTPUT].value = 10.0f;
          outputs[VCCENTER_OUTPUT].value = pos.y * 10.0f;
          served = false;
        }
      }

      outputs[X_OUTPUT].value = pos.x * 10.0f;
      outputs[Y_OUTPUT].value = pos.y * 10.0f;    
    }

  }
};

struct PiongDisplay : SVGWidget {
  Piong *module;
  NVGcolor foreground = nvgRGB(0xe6, 0xd9, 0xcc);
  NVGcolor background = nvgRGB(0x36, 0x0a, 0x0a);
  // std::shared_ptr<Font> font;

  PiongDisplay(int x, int y) {
    // setSVG(APP->window->loadSvg(asset::plugin(pluginInstance,"art/svg/PiongDisplay.svg")));
    box.pos = Vec(x, y);
    box.size = Vec(180,135);
  }

  void drawLayer(const DrawArgs& args, int layer) override {
    if(module && layer == 1){
      // BALL
      if(module->ball){
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, module->pos.x * 180 - module->ballWidth * 180.0f * 0.5f, module->pos.y * 135 - module->ballWidth * 180.0f * 0.5f, module->ballWidth * 180.0f, module->ballWidth * 180.0f, 0.0);
        nvgFillColor(args.vg,foreground);
        nvgFill(args.vg);
      }

      //PLAYERS
      nvgBeginPath(args.vg);
      nvgRoundedRect(args.vg, 0, module->p1 * 135, module->playerWidth * 180.0f, module->s1 * 135, 0.0);
      nvgFillColor(args.vg,foreground);
      nvgFill(args.vg);

      nvgBeginPath(args.vg);
      nvgRoundedRect(args.vg, 174, module->p2 * 135, module->playerWidth * 180.0f, module->s2 * 135, 0.0);
      nvgFillColor(args.vg,foreground);
      nvgFill(args.vg);
    }
  }
};


struct PiongWidget : ModuleWidget {
  PiongWidget(Piong *module){
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance,"art/svg/panels/Piong.svg")));

    addParam(createParam<AutoOrGate>(Vec(170, 18), module, Piong::AUTOBALL_PARAM));
    
    int x = 18;
    int y = 58;
    int space = 30;
    y = 23;
    // addInput(createOutput<PJ301MPort>(mm2px(Vec(3.51261, 60.4008)), module, VCA_1::CV_INPUT));

    addInput(createInput<SmallBrightPort>(Vec(62, y), module, Piong::CLK_INPUT));
    addInput(createInput<SmallBrightPort>(Vec(124, y), module, Piong::SPEED_INPUT));
    y += 15;
    addInput(createInput<SmallBrightPort>(Vec(35, y), module, Piong::S1_INPUT));
    addInput(createInput<SmallBrightPort>(Vec(149, y), module, Piong::S2_INPUT));

    y = 56;
    addInput(createInput<SmallBrightPort>(Vec(10, y), module, Piong::P1_INPUT));
    addInput(createInput<SmallBrightPort>(Vec(63,y), module, Piong::ANGLE_INPUT));
    addInput(createInput<SmallBrightPort>(Vec(93,y), module, Piong::BALL_INPUT));
    addInput(createInput<SmallBrightPort>(Vec(123,y), module, Piong::Y_INPUT));
    addInput(createInput<SmallBrightPort>(Vec(175, y), module, Piong::P2_INPUT));

    y = 228;
    addParam(createParam<PiongKnob>(Vec(26, y), module, Piong::P1_PARAM));
    addParam(createParam<PiongKnobSmall>(Vec(71, y), module, Piong::S1_PARAM));
    addParam(createParam<PiongKnobSmall>(Vec(120, y), module, Piong::S2_PARAM));
    addParam(createParam<PiongKnob>(Vec(155, y), module, Piong::P2_PARAM));
    

    x = 40;
    y = 271;
    addOutput(createOutput<SmallDarkPort>(Vec(x,y), module, Piong::X_OUTPUT));
    x += 105;
    addOutput(createOutput<SmallDarkPort>(Vec(x,y), module, Piong::Y_OUTPUT));

    addOutput(createOutput<DarkHole>(Vec(80, y + 4), module, Piong::CENTER_OUTPUT));
    addOutput(createOutput<SmallDarkPort>(Vec(110, y), module, Piong::VCCENTER_OUTPUT));
    
    y = 338;
    x = 23;
    addOutput(createOutput<DarkHole>(Vec(x, y), module, Piong::HIT1_OUTPUT));
    x += space;
    addOutput(createOutput<DarkHole>(Vec(x, y), module, Piong::LEFT_OUTPUT));
    x += space;
    addOutput(createOutput<DarkHole>(Vec(x, y), module, Piong::WALLT_OUTPUT));
    x += space;
    addOutput(createOutput<DarkHole>(Vec(x, y), module, Piong::WALLB_OUTPUT));
    x += space;
    addOutput(createOutput<DarkHole>(Vec(x, y), module, Piong::RIGHT_OUTPUT));
    x += space;
    addOutput(createOutput<DarkHole>(Vec(x, y), module, Piong::HIT2_OUTPUT));

    y = 304;
    x = 18;
    addOutput(createOutput<SmallDarkPort>(Vec(x, y), module, Piong::VCHIT1_OUTPUT));
    x += space;
    addOutput(createOutput<SmallDarkPort>(Vec(x, y), module, Piong::VCLEFT_OUTPUT));
    x += space;
    addOutput(createOutput<SmallDarkPort>(Vec(x, y), module, Piong::VCWALLT_OUTPUT));
    x += space;
    addOutput(createOutput<SmallDarkPort>(Vec(x, y), module, Piong::VCWALLB_OUTPUT));
    x += space;
    addOutput(createOutput<SmallDarkPort>(Vec(x, y), module, Piong::VCRIGHT_OUTPUT));
    x += space;
    addOutput(createOutput<SmallDarkPort>(Vec(x, y), module, Piong::VCHIT2_OUTPUT));

    {
      PiongDisplay *display = new PiongDisplay(15,90);
      display->module = module;
      addChild(display);
    }

    addChild(createWidget<PeaceScrew>(Vec(30, 0)));
    addChild(createWidget<PeaceScrew>(Vec(box.size.x-45, 0)));
    addChild(createWidget<PeaceScrew>(Vec(30, 365)));
    addChild(createWidget<PeaceScrew>(Vec(box.size.x-45, 365)));
  }
};

Model *modelPiong = createModel<Piong, PiongWidget>("piong");
