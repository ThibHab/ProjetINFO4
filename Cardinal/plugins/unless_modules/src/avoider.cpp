#include "unless.hpp"
#include "widgets.hpp"
#include "dsp/digital.hpp"
#include "utils.hpp"
#include <algorithm>
#include "../art/components.hpp"
#include "_arth.hpp"

using namespace arth;

#define MODULE_WIDTH 4
#define MAX_AVOIDS 4


static const std::vector<int> interval_perm[7] = {
{
  0b000000
},
{
  0b010000,
  0b001000,
  0b000100,
  0b000010,
  0b000001,
  0b100000
},
{
  0b011000,
  0b010100,
  0b001100,
  0b010010,
  0b010001,
  0b001010,
  0b001001,
  0b000110,
  0b110000,
  0b000101,
  0b101000,
  0b100100,
  0b000011,
  0b100010,
  0b100001
},
{
  0b011100,
  0b011010,
  0b010110,
  0b011001,
  0b010101,
  0b001110,
  0b111000,
  0b001101,
  0b110100,
  0b010011,
  0b101100,
  0b001011,
  0b110010,
  0b000111,
  0b101010,
  0b110001,
  0b100110,
  0b101001,
  0b100101,
  0b100011
},
{
  0b011110,
  0b011101,
  0b111100,
  0b011011,
  0b010111,
  0b111010,
  0b001111,
  0b111001,
  0b110110,
  0b110101,
  0b101110,
  0b101101,
  0b110011,
  0b101011,
  0b100111
},
{
  0b011111,
  0b111110,
  0b111101,
  0b111011,
  0b110111,
  0b101111
},
{
  0b111111
}
};

static const std::string interval_names[6] = {"m2", "M2", "m3", "M3", "P4", "TT"};

struct Avoider : Module {
  enum ParamIds {
    ENUMS(AVOIDSWITCH_PARAMS, 6),
    // MODESWITCH_PARAM,
    AVOID_PARAM,
    INTERVAL_COUNT_PARAM,
    KEEP_CHORD_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    CV_INPUT,
    GATE_INPUT,
    CHORD_CV_INPUT,
    CHORD_GATE_INPUT,
    AVOID_INPUT,
    INTERVAL_COUNT_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    CV_OUTPUT,
    GATE_OUTPUT,
    AVOIDING_OUTPUT,
    PES_OUTPUT,
    PLS_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  bool dirty = true;

  float current_note = 0.0f;

  TriggerSwitch trigger;
  void set_dirty(){
    dirty = true;
  }
  void onReset() override{
    // set_dirty();
  }

  uint8_t interval_between(int a, int b){
    int i = abs(a - b);
    if(i > 6) return 12 - i;
    else return i;
  }
  uint8_t interval_between(float a, float b){
    return interval_between(Midi::cvToKey(a), Midi::cvToKey(b));
  }

  struct AvoidNote{
    bool avoid = false;
    float note = 0.0f;
  };

  AvoidNote should_avoid(float note, uint8_t interval){
    AvoidNote avoid;
    if(!inputs[CHORD_CV_INPUT].isConnected())
      return avoid;

    uint8_t c = inputs[CHORD_CV_INPUT].getChannels();
    for(uint8_t i = 0; i<c; i++){
      if(inputs[CHORD_GATE_INPUT].isConnected() && inputs[CHORD_GATE_INPUT].getVoltage(i % inputs[CHORD_GATE_INPUT].getChannels()) < 0.1f)
        continue;

      float chord_note = inputs[CHORD_CV_INPUT].getVoltage(i);
      uint8_t iv = interval_between(note, chord_note);
      if(interval != 0 && iv == 0){
        avoid.avoid = false;
        return avoid;
      }else if(iv == interval){
        avoid.avoid = true;
        avoid.note = chord_note;
      }
    }
    return avoid;
  }

  float quantize_to_pls(float cv){
    int scale_length = available_notes.size();
    float oct = floor(cv);
    float note = cv - oct;
    float distance = 2.0f;
    float scale_note = note;
    for(int i = 0; i<scale_length; i++){
      float scale_volt = available_notes.at(i);
      float sn = scale_volt - floor(scale_volt);
      float d = circle_dist(note, sn);
      if(d < distance){
        distance = d;
        scale_note = sn;
      }
    }

    float dir_to_scale_note = scale_note - note;
    if(dir_to_scale_note > 0.5f)
      scale_note = scale_note + oct - 1.0f;
    else if(dir_to_scale_note < -0.5f){
      scale_note = scale_note + oct + 1.0f;
    }else{
      scale_note = scale_note + oct;
    }
    return scale_note;

  }

  bool should_be_avoided(int key, int interval){
    uint8_t c = inputs[CHORD_CV_INPUT].getChannels();
    for(uint8_t i = 0; i<c; i++){
      if(inputs[CHORD_GATE_INPUT].isConnected() && inputs[CHORD_GATE_INPUT].getVoltage(i % inputs[CHORD_GATE_INPUT].getChannels()) < 0.1f)
        continue;
      float chord_note = inputs[CHORD_CV_INPUT].getVoltage(i);
      uint8_t iv = interval_between(key, Midi::cvToKey(chord_note));
      if(iv == interval)
        return true;
    }
    return false;
  }

  const float eqt = 0.08333333333f;
  const int cc = 12;
  bool in_chord(int k){
    int chord_channels = inputs[CHORD_CV_INPUT].getChannels();
    for(int i = 0; i < chord_channels; i++){
      if(inputs[CHORD_GATE_INPUT].isConnected() && inputs[CHORD_GATE_INPUT].getVoltage(i % inputs[CHORD_GATE_INPUT].getChannels()) < 0.1f){
        continue;
      }else{
        int key = Midi::cvToKey(inputs[CHORD_CV_INPUT].getVoltage(i));
        if(key == k)
          return true;
      }
    }
    return false;
  }
  bool avoid_it_chord(uint8_t k){
    for(int i = 0; i<6; i++){
      if(params[AVOIDSWITCH_PARAMS + i].getValue() > 0 && should_be_avoided(k, i + 1) && !in_chord(k))
        return true;
    }
    return false;
  }
  bool avoid_it_chordless(uint8_t k){
    for(int i = 0; i<6; i++){
      if(params[AVOIDSWITCH_PARAMS + i].getValue() > 0 && (should_be_avoided(k, i + 1) || in_chord(k)))
        return true;
    }
    return false;
  }
  void output_pes(){
    outputs[PES_OUTPUT].setChannels(cc);
    if(params[KEEP_CHORD_PARAM].getValue() > 0.1f){
      for(int k = 0; k < cc; k++){
        outputs[PES_OUTPUT].setVoltage(avoid_it_chord(k) ? 0.0f : 10.0f, k);
      }
    }else{
      for(int k = 0; k < cc; k++){
        outputs[PES_OUTPUT].setVoltage(avoid_it_chordless(k) ? 0.0f : 10.0f, k);
      }
    }
  }
  std::vector<float> available_notes;
  void output_pls(){
    available_notes.clear();
    // int chord_channels = inputs[CHORD_CV_INPUT].getChannels();
    if(params[KEEP_CHORD_PARAM].getValue() > 0.1f){
      for(int k = 0; k < cc; k++){
        if(!avoid_it_chord(k))
          available_notes.push_back(k * eqt);
      }
    }else{
      for(int k = 0; k < cc; k++){
        if(!avoid_it_chordless(k))
          available_notes.push_back(k * eqt);
      }
    }

    int c = available_notes.size();
    outputs[PLS_OUTPUT].setChannels(c);
    for(int i = 0; i<c; i++)
      outputs[PLS_OUTPUT].setVoltage(available_notes.at(i), i);
  }
  SampleDivider divider;
  void process(const ProcessArgs &args) override {
    if(!divider.step())
      return;

    // int index = clamp((int)floor(params[AVOID_PARAM].getValue() + inputs[AVOID_INPUT].getVoltage() * 0.1f * 63.0f), 0, 63);
    int count = clamp((int)(params[INTERVAL_COUNT_PARAM].getValue() + inputs[INTERVAL_COUNT_INPUT].getVoltage()), 0, 6);
    int index = clamp(
      (int)floor((params[AVOID_PARAM].getValue() + inputs[AVOID_INPUT].getVoltage() * 0.1f) * (float) interval_perm[count].size()), 
      0, 
      (int) interval_perm[count].size() - 1);

    int perm = interval_perm[count][index];
    for(int i = 0; i<6; i++){
      params[AVOIDSWITCH_PARAMS + i].setValue(((perm >> i) & 1) == 1 ? 1 : 0);
    }

    if(outputs[PES_OUTPUT].isConnected())
      output_pes();

    // if(outputs[PLS_OUTPUT].isConnected())
      output_pls();

    bool calculate = !inputs[GATE_INPUT].isConnected() || trigger.update(inputs[GATE_INPUT].getVoltage()) == PRESSED;
    ;
    if(calculate){
      float note = !inputs[CV_INPUT].isConnected() ? Midi::midiToCv(random::uniform() * 127.0f) : inputs[CV_INPUT].getVoltage();
      AvoidNote avoid;

      for(int i = 0; i<6; i++){
        if(params[AVOIDSWITCH_PARAMS + i].getValue() > 0 && !avoid.avoid){
          avoid = should_avoid(note, i + 1);
        }
      }
      if(params[KEEP_CHORD_PARAM].getValue() < 0.1f){
        if(in_chord(Midi::cvToKey(note))){
          avoid.avoid = true;
          avoid.note = quantize_to_pls(note);
        }
      }
      if(avoid.avoid){
        avoid.note = quantize_to_pls(note);
        outputs[GATE_OUTPUT].setVoltage(10.0f);
        float note_oct = floor(note);
        outputs[CV_OUTPUT].setVoltage(note_oct + (avoid.note - floor(avoid.note)));
        outputs[AVOIDING_OUTPUT].setVoltage(10.0f);
      }else{
        outputs[CV_OUTPUT].setVoltage(note);
        outputs[GATE_OUTPUT].setVoltage(10.0f);
        outputs[AVOIDING_OUTPUT].setVoltage(0.0f);
      }
    }else if(trigger.state == RELEASED){
      outputs[GATE_OUTPUT].setVoltage(0.0f);
    }
  }

  json_t *dataToJson() override {
    json_t *rootJ = json_object();
    json_object_set(rootJ, "divider", divider.save());
    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
    divider.load(json_object_get(rootJ, "divider"));
  }
  Avoider(){
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    for(int i = 0; i<6; i++){
      int def = (i == 0 || i == 5) ? 1 : 0;
      configParam(AVOIDSWITCH_PARAMS + i, 0, 1, def, "avoid interval " + to_string(i + 1) + " steps ("+interval_names[i] + ")");
    }
    // configParam(MODESWITCH_PARAM, 0, 1, 0, "manual/knob mode");
    configParam(AVOID_PARAM, 0, 1.0f, 1.0f, "harmony");
    configParam(INTERVAL_COUNT_PARAM, 1, 5, 2, "avoided count");
    configParam(KEEP_CHORD_PARAM, 0, 1, 0, "keep chord notes");
    
    configInput(CV_INPUT, "cv");
    configInput(GATE_INPUT, "gate");
    configInput(CHORD_CV_INPUT, "chord cv");
    configInput(CHORD_GATE_INPUT, "chord gate");
    configInput(AVOID_INPUT, "avoid");
    configInput(INTERVAL_COUNT_INPUT, "interval count");


    configOutput(CV_OUTPUT, "cv");
    configOutput(GATE_OUTPUT, "gate");
    configOutput(AVOIDING_OUTPUT, "avoiding");
    configOutput(PES_OUTPUT, "pes scale");
    configOutput(PLS_OUTPUT, "pls scale");
    onReset();
  }
};
struct IntervalButton : unless::TextButtonWidget{
  float value;
  int i = 0;
  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
  }
  void render() override{
    if(state > 0){
      bg = &theme.input;
      fg = &theme.output;
    }
    else{
      bg = &theme.output;
      fg = &theme.input_dark;
    }
    unless::TextButtonWidget::render();
  }
  void symbol() override{
    // text(interval_names[i].c_str(), 0.0f, 0.0f, height * 0.5f);
    // text("+", -10.0f, 0.0f, height * 0.5f);
    text(std::to_string(i + 1), 0.0f, 0.0f, height * 0.55f);
  }
};

struct ChordNoteButton : unless::TextButtonWidget{
  float value;
  int i = 0;
  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
  }
  void render() override{
    if(state > 0){
      bg = &theme.input;
      fg = &theme.output;
    }
    else{
      bg = &theme.output;
      fg = &theme.input_dark;
    }
    unless::TextButtonWidget::render();
  }
  void symbol() override{
    // text(interval_names[i].c_str(), 0.0f, 0.0f, height * 0.5f);
    // text("+", -10.0f, 0.0f, height * 0.5f);
    text(state == 1 ? "+" : "-", 0.0f, 0.0f, height * 0.75f);
  }
};

struct TwoWayArrowButton : unless::TextButtonWidget{
  float value;
  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
  }
  // void render() override{
  //     bg = &theme.input;
  //   else
  //     bg = &theme.input_dark;

  //   unless::TextButtonWidget::render();
  // }
  void symbol() override{
    // translate(center);
    stroke(fg);
    if(state > 0)
      rotate_pi(-0.5f);
    float o = center.x * 0.5f;
    float h = center.y * 0.3f;
    // stroke(theme.fg);
    line(- o, 0.0f, o, 0.0f);
    begin();
    point(o - h, -h);
    point(o, 0.0f);
    point(o - h, h);
    end();
    // text(std::to_string(i + 1), 0.0f, 0.0f, height * 0.5f);
  }
};

struct AvoiderPanel : _less::Widget {
  Avoider* module;
  LayoutData* art;
  AvoiderPanel(LayoutData *a){
    art = a;
  }
  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
  }
  // void onButton(const event::Button &e) override{
  //   if(e.action == GLFW_PRESS && e.button == 0){
  //     printf("%f\n", e.pos.x);
  //   }
  // }
  void render() override{
    fill(theme.bg);
    rect(0,0,box.size.x, box.size.y);
    stroke(theme.output);
    fill(lerp(theme.bg, theme.input, 0.3f));

    fill(theme.fg);
    text("AVOIDER", center.x, 15.0f, 14.0f);
    text("INTERVALS", center.x, art->params[0].box.pos.y - 23.0f, 12.0f);
    text("CHORD", art->inputs[2].box.pos.x, art->inputs[2].box.pos.y - 20.0f, 10.0f);

    text("NOTE", art->inputs[0].box.pos.x, art->inputs[0].box.pos.y - 20.0f, 10.0f);
    text("NOTE", art->outputs[0].box.pos.x, art->outputs[0].box.pos.y - 20.0f, 10.0f);
    // text("KEEP", art->params[Avoider::KEEP_CHORD_PARAM].box.pos.x, art->params[Avoider::KEEP_CHORD_PARAM].box.pos.y - 20.0f, 10.0f);
    text("COUNT", art->params[7].box.pos.x, art->params[7].box.pos.y + 20.0f, 10.0f);
    text("HARM", art->inputs[Avoider::AVOID_INPUT].box.pos.x, art->inputs[Avoider::AVOID_INPUT].box.pos.y + 20.0f, 10.0f);
    text("AVOID", art->outputs[2].box.pos.x, art->outputs[2].box.pos.y - 20.0f, 10.0f);
    text("SCALE", art->outputs[4].box.pos.x, art->outputs[4].box.pos.y - 20.0f, 10.0f);
    text("PLS", art->outputs[4].box.pos.x, art->outputs[4].box.pos.y + 15.0f, 6.0f);
    text("PES", art->outputs[3].box.pos.x, art->outputs[3].box.pos.y - 15.0f, 6.0f);

    

    // float b = 2.0f;
    // stroke_width(b * 2.0f);
    // stroke(theme.input);
    // line(center.x - b, art->params[5].box.pos.y + 18.0f, center.x - b, art->outputs[1].box.pos.y + 15.0f);
    // stroke(theme.output);
    // line(center.x + b, art->params[5].box.pos.y + 18.0f, center.x + b, art->outputs[1].box.pos.y + 15.0f);


    stroke(theme.input);
    stroke_width(2.0f);
    // center_round_rect(center.x, lerpf(art->params[0].box.pos.y, art->params[3].box.pos.y, 0.5f), width - 10.0f, (art->params[3].box.pos.y - art->params[0].box.pos.y) + 30.0f, 3.0f, STROKE);
    // float d = 5.0f;
    // float o = 17.0f;
    // float w2 = art->params[5].box.size.x * 0.5f + 2.0f;
    // begin();
    // point(art->params[2].box.pos.plus(Vec(-w2, o)));
    // point(art->params[2].box.pos.plus(Vec(-w2, o + d)));
    // point(art->params[4].box.pos.plus(Vec(w2, o + d)));
    // point(art->params[4].box.pos.plus(Vec(w2, o)));
    // end();
    // line(center.x, art->params[4].box.pos.y + o + d, center.x, art->params[4].box.pos.y + o + d + d);


    // begin();
    // float od = art->params[6].box.pos.x - (art->params[5].box.pos.x - art->params[5].box.size.x * 0.5f + 2.0f);
    // point(art->params[5].box.pos.plus(Vec(-w2, -o)));
    // point(art->params[6].box.pos.plus(Vec(-od, od)));
    // point(art->params[6].box.pos.plus(Vec(0.0f, 0.0f)));
    // point(art->params[6].box.pos.plus(Vec(od, od)));

    // point(art->params[1].box.pos.plus(Vec(w2, -o)));
    // end();

    float e = -10.0f;
    line(art->inputs[2].box.pos, art->inputs[3].box.pos.plus(Vec(0, e)));
    line(art->inputs[2].box.pos.plus(Vec(5.0f, 0.0)), art->inputs[3].box.pos.plus(Vec(5.0f, e)));
    line(art->inputs[2].box.pos.minus(Vec(5.0f, 0.0)), art->inputs[3].box.pos.minus(Vec(5.0f, -e)));

    // line(art->inputs[4].box.pos.plus(Vec(0.0f, -e)), art->params[6].box.pos);
    // line(art->inputs[5].box.pos.plus(Vec(0.0f, e)), art->params[7].box.pos);

    line(art->inputs[4].box.pos, art->params[6].box.pos);
    line(art->inputs[5].box.pos, art->params[7].box.pos);

    stroke_width(4.0f);

    line(art->inputs[0].box.pos, art->inputs[1].box.pos.plus(Vec(0, e)));
    line(art->params[Avoider::KEEP_CHORD_PARAM].box.pos, art->inputs[Avoider::CHORD_CV_INPUT].box.pos);


    stroke(theme.output);

    line(art->outputs[0].box.pos, art->outputs[1].box.pos.plus(Vec(0, e)));
    fill(theme.fg);

    text("unless", center.x, box.size.y - 13, 13.0f);
    fill(theme.bg_dark);
    // text("A", center.x, box.size.y - 85, 40.0f);
    // text("unfinished", center.x, box.size.y - 7, 10.0f);
  }

};
struct AvoiderDisplay : _less::Widget {
  Avoider* module;
  _less::Panel *panel;
  int drag_state = 0;
  AvoiderDisplay(_less::Panel *p){
    panel = p;
  }

  void onButton(const event::Button &e) override{
  }

  void onHover(const event::Hover &e) override{
  }

  // void onDragLeave(const event::DragLeave &e) override{
  //   e.consume(this);
  //   printf("left widiget\n");
  //   drag_state = 0;
  // }

  void onDragEnd(const event::DragEnd &e) override{
  }

  void onDragHover(const event::DragHover &e) override{
  }
  // void onDragMove(const event::DragMove &e) override{
  // }
  void init(NVGcontext *vg) override{
    load_font("res/font/Terminus.ttf");
  }
  void render() override{
    if(module){
      fill(theme.bg_dark);
      rect(0,0,box.size.x, box.size.y);
    }
  }

};

struct AvoiderSetting : MenuItem {
  bool *setting = NULL;
  void onAction(const event::Action &e) override {
    *setting = !*setting;
  }
  void step() override {
    rightText = *setting ? "✔" : "";
    MenuItem::step();
  }
};


struct AvoidKnob : unless::DrawKnob{
  bool is_input = true;
  void render()override{
    bg = &theme.input;
    fg = &theme.input_dark;
    thickness = 2.f;
    unless::DrawKnob::render();
  }
};

struct AvoiderWidget : ModuleWidget {
  _less::Panel *panelWidget = NULL;
  Avoider *moduleInstance = NULL;
  Art art;
  void print_bits(int i, int n){
    printf("%d = ", i);
    for(int i = 0; i < 6; i++){
      int b = (n >> i) & 1;
      printf("%d", b);
    }
    printf("\n");
  }
  AvoiderWidget(Avoider *module){
    setModule(module);
    art = Art::load(this, "avoider", Avoider::NUM_INPUTS, Avoider::NUM_OUTPUTS, Avoider::NUM_PARAMS);

    // for(int i = 0; i < 64; i++){
    //   print_bits(i, interval_perm[i]);
    // }

    moduleInstance = module;
    box.size = Vec(art.layout.width * RACK_GRID_WIDTH, 380);

    panelWidget = new _less::Panel(box.size, nvgHSLA(0, 0, 0, 0));
    AvoiderPanel *panel = new AvoiderPanel(&art.layout);
    panel->module = module;
    panel->set_box(0.0f,0.0f,box.size.x, box.size.y);

    panelWidget->fb->addChild(panel);
    addChild(panelWidget);
    // addChild(panel);
    
    art.input<_less::Port<unless::GateInput>>(Avoider::GATE_INPUT);
    art.input<_less::Port<unless::CvInput>>(Avoider::CV_INPUT);
    art.input<_less::Port<unless::GateInput>>(Avoider::CHORD_GATE_INPUT);
    art.input<_less::Port<unless::CvInput>>(Avoider::CHORD_CV_INPUT);
    art.input<_less::Port<unless::RingHoleInput>>(Avoider::AVOID_INPUT);
    art.input<_less::Port<unless::RingHoleInput>>(Avoider::INTERVAL_COUNT_INPUT);
    art.output<_less::Port<unless::GateOutput>>(Avoider::GATE_OUTPUT);
    art.output<_less::Port<unless::CvOutput>>(Avoider::CV_OUTPUT);
    art.output<_less::Port<unless::GateHoleOutput>>(Avoider::AVOIDING_OUTPUT);
    art.output<_less::Port<unless::GateHoleOutput>>(Avoider::PES_OUTPUT);
    art.output<_less::Port<unless::RingHoleOutput>>(Avoider::PLS_OUTPUT);
    art.param<_less::Knob<AvoidKnob>>(Avoider::AVOID_PARAM);
    art.param<_less::Knob<AvoidKnob>>(Avoider::INTERVAL_COUNT_PARAM, true);
    art.param<_less::Button<ChordNoteButton>>(Avoider::KEEP_CHORD_PARAM);
    
    // art.param<_less::Button<TwoWayArrowButton>>(Avoider::MODESWITCH_PARAM, false);
    // for(int i = 0; i<MAX_AVOIDS; i++){
      // art.param<_less::Knob<AvoidKnob>>(Avoider::AVOID_PARAMS + i, true);
      // if(i < 6)
      // art.input<_less::Port<unless::RingHoleInput>>(Avoider::AVOID_INPUTS + i);
    // }
    
    const int order[6] = {0, 2, 5, 1, 3, 4};
    for(int i = 0; i<6; i++){
      _less::Button<IntervalButton> *b = art.param<_less::Button<IntervalButton>>(Avoider::AVOIDSWITCH_PARAMS + order[i], false);
      // b->box.pos.x = 5.0f + (i % 3) * 32.5f;
      // b->box.pos.y = 5.0f + (i / 3) * 32.5f;
      b->sw->i = order[i];
    }
  }
  void step() override{
    ModuleWidget::step();
    if(moduleInstance && moduleInstance->dirty){
      // panelWidget->set_dirty();
      moduleInstance->dirty = false;
    }
  }
  void appendContextMenu(Menu *menu) override {
    Avoider *view = dynamic_cast<Avoider*>(module);
    assert(view);
    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<SampleDividerMenuItem>(&SampleDividerMenuItem::divider, &moduleInstance->divider));

  }
};


Model *modelAvoider = createModel<Avoider, AvoiderWidget>("avoider");
/*
  TODO

  step mods

  output scales only when chord changes

*/
