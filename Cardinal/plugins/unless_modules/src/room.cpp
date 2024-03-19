#include "unless.hpp"
#include "utils.hpp"
#include <algorithm>
#include "../art/components.hpp"

#define MODULE_WIDTH 3

struct Room : Module {
  enum ParamIds {
    NUM_PARAMS
  };
  enum InputIds {
    NUM_INPUTS
  };
  enum OutputIds {
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  bool inclusive = false;
  bool stuckHorizontal = false;

  bool strip_mode = false;

  void onReset() override{
  }
  json_t *dataToJson() override {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "inclusive", json_boolean(inclusive));
    json_object_set_new(rootJ, "strip_mode", json_boolean(strip_mode));
    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
    inclusive = json_boolean_value(json_object_get(rootJ, "inclusive"));
    strip_mode = json_boolean_value(json_object_get(rootJ, "strip_mode"));
  }
  Room(){
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    onReset();
  }
};

struct RoomPanel : _less::Widget {
  Room* module;
  RoomPanel(){
  }
  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
  }
  void render() override{
    fill(theme.bg);
    rect(0,0,box.size.x, box.size.y);
    fill(theme.fg);

    stroke(theme.fg);
    fill(theme.fg);
    float fs = 24.0f;
    text(("R"), width * 0.5f, RACK_GRID_WIDTH + 6.0f, fs);
    fill(theme.fg);
    stroke(theme.fg);
    text("unless", width * 0.5f, RACK_GRID_HEIGHT - 15.0f, 10.0f);
  }

};
struct RoomDisplay : _less::Widget {
  Room* module;
  _less::Panel *panel;
  int drag_state = 0;
  int pressed = -1;

  float thickness = 4.0f;
  RoomDisplay(_less::Panel *p){
    panel = p;
  }
  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
  }
  void arrow(float x, float y, bool up = true, float w = 10.0f, float l = 8.0f){
    push();
    translate(x, y);
    if(up)
      rotate(3.141592f);
    path(0, - l);
    point(0, l);
    end();
    path(w, l - w);
    point(0, l);
    point(-w, l - w);
    end();
    pop();

  }
  void plus(float x, float y, float r = 10.0f){
    push();
    translate(x, y);
    rotate(3.141592f * 0.25f);
    cross(0, 0, r);
    end();
    pop();

  }
  void cross(float x, float y, float r = 10.0f){
    push();
    translate(x, y);
    path(r, r);
    point(-r, -r);
    move(-r, r);
    point(r, -r);
    end();
    pop();

  }
  void render() override{
    // if(module){
    int l = 5;
    float h = box.size.y / (float) l;
    float r = 7.0f;
    float w = r * 1.8f;
    float p = 4.0f;
    float hh = h - p * 2.0f;
    float ww = width - p * 2.0f;
    stroke(theme.fg);
    stroke_width(1.5f);
    for(int i = 0; i<l; i++){
      float y = i * h - thickness;
      if(i == pressed)
        y += thickness;
      push();
      translate(width * 0.5f, y + h * 0.5f);
      fill(theme.bg_dark);
      stroke(theme.bg_dark);
      center_round_rect(0,thickness * 3.0f - (i == pressed ? thickness : 0),ww, hh, 6.0f, BOTH);
      fill(theme.input);
      stroke(theme.output);
      center_round_rect(0,0,ww, hh, 6.0f, BOTH);
      float dir = h * 0.17f;
      if(i > 2)
        rotate(3.141592f);

      if(i == 2){
        float l = r * 2.1f;
        float rr = r * 0.7f;
        arrow(0,-dir + dir * 1.0f, true, rr, l);
        rotate(3.141592f);
        arrow(0,-dir + dir * 1.0f, true, rr, l);

        rotate(3.141592f * 0.5f);
        arrow(0,-dir + dir * 1.0f, true, rr, l);
        rotate(3.141592f);
        arrow(0, -dir + dir * 1.0f, true, rr, l);
      }else if(i == 0 || i == 4){
        plus(0,dir, r - 1.0f);
        arrow(0,-dir, true, r - 2.0f, 6.0f);
      }else{
        center_rect(0, -dir, w, w, STROKE);
        arrow(0,dir, false, r - 2.0f, 6.0f);
      }
      pop();
    }
  }
};

struct RoomSetting : MenuItem {
  bool *setting = NULL;
  void onAction(const event::Action &e) override {
    *setting = !*setting;
  }
  void step() override {
    rightText = *setting ? "✔" : "";
    MenuItem::step();
  }
};

struct RoomWidget : ModuleWidget {
  bool dragging = false;
  _less::Panel *panelWidget = NULL;
  RoomDisplay *display = NULL;
  Room *moduleInstance = NULL;
  int padding = 42;
  bool inclusive = false;
  RoomWidget(Room *module){
    setModule(module);
    moduleInstance = module;
    box.size = Vec(MODULE_WIDTH * RACK_GRID_WIDTH, 380);

    panelWidget = new _less::Panel(box.size, nvgHSLA(0, 0, 0, 0));
    RoomPanel *panel = new RoomPanel();
    panel->module = module;
    panel->set_box(0.0f,0.0f,box.size.x, box.size.y);

    panelWidget->fb->addChild(panel);
    addChild(panelWidget);
    

    display = new RoomDisplay(panelWidget);
    display->set_box(
      0.0f, (float)padding, 
      box.size.x, box.size.y - 2 * padding);
    display->module = module;
    panelWidget->fb->addChild(display);
  }
  void step() override{
    ModuleWidget::step();
  }
  int modulesInRow(int y, int x){
    int acc = 0;
    for (widget::Widget* w : APP->scene->rack->getModuleContainer()->children) {
      ModuleWidget* moduleWidget = dynamic_cast<ModuleWidget*>(w);
      assert(moduleWidget);
      if(moduleWidget->box.pos.y == y && moduleWidget->box.pos.x != x)
        acc ++;
    }
    return acc;
  }
  void moveModules(int y, bool top, bool inc = false){
    float dir = (top ? -RACK_GRID_HEIGHT : RACK_GRID_HEIGHT);
    for (widget::Widget* w : APP->scene->rack->getModuleContainer()->children) {
      ModuleWidget* moduleWidget = dynamic_cast<ModuleWidget*>(w);
      assert(moduleWidget);
      bool target = top ? moduleWidget->box.pos.y < y : moduleWidget->box.pos.y > y;
      if(inc && !target)
        target = moduleWidget->box.pos.y == y && moduleWidget->module->id != module->id;;
      if(target){
        moduleWidget->box.pos.y += dir;
      }
    }
  }

  int lastY(int y, bool up){
    for (widget::Widget* ww : APP->scene->rack->getModuleContainer()->children) {
      ModuleWidget* mw = dynamic_cast<ModuleWidget*>(ww);
      assert(mw);
      if(up){
        if(mw->box.pos.y < y)
          y = mw->box.pos.y;
      }else{
        if(mw->box.pos.y > y)
          y = mw->box.pos.y;
      }
    }
    return y;
  }
  void rotateModules(int y, bool top){
    int last_y = lastY(y, top);
    float dir = (top ? RACK_GRID_HEIGHT : -RACK_GRID_HEIGHT);
    for (widget::Widget* w : APP->scene->rack->getModuleContainer()->children) {
      ModuleWidget* moduleWidget = dynamic_cast<ModuleWidget*>(w);
      assert(moduleWidget);
      bool target = top ? moduleWidget->box.pos.y < y : moduleWidget->box.pos.y > y;
      if(target){
        if(moduleWidget->box.pos.y + dir == y)
          moduleWidget->box.pos.y = last_y;
        else
          moduleWidget->box.pos.y += dir;
      }
    }
  }
  bool intersectInRow(rack::Rect a, rack::Rect b){
    return (a.pos.x == b.pos.x) 
        || (a.pos.x < b.pos.x && a.size.x > b.pos.x - a.pos.x)
        || (a.pos.x > b.pos.x && b.size.x > a.pos.x - b.pos.x);
  }

  void switchRows(int ay, int by){
    for (widget::Widget* w : APP->scene->rack->getModuleContainer()->children) {
      ModuleWidget* moduleWidget = dynamic_cast<ModuleWidget*>(w);
      assert(moduleWidget);
      bool a = moduleWidget->box.pos.y == ay;
      bool b = moduleWidget->box.pos.y == by;
      if(a)
        moduleWidget->box.pos.y = by;
      else if(b)
        moduleWidget->box.pos.y = ay;
    }
  }

  void offsetRow(int y, int x){
    for (widget::Widget* w : APP->scene->rack->getModuleContainer()->children) {
      ModuleWidget* moduleWidget = dynamic_cast<ModuleWidget*>(w);
      assert(moduleWidget);
      bool target = moduleWidget->box.pos.y == y && moduleWidget->module->id != module->id;
      if(target)
        moduleWidget->box.pos.x += x;
    } 
  }

  void moveRow(bool up, int y){
    moveModules(y, !up);
    switchRows(box.pos.y - RACK_GRID_HEIGHT, box.pos.y + RACK_GRID_HEIGHT);
    rotateModules(box.pos.y, up);
  }

  int dragStartY = 0.0f;
  int dragStartX = 0.0f;

  struct comp{
  // template<typename T>
  bool operator()(ModuleWidget *a, ModuleWidget *b)const{
    return a->box.pos.x < b->box.pos.x;
  }
  };

  std::vector<ModuleWidget*> strip = {};

  void setStrip(){
    strip.clear();
    strip.push_back(this);
    // bool right_mode = true;
    std::vector<ModuleWidget *> mws = {};
    for (widget::Widget* w : APP->scene->rack->getModuleContainer()->children) {
      ModuleWidget* mw = dynamic_cast<ModuleWidget*>(w);
      assert(mw);
      if(mw->box.pos.y == box.pos.y && mw->box.pos.x != box.pos.x){
        if((mw->box.pos.x < box.pos.x)){
          mws.push_back(mw);
        }
      }
    }
    if(mws.size() > 0){
      std::sort(mws.begin(), mws.end(), comp());
      float last = box.pos.x;
      for(int i = mws.size() - 1; i >= 0; i--){
        if(mws.at(i)->box.pos.x + mws.at(i)->box.size.x == last){
          strip.push_back(mws.at(i));
          last -= mws.at(i)->box.size.x;
        }else break;
      }
    }
    mws.clear();

    for (widget::Widget* w : APP->scene->rack->getModuleContainer()->children) {
      ModuleWidget* mw = dynamic_cast<ModuleWidget*>(w);
      assert(mw);
      if(mw->box.pos.y == box.pos.y){
        if(mw->box.pos.x > box.pos.x){
          mws.push_back(mw);
        }
      }
    }
    if(mws.size() > 0){
      std::sort(mws.begin(), mws.end(), comp());
      float last = box.pos.x + box.size.x;
      for(auto mw : mws){
        if(mw->box.pos.x == last){
          strip.push_back(mw);
          last += mw->box.size.x;
        }else break;
      }
    }

    std::sort(strip.begin(), strip.end(), comp());
  }
  bool stripHasSpace(int y){
    if(strip.size() > 0){
      float start = strip.at(0)->box.pos.x;
      float end = strip.at(strip.size() - 1)->box.pos.x + strip.at(strip.size() - 1)->box.size.x;
      for (widget::Widget* w : APP->scene->rack->getModuleContainer()->children) {
        ModuleWidget* mw = dynamic_cast<ModuleWidget*>(w);
        assert(mw);
        if(mw->box.pos.y == y && (mw->box.pos.x + mw->box.size.x > start) && mw->box.pos.x < end){
          return false;
        }
      }
      return true;
    }
    return false;
  }
  void offsetStrip(float x){
    int i = strip.size();
    if(i > 0){
      for(auto mw : strip){
        i --;
        mw->box.pos.x += x;
      }
    }
  }

  void moveStrip(int y){
    box.pos.y = y;
    for(auto mw : strip){
      mw->box.pos.y = y;
    }
  }
  void onDragStart(const event::DragStart &e) override{
    dragStartY = box.pos.y;
    dragStartX = box.pos.x;
    ModuleWidget::onDragStart(e);
  }
  void onDragMove(const event::DragMove &e)override{
    int y = box.pos.y;
    Vec dragPos = Vec(box.size.x * 0.5f,box.size.y * 0.5f);
    if(dragging){
      if(moduleInstance->strip_mode || (APP->window->getMods() & RACK_MOD_MASK) == RACK_MOD_CTRL){
        int _x = floor((((int) APP->scene->rack->getMousePos().minus(dragPos).x) / (int) RACK_GRID_WIDTH) * RACK_GRID_WIDTH);
        int x = floor(_x - dragStartX);
        if(_x != dragStartX){
          offsetStrip(x);
          dragStartX = _x;
          int i = x < 0 ? 0 : strip.size() - 1;
          APP->scene->rack->setModulePosForce(strip.at(i), strip.at(i)->box.pos);
        }
        Vec v = APP->scene->rack->getMousePos().minus(dragPos);
        float nt = (dragStartY + (RACK_GRID_HEIGHT / 2)) - v.y;
        y = dragStartY + RACK_GRID_HEIGHT * (nt < 0 ? -floor(nt / RACK_GRID_HEIGHT) : (nt > RACK_GRID_HEIGHT ? - floor(nt / RACK_GRID_HEIGHT) : 0));
        if(y != dragStartY && stripHasSpace(y)){
          moveStrip(y);
          dragStartY = y;
        }
      }else{
        if(y == dragStartY){
          if (e.button != GLFW_MOUSE_BUTTON_LEFT)
            return;
          box.pos = APP->scene->rack->getMousePos().minus(dragPos);
          box.pos = Vec(floor((((int) box.pos.x) / (int) RACK_GRID_WIDTH) * RACK_GRID_WIDTH), y);
          int x = box.pos.x; 
          if(x != dragStartX){
            offsetRow(y, x - dragStartX);
            dragStartX = x;
          }
        }

        float x = box.pos.x;
        box.pos = APP->scene->rack->getMousePos().minus(dragPos);
        float nt = (dragStartY + (RACK_GRID_HEIGHT / 2)) - box.pos.y;
        box.pos.y = dragStartY + RACK_GRID_HEIGHT * (nt < 0 ? 1 : (nt > RACK_GRID_HEIGHT ? -1 : 0));
        box.pos.x = x;
        y = box.pos.y;
        if(y != dragStartY){
          offsetRow(dragStartY, x - dragStartX);
          switchRows(dragStartY, y);
          dragStartY = y;
        }
      }
    }else{
      ModuleWidget::onDragMove(e);
    }
  }

  void onDragEnd(const event::DragEnd &e) override{
    ModuleWidget::onDragEnd(e);
    dragging = false;
    display->pressed = -1;
    panelWidget->fb->dirty = true;
  }
  void onButton(const event::Button &e) override{
    if(e.pos.y > padding && e.pos.y < RACK_GRID_HEIGHT - padding && e.button == 0 && e.action == GLFW_PRESS){
      int i = floor(rescale(e.pos.y, padding, RACK_GRID_HEIGHT - padding, 0.0f, 1.0f) * 5.0f);
      display->pressed = i;

      APP->scene->rack->updateModuleOldPositions();

      if(i == 2){
        dragging = true;
        setStrip();
        ModuleWidget::onButton(e);
      }else if(i == 0 || i == 4){
        if(!moduleInstance->inclusive || modulesInRow(box.pos.y, box.pos.x) > 0)
        moveModules(box.pos.y, i == 0, moduleInstance->inclusive);
      }else{
        rotateModules(box.pos.y, i == 1);
      }

      history::ComplexAction* h = APP->scene->rack->getModuleDragAction();
      if (!h)
        delete h;
      else
        APP->history->push(h);

      panelWidget->fb->dirty = true;
    }else{
      if(e.action == GLFW_RELEASE){
        display->pressed = -1;
        panelWidget->fb->dirty = true;
      }

    }
    ModuleWidget::onButton(e);
    e.consume(this);
  }
  std::vector<std::string> lines = {
    "drag the central button to move the whole row around",
    "the top-bottom buttons insert empty rows above or below",
    "the inbetween buttons cycle all rows above or below",
  };
  void appendContextMenu(Menu *menu) override {
    Room *view = dynamic_cast<Room*>(module);
    assert(view);
    menu->addChild(construct<MenuLabel>());
    menu->addChild(new _less::HelpItem(&lines));

    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<RoomSetting>(&MenuItem::text, "add new rows here instead of upper/lower", &RoomSetting::setting, &view->inclusive));
    menu->addChild(construct<RoomSetting>(&MenuItem::text, "strip mode (default by holding ctrl/cmd while moving)", &RoomSetting::setting, &view->strip_mode));
  }
};


Model *modelRoom = createModel<Room, RoomWidget>("room");
/*
  TODO
*/
