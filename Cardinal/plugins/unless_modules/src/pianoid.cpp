#include "unless.hpp"
#include "widgets.hpp"
#include "dsp/digital.hpp"
#include "utils.hpp"
#include <array>
#include <algorithm>
#include <functional>
#include "../art/components.hpp"
#include "_arth.hpp"

using namespace arth;

#define MAX_CHANNELS 16

struct ValueChange{
  float last_value = -1.0f;
  bool happened = false;
  bool update(float v){
    happened = last_value != v;
    if(happened){
      last_value = v;
    }
    return happened;
  }
};

struct BoolChange{
  bool last_value = false;
  bool happened = false;
  bool update(float v){
    happened = last_value != v;
    if(happened){
      last_value = v;
    }
    return happened;
  }
};

struct Pianoid : Module {
  enum ParamIds {
    OFFSET_PARAM,
    RANGE_PARAM,
    SUSTAIN_PARAM,
    CHANNELS_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    CV_INPUT,
    GATE_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    CV_OUTPUT,
    GATE_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    SUSTAIN_LIGHT,
    INTERNAL_LIGHT,
    NUM_LIGHTS
  };

  struct NoteState{
    float value;
    bool pressed;
    int place_time = 0;

    json_t *dataToJson() {
      json_t *rootJ = json_object();
      json_object_set(rootJ, "pressed", json_boolean(pressed));
      json_object_set(rootJ, "value", json_real(value));
      json_object_set(rootJ, "place_time", json_integer(place_time));
      return rootJ;
    }

    void dataFromJson(json_t *rootJ) {
      place_time = json_integer_value(json_object_get(rootJ, "place_time"));
      value = json_number_value(json_object_get(rootJ, "value"));
      pressed = json_boolean_value(json_object_get(rootJ, "pressed"));
    }
    NoteState(float v = 0.0f, bool p = false){
      value = v; pressed = p;
    }
  };

  std::array<NoteState, MAX_CHANNELS> notes;

  bool dirty = true;

  bool sampling = false;

  SampleDivider divider = SampleDivider(16);

  bool hold = false;


  float color = 0.223776728f;
  TriggerSwitch triggers[MAX_CHANNELS];


  int channels = MAX_CHANNELS;

  ValueChange channel_change;

  BoolChange cv_out_connected;
  BoolChange gate_out_connected;

  void set_dirty(){
    dirty = true;
  }
  void clear_notes(){
    for(int i = 0; i<MAX_CHANNELS; i++){
      notes[i].pressed = false;
      notes[i].place_time = 1;
    }
  }
  void onReset() override{
    clear_notes();
    divider.reset();
    // color = random::uniform();
    initialize();
    output_state();
    set_dirty();
  }

  void clear_gates(int c = MAX_CHANNELS){
    for(int i = 0; i<c; i++){
      notes[i].pressed = false;
      outputs[GATE_OUTPUT].setVoltage(0.0f, i);
    }
  }

  bool all_gates_up(int c){
    for(int i = 0; i<c; i++)
      if(triggers[i].state > 0) return false;
    return true;
  }
  bool needs_update = true;

  bool last_not_sustain = true;

  void initialize(){
    for(int i = 0; i<MAX_CHANNELS; i++){
      notes[i].value = 0.0f;
      notes[i].pressed = false;
    }
  }
  void output_state(){
    outputs[CV_OUTPUT].setChannels(channels);
    outputs[GATE_OUTPUT].setChannels(channels);
    for(int i = 0; i < channels; i++){
      outputs[CV_OUTPUT].setVoltage(notes[i].value, i);
      outputs[GATE_OUTPUT].setVoltage(notes[i].pressed ? 10.0f : 0.0f, i);
    }
    needs_update = false;
  }
  void enable_input_key(int c, int i, bool g = true){
    outputs[GATE_OUTPUT].setChannels(c);
    outputs[CV_OUTPUT].setChannels(c);
    outputs[CV_OUTPUT].setVoltage(inputs[CV_INPUT].getVoltage(i), i);
    outputs[GATE_OUTPUT].setVoltage(g ? 10.0f : 0.0f, i);
    notes[i].pressed = g;
    notes[i].value = inputs[CV_INPUT].getVoltage(i);
  }
  void channels_to(int c){
    channels = c;
    params[CHANNELS_PARAM].setValue(c);
    if(channels < MAX_CHANNELS){
      for(int i = channels; i<MAX_CHANNELS;i++)
        notes[i].pressed = false;
      set_dirty();
    }
    outputs[CV_OUTPUT].setChannels(channels);
    outputs[GATE_OUTPUT].setChannels(channels);
  }

  void change_channel(float v){
    channel_change.update(v);
    if(channel_change.happened){
      channels_to(v);
    }
    params[CHANNELS_PARAM].setValue(v);
  }

  void output_cv_no_gate(){
    for(int i = 0; i< channels; i++){
      float v = inputs[CV_INPUT].getVoltage(i);
      // if(notes[i].value != v){
      notes[i].value = v;
      notes[i].pressed = true;
      outputs[CV_OUTPUT].setVoltage(notes[i].value, i);
      outputs[GATE_OUTPUT].setVoltage(10.0f, i);
      // }
    }
    set_dirty();
  }
  void process(const ProcessArgs &args) override{
    if(needs_update){
      output_state();
      return;
    }
    int cv_channels = inputs[CV_INPUT].getChannels();
    int gate_channels = inputs[GATE_INPUT].getChannels();

    bool sustain_change = false;
    if((params[SUSTAIN_PARAM].getValue() == 0) != last_not_sustain){
      sustain_change = true;
      last_not_sustain = !last_not_sustain;
    }

    if(cv_out_connected.update(outputs[CV_OUTPUT].isConnected()) || gate_out_connected.update(outputs[GATE_OUTPUT].isConnected()))
      channels_to(channels);

    if(cv_channels != 0){
    // CV INPUT IS CONNECTED

      change_channel(cv_channels);
      
      if(last_not_sustain){
        // not sustaining
        if(gate_channels == 0){
          output_cv_no_gate();
        }else{
          // gate input connected -> output by wrapped gate input
          for(int i = 0; i<cv_channels;i++){
            notes[i].value = inputs[CV_INPUT].getVoltage(i);
            notes[i].pressed = inputs[GATE_INPUT].getVoltage(i % gate_channels) > 0.0f;
            outputs[CV_OUTPUT].setVoltage(notes[i].value, i);
            outputs[GATE_OUTPUT].setVoltage(notes[i].pressed ? 10.0f : 0.0f, i);
          }
          set_dirty();
        }
      }else{

        if(gate_channels == 0)
          output_cv_no_gate();
        else{
          // sustaining with cv input and gates

          for(int i = 0; i<cv_channels; i++){
            triggers[i].update(inputs[GATE_INPUT].getVoltage(i % gate_channels));
            if(triggers[i].state == PRESSED){
              if(!sampling){
                clear_gates();
                sampling = true;
              }
              enable_input_key(cv_channels, i);
              set_dirty();
            }
          }
          if(sampling && all_gates_up(cv_channels)){
            sampling = false;
            set_dirty();
          }
        }
      }
    }else{
      // NO CV INPUT CONNECTED, internal mode

      change_channel(params[CHANNELS_PARAM].getValue());

      if(last_not_sustain){
        // not sustaining
        if(sustain_change){
          for(int i = 0; i<channels;i++){
            notes[i].pressed = false;
          }
          set_dirty();
        }

        for(int i = 0; i<channels;i++){
          outputs[CV_OUTPUT].setVoltage(notes[i].value, i);
          outputs[GATE_OUTPUT].setVoltage(notes[i].pressed ? 10.0f : 0.0f, i);
        }

      }else{
        // sustaining
        for(int i = 0; i<channels;i++){
          outputs[CV_OUTPUT].setVoltage(notes[i].value, i);
          outputs[GATE_OUTPUT].setVoltage(notes[i].pressed ? 10.0f : 0.0f, i);
        }
      }

    }
  }
  struct sort_notes{
    bool operator()(NoteState &a, NoteState &b)const{
      if(!b.pressed){
        return true;
      }else{
        if(!a.pressed){
          return false;
        }else{ // both notes are active
          return a.value < b.value;
        }
      }
    }
  };

  void sort_and_output(){
    // std::sort(notes.begin(), notes.end(), sort_notes());
    outputs[CV_OUTPUT].setChannels(channels);
    outputs[GATE_OUTPUT].setChannels(channels);
    for(int i = 0; i<channels; i++){
      if(notes[i].pressed){
        notes[i].place_time++;
      }
      outputs[CV_OUTPUT].setVoltage(notes[i].value, i);
      outputs[GATE_OUTPUT].setVoltage(notes[i].pressed ? 10.0f : 0.0f, i);
    }
  }
  void release_key(int n){
    if(params[SUSTAIN_PARAM].getValue() == 0){
      float v = Midi::midiToCv(n);
      for(int c = 0; c<channels; c++){
        if(notes[c].pressed){
          if(notes[c].value == v){
            notes[c].pressed = false;
            sort_and_output();
            return;
          }
        }
      }
    }
  }

  void press_key(int n){
    float v = Midi::midiToCv(n);
    // release note if it exists
    int free_channel = -1;
    for(int c = 0; c<channels; c++){
      if(notes[c].pressed){
        if(notes[c].value == v){
          notes[c].pressed = false;
          sort_and_output();
          return;
        }
      }else{
        free_channel = c;
      }
    }
    // new note
    int i;
    // use free channel if there is some
    if(free_channel > -1){
      i = free_channel;
    }else{
      // find the oldest note
      int oldest = 0;
      for(int c = 1; c<channels; c++){
        if(notes[c].place_time > notes[oldest].place_time)
          oldest = c;
      }
      i = oldest;
    }

    notes[i].value = v;
    notes[i].pressed = true;
    notes[i].place_time = 0;

    sort_and_output();
  }
  json_t *dataToJson() override {
    json_t *rootJ = json_object();
    json_t *notesJ = json_array();
    for(int i = 0; i < MAX_CHANNELS; i++){
      json_array_append(notesJ, notes[i].dataToJson());
    }

    json_object_set(rootJ, "channels", json_integer(channels));
    json_object_set(rootJ, "color", json_real(color));
    json_object_set(rootJ, "notes", notesJ);
    json_object_set(rootJ, "divider", divider.save());
    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
    channels_to(json_integer_value(json_object_get(rootJ, "channels")));
    if(channels > 0){
      json_t *notesJ = json_object_get(rootJ, "notes");
      for(int i = 0; i < MAX_CHANNELS; i++){
        notes[i].dataFromJson(json_array_get(notesJ,i));
      }
    }
    color = json_number_value(json_object_get(rootJ, "color"));
    divider.load(json_object_get(rootJ, "divider"));
    needs_update = true;
    channel_change.last_value = channels;
    cv_out_connected.last_value = outputs[CV_OUTPUT].isConnected();
    gate_out_connected.last_value = outputs[GATE_OUTPUT].isConnected();
    set_dirty();
  }
  void offset_color(){
    color = float_wrap(color + 0.0613f);
    set_dirty();
  }
  Pianoid(){
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(OFFSET_PARAM, 0, 1.0f, 0.4805f, "range(horizontal), offset");
    configParam(RANGE_PARAM, 0.09f, 1.0, 0.2818f, "range", " semitones", 0.0f, 128.0f, 0.0f);

    configSwitch(SUSTAIN_PARAM, 0, 1, 1, "sustain", {"off", "on"});
    configParam(CHANNELS_PARAM, 1, 16, 3, "internal polyphony");

    configInput(CV_INPUT,"cv");
    configInput(GATE_INPUT,"gate");
    configOutput(CV_OUTPUT,"cv");
    configOutput(GATE_OUTPUT,"gate");

    onReset();
  }
};

struct PianoidPanel : _less::Widget {
  ArtData* art;
  PianoidPanel(){
  }
  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
  }
  void render() override{
    float y = 378.0f;
    // float h = 32.5f;
    // float w = 35.0f;
    float r = 5.0f;
    fill(theme.shadow);
    stretch_round_rect(0.0f, 0.0f, width, y + 2.0f, r * 1.5f, FILL);

    fill(lerp(theme.input, theme.bg, 0.52f));
    stretch_round_rect(0.0f, 0.0f, width, y, r, FILL);

    // NVGcolor ic = lerp(theme.input, theme.bg, 0.6f);
    // NVGcolor oc = lerp(theme.output, theme.bg, 0.5f);

    translate(center.x, 0.0f);
    fill(theme.fg);
    text("pianoid", 0.0f, RACK_GRID_WIDTH * 0.7f, 12.0f);

    fill(theme.fg);
    text("unless", 0.0f, height - RACK_GRID_WIDTH * 0.7f, 11.0f);
  }

};
struct KeyPiece{
  int note = -1;
  bool pressed = false;
  bool sharp = false;
  Rect box;
  KeyPiece(int n = -1, Rect b = Rect(0,0,1,1), bool s = false, bool p = false){
    note = n;
    pressed = p;
    sharp = s;
    box = b;
  }
};


struct PianoidSlider : _less::Widget {
  Pianoid* module;
  int drag_state = 0;
  int lowest = 0;
  int highest = 128;

  ParamQuantity *x_slider;

  ParamQuantity *paramQuantity = NULL;

  float scaled_value = 0.0f;

  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
  }
  void render() override{
    fill(theme.shadow);
    center_round_rect(Vec(center.x + 2.0f, center.y), box.size, 2.0f);

    float mc = module ? module->color : 0.223776728f;
    NVGcolor c = theme.rainbow_color(mc);
    fill(lerp(c, theme.input, 0.42f));
    rect(box);
  }
};

struct PianoidDisplay : _less::Widget {
  Pianoid* module;
  _less::BufferedWidget *buffer;
  int drag_state = 0;

  int lowest = 0;
  int highest = 128;

  std::vector<int> pressed_ids = {};


  const float steps[12] = { 1.0f, 1.0f, 1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 1.0f};
  const float offsets[12] = { 3.0f, -0.67f, 3.0f, 0.67f, 3.0f, 3.0f, -1.0f, 3.0f, 0.0f, 3.0f, 1.0f, 3.0f};
  const int sharps[12] = { false, true, false, true, false, false, true, false, true, false, true, false };
  _less::XYSlider<PianoidSlider> *offset_param = NULL;
  _less::XSlider *range_param = NULL;

  KeyPiece keys[128];
  std::vector<KeyPiece*> blacks;
  std::vector<KeyPiece*> whites;

  float scaled_value = 0.0f;

  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
    float white_y = 0.0f;
    float black_y = 0.75f;
    whites.clear();
    blacks.clear();
    for(int i = 0; i<highest; i++){
      int n = i % 12;
      if(sharps[n]){
        const float black_h = 0.7f;
        float offset = offsets[n] * black_h * 0.2f;
        keys[i] = KeyPiece(i, Rect(0.4f, white_y - black_h * 0.5f + offset, 0.6f, black_h), true);
        blacks.push_back(&keys[i]);
        black_y += steps[n];
      }else{
        keys[i] = KeyPiece(i, Rect(0, white_y, 1.0f, 1.0f), false);
        whites.push_back(&keys[i]);
        white_y += 1.0f;
      }
    }
  }
  void draw_black_keys(const std::vector<KeyPiece*> &ks, const NVGcolor &a, const NVGcolor &b, const float &offset, const float &count){
    // const float pad = 0.0f;
    Vec s = ks.at(0)->box.size.mult(Vec(width, 1.0f));
    float corner = 0.2f;
    for(auto k : ks){
      if(k->box.pos.y < offset - 1.0f)
        continue;
      else if(k->box.pos.y > offset + count)
        break;
      Vec p = k->box.pos.mult(Vec(width, 1.0f));
      if(k->pressed){
        // float bs = 0.02f;
        fill(theme.shadow);
        round_rect(p.minus(Vec(0.0f, 0.01f)), s, corner * 2.0f);
        fill(b);
        round_rect(p, s, corner);
      }else{
        fill(theme.shadow);
        round_rect(p.minus(Vec(0.0f, 0.045f)), s, corner * 2.0f);
        fill(a);
        round_rect(p, s, corner);
      }
    }
  }
  void draw_white_keys(const std::vector<KeyPiece*> &ks, const NVGcolor &a, const NVGcolor &b, const float &offset, const float &count, const float browser_fix){
    const float pad = 0.0f;
    Vec s = ks.at(0)->box.size.mult(Vec(width, 1.0f));
    stroke_width(0.07f);
    for(auto k : ks){
      if(k->box.pos.y < offset - 1.0f)
        continue;
      else if(k->box.pos.y > offset + count - browser_fix)
        break;
      if(k->pressed){
        fill(b);
        rect(k->box.pos.plus(Vec(pad, 0.0f)),s);
      }else{
        fill(theme.shadow);
        round_rect(k->box.pos.plus(Vec(pad, -0.035f)), s, 0.2f);
        fill(a);
        round_rect(k->box.pos.plus(Vec(pad, 0.0f)),s, 0.2f);

      }
    }
  }



  void render() override{
    float range = 0.2818f;
    float count = range * 75.0f;
    float offset = 0.52f * (75.0f - count);
    float scroll_param = 0.0f;
    float browser_fix = 1.0f;
    if(module){
      browser_fix = 0.0f;
      range = range_param->getParamQuantity()->getValue();
      count = range * 75.0f;
      scroll_param = offset_param->getParamQuantity()->getValue();
      offset = (1.0f - scroll_param) * (75.0f - count);
    }


    for(int i = 0; i < 128; i++)
      keys[i].pressed = false;


    int channels = module ? module->channels : 0;
    pressed_ids.clear();
    for(int i = 0; i<channels; i++){
      float v = module->notes[i].value;
      int j = Midi::cvToMidi(v);
      if(module->notes[i].pressed){
        pressed_ids.push_back(j);
        keys[j].pressed = true;
      }
    }


    float mc = module ? module->color : 0.223776728f;
    // float t = 0.5f;
    NVGcolor ic = theme.rainbow_color(mc);
    NVGcolor oc = theme.rainbow_color(mc + 0.1f);

    NVGcolor colors[4] = {
      theme.input, theme.output, lerp(ic, theme.input, 0.42f), lerp(oc, theme.input, 0.42f)
    };

    push();
    // fill(theme.bg_dark);
    // rect(0,0,box.size.x - 5.0f, box.size.y);
    translate(box.size);
    rotate_pi(1.0f);

    float h = height / (float)((((float)highest / 12.0f)) * 7.0f);
    scale(Vec(1.0f, h * (1.0f / range)));

    translate(0.0f, -offset);
    draw_white_keys(whites, colors[0], colors[2], offset, count, browser_fix);
    draw_black_keys(blacks, colors[1], colors[3], offset, count);

    // stroke(colors[1]);
    // center_rect(keys[60].box.pos.plus(Vec(width * 0.1f, 0.5f)), Vec(5.0f, 0.3f), STROKE);

    pop();

    stroke(theme.output);
    // const float o = scroll_param * (1.0f - range);
    stroke_width(5.0f);
    // round_cap();
    // const float p = 1.0f;
    float x = 1.0f;
    float x2 = x * 0.5f;
    // float hh = height - p * 2.0f;

    // FRAME

    fill(theme.input_dark);
    rect(0.0f, 0.0f, 4.0f, height, FILL);

    stroke_width(x * 2.0f);
    stroke(lerp(theme.input, theme.bg, 0.42f));
    rect(0, 0, width, height, STROKE);

    stroke_width(x);
    stroke(theme.input_dark);
    round_rect(x2, x2, width - x, height - x, x, STROKE);


    // stroke_width(2.0f);
    // float w = 0.0f;
    // fill(theme.white);
    // for(auto i : pressed_ids){
    //   if(keys[i].pressed){
    //     rect(0.0f, height * (1.0f - (float) i / 128.0f) - 2.0f, 3.0f, 2.0f, FILL);
    //   }
    // }
    // stroke_width(7.0f);
    // stroke(transp(theme.white, 0.4f));
    // line(0.0, o * hh + p, 0.0f, o * hh + range * hh + p);
  }

  void render_lights() override{
    float range = 0.2818f;
    // float count = range * 75.0f;
    // float offset = 0.52f * (75.0f - count);
    float scroll_param = 0.0f;
    // float browser_fix = 1.0f;
    if(module){
      // browser_fix = 0.0f;
      range = range_param->getParamQuantity()->getValue();
      // count = range * 75.0f;
      scroll_param = offset_param->getParamQuantity()->getValue();
      // offset = (1.0f - scroll_param) * (75.0f - count);
    }



    const float p = 1.0f;
    const float o = scroll_param * (1.0f - range);
    // float x = 1.0f;
    // float x2 = x * 0.5f;
    float hh = height - p * 2.0f;

    // MINIMAP
    stroke_width(2.0f);
    // float w = 0.0f;
    fill(theme.white);
    for(auto i : pressed_ids){
      if(keys[i].pressed){
        rect(-0.5f, height * (1.0f - (float) i / 128.0f) - 2.0f, 3.0f, 2.0f, FILL);
      }
    }
    stroke_width(3.0f);
    stroke(transp(theme.white, 0.4f));
    line(1.0f, o * hh + p, 1.0f, o * hh + range * hh + p);
  }
  KeyPiece *key_from_pos(Vec pos){
    float range = range_param->getParamQuantity()->getValue();
    float offset = (1.0f - offset_param->getParamQuantity()->getValue()) * (1.0f - range);
    
    float y = pos.y / height;
    y = 1.0f - y;
    y *= range;
    y += offset;
    y *= 75.0f;

    float x = 1.0f - (pos.x / width);

    Vec p = Vec(x, y);

    for(auto k : blacks){
      if(k->box.isContaining(p))
        return k;
    }
    for(auto k : whites){
      if(k->box.isContaining(p))
        return k;
    }
    return NULL;
  }
  int held_note = -1;
  void onDragStart(const event::DragStart& e) override{
    if(held_note != -1){
    // float scroll_param = 0.0f;
      module->press_key(held_note);
      buffer->set_dirty();
    }
  }
  void onDragEnd(const event::DragEnd& e) override{
    if(held_note != -1){
      module->release_key(held_note);
      held_note = -1;
      buffer->set_dirty();
    }
  }
  void onButton(const event::Button &e)override{
    if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.pos.x > 6.0f  && e.pos.x < width - 1.0f){
      e.consume(this);
      if(e.action == GLFW_PRESS){
        KeyPiece *k = key_from_pos(e.pos);
        if(k)
          held_note = k->note;
      }
    }
  }

  void onDoubleClick(const event::DoubleClick &e) override{
    module->clear_notes();
  }
};


struct PianoidSetting : MenuItem {
  bool *setting = NULL;
  void onAction(const event::Action &e) override {
    *setting = !*setting;
  }
  void step() override {
    rightText = *setting ? "✔" : "";
    MenuItem::step();
  }
};

struct SustainButton : unless::TextButtonWidget{
  int state = 0;
  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
  }
  void symbol() override{
    text("sus", 0.0f, -1.5f, height * 0.44f);
  }
  void render() override{
    if(state == 1){
      bg = &theme.input;
      fg = &theme.output;
    }else{
      bg = &theme.output;
      fg = &theme.input_dark;
      // bg = &theme.output;
      // fg = &theme.input_dark;
    }
    unless::TextButtonWidget::render();
  }
};

struct PianoidWidget : ModuleWidget {
  _less::BufferedWidget *bufferWidget = NULL;
  Pianoid *moduleInstance = NULL;
  Art art;
  _less::XYSlider<PianoidSlider> *scroll;

  PianoidWidget(Pianoid *module){
    setModule(module);
    moduleInstance = module;

    art = Art::load(this, "pianoid", Pianoid::NUM_INPUTS, Pianoid::NUM_OUTPUTS, Pianoid::NUM_PARAMS);

    PianoidPanel *panel = new PianoidPanel();
    art.panel(panel);
    
    bufferWidget = new _less::BufferedWidget(Helper::cornered_rect(art.layout.widgets[0].box));
    bufferWidget->box.size.x += 6.0f;
    bufferWidget->box.size.y += 6.0f;
    addChild(bufferWidget);    

    PianoidDisplay *keys = new PianoidDisplay();
    keys->buffer = bufferWidget;
    art.widget(keys, false);
    keys->box.pos = Vec(0, 0);
    keys->box.size = art.layout.widgets[0].box.size;

    bufferWidget->fb->addChild(keys);

    _less::XSlider *range = art.param<_less::XSlider>(Pianoid::RANGE_PARAM);
    range->speed = -1.0f;
    scroll = art.param<_less::XYSlider<PianoidSlider>>(Pianoid::OFFSET_PARAM);
    scroll->set_x_param(range);
    scroll->handle->module = moduleInstance;
    // scroll->visible = false;

    keys->module = moduleInstance;
    keys->range_param = range;
    keys->offset_param = scroll;

    art.param<_less::Button<SustainButton>>(Pianoid::SUSTAIN_PARAM);
    art.input<_less::Port<unless::GateInput>>(Pianoid::GATE_INPUT);
    art.input<_less::Port<unless::CvInput>>(Pianoid::CV_INPUT);
    art.output<_less::Port<unless::GateOutput>>(Pianoid::GATE_INPUT);
    art.output<_less::Port<unless::CvOutput>>(Pianoid::CV_INPUT);
    art.param<_less::FixedIntSelectButton<unless::ChannelsButton>>(Pianoid::CHANNELS_PARAM);
  }
  void onButton(const event::Button &e) override{
    if(e.action == GLFW_PRESS && e.button == 0){
      float w = 15.0f * 3;
      Rect color_button = Rect(box.size.x * 0.5f - w * 0.5f, box.size.y - RACK_GRID_WIDTH, w, RACK_GRID_WIDTH);
      if(color_button.isContaining(e.pos)){
        moduleInstance->offset_color();
        scroll->handle->dirty = true;
        // return;
      }
    }
    ModuleWidget::onButton(e);
  }
  void step() override{
    ModuleWidget::step();
    if(moduleInstance && moduleInstance->dirty){
      moduleInstance->dirty = false;
      bufferWidget->set_dirty();
    }
    if(scroll && scroll->handle->dirty){
      scroll->handle->dirty = false;
      bufferWidget->set_dirty();
    }
  }
  void appendContextMenu(Menu *menu) override {
    Pianoid *view = dynamic_cast<Pianoid*>(module);
    assert(view);
    add_color_slider(menu, moduleInstance);
    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<SampleDividerMenuItem>(&SampleDividerMenuItem::divider, &moduleInstance->divider));

  }
};
Model *modelPianoid = createModel<Pianoid, PianoidWidget>("pianoid");
/*
  TODO
*/
