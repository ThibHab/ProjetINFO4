#pragma once

#include "../src/_lessjobb.hpp"

namespace unless{

static const float gate_roundness = 3.0f;
static const float default_stroke = 1.7f;
static const float depth = 1.7f;

struct GatePort : _less::Widget{
  bool input = false;
  NVGcolor *bg = &theme.input;
  NVGcolor *fg = &theme.input_dark;

  void init(NVGcontext *vg) override{
    set_box(box.pos.x, box.pos.y,box.size.x, box.size.y);
  }
  void render() override{
        float w = width * 0.96f;
    float s = default_stroke;
    float s2 = default_stroke * 0.5f;
    
    Vec size = Vec(w - s2, w - s2);
    fill(bg);
    // stroke(bg);
    // center_round_rect(Vec(center.x, center.y + height * 0.04f), size, gate_roundness, BOTH);

    Vec sp = Vec(s2, height * 0.27f);
    NVGpaint shadow = nvgBoxGradient(_vg, sp.x, sp.y, size.x, size.y, gate_roundness * 2.0f, 4.0f, theme.shadow, theme.transparent);

    fill(shadow);
    round_rect(sp, size, gate_roundness, FILL);

    // fill(bg);
    // center_round_rect(center, size, gate_roundness, FILL);

    fill(bg);
    stroke(fg);
    stroke_width(s);
    center_round_rect(center, size, gate_roundness, BOTH);

    hole();
  }
};

struct CvPort : GatePort{
  void render() override{      
    float s = default_stroke;
    float w2 = width * 0.5f;

    // fill(bg);
    // nvgBeginPath(_vg);
    // nvgEllipse(_vg, center.x, center.y + height * 0.05f, w2, width * 0.475f);
    // finish(FILL);
    

    Vec sp = Vec(center.x, center.y + height * 0.1f);
    NVGpaint shadow = nvgRadialGradient(_vg, sp.x, sp.y, w2 - s * 1.5f, w2, theme.shadow, theme.transparent);
    fill(shadow);
    circle(sp, w2, FILL);

    stroke_width(s);
    fill(bg);
    stroke(fg);
    circle(center.x, center.y, w2 - s * 0.5f, BOTH);

    hole();
  }
};

struct GateInput : GatePort {
  GateInput(){
    input = true;
    bg = &theme.input;
    fg = &theme.input_dark;
  }
};

struct GateOutput : GatePort {
  GateOutput(){
    input = false;
    bg = &theme.output;
    fg = &theme.output_dark;
  }
};

struct PointInput : _less::Widget{
  bool input = true;
  void init(NVGcontext *vg) override{
    set_box(box.pos.x, box.pos.y,box.size.x, box.size.y);
    color_mode(HSLA);
  }
  void render() override{
    fill(theme.output_dark);
    circle(center, 3.0f);
    // hole(std::min(box.size.x * 0.5f, 6.0f));
  }
};


struct PointOutput : PointInput{
  PointOutput(){
    input = false;
  }
};

struct HoleInput : _less::Widget{
  bool input = true;
  void init(NVGcontext *vg) override{
    set_box(box.pos.x, box.pos.y,box.size.x, box.size.y);
    color_mode(HSLA);
  }
  void render() override{
    hole(std::min(box.size.x * 0.5f, 6.0f));
  }
};


struct HoleOutput : HoleInput{
  HoleOutput(){
    input = false;
  }
};

struct RingHolePort : GatePort{
  void render() override{
    stroke(bg);
    stroke_width(1.63f);
    fill(theme.bg);
    circle(center.x, center.y, box.size.x * 0.42f, BOTH);
    hole();
  }
};

struct RingHoleInput : RingHolePort{
  RingHoleInput(){
    input = true;
    bg = &theme.input;
    fg = &theme.input_dark;
  }
};

struct RingHoleOutput : RingHolePort{
  RingHoleOutput(){
    input = false;
    bg = &theme.output;
    fg = &theme.output_dark;
  }
};

struct GateHolePort : GatePort{
  void render() override{
    hole();
    stroke(bg);
    stroke_width(1.63f);
    center_round_rect(center.x, center.y, box.size.x * 0.84f, box.size.x * 0.84f, 3.0f, STROKE);
  }
};

struct GateHoleInput : GateHolePort{
  GateHoleInput(){
    input = true;
    bg = &theme.input;
    fg = &theme.input_dark;
  }
};

struct GateHoleOutput : GateHolePort{
  GateHoleOutput(){
    input = false;
    bg = &theme.output;
    fg = &theme.output_dark;
  }
};

struct CvInput : CvPort{
  CvInput(){
    input = true;
    bg = &theme.input;
    fg = &theme.input_dark;
  }
};

struct CvOutput : CvPort{
  CvOutput(){
    input = false;
    bg = &theme.output;
    fg = &theme.output_dark;
  }
};

struct ColoredPort : _less::Widget{
  bool input = true;
  NVGcolor bg = nvgRGB(0,0,0);

  void init(NVGcontext *vg) override{
    set_box(box.pos.x, box.pos.y,box.size.x, box.size.y);
    color_mode(HSLA);
  }
  void render() override{      
    fill(bg);
    nvgBeginPath(_vg);
    nvgEllipse(_vg, center.x, center.y + height * 0.05f, width * 0.5f, width * 0.475f);
    finish(FILL);

    fill(0.0f, 0.0f, 0.0f, 0.15f);
    circle(center.x, center.y + height * 0.1f, width * 0.5f, FILL);

    fill(bg);
    circle(center.x, center.y - height * 0.01f, width * 0.5f, FILL);

    hole();
  }
};

struct DrawKnob : _less::Widget{
  NVGcolor *bg = &theme.bg_dark;
  NVGcolor *fg = &theme.fg;
  float thickness = 4.0f;
  void init(NVGcontext *vg) override{
    set_box(box.pos.x, box.pos.y,box.size.x, box.size.y);
  }
  void render() override{
    fill(bg);
    circle(center.x, center.y, width * 0.5f, FILL);

    stroke(fg);
    line(center.x, center.y, center.x, center.y - width * 0.5f, thickness);
  }
};

struct InputKnob : DrawKnob{
  InputKnob(){
    bg = &theme.input;
    fg = &theme.output;
  }
};

struct OutputKnob : DrawKnob{
  OutputKnob(){
    bg = &theme.output_dark;
    fg = &theme.input;
  }
};


struct DrawSlider : _less::Widget {
  float scaled_value = 0.0f;
  float *modded_value = NULL;

  bool *bipolar = NULL;
  float color_pos = 0.0f;
  NVGcolor neg_color = theme.rainbow_color(color_pos);
  NVGcolor pos_color = theme.rainbow_color(color_pos + 0.5f);

  float sw = 2.0f;

  void drawBipolar(float v){
    float hp = height - sw * 2.0f;
    fill(theme.output);
    path(1, center.y);
    float y = center.y + (v - 0.5f) * - hp;
    point(1, y);
    point(width - 1, y);
    point(width - 1, center.y);
    close(FILL);
    if(v > 0.5f)
      stroke(lerp(theme.output, pos_color, (v - 0.5f) * 2.0f));
    else
      stroke(lerp(neg_color, theme.output, v * 2.0f));
    line(1, y, width - 1, y);

    if(modded_value){
      if(*modded_value > 0.0f)
        stroke(lerp(theme.output, pos_color, rescale(*modded_value, 0.0f, 5.0f, 0.0, 1.0f)));
      else
        stroke(lerp(neg_color, theme.output, rescale(*modded_value, -5.0f, 0.0f, 0.0, 1.0f)));

      y = center.y + (rescale(*modded_value, -5.0f, 5.0f, 0.0, 1.0f) - 0.5f) * -hp;
      line(1, y, width - 1, y);
    }
  }

  void drawUnipolar(float v){
    fill(theme.output);
    float y = rescale(v, 0.0f, 1.0f, height - sw, sw);
    point(1, height - 1);
    point(1, y);
    point(width - 1, y);
    point(width - 1, height - 1);
    close(FILL);
    stroke(lerp(theme.output, pos_color, v));
    line(1, y, width - 1, y);

    if(modded_value){
      stroke(lerp(theme.output, pos_color, rescale(*modded_value, 0.0f, 10.0f, 0.0, 1.0f)));
      y = rescale(rescale(*modded_value, 0.0f, 10.0f, 0.0, 1.0f), 0.0f, 1.0f, height - sw, sw);
      line(1, y, width - 1, y);
    }
  }

  void render() override{
    scissor();
    stroke_width(sw);
    stroke(theme.fg);
    fill(theme.output_dark);
    rect(0,0,width, height, BOTH);
    fill(theme.output);
    // float h = hp * scaled_value;

    if(bipolar && *bipolar){
      drawBipolar(scaled_value);
    }else{
      drawUnipolar(modded_value ? scaled_value : random::uniform());
    }
  }
};



struct TextButtonWidget : _less::Widget{
  bool pressed = false;
  int state = 0;
  std::string label = "";
  NVGcolor *bg = &theme.input;
  NVGcolor *fg = &theme.bg_dark;
  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
  }
  void render() override{
    const float depth = 1.7f;
    const float roundness = 2.0f;
    translate(0.0f, 0.0f);
    float p = 0.0f;
    float sp = center.y + depth * 1.2f;
    float h = height - depth * 5.0f;
    float bh = h + depth * 2.0f;
    if(pressed){
      p += depth;
      sp -= depth * 0.5f;
      bh -= depth;
    }

    fill(bg);
    round_rect(0.0f, p, width, bh, roundness, FILL);

    NVGpaint shadow = nvgBoxGradient(_vg, 0.5f, sp - h * 0.5f, width - 1.0f, h - 1.0f, roundness * 2.0f, depth * 2.0f, theme.shadow, theme.transparent);
    fill(shadow);
    // fill(theme.shadow);
    center_round_rect(center.x, sp, width, h, roundness, FILL);

    fill(bg);
    round_rect(0.0f, p, width, h, roundness, FILL);

    fill(fg);

    if(label != ""){
      text(label, center.x, p + h * 0.5f, height * 0.67f);
    }
    translate(center.x, p + h * 0.5f);
    symbol();

  }
  virtual void symbol(){}
};
struct ChannelsButton : TextButtonWidget{
  float value;
  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
  }
  void symbol() override{
    text(std::to_string((int)floor(value)), 0.0f, 0.0f, height * 0.5f);
  }
};



}
