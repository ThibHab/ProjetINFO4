#include "unless.hpp"
#include "widgets.hpp"
#include "dsp/digital.hpp"
#include "utils.hpp"
#include <algorithm>
#include "../art/components.hpp"
#include "cantree.hpp"

#define MODULE_WIDTH 25
#define SIZE 5

struct Cantor : Module {
  
  enum ParamIds {
    GATE_SELECT_PARAM,
    MODE_PARAM,
    ENUMS(CUT_PARAMS, SIZE),
    ENUMS(GATE_PARAMS, 2 + 4 + 8 + 16 + 32),
    NUM_PARAMS
  };
  enum InputIds {
    ENUMS(PHASE_INPUT, SIZE),
    ENUMS(RATIO_INPUT, SIZE),
    ENUMS(GATE_INPUTS, SIZE + 1),
    NUM_INPUTS
  };
  enum OutputIds {
    ENUMS(GATE_OUTPUT, SIZE),
    ENUMS(PHASE_OUTPUT, SIZE),
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  bool dirty = true;
  
  bool animate = false;

  int buffered_mode = 0;


  std::vector<float> cut_offsets = std::vector<float>(SIZE);
  std::vector<float> cut_knobs = std::vector<float>(SIZE);
  float times[SIZE][16];
  bool offsetter = true;
  trees::cantree *tree;
  std::vector<float> cuts = std::vector<float>(SIZE + 1);

  std::string row_names[SIZE] = {"I", "II", "III", "IV", "V"};

  const uint8_t level_offsets[SIZE + 1] = {0, 2, 4, 8, 16, 32};
  bool leftMessages[2][62] = {};

  float color;

  SampleDivider divider = SampleDivider(64);

  int cell_id(int row, int index){
    int i = 0;
    for(int r = 0; r<row; r++)
      i += level_offsets[r];
    return i + index;
  }
  void set_dirty(){
    dirty = true;
  }

  int s = 0;

  float cut(int i){
    return math::clamp(cut_knobs[i] + cut_offsets[i], 0.0f, 1.0f);
  }

  float float_wrap(float x){
    if(x >= 0)
      return fmod(x, 1.0f);
    else
      return 1.0f - fmod(abs(x), 1.0f);
  }

  float phase_input_time(float v){
    return rescale((v < 0.0f ? 10.0f + v : v), 0, 10.0f, 0, 1.0f);
  }

  void output_tree(){
    for(int i = 0; i<SIZE; i++){
      int c = 1;
      if(inputs[PHASE_INPUT + i].isConnected()){
        c = inputs[PHASE_INPUT + i].getChannels();
        for(int j = 0; j<c; j++)
          times[i][j] = phase_input_time(inputs[PHASE_INPUT + i].getVoltage(j));
      }
      else{
        times[i][0] = times[i == 0 ? SIZE - 1 : i - 1][0];
      }
      cut_offsets[i] = inputs[RATIO_INPUT + i].isConnected() ? inputs[RATIO_INPUT + i].value * 0.1f : 0.0f;
      cuts[i] = cut(i);

      if(outputs[GATE_OUTPUT + i].isConnected() || outputs[PHASE_OUTPUT + i].isConnected()){
        outputs[PHASE_OUTPUT + i].setChannels(c);
        outputs[GATE_OUTPUT + i].setChannels(c);
        for(int j = 0; j<c; j++){
          bool g = tree->is_gate(i, times[i][j], params[MODE_PARAM].value == 0);
          outputs[PHASE_OUTPUT + i].setVoltage(g ? tree->time_of(i, tree->root, times[i][j]) * 10.0f : 0.0f, j);
          outputs[GATE_OUTPUT + i].setVoltage(g ? 10.0f : 0.0f, j);
        }
      }
    }

  }

  void update_gate(int level, int index, bool b){
    if(tree){
      tree->set_gate_at(level, index, b);
    }
  }

  void fill_row(int level, float v){
    for(int i = 0; i<level_offsets[level + 1]; i++){
      params[GATE_PARAMS + cell_id(level + 1, i)].setValue(v);
    }
  }
  bool get_input(int level, int index){
    if(level == 4 && index >= 16){
      index -= 16;
      level += 1;
    }
    return inputs[GATE_INPUTS + level].getVoltage(index) > 1.0f;
  }
  void update_gates(int i){
    for(int g = 0; g<level_offsets[i + 1]; g++){
      update_gate(i, g, params[GATE_PARAMS + cell_id(i + 1, g)].value > 0);
    }
  }
  void update_rows(){
    for(int i = 0; i<SIZE; i++){
      float c = params[CUT_PARAMS + i].value;
      if(cut_knobs[i] != c){
        cut_knobs[i] = c;
        set_dirty();
      }
      update_gates(i);
    }
  }
  bool get_gate_param(int level, int index){
    return params[GATE_PARAMS + level_offsets[level] + index].value > 0;
  }
  void process(const ProcessArgs &args) override {
    if(divider.step()){
      if(buffered_mode != (int) params[MODE_PARAM].value){
        buffered_mode = (int) params[MODE_PARAM].value;
        set_dirty();
      }
      update_rows();
      output_tree();
    }
  }
  void offset_color(){
    color = float_wrap(color + 0.112f);
    set_dirty();
  }

  json_t *dataToJson() override {
    json_t *rootJ = json_object();
    json_t *cutsJ = json_array();
    for (int i = 0; i<SIZE; i++) {
      json_array_append(cutsJ, json_real(cut_knobs[i]));
    }
    json_t *gatesJ = tree->toJson();
    json_object_set(rootJ, "animate", json_boolean(animate));
    json_object_set(rootJ, "gates", gatesJ);
    json_object_set(rootJ, "mode", json_integer(params[MODE_PARAM].value));
    json_object_set(rootJ, "cuts", cutsJ);
    json_object_set(rootJ, "color", json_real(color));
    json_object_set(rootJ, "divider", divider.save());
    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
    tree->fromJson(json_object_get(rootJ, "gates"));
    json_t *cutsJ = json_object_get(rootJ, "cuts");
    animate = json_boolean_value(json_object_get(rootJ, "animate"));
    for (int i = 0; i<SIZE; i++) {
      cut_knobs[i] = json_number_value(json_array_get(cutsJ, i));
    }

    color = json_number_value(json_object_get(rootJ, "color"));
    divider.load(json_object_get(rootJ, "divider"));
  }
  void onReset() override{
    cuts.clear();
    for(int i = 0; i<SIZE + 1; i++){
      cuts.push_back(0.5f);
    }
    for(int i = 0; i<SIZE; i++){
      cut_knobs[i] = 0.5f;
      cut_offsets[i] = 0.0f;
      // printf("%d\n", i);
      tree->rec_set_gate(i, tree->root, true);
    }
    set_dirty();
    tree->set_gate_at(0, 0, true);
    divider.reset();
  }
  Cantor(){
    tree = new trees::cantree(SIZE + 1, &cuts, &cut_knobs);


    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configSwitch(GATE_SELECT_PARAM, 0, 1, 0, "view", {"ratios", "gates"});
    configParam(MODE_PARAM, 0, 1, 0, "relative mode");
    for(int i = 0; i<SIZE; i++){
      for(int g = 0; g<level_offsets[i + 1]; g++){
        int gate_index = cell_id(i + 1, g);
        configParam(GATE_PARAMS + gate_index, 0, 1, 1, row_names[i] + "/" + to_string(g + 1) + " gate");
      }

      configParam(CUT_PARAMS + i, 0.0f, 1.0f, 0.5f, row_names[i] + " ratio");
      configInput(PHASE_INPUT + i, row_names[i] + " phase");
      configInput(RATIO_INPUT + i, row_names[i] + " ratio");
      configOutput(PHASE_OUTPUT + i, row_names[i] + " phase");
      configOutput(GATE_OUTPUT + i, row_names[i] + " gate");
    }


    color = random::uniform();
    onReset();

    leftExpander.producerMessage = leftMessages[0];
    leftExpander.consumerMessage = leftMessages[1];
  }
  ~Cantor() override{
    delete tree;
  }
};

struct CantorPanel : _less::Widget {
  Cantor* module;
  float drag_value = 0.0f;
  bool started_drag = false;
  bool gate_select = false;
  CantorPanel(){
  }
  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
    float w = 15.0f * 4.0f;
    float h = 30.0f;
    color_button = Rect(box.size.x * 0.5f - w, 0, w, h);
  }
  Rect color_button;
  void onButton(const event::Button &e) override{
    if(e.action == GLFW_PRESS && e.button == 0){
      if(color_button.isContaining(e.pos)){
        module->offset_color();
      }
    }
  }
  void switch_view(bool g){
    gate_select = g;
    // render();
  }
  void render() override{
    
    fill(theme.bg);
    rect(0,0,width, height);

    fill(theme.fg);
    text("unless táncor", box.size.x * 0.5f, RACK_GRID_WIDTH, 16.0f);
  }

};
template <class TLessWidget>
struct CellButton : Switch {
  bool doubled = false;
  TLessWidget *sw;
  CellButton(math::Vec pos, Cantor *m, int paramId, float w = 24.0f, bool mom = true, float h = -1.0f){
    if(h < 0.0f)
      h = w;
    momentary = mom;


    Switch::module = m;
    Switch::paramId = paramId;
    Switch::initParamQuantity();
    
    // if(m){
    //   paramQuantity = m->paramQuantities[paramId];
    // }


    sw = new TLessWidget;
    addChild(sw);

    box.pos = pos;
    box.size = Vec(w,h);
    box.pos = box.pos.minus(Vec(w * 0.5f, h * 0.5f));

    sw->box.size = Vec(w, h);

  }
  void onDoubleClick(const event::DoubleClick& e) override{
    e.consume(this);
    e.stopPropagating();
    start_drag(1.0f - getParamQuantity()->getScaledValue());
    sw->module->fill_row(sw->level, sw->panel->drag_value);
  }
  void onChange(const event::Change& e) override{
    sw->state = floor(getParamQuantity()->getValue());
    Switch::onChange(e);
  }
  void onDragStart(const event::DragStart& e) override{
    if(e.button == GLFW_MOUSE_BUTTON_LEFT)
      sw->panel->started_drag = true;
  }

  void onDragHover(const event::DragHover &e) override{
    if(sw->panel->started_drag){
      getParamQuantity()->setValue(sw->panel->drag_value);
      // sw->module->set_cells_until(id); TODO
      sw->module->set_dirty();
    }
  }

  void onDragEnd(const event::DragEnd &e) override{
    sw->panel->started_drag = false;
  }

  void start_drag(float v){
    sw->panel->drag_value = v;
    getParamQuantity()->setValue(sw->panel->drag_value);
    sw->panel->started_drag = true;
    sw->module->set_dirty();
  }

  void onButton(const event::Button &e) override{
    if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS){
      e.consume(this);
      e.stopPropagating();
      start_drag(1.0f - getParamQuantity()->getScaledValue());
    }else{
      Switch::onButton(e);
    }
  }
};


struct DrawCellButton : _less::Widget{
  int state = 0;
  int last_state = 0;
  bool hover_set = false;
  CellButton<DrawCellButton> *button = NULL;
  CantorPanel *panel;
  int level = -1;
  Cantor *module;
  bool doubled = false;
  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
  }
  void onButton(const event::Button& e) override {
  }
  void step() override{
    if(last_state != state){
      last_state = state;
      module->set_dirty();
    }
  }
};




struct CantorDisplay : _less::Widget {
  Cantor* module;
  trees::cantree *tree;
  std::vector<float> *cuts;
  float *color;
  float default_color = 0.1f;
  CantorPanel *panel;
  int drag_state = 0;
  bool drawing = false;
  Vec mouse = Vec(0,0);
  bool doubled = false;
  bool gate_state = false;

  std::vector<std::vector<CellButton<DrawCellButton>*>> *cells = NULL;

  CantorDisplay(CantorPanel *p, std::vector<std::vector<CellButton<DrawCellButton>*>> *cs){
    cells = cs;
    panel = p;
  }

  int pickRow(float y){
    return floor((y / box.size.y ) * (float) SIZE);
  }
  float ratio(float x){
    return x / box.size.x;
  }
  void init(NVGcontext *vg) override{
    // printf("initing cantor\n");
    load_font("font/Terminus.ttf");
    if(module){
      color = &module->color;
      cuts = &module->cut_knobs;
      tree = module->tree;
    }else{
      default_color = random::uniform();
      color = &default_color;
      cuts = new std::vector<float>(SIZE + 1);
      for(int i = 0; i<SIZE + 1; i++)
        cuts->at(i) = 0.5f + 0.3f * (random::uniform() - 0.5f);
      tree = new trees::cantree(SIZE + 1, cuts, cuts);
    }
  }

  void onHover(const event::Hover &e) override{
    e.consume(this);
    mouse.x = e.pos.x;
    mouse.y = e.pos.y;
  }

  void onDragLeave(const event::DragLeave &e) override{
    e.consume(this);
    drawing = false;
  }

  int cell_offsets[SIZE] = {0, 2, 4, 8, 16};



  void draw_gate(trees::binode *n, float s, float e, float pw, int i, float h){
    bool rel_g = n->is_gate_upstream();

    if(!module || module->params[Cantor::MODE_PARAM].value == 0){
      rel_g = true;
    }
    float l = e - s;
    float lc = l * pw;

    if(n->gate){
      if(rel_g){
        fill(theme.input);
      }else
        fill(transp(theme.input, 0.6f));

    }
    else
      fill(theme.transparent);
    stroke(theme.bg_dark);

    if(panel->gate_select)
      rect(s,i * h, lc, h, BOTH);
    else
      rect(s,i * h, lc, h, FILL);

    int index = n->get_id(0,0);
    if(cells){
      // printf("settings gate button level = %d, index = %d\n", n->level, index);
      CellButton<DrawCellButton> *c = cells->at(i).at(index);
      float w = lc;
      c->box.pos = Vec(s + box.pos.x, i * h + box.pos.y);
      c->box.size = Vec(l, h);
      // c->fb->box.pos = Vec(0, 0);
      // c->fb->box.size = Vec(l, h);
      c->sw->box.pos = Vec(0, 0);
      c->sw->box.size = Vec(w, h);
      
      // c->fb->dirty = true;
    }
  }

  void render_nodes(int depth, trees::binode *node, float s, float e, float h, int i = 0, float pw = 0.5f, PathMode m = BOTH){
    float c = panel->gate_select || (module && !module->animate) ? *node->raw_value : *node->value ;
    float l = (e - s);
    if(node->level == depth){
      bool left_gate = node->left->gate;
      bool right_gate = node->right->gate;

      if(panel->gate_select){
        draw_gate(node->left, s, s + c * l, 1.0f, depth, h);
        draw_gate(node->right, s + c * l, e, 1.0f, depth, h);
      }else{
        if(left_gate) draw_gate(node->left, s, s + c * l, pw, depth, h);
        if(right_gate) draw_gate(node->right, s + c * l, e, pw, depth, h);
      }
    }else{
      render_nodes(depth, node->left, s, s + c * l, h, i + 1, pw, m);
      render_nodes(depth, node->right, s + c * l, e, h, i + 1, pw, m);
    }
  }




  void render() override{
    float h = box.size.y / (float) SIZE;
    push();
    scissor();
    for(int i = 0; i<SIZE; i++){
      // float t = (float) i / (float) SIZE;
      fill(nvgLerpRGBA(theme.rainbow_color(fmod(*color + (i % 2 == 0 ? 0.03f : 0.0f), 1.0f)), theme.bg_dark, 0.7f));
      rect(0,i * h,width, h);
    }
    for(int i = 0; i<SIZE; i++){
      if(panel->gate_select){
        render_nodes(i, tree->root, 0.0f, width, h, 0, 1.0f, BOTH);
      }else{
        render_nodes(i, tree->root, 0.0f, width, h, 0, 0.5f, FILL);
      }
    }

    if(module && module->animate){
      module->set_dirty();
    }
    pop();
  }
};

struct CantorPlayheads : _less::Widget{
  Cantor *module;
  float deadw = 4.2f;
  float sw = 0.5f;

  float slider_out(float w, float x, float pad = 7.0f){
    return pad + x * (w - pad * 2.0f);
  }

  void drawPlayheads(float h){
    stroke_width(4.0f);
    for(int i = 0; i<SIZE; i++){   
      stroke(transp(theme.white, 0.3f));
      // if(module->outputs[Cantor::GATE_OUTPUT + i].getVoltage() > 1.0f){
      //   stroke(transp(theme.rainbow_color(fmod(module->color + coffs, 1.0f)), 0.8f));
      // }else{
      //   stroke(transp(theme.rainbow_color(fmod(module->color + coffs, 1.0f)), 0.3f));
      // }

      int c = module->inputs[Cantor::PHASE_INPUT + i].getChannels();
      if(c == 0)
        c = 1;
      for(int j = 0; j<c; j++){
        float x = box.size.x * module->times[i][j];
        line(x, i * h + sw, x, (i + 1) * h - sw);
      }
    }
  }
  void draw_mods(){
    if(module){
      stroke_width(4.0f);
      float h = height / (float) SIZE;
      stroke(transp(theme.output_dark, 0.3f));
      for(int i = 0; i<SIZE; i++){
        float x = slider_out(width, clamp(module->cut_knobs.at(i) + module->cut_offsets.at(i), 0.0f, 1.0f), deadw);
        line(x, i * h + sw, x, (i + 1) * h - sw);
      }
    }
  }

  void render_lights() override{
    if(module){
      if(!module->animate){
        draw_mods();
      }

      float h = box.size.y / (float)SIZE;
      drawPlayheads(h);
    }
    // stroke_width(4.0f);
    // stroke(theme.output);
    // rect(0,0,width, height + 2.0f, STROKE);
  }
};

struct CantorSetting : MenuItem {
  bool *setting = NULL;
  Cantor *module = NULL;
  void onAction(const event::Action &e) override {
    *setting = !*setting;
    module->set_dirty();
  }
  void step() override {
    rightText = *setting ? "✔" : "";
    MenuItem::step();
  }
};

struct DrawSlider : _less::Widget {
  float scaled_value = 0.0f;
  float *modded_value = NULL;

  bool *bipolar = NULL;
  float color_pos = 0.0f;
  ParamQuantity *paramQuantity = NULL;
  float deadw = 4.2f;


  float sw = 4.0f;

  float slider_out(float w, float x, float pad = 7.0f){
    return pad + x * (w - pad * 2.0f);
  }
  void render() override{
    scissor();
    stroke_width(sw);
    stroke(theme.fg);
    rect(0,0,width, height, STROKE);

    if(paramQuantity){
      fill(theme.fg);
      float x = slider_out(width, clamp(paramQuantity->getScaledValue(), 0.0f, 1.0f), deadw);
      center_rect(x,height * 0.5f, deadw * 2 - 2, height - 2, FILL);
    }
  }
};


struct GateModeButton : _less::Widget{
  int state = 0;
  CantorPanel *panel = NULL;
  ui::Menu* menu = NULL;
  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
  }
  void onButton(const event::Button &e) override{
    if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS){
      panel->switch_view(!panel->gate_select);
      e.consume(this);
    }
  }
  void onDragStart(const event::DragStart& e) override{
    box.pos.y += 1.0f;
  }
  void onDragEnd(const event::DragEnd& e) override{
    box.pos.y -= 1.0f;
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
    if(panel && panel->gate_select){
      text("|", center.x + 0.5f, center.y, height * 0.6f);
    }else{
      text("[]", center.x + 0.5f, center.y, height * 0.6f);
    }
  }
};



struct ModeButton : _less::Widget{
  int state = 0;
  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
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
    if(state == 1){
      text("r", center.x + 0.5f, center.y, height * 0.6f);
    }else{
      text("a", center.x + 0.5f, center.y, height * 0.6f);
    }
  }
};


struct CantorWidget : ModuleWidget {
  int cell_offsets[SIZE + 1] = {0, 2, 4, 8, 16, 32};
  int cell_id(int row, int index){
    int i = 0;
    for(int r = 0; r<row; r++)
      i += cell_offsets[r];
    return i + index;
  }

  _less::Panel *panelWidget = NULL;
  CantorPanel *panel = NULL;
  Cantor *moduleInstance = NULL;
  _less::Slider<DrawSlider> *sliders[SIZE];
  std::vector<std::vector<CellButton<DrawCellButton>*>> cells;
  CantorDisplay *display = NULL;

  bool gate_select = false;

  void set_gate_mode(bool g){
    panel->switch_view(g);
    show_sliders(g);
    moduleInstance->set_dirty();
  }
  void show_sliders(bool g){
    for(int i = 0; i<SIZE; i++)
      sliders[i]->visible = !g;
  }


  CantorWidget(Cantor *module){

    setModule(module);
    moduleInstance = module;
    box.size = Vec(MODULE_WIDTH * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    addChild(createWidget<PeaceScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<PeaceScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


    panelWidget = new _less::Panel(box.size, nvgHSLA(0, 0, 0, 0));
    panel = new CantorPanel();
    panel->module = module;
    panel->set_box(0.0f,0.0f,box.size.x, box.size.y);

    panelWidget->fb->addChild(panel);
    addChild(panelWidget);
    

    float x = RACK_GRID_WIDTH + 10;

    float headerh = (RACK_GRID_HEIGHT - (RACK_GRID_HEIGHT / 6) * 5.0f ) / 2.0f;
    float y = headerh - 5.0f;
    float h = RACK_GRID_HEIGHT / 6;

    float w = box.size.x - (2 * (x + 20));

    float bw = 45.0f;
    float bh = 32.0f;
    {
      GateModeButton * wb = new GateModeButton();
      // wb->set_box(RACK_GRID_WIDTH * 3.0f, y - 32.0f - bh * 0.5f, bw, bh);
      wb->set_box_center(box.size.x * 0.5f, box.size.y - bh * 0.5f - 3.0f, bw, bh);
      // wb->set_box(box.size.x - RACK_GRID_WIDTH * 2.7f, y - 32.0f - bh * 0.5f, bw, bh);
      wb->panel = panel;
      addChild(wb);
    }

    // {
    //   _less::Button<ModeButton> * wb = new _less::Button<ModeButton>(
    //       Vec(box.size.x - 30.0f - 30.0f, y - 32.0f),
    //       module,
    //       Cantor::MODE_PARAM,
    //       20.0f, false
    //   );
    //   addParam(wb);
    // }

    display = new CantorDisplay(panel, &cells);
    std::vector<float> *vs; 

    if(module){
      vs = &module->cut_knobs;
    }else{
      vs = new std::vector<float>(SIZE);
      for(int i = 0; i<SIZE; i++)
        vs->at(i) = random::uniform();
    }

    display->set_box(
      x + 20, y, 
      w, box.size.y - headerh * 2.0f);

    display->module = module;
    panelWidget->fb->addChild(display);
    for(int i = 0; i<SIZE; i++){
      std::vector<CellButton<DrawCellButton>*> ls;
      for(int c = 0; c < cell_offsets[i + 1]; c++){
        int gate_index = cell_id(i + 1, c);
        float cx = (float) c / (float) cell_offsets[i + 1];

        CellButton<DrawCellButton> *cell = new CellButton<DrawCellButton>(
          Vec(cx * w, y + h * i - 15.0f),
          module, 
          Cantor::GATE_PARAMS + gate_index, 
          20.0f, false
        );
        cell->sw->button = cell;
        cell->sw->panel = panel;
        cell->sw->module = module;
        cell->sw->level = i;
        ls.push_back(cell);
        addParam(cell);
      }
      cells.push_back(ls);
    }

    CantorPlayheads *ph =  new CantorPlayheads();
    ph->set_box(
      x + 20, y, 
      w, box.size.y - headerh * 2.0f);
    ph->module = module;
    addChild(ph);

    y += 15.0f;

    for(int i = 0; i<SIZE; i++){
      
      addInput(
        new _less::Port<unless::CvInput>(
          Vec(x, y + h * i),
          module, 
          Cantor::PHASE_INPUT + i
      ));




      sliders[i] = new _less::Slider<DrawSlider>(
        Vec(x + 20.0f, y + h * i - 15.0f), Vec(w,h),
        module, 
        Cantor::CUT_PARAMS + i, 
        false, 
        false,
        false
      );
      addParam(sliders[i]);


      addInput(
        new _less::Port<unless::RingHoleInput>(
          Vec(x, y + 30 + h * i),
          module, 
          Cantor::RATIO_INPUT + i
      ));

      addOutput(
        new _less::Port<unless::RingHoleOutput>(
          Vec(RACK_GRID_WIDTH * MODULE_WIDTH - x, y + h * i),
          module, 
          Cantor::PHASE_OUTPUT + i
      ));


      addOutput(
        new _less::Port<unless::GateOutput>(
          Vec(RACK_GRID_WIDTH * MODULE_WIDTH - x, y + h * i + 30),
          module, 
          Cantor::GATE_OUTPUT + i
      ));
    }

  }
  void step() override{
    ModuleWidget::step();
    if(moduleInstance){
      if(moduleInstance->dirty){
        panelWidget->set_dirty();
        display->dirty = true;
        moduleInstance->dirty = false;
      }

      if(gate_select != panel->gate_select){
        gate_select = panel->gate_select;
        set_gate_mode(gate_select);
      }
    }
  }
  std::vector<std::string> lines = {
    "connect a phase signal into any of the primary inputs",
    "they are on the left side",
    "a phase signal could be from a rising sawtooth LFO",
    "inputs are cascading downwards",
    "cv and gate outputs on the right",
    "round cv ports output the phase of the gate",
  };
  void appendContextMenu(Menu *menu) override {
    Cantor *view = dynamic_cast<Cantor*>(module);
    assert(view);
    add_color_slider(menu, moduleInstance);
    menu->addChild(construct<MenuLabel>());
    menu->addChild(new _less::HelpItem(&lines));
    menu->addChild(construct<SampleDividerMenuItem>(&SampleDividerMenuItem::divider, &moduleInstance->divider));
    menu->addChild(construct<CantorSetting>(&MenuItem::text, "animate mods", &CantorSetting::setting, &view->animate, &CantorSetting::module, view));
  }
};

Model *modelCantor = createModel<Cantor, CantorWidget>("cantor");
/*
  todo

  set pulsewidth
  enable relative mode

*/
