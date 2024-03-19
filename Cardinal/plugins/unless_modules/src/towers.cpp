#include "unless.hpp"
#include "widgets.hpp"
#include "dsp/digital.hpp"
#include "utils.hpp"
#include <algorithm>
// #include <ui.hpp>
#include <settings.hpp>
#include "../art/components.hpp"
#include "../art/towers.hpp"
#include "_arth.hpp"

using namespace arth;

#define MODULE_WIDTH 18

#define ROWS 2
#define FADERS 16

struct ChannelSetting{
  bool dirty = true;
  int channels = 8;
  void set(int c){
    channels = c;
    dirty = true;
  }
  int update(){
    if(dirty){
      dirty = false;
      return channels;
    }else
      return -1;
  }
};


enum ChannelSetMode{
  BUTTON_ONLY,
  AUTO_COPY_INPUT,
  COPY_INPUT_ON_BUTTON,
  CHANNEL_SET_MODES
};

enum TowersSampleMode{
  SAMPLE_ABSOLUTE,
  SAMPLE_BY_MODS,
  TOWERS_SAMPLE_MODES
};

struct TowerRow {
  bool bipolar = false;
  float modded_values[FADERS] = {};
  int channels = FADERS;
  ChannelSetMode channel_set_mode = BUTTON_ONLY;
  TowersSampleMode sample_mode = SAMPLE_ABSOLUTE;
  TowersViewMode view_mode = SLIDER;

  std::string name = "A";


  json_t *toJson(){
    json_t *rootJ = json_object();
    json_object_set(rootJ, "view_mode", json_integer(view_mode));
    json_object_set(rootJ, "channel_set_mode", json_integer(channel_set_mode));
    json_object_set(rootJ, "sample_mode", json_integer(sample_mode));
    return rootJ;
  }

  void fromJson(json_t *rootJ){
    view_mode = (TowersViewMode) json_integer_value(json_object_get(rootJ, "view_mode")); 
    channel_set_mode = (ChannelSetMode) json_integer_value(json_object_get(rootJ, "channel_set_mode")); 
    sample_mode = (TowersSampleMode) json_integer_value(json_object_get(rootJ, "sample_mode")); 
  }

  TowerRow(std::string n){
    name = n;
    for(int i = 0; i<FADERS; i++)
      modded_values[i] = 0.0f;
  }
};


struct Towers : Module {
  enum ParamIds {
    ENUMS(FADER_PARAMS, FADERS * ROWS),
    ENUMS(POLARITY_PARAM, ROWS),
    ENUMS(TRIM_PARAMS, ROWS),
    ENUMS(SAMPLE_PARAMS, ROWS),
    ENUMS(CHANNEL_PARAMS, ROWS),
    NUM_PARAMS
  };
  enum InputIds {
    ENUMS(FADER_INPUTS, ROWS),
    ENUMS(TRIM_INPUTS, ROWS),
    ENUMS(SAMPLE_INPUTS, ROWS),
    NUM_INPUTS
  };
  enum OutputIds {
    ENUMS(FADER_OUTPUTS, ROWS),
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  bool dirty = true;

  SampleDivider divider = SampleDivider(64);

  ChannelSetting channel_settings[2];

  TriggerSwitch triggers[ROWS];
  TriggerSwitch poly_triggers[ROWS][FADERS];

  TowerRow rows[2] = {TowerRow("A"), TowerRow("B")};
  
  void set_dirty(){
    dirty = true;
  }
  void onReset() override{
    color = 0.0f;
    divider.reset();
    set_dirty();
  }
  void toggle_mode(int r){
    rows[r].bipolar = !rows[r].bipolar;
  }

  void set_channels(int r, int c){
    channel_settings[r].channels = c;
    update_row(r);
  }
  void update_channel_param(int row, int c = -1){
    if(c == -1)
      params[CHANNEL_PARAMS + row].setValue(channel_settings[row].channels);
    else
      params[CHANNEL_PARAMS + row].setValue(c);
  }
  void update_row(int r){
    if(rows[r].channel_set_mode == AUTO_COPY_INPUT){
      if(inputs[FADER_INPUTS + r].isConnected()){
        int c = inputs[FADER_INPUTS + r].getChannels();
        if(c != channel_settings[r].channels){
          channel_settings[r].set(c);
          update_channel_param(r);
        }
      }
    }
    int _channels = (int)floor(params[CHANNEL_PARAMS + r].getValue());
    if(_channels != channel_settings[r].channels){
      channel_settings[r].set(_channels);
      update_channel_param(r);
    }

    // if(channel_settings[r].update() > -1){
    //   set_channels(r, channel_settings[r].channels);
    //   set_dirty();
    //   return;
    // }

    bool bipolar = params[POLARITY_PARAM + r].value > 0;
    if(bipolar != rows[r].bipolar){
      rows[r].bipolar = bipolar;
    }
    float _min = rows[r].bipolar ? -5.0f : 0.0f;
    float _max = _min + 10.0f;
    if(inputs[FADER_INPUTS + r].isConnected()){
      // if(outputs[r].isConnected()){
        outputs[r].setChannels(channel_settings[r].channels);
        int input_channels = inputs[FADER_INPUTS + r].getChannels();
        int row = r * FADERS;
        float offset = rows[r].bipolar ? -5.0f : 0.0f;
        if(inputs[TRIM_INPUTS + r].isConnected()){
          int trim_channels = inputs[TRIM_INPUTS + r].getChannels();
          for(int i = 0; i<channel_settings[r].channels; i++){
            rows[r].modded_values[i] = clamp(
              params[FADER_PARAMS + i + row].value + offset
              + inputs[FADER_INPUTS + r].getVoltage(i % input_channels)
              * inputs[TRIM_INPUTS + r].getVoltage(i % trim_channels) * 0.2f * params[TRIM_PARAMS + r].value, _min, _max);
            outputs[r].setVoltage(rows[r].modded_values[i],i);
          }
        }else{
          for(int i = 0; i<channel_settings[r].channels; i++){
            rows[r].modded_values[i] = clamp(
              params[FADER_PARAMS + i + row].value + offset
              + params[TRIM_PARAMS + r].value * inputs[FADER_INPUTS + r].getVoltage(i % input_channels),_min, _max);
            outputs[r].setVoltage(rows[r].modded_values[i],i);
          }
        }
      // }
    }else{
      // if(outputs[r].isConnected()){
        outputs[r].setChannels(channel_settings[r].channels);
        float offset = rows[r].bipolar ? -5.0f : 0.0f;
        int row = r * FADERS;
        for(int i = 0; i<channel_settings[r].channels; i++){
          rows[r].modded_values[i] = clamp(params[FADER_PARAMS + i + row].value + offset, _min, _max);
          outputs[r].setVoltage(rows[r].modded_values[i],i);
        }
      // }
    }
  }
  void try_settings_channels(int r){
    if(rows[r].channel_set_mode == COPY_INPUT_ON_BUTTON){
      int c = inputs[FADER_INPUTS + r].getChannels();
      if(c != channel_settings[r].channels){
        channel_settings[r].set(c);
        params[CHANNEL_PARAMS + r].setValue(channel_settings[r].channels);
        set_dirty();
      }
    }
  }
  void sample_absolute(int r){
    try_settings_channels(r);
    int ic = inputs[FADER_INPUTS + r].getChannels();
    int row_offset = r * FADERS;
    for(int i = 0; i<FADERS; i++){
      float cv = inputs[FADER_INPUTS + r].getVoltage(i % ic);
      params[i + row_offset].setValue( rows[r].bipolar ? cv + 5.0f : cv);
    }
  }
  void sample_by_mods(int r){
    try_settings_channels(r);
    int row_offset = r * FADERS;
    float bioffset = rows[r].bipolar ? 5.0f : 0.0f;
    for(int i = 0; i<FADERS; i++){
      params[i + row_offset].setValue( rows[r].modded_values[i] + bioffset);
    }
  }
  void randomize_row(int r, float mod_scaling){
    try_settings_channels(r);
    if(rows[r].sample_mode == SAMPLE_ABSOLUTE){
      if(rows[r].bipolar){
        for(int i = 0; i<FADERS; i++){
          params[i + r * FADERS].setValue(5.0f + (1.0f - random::uniform() * 2.0f) * 5.0f * abs(mod_scaling));
        }
      }else{
        for(int i = 0; i<FADERS; i++){
          params[i + r * FADERS].setValue(random::uniform() * 10.0f * abs(mod_scaling));
        }
      }
    }else{
      float offset = rows[r].bipolar ? -5.0f : 0.0f;
      for(int i = 0; i<FADERS; i++){
        float ra = offset + 10.0f * random::uniform();
        params[i + r * FADERS].setValue(lerpf(params[i + r * FADERS].getValue(), ra, abs(mod_scaling)));
      }
    }
  }
  void sample(int r){
    switch(rows[r].sample_mode){
      case SAMPLE_ABSOLUTE : sample_absolute(r); break;
      case SAMPLE_BY_MODS : sample_by_mods(r); break;
      default : break;
    }
  }
  void try_mutate_row(int r){
    if(inputs[FADER_INPUTS + r].isConnected()){
      if(inputs[SAMPLE_INPUTS + r].isConnected()){
        int c = inputs[SAMPLE_INPUTS + r].getChannels();
        if(c == 1){
          // single channel copy trigger
          triggers[r].update(inputs[SAMPLE_INPUTS + r].getVoltage(0) + params[SAMPLE_PARAMS + r].getValue());
          if(triggers[r].state == PRESSED){
            sample(r);
          }
        }else{
          // multi channel copy trigger
          int tc = rows[r].channels;
          if(rows[r].sample_mode == SAMPLE_ABSOLUTE){
            for(int i = 0; i < tc; i++){
              poly_triggers[r][i].update(inputs[SAMPLE_INPUTS + r].getVoltage(i % c) + params[SAMPLE_PARAMS + r].getValue());
              if(poly_triggers[r][i].state == PRESSED){
                int fc = inputs[FADER_INPUTS + r].getChannels();
                float cv = inputs[FADER_INPUTS + r].getVoltage(i % fc);
                params[i + r * FADERS].setValue(rows[r].bipolar ? cv + 5.0f : cv);
              }
            }
          }else{
            sample(r);

          }
        }
      }else{
        // copy button only
        triggers[r].update(params[SAMPLE_PARAMS + r].getValue());
        if(triggers[r].state == PRESSED)
          sample(r);

      }
    }else{
      // no input
      triggers[r].update(inputs[SAMPLE_INPUTS + r].getVoltage(0) + params[SAMPLE_PARAMS + r].getValue());
      if(triggers[r].state == PRESSED)
        randomize_row(r, params[TRIM_PARAMS + r].getValue());
    }
  }
  void process(const ProcessArgs &args) override {
    if(divider.step()){
      try_mutate_row(0);
      try_mutate_row(1);
      update_row(0);
      update_row(1);        
    }
  }

  json_t *dataToJson() override {
    json_t *rootJ = json_object();
    // json_t *csJ = json_array();
    json_t *rowsJ = json_array();
    for(int i = 0; i<ROWS; i++){
      json_array_append(rowsJ, rows[i].toJson()); 
      // json_array_append(csJ, json_integer(channel_settings[i].channels)); 
    }
    // json_object_set(rootJ, "channels", csJ);
    json_object_set(rootJ, "rows", rowsJ);
    
    json_object_set(rootJ, "color", json_real(color));
    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
    json_t *rowsJ = json_object_get(rootJ, "rows");
    for(int i = 0; i<ROWS; i++){
      // channel_settings[i].set(json_integer_value(json_array_get(csJ, i))); 
      rows[i].fromJson(json_array_get(rowsJ, i));
    }
    divider.load(json_object_get(rootJ, "divider"));
    color = json_number_value(json_object_get(rootJ, "color"));

    json_t *legacy_channels = json_object_get(rootJ, "channels");
    if(legacy_channels){
      update_channel_param(0, json_integer_value(json_array_get(legacy_channels, 0)));
      update_channel_param(1, json_integer_value(json_array_get(legacy_channels, 1)));
    }

  }

  void configFadersInRow(int r, bool bi){
    for(int i = 0; i<FADERS; i++)
      configParam(FADER_PARAMS + i + r * FADERS, 0.0f, 10.0f, 5.0f, rows[r].name + ":" + to_string(i + 1));
  }
  float color = 0.0f;
  void offset_color(){
    color = float_wrap(color + 0.0713f);
    set_dirty();
  }

  Towers(){
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    for(int r = 0; r<ROWS; r++){
      configParam(TRIM_PARAMS + r, -1.0f, 1.0f, 0.0f, rows[r].name + ":mod-scaling");
      configSwitch(POLARITY_PARAM + r, 0, 1, 0, rows[r].name + ":polarity", {"unipolar", "bipolar"});
      configButton(SAMPLE_PARAMS + r, rows[r].name + ":sample");
      configParam(CHANNEL_PARAMS + r, 1, 16, 8, rows[r].name + ":poly-channels");
      configFadersInRow(r, false);

      configInput(FADER_INPUTS + r, rows[r].name + ":fader");
      configInput(TRIM_INPUTS + r, rows[r].name + ":mod");
      configInput(SAMPLE_INPUTS + r, rows[r].name + ":sample");

      configOutput(FADER_OUTPUTS + r, rows[r].name + ":fader");
    }
    onReset();
  }
};



// struct Towers : Module {
//   enum ParamIds {
//     ENUMS(FADER_PARAMS, FADERS * ROWS),
//     ENUMS(POLARITY_PARAM, ROWS),
//     ENUMS(TRIM_PARAMS, ROWS),
//     ENUMS(SAMPLE_PARAMS, ROWS),
//     ENUMS(CHANNEL_PARAMS, ROWS),
//     NUM_PARAMS
//   };
//   enum InputIds {
//     ENUMS(FADER_INPUTS, ROWS),
//     ENUMS(TRIM_INPUTS, ROWS),
//     ENUMS(SAMPLE_INPUTS, ROWS),
//     NUM_INPUTS
//   };
//   enum OutputIds {
//     ENUMS(FADER_OUTPUTS, ROWS),
//     NUM_OUTPUTS
//   };

struct TowersPanel : _less::Widget {
  LayoutData* art;
  TowersPanel(LayoutData *a){
    art = a;
  }
  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
  }
  void render() override{
    fill(theme.bg);
    rect(0,0,box.size.x, box.size.y);
    fill(theme.fg);

    // Vec p = Vec(RACK_GRID_WIDTH, 30.0f);
    // float W = (width - p.x * 2);
    // float w = W / FADERS;

    float o = -18.0f;
    float y = art->inputs[0].box.pos.y + o;;
    fill(theme.fg);

    fill(theme.fg);
    stroke(theme.fg);
    
    text("IN", art->inputs[Towers::FADER_INPUTS].box.pos.x, y, 10.0f);
    
    float sx = lerpf(art->inputs[Towers::SAMPLE_INPUTS].box.pos.x, art->params[Towers::SAMPLE_PARAMS].box.pos.x, 0.5f);
    text("SAMPLE", sx, y, 10.0f);
    
    float mx = lerpf(art->inputs[Towers::TRIM_INPUTS].box.pos.x, art->params[Towers::TRIM_PARAMS].box.pos.x, 0.5f);
    text("MODS", mx, y, 10.0f);

    text("VOLT", art->params[Towers::POLARITY_PARAM].box.pos.x, y, 10.0f);
    text("POLY", art->params[Towers::CHANNEL_PARAMS].box.pos.x, y, 10.0f);

    text("OUT", art->outputs[Towers::FADER_OUTPUTS].box.pos.x, y, 10.0f);
    text("unless towers", center.x, box.size.y - RACK_GRID_WIDTH + 6.0f, 10.0f);

  }

};

struct PolarityButton : unless::TextButtonWidget{
  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
  }
  void render() override{
    label = state == 1 ? "+-" : "+";
    unless::TextButtonWidget::render();
  }
};

struct SampleButton : unless::TextButtonWidget{
  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
    label = "=";
  }
};
struct TowersButton : _less::Widget{
  int state = 0;
  std::string texts[2] = {"a", "b"};
  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
  }
  void set_text(std::string a, std::string b = ""){
    texts[0] = a;
    if(b == "")
      texts[1] = a;
    else
      texts[1] = b;
  }
  void render() override{
    float roundness = 2.0f;
    fill(theme.input_dark);
    center_round_rect(center.x, center.y + 3.0f, width, height * 0.7f, roundness, FILL);
    fill(theme.input);
    center_round_rect(center.x, center.y, width, height * 0.7f, roundness, FILL);
    stroke_width(3.0f);
    stroke(theme.output);
    fill(theme.bg_dark);
    text(texts[state], center.x, center.y, height * 0.8f);
  }
};

struct TooKnob : unless::DrawKnob{
  void init(NVGcontext *vg) override{
    fg = &theme.input_dark;
    bg = &theme.input;
    thickness = 2.0f;
    DrawKnob::init(vg);
  }
};

struct TowersSlider : _less::Slider<BipolarSlider>{
  int row = -1;
  float last_value = 0.0f;
  TowersSlider(Vec pos, Vec size, engine::Module *m, int parid, bool sn = false, bool centered = false, bool vertical = true) : _less::Slider<BipolarSlider>(pos, size, m, parid, sn, centered, vertical){

  }
  // void onDoubleClick(const event::DoubleClick& e)override{
  //   if(paramQuantity->getValue() != last_value){
  //     Knob::onDoubleClick(e);
  //     last_value = paramQuantity->getValue();
  //   }else{
  //     paramQuantity->setValue(paramQuantity->minValue);
  //   }
  // }
};

struct TowersWidget : ModuleWidget {
  _less::Panel *panelWidget = NULL;
  Towers *moduleInstance = NULL;
  
  _less::Slider<BipolarSlider>* sliders[FADERS * ROWS];
  int channels[2] = {FADERS, FADERS};

  Vec p = Vec(RACK_GRID_WIDTH * 0.8f, 30.0f);

  Art art;

  void arrange_sliders(int r, int c){
    float w = (box.size.x - p.x * 2) / (float) c;
    for(int i = 0; i<FADERS; i++){
      int index = r * FADERS + i;
      if(i < channels[r]){
        sliders[index]->visible = true;
        sliders[index]->box.pos.x = p.x + w * i;
        sliders[index]->box.size.x = w;
        sliders[index]->handle->set_box(0.0f,0.0f,w,sliders[index]->handle->height);
      }else{
        sliders[index]->visible = false;
      }
    }
  }
  void update_row(int r){
    if(moduleInstance->channel_settings[r].channels != channels[r]){
      channels[r] = moduleInstance->channel_settings[r].channels;
      arrange_sliders(r, channels[r]);
      panelWidget->set_dirty();
    }
  }

  void add_slider(float y, float w, float h, int i, int r){
    int rindex = r * FADERS + i;
    _less::Slider<BipolarSlider> *af = new TowersSlider(
      Vec(p.x + w * i, y), Vec(w,h),
      module, 
      Towers::FADER_PARAMS + rindex
    );
    af->handle->bipolar = moduleInstance ? &moduleInstance->rows[r].bipolar : NULL;
    af->handle->view_mode = moduleInstance ? &moduleInstance->rows[r].view_mode : NULL;
    af->handle->modded_value = moduleInstance ? &moduleInstance->rows[r].modded_values[i] : NULL;
    af->handle->color_pos = moduleInstance ? &moduleInstance->color : NULL;
    af->handle->index = i + 1;

    addParam(af);
    sliders[rindex] = af;
  }

  TowersWidget(Towers *module){
    setModule(module);
    art = Art::load(this, "towers", Towers::NUM_INPUTS, Towers::NUM_OUTPUTS, Towers::NUM_PARAMS);

    moduleInstance = module;
    box.size = Vec(MODULE_WIDTH * RACK_GRID_WIDTH, 380);

    panelWidget = new _less::Panel(box.size, nvgHSLA(0, 0, 0, 0));
    TowersPanel *panel = new TowersPanel(&art.layout);

    panel->set_box(0.0f,0.0f,box.size.x, box.size.y);
    panelWidget->fb->addChild(panel);
    addChild(panelWidget);

    float centery = box.size.y * 0.406f;
    float w = (box.size.x - RACK_GRID_WIDTH * 2) / (float) FADERS;
    float h = (box.size.y - p.y * 2) / 2.5f;

    for(int i = 0; i<FADERS; i++){
      add_slider(centery - h - 9.0f, w, h, i, 0);
      add_slider(centery + 9.0f, w, h, i, 1);
    }

    for(int i = 0; i<2; i++){
      art.input<_less::Port<unless::CvInput>>(Towers::FADER_INPUTS + i);
      art.input<_less::Port<unless::GateHoleInput>>(Towers::SAMPLE_INPUTS + i);
      art.input<_less::Port<unless::RingHoleInput>>(Towers::TRIM_INPUTS + i);
      art.param<_less::Button<PolarityButton>>(Towers::POLARITY_PARAM + i);
      art.param<_less::Knob<TooKnob>>(Towers::TRIM_PARAMS + i);
      art.param<_less::Button<SampleButton>>(Towers::SAMPLE_PARAMS + i, true);
      art.param<_less::FixedIntSelectButton<unless::ChannelsButton>>(Towers::CHANNEL_PARAMS + i);
      art.output<_less::Port<unless::CvOutput>>(Towers::FADER_OUTPUTS + i);
    }
  }

  void step() override{
    ModuleWidget::step();
    if(moduleInstance){
      if(moduleInstance->dirty){
        panelWidget->set_dirty();
        moduleInstance->dirty = false;
      }
      update_row(0);
      update_row(1);
    }
  }
  void onButton(const event::Button &e) override{
    if(e.action == GLFW_PRESS && e.button == 0){
      float w = 35.0f;
      Rect color_button = Rect(
        box.size.x * 0.5f - w, 
        box.size.y - RACK_GRID_WIDTH, 
        w,
        RACK_GRID_WIDTH
      );
      
      if(color_button.isContaining(e.pos)){
        moduleInstance->offset_color();
        // return;
      }
    }
    ModuleWidget::onButton(e);
  }

  std::vector<std::string> view_mode_names = {
    "faders (default)",
    "staves in C",
    "Psilocybe Omrinsis",
    "windows", 
    // "actual towers",
  };


  std::vector<std::string> channel_set_mode_names = {
    "POLY button (default)",
    "IN port (auto)",
    "IN port (on SAMPLE trigger)"
  };
  std::vector<std::string> sample_mode_names = {
    "IN absolute (default)",
    "IN relative (with MODS)"
  };
  
  void appendContextMenu(Menu *menu) override {
    Towers *view = dynamic_cast<Towers*>(module);
    assert(view);
    add_color_slider(menu, moduleInstance);

    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<SampleDividerMenuItem>(&SampleDividerMenuItem::divider, &moduleInstance->divider));

    std::vector<std::string> row_names = {
      "A",
      "B"
    };

    for(int i = 0; i<ROWS; i++){
      menu->addChild(construct<MenuLabel>());
      menu->addChild(construct<MenuLabel>(&MenuLabel::text, row_names[i]));
      menu->addChild(construct<EnumMenuItem<TowersSampleMode>>(
        &EnumMenuItem<TowersSampleMode>::names, &sample_mode_names, 
        &EnumMenuItem<TowersSampleMode>::current_value, &moduleInstance->rows[i].sample_mode,
        &EnumMenuItem<TowersSampleMode>::text, "SAMPLE mode" 
      ));
      menu->addChild(construct<EnumMenuItem<ChannelSetMode>>(
        &EnumMenuItem<ChannelSetMode>::names, &channel_set_mode_names, 
        &EnumMenuItem<ChannelSetMode>::current_value, &moduleInstance->rows[i].channel_set_mode,
        &EnumMenuItem<ChannelSetMode>::text, "set POLY channels by" 
      ));
      menu->addChild(construct<EnumMenuItem<TowersViewMode>>(
        &EnumMenuItem<TowersViewMode>::names, &view_mode_names, 
        &EnumMenuItem<TowersViewMode>::current_value, &moduleInstance->rows[i].view_mode,
        &EnumMenuItem<TowersViewMode>::text, "view mode" 
      ));
    }
  }
};


Model *modelTowers = createModel<Towers, TowersWidget>("towers");
/*
  TODO
*/
