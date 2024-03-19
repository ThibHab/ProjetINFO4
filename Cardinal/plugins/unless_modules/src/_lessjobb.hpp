#pragma once
#include <app/common.hpp>
#include <settings.hpp>
#include <app/PortWidget.hpp>
#include <widget/FramebufferWidget.hpp>
#include <app/CircularShadow.hpp>
#include "_less.hpp"
// slightly modified SvgPort and SvgKnob classes from Rack for procedural drawing
namespace _less{

template <class TLessWidget>
struct Port : PortWidget {
  widget::FramebufferWidget *fb;
  TLessWidget *sw;
  Port(math::Vec pos, engine::Module *m, int _portId, float s = 24.0f){
    module = m;
    portId = _portId;

    fb = new widget::FramebufferWidget;
    addChild(fb);
    sw = new TLessWidget;
    type = sw->input ? engine::Port::INPUT : engine::Port::OUTPUT;
    fb->addChild(sw);


    box.pos = pos;
    box.size = Vec(s, s);
    box.pos = box.pos.minus(box.size.div(2));

    fb->box.size = Vec(s,s * 1.4f);
    fb->box.pos.y -= s * 0.2f;
    // fb->box.size = box.size;
    sw->box.size = Vec(s, s * 1.4f);

    fb->dirty = true;
  }
  void set_dirty(){
    fb->dirty = true;
  }
};

template <class TLessWidget>
struct Knob : rack::Knob {
  CircularShadow *shadow;
  widget::FramebufferWidget *fb;
  widget::TransformWidget *tw;
  TLessWidget *sw;
  /** Angles in radians */
  float maxAngle = 3.1415f - (3.1415f * 0.25f);
  float minAngle = -3.1415f + (3.1415f * 0.25f);
  Knob(math::Vec pos, Vec size, engine::Module *m, int paramId, bool sn = false, bool centered = true){
    snap = sn;

    Knob::module = m;
    Knob::paramId = paramId;
    Knob::initParamQuantity();
    // module = m;
    // if(m){
    //   paramQuantity = m->paramQuantities[paramId];
    // }

    fb = new widget::FramebufferWidget;
    addChild(fb);

    shadow = new CircularShadow;
    fb->addChild(shadow);

    tw = new widget::TransformWidget;
    fb->addChild(tw);

    sw = new TLessWidget;
    tw->addChild(sw);

    box.pos = pos;

    box.size = size;

    if(centered)
      box.pos = box.pos.minus(size.mult(0.5f));

    tw->box.size = size;
    tw->box.pos = Vec(0.0f, 0.0f);

    sw->box.size = size;
    fb->box.size = size;
    
    shadow->box.size = size;

    shadow->box.pos = Vec(0,3.0f);

    fb->dirty = true;
  }
  void onChange(const event::Change &e) override{
    engine::ParamQuantity *paramQuantity = getParamQuantity();
    // Re-transform the widget::TransformWidget
    if (paramQuantity) {
      float angle;
      if (paramQuantity->isBounded()) {
        angle = math::rescale(paramQuantity->getValue(), paramQuantity->getMinValue(), paramQuantity->getMaxValue(), minAngle, maxAngle);
      }
      else {
        angle = math::rescale(paramQuantity->getValue(), -1.f, 1.f, minAngle, maxAngle);
      }
      angle = std::fmod(angle, 2.0 * M_PI);
      tw->identity();
      // Rotate SVG
      math::Vec center = sw->box.getCenter();
      tw->translate(center);
      tw->rotate(angle);
      tw->translate(center.neg());

      // sw->rotation = angle;
      fb->dirty = true;
    }
    rack::Knob::onChange(e);
  }
};


template <class TLessWidget>
struct Slider : rack::Knob {
  widget::FramebufferWidget *fb;
  TLessWidget *handle;
  
  int paramId;
  bool absolute_mode = false;

  bool gate_mode = false;
  bool gate_state = false;

  Slider(Vec pos, Vec size, engine::Module *m, int parid, bool sn = false, bool centered = false, bool vertical = true){
    paramId = parid;
    snap = sn;
    box.pos = pos;
    box.size = size;

    if(centered)
      box.pos = box.pos.minus(size.mult(0.5f));
    // fb = new widget::FramebufferWidget;
    // fb->box.size = size;
    // addChild(fb);
    Knob::horizontal = !vertical;

    handle = new TLessWidget;
    handle->set_box(0.0f,0.0f,size.x, size.y);

    addChild(handle);
    // fb->addChild(handle);


    module = m;
    Knob::module = m;
    Knob::paramId = paramId;
    if(m){
      handle->paramQuantity = Knob::getParamQuantity();
    }
    // if(m){
    //   Knob::paramQuantity = m->paramQuantities[paramId];
    //   handle->paramQuantity = m->paramQuantities[paramId];
    // }


    // fb->dirty = true;
  }
  void onChange(const event::Change& e) override{
    // if(gate_mode) return;
    engine::ParamQuantity *paramQuantity = getParamQuantity();

    if (paramQuantity) {
      float v = paramQuantity->getScaledValue();

      handle->scaled_value = v;
      // handle->real_value = paramQuantity->getValue();
      // fb->dirty = true;
    }
    ParamWidget::onChange(e);
  }
  void setAbsoluteValue(Vec pos){
    engine::ParamQuantity *paramQuantity = getParamQuantity();
    float _min = paramQuantity->getMinValue();
    float _max = paramQuantity->getMaxValue();
    float t = Knob::horizontal ? pos.x / box.size.x : 1.0f - (pos.y / box.size.y);
    paramQuantity->setValue(lerpf(_min, _max, t));
  }
  void onHover(const event::Hover& e) override {
    int mods = APP->window->getMods();
    if ((mods & RACK_MOD_MASK) == (GLFW_MOD_ALT)) {
      // setAbsoluteValue(e.pos);
    }
    ParamWidget::onHover(e);
  }
  bool alt_mode(){
    int mods = APP->window->getMods();
    bool control_alt = (mods & RACK_MOD_MASK) == (RACK_MOD_CTRL | GLFW_MOD_ALT);
    if(absolute_mode)
      return !control_alt;
    else
      return control_alt;
  }
  void flip_value(){
    engine::ParamQuantity *paramQuantity = getParamQuantity();
    gate_state = !gate_state;
    paramQuantity->setValue(gate_state ? 10.0f : 0.0f);
  }
  void onButton(const event::Button& e) override{
    if(gate_mode){
      if(e.action == GLFW_PRESS){
        flip_value();
        onChange( event::Change());
        // e.consume(this);
      }
    }
    if (alt_mode()) {
      e.consume(this);
    }else
      ParamWidget::onButton(e);
  }

  void onDragStart(const event::DragStart& e) override{
    if(gate_mode) return;
    if(!alt_mode())
      rack::Knob::onDragStart(e);
  }

  void onDragHover(const event::DragHover& e) override{
    if(gate_mode) return;
    if (alt_mode() && e.button == GLFW_MOUSE_BUTTON_LEFT){
      setAbsoluteValue(e.pos);
    }else{
      rack::Knob::onDragHover(e);
    }

  }
  void onDragMove(const event::DragMove& e) override{
    if(gate_mode) return;
    if (!alt_mode())
      rack::Knob::onDragMove(e);
  }

};

struct XSlider : rack::Knob {
  int paramId;

  XSlider(Vec pos, Vec size, engine::Module *m, int parid, bool sn = false, bool centered = false, bool vertical = true){
    paramId = parid;
    snap = sn;
    box.size = Vec(0,0);
    Knob::horizontal = true;
    if(m){
      Knob::module = m;
      Knob::paramId = paramId;
      Knob::initParamQuantity();
      Knob::speed = -1.0f;
      // Knob::paramQuantity = m->paramQuantities[paramId];
    }
  }
};

template <class TLessWidget>
struct XYSlider : rack::Knob {
  widget::FramebufferWidget *fb;
  TLessWidget *handle;
  
  int paramId;
  bool absolute_mode = false;

  bool gate_mode = false;
  bool gate_state = false;

  XSlider *x_slider;
  float x_threshold = 0.1f;

  XYSlider(Vec pos, Vec size, engine::Module *m, int parid, bool sn = false, bool centered = false, bool vertical = true){
    paramId = parid;



    snap = sn;
    box.pos = pos;
    box.size = size;
    if(centered)
      box.pos = box.pos.minus(size.mult(0.5f));
    // fb = new widget::FramebufferWidget;
    // fb->box.size = size;
    // addChild(fb);
    Knob::horizontal = !vertical;

    handle = new TLessWidget;
    handle->set_box(0.0f,0.0f,size.x, size.y);

    addChild(handle);
    // fb->addChild(handle);


    if(m){
      module = m;
      Knob::module = m;
      Knob::paramId = paramId;
      Knob::initParamQuantity();
      engine::ParamQuantity *kp = Knob::getParamQuantity();
      // Knob::paramQuantity = m->paramQuantities[paramId];
      kp->displayOffset = 1.0f;
      kp->displayMultiplier = -1.0f;

      handle->paramQuantity = kp;
    }
    // fb->dirty = true;
  }
  void set_x_param(XSlider *s){
    x_slider = s;
    // handle->x_slider = s->getParamQuantity();
    x_slider->visible = false;
    x_slider->box = box;
  }
  void onChange(const event::Change& e) override{
    engine::ParamQuantity *paramQuantity = getParamQuantity();
    if (paramQuantity) {
      float v = paramQuantity->getScaledValue();
      handle->scaled_value = v;
      handle->dirty = true;
    }
    ParamWidget::onChange(e);
  }
  void onEnter(const event::Enter& e)override{
    ParamWidget::onEnter(e);
    // x_slider->onEnter(e);
    // if(tooltip){
    // }
  }
  void onHover(const event::Hover & e) override {
    ParamWidget::onHover(e);
  }
  void onLeave(const event::Leave& e)override{
    ParamWidget::onLeave(e); 
    // x_slider->onLeave(e);
  }
  void onButton(const event::Button& e) override{
    ParamWidget::onButton(e);
  }
  void onDoubleClick(const event::DoubleClick& e) override{
    x_slider->onDoubleClick(e);
    Knob::onDoubleClick(e);
  }


  void onDragStart(const event::DragStart& e) override{
    rack::Knob::onDragStart(e);
  }
  void onDragMove(const event::DragMove& e) override{
    bool x_drag = abs(e.mouseDelta.x) - x_threshold > abs(e.mouseDelta.y);
    if(x_drag){
      x_slider->onDragMove(e);
    }else{
      speed = 1.0 - (1.0f - x_slider->getParamQuantity()->getValue()) * 1.0f;
      rack::Knob::onDragMove(e);
    }
    handle->dirty = true;
  }

};

template <class TLessWidget>
struct Button : Switch {
  widget::FramebufferWidget* fb;
  TLessWidget *sw;
  Button(math::Vec pos, Vec size, engine::Module *m, int paramId, bool mom = true, bool centered = true){
    momentary = mom;

    Switch::module = m;
    Switch::paramId = paramId;
    Switch::initParamQuantity();

    // if(m){
    //   paramQuantity = m->paramQuantities[paramId];
    // }

    fb = new widget::FramebufferWidget;
    addChild(fb);

    sw = new TLessWidget;
    fb->addChild(sw);

    box.pos = pos;
    box.size = size;
    if(centered)
      box.pos = box.pos.minus(size.mult(0.5f));

    sw->box.size = size;
    // fb->box.size = size;
    float depth = 1.7f;
    fb->box.size = Vec(size.x,size.y + depth * 2.0f);
    sw->box.size = Vec(size.x,size.y + depth * 2.0f);
    // sw->box.size = Vec(size.x,size.y * 1.4f);
    // fb->box.pos.y -= size.y * 0.2f;
    sw->box.pos.y += depth * 0.5f;


    fb->dirty = true;
  }
  void onChange(const event::Change& e) override{
    sw->state = floor(getParamQuantity()->getValue());
    fb->dirty = true;
    Switch::onChange(e);
  }
  void onDragStart(const event::DragStart& e) override{
    // sw->box.pos.y += 1.0f;
    sw->pressed = true;

    Switch::onDragStart(e);
  }
  void onDragEnd(const event::DragEnd& e) override{
    // sw->box.pos.y -= 1.0f;
    sw->pressed = false;

    Switch::onDragEnd(e);
  }


  void onButton(const event::Button &e) override{
    if(e.button == 0){
      e.consume(this);
    }
      // e.consume(this);
      // sw->onButton(e);
    // }else{
    Switch::onButton(e);
    // }
  }

  void step()override{
    if(sw->dirty)
      fb->dirty = true;
    Switch::step();
  }
};

struct FixedIntValueItem : MenuItem {
  int value;
  ParamQuantity *paramQuantity = NULL;
  void onAction(const event::Action& e) override {
    if(paramQuantity)
      paramQuantity->setValue(value);
  }
};

template <class TLessWidget>
struct FixedIntSelectButton : Switch {
  widget::FramebufferWidget* fb;
  TLessWidget *sw;

  std::vector<int> possible_values = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

  std::string name = "channels";

  FixedIntSelectButton(math::Vec pos, Vec size, engine::Module *m, int paramId, bool mom = true, bool centered = true){
    module = m;
    paramId = paramId;


    if(m){    
      Switch::module = m;
      Switch::paramId = paramId;
      Switch::initParamQuantity();
    }

    fb = new widget::FramebufferWidget;
    addChild(fb);

    sw = new TLessWidget;
    fb->addChild(sw);

    box.pos = pos;
    box.size = size;
    if(centered)
      box.pos = box.pos.minus(size.mult(0.5f));

    sw->value = possible_values.at(floor(random::uniform() * possible_values.size()));
    sw->box.size = size;
    // fb->box.size = size;
    float depth = 1.7f;
    fb->box.size = Vec(size.x,size.y + depth * 2.0f);
    sw->box.size = Vec(size.x,size.y + depth * 2.0f);
    // sw->box.size = Vec(size.x,size.y * 1.4f);
    // fb->box.pos.y -= size.y * 0.2f;
    sw->box.pos.y += depth * 0.5f;


    fb->dirty = true;
  }
  void onChange(const event::Change& e) override{
    sw->value = floor(getParamQuantity()->getValue());
    Switch::onChange(e);
  }
  void onDragStart(const event::DragStart& e) override{
    if(e.button == 0){
      sw->pressed = true;
      createContextMenu();
    }
  }
  void onDragEnd(const event::DragEnd& e) override{
    sw->pressed = false;
    Switch::onDragEnd(e);
  }
  
  void createContextMenu() {
    ui::Menu* menu = createMenu();
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, name));
    for(auto v : possible_values){
      FixedIntValueItem* item = new FixedIntValueItem;
      item->text = std::to_string(v);
      item->rightText = CHECKMARK(getParamQuantity()->getValue() == v);
      item->value = v;
      item->paramQuantity = getParamQuantity();
      menu->addChild(item); 
    }
  }

  void onButton(const event::Button &e) override{
    if(e.button == 0){
      e.consume(this);
    }else{
      Switch::onButton(e);
    }
  }

  void step()override{
    if(sw->dirty)
      fb->dirty = true;
    Switch::step();
  }
};


template <typename TLight>
struct TinyLight : TLight {
  TinyLight() {
    this->box.size = Vec(4.0f, 4.0f);
  }
};

template <class TLessWidget>
struct OptionSwitch : Button<TLessWidget>{
  void onButton(const event::Button &e) override{
    Switch::onButton(e);
  }
};

struct BoolMenuItem : MenuItem {
  bool *setting = NULL;
  _less::Panel *panel;
  void onAction(const event::Action &e) override {
    *setting = !*setting;
    if(panel)
      panel->set_dirty();
  }
  void step() override {
    rightText = *setting ? "✔" : "";
    MenuItem::step();
  }
};

struct IntValueItem : MenuItem {
  int *setting;
  int value;
  _less::Panel *panel;
  void onAction(const event::Action& e) override {
    if(panel)
      panel->set_dirty();
    *setting = value;
  }
};

struct IntMenuItem : MenuItem {
  int* setting;
  std::vector<int> values;
  int min_value = 0;
  int max_value = 16;
  _less::Panel *panel;
  Menu* createChildMenu() override {
    Menu* menu = new Menu;
    int c = values.size() > 0 ? values.size() : max_value + 1;
    for (int m = values.size() > 0 ? 0 : min_value; m < c; m++) {
      IntValueItem* item = new IntValueItem;
      item->value = m;
      item->panel = panel;
      item->setting = setting;
      item->text = std::to_string(m);
      item->rightText = *setting == m ? "✔" : "";
      menu->addChild(item);
    }
    return menu;
  }
};


struct HelpLineItem : MenuLabel {
  HelpLineItem(std::string t){
    text = t;
  }
};

struct HelpItem : MenuItem {
  std::vector<std::string> *lines = NULL;
HelpItem(std::vector<std::string> *ls){
    lines = ls;
    text = "help";
    rightText = RIGHT_ARROW;
  }

  Menu* createChildMenu() override {
    Menu* menu = new Menu;
    for(auto l : *lines)
      menu->addChild(new HelpLineItem(l));
    return menu;
  }
};
}

