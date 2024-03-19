#include "unless.hpp"
#include "widgets.hpp"
#include "dsp/digital.hpp"
#include "utils.hpp"
#include <algorithm>
#include "../art/components.hpp"
#include "../art/chainkov.hpp"
#define CLASSIC_SKIN 0
#define MINIMAL_SKIN 1

struct Chainkov : Module {
  struct Edge {
    int note = 0;
    int count = 1;
    Edge(int n, int c){
      note = n;
      count = c;
    }
  };
  struct Node {
    int note = 0;
    int count = 1;
    int last = -1;
    std::vector<Edge> edges;
    Node(int n, int c){
      note = n;
      count = c;
    }
    int findEdge(int n){
      for(int i = 0; i<(int)edges.size(); i++){ 
        if(edges.at(i).note == n)
          return i;
      }
      return -1;
    }
    static bool sortByCount(const Edge &a, const Edge &b){
      return a.count > b.count;
    }
    void addEdge(int n, int c){
      int i = findEdge(n);
      if(i < 0 || i >= (int)edges.size())
        edges.push_back(Edge(n, c));
      else
        edges.at(i).count += c;
      std::sort(edges.begin(), edges.end(), sortByCount);
    }
    // void reverseWeights(){
    // }
    int getNext(float randomness){
      float all = 0;
      float mult = randomness * -1.0f;
      if(randomness > 0)
        mult *= 0.9f;
      if(randomness < -0.9f && edges.size() > 0)
        return edges.at(0).note;
      for(auto& e : edges)
        all+= (float)e.count + ((float)e.count * mult);
      float r = random::uniform() * all;
      float acc = 0;
      for(int i = 0; i<(int)edges.size(); i++){
        acc += (float)edges.at(i).count + ((float)edges.at(i).count * mult);
        if(r <= acc)
          return edges.at(i).note;
      }

      return -1;
    }
  };

  struct ChainkovChain{
    std::vector<Node> nodes;
    bool hasNodes = false;
    int current = -1;

    void clear(){
      nodes.clear();
      current = -1;
      hasNodes = false;
    }
    int findNode(int n){
      for(int i = 0; i<(int) nodes.size();i++){
        if(nodes.at(i).note == n)
          return i;
      }
      return -1;
    }
    void addNode(int n, int c){
      hasNodes = true;
      nodes.push_back(Node(n, c));
    }
    void setNote(int n){
      if(hasNodes){  
        int closest = 1000;
        for(int i = 0; i<(int) nodes.size();i++){
          int nt = nodes.at(i).note;
          if(nt == n){
            closest = nt;
            break;
          }else if(abs(nt - n) < abs(closest - n)){
            closest = nt;
          }
        }
        closest = findNode(closest);
        if(closest < (int)nodes.size() && closest >= 0)
          current = closest;
        // printf("%d\n", closest);
      }
    }
    void forget(){
      if(nodes.size() > 0){
        int n = nodes.at(current).note;
        nodes.erase(nodes.begin() + current);
        for (auto& node : nodes){
          if(node.edges.size() > 0){ 
            for(int i = (int)node.edges.size() - 1; i>= 0; i--)
              if(node.edges.at(i).note == n){
                node.count-= node.edges.at(i).count;
                node.edges.erase(node.edges.begin() + i);
              }
          }
        }
        if(nodes.size() == 0){
          hasNodes = false;
          current = -1;
        }else{
          setNote(current);
        }
      }
    }
    void add(int n, float randomness){
      int i = findNode(n);
      int c = randomness > 0.0f ? 1 + (int)floor(random::uniform() * 8.0f) : 1;
      if(i == -1)
        addNode(n, c);
      if(current > -1)
        nodes.at(current).addEdge(n, c);
      current = i < 0 ? ((int) nodes.size()) - 1 : i;
    }
    int randomNode(){
      return clamp((int)floor(random::uniform() * (float) nodes.size()), 0, ((int) nodes.size()) - 1);
    }
    int step(float randomness){
      // printf("%f\n", randomness);
      if(current < 0 && hasNodes){
        current = 0;
        return current;
      }else if(current >= 0 && current < (int) nodes.size()){
        // DEAD END
        if(nodes.at(current).edges.size() == 0 || ((int)nodes.at(current).edges.size() == 1 && nodes.at(current).edges.at(0).note == nodes.at(current).note)){
          if(randomness < -0.5f){
            // ORDER
            return current;
          }else{
            // RANDOM
            if(random::uniform() * 1.5f < randomness + 0.5f)
              return randomNode();
            else
              return current;
          }
        }else{
          int next = nodes.at(current).getNext(randomness);
          if(randomness > 0.5f){
            float r = (randomness - 0.5f) * 2.0f;
            if(random::uniform() < r){
              next = -1;
            }
          }
          current = next < 0 ? (randomness > -0.9f ? randomNode() : current) : findNode(next);
        }
      }
      return current;
    }
    int getCurrentNote(){
      if(current >= 0)
        return nodes.at(current).note;
      else
        return -1;
    }
  };
  enum ParamIds {
    LEARN_PARAM,
    CLEAR_PARAM,
    RANDOMNESS_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    LEARN_INPUT,
    CLEAR_INPUT,
    RANDOMNESS_INPUT,
    CV_INPUT,
    GATE_INPUT,
    TRIGGER_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    CV_OUTPUT,
    GATE_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    LEARN_LIGHT,
    // DEADEND_LIGHT,
    NUM_LIGHTS
  };

  bool face_wiggle = true, keep_clothes = false;
  bool blackkeys[12] = {false, true, false, true, false, false, true, false, true, false, true, false};

  TriggerSwitch learn_trigger;
  TriggerSwitch forget_trigger;
  
  TriggerSwitch gate_trigger;
  TriggerSwitch trigger_trigger;

  bool learning = false;
  ChainkovChain chain = ChainkovChain();
  float randomness = 0.0f;
  bool dirty = true;

  unless::ChainkovTheme theme = unless::ChainkovTheme();
  


  void onReset() override{
    chain.clear();
    set_dirty();
  }

  int cvToMidi(float v){
    return round((clamp(v + 5.0f, 0.0f, 10.58f) / 10.58f) * 127.0f);
  }
  float midiToCV(int m){
    return ((float) m - 60) / 12.0f;
  }
  float cvToHertz(float cv){
    return 261.626f * powf(2.0f, cv);
  }
  void bypass(){
    outputs[CV_OUTPUT].value = inputs[CV_INPUT].value;
    outputs[GATE_OUTPUT].value = inputs[GATE_INPUT].value;
  }
  void updateTriggers(){
    learn_trigger.update(params[LEARN_PARAM].value + inputs[LEARN_INPUT].value);
    forget_trigger.update(params[CLEAR_PARAM].value + inputs[CLEAR_INPUT].value); 
    trigger_trigger.update(inputs[TRIGGER_INPUT].value);
    gate_trigger.update(inputs[GATE_INPUT].value);
  }
  void set_dirty(){
    dirty = true;
  }

  void step() override {
    updateTriggers();
    
    int inputMidi = cvToMidi(inputs[CV_INPUT].value);
    float pr = params[RANDOMNESS_PARAM].value;
    randomness = clamp(pr + inputs[RANDOMNESS_INPUT].value * 0.1f, -1.0f, 1.0f);
    
    if(forget_trigger.state == PRESSED){
      chain.forget();
      set_dirty();
    }
    
    if(learn_trigger.state == PRESSED)
      learning = !learning;
    
    if(learning){
      if(gate_trigger.state > 0 && chain.getCurrentNote() != inputMidi){
        bypass();
        if(gate_trigger.state == PRESSED)
          outputs[GATE_OUTPUT].value = 0.0f;
        chain.add(inputMidi, randomness);
        set_dirty();
      }
    }

    if(trigger_trigger.state == PRESSED){
      chain.step(randomness);
      outputs[GATE_OUTPUT].value = 0.0f;
      set_dirty();
    }else if(trigger_trigger.state == RELEASED || trigger_trigger.state == UP){
      outputs[GATE_OUTPUT].value = 0.0f;
    }else{
      outputs[GATE_OUTPUT].value = 10.0f;
    }

    if(gate_trigger.state == PRESSED && !learning){
      chain.setNote(cvToMidi(inputs[CV_INPUT].value));
      set_dirty();
    }

    outputs[CV_OUTPUT].value = midiToCV(chain.getCurrentNote());
    
    lights[LEARN_LIGHT].value = learning ? 10.0f : 0.0f;
  }

  json_t *dataToJson() override {
    json_t *rootJ = json_object();
    json_t *nodesJ = json_array();
    int i = 0;
    for (auto& node : chain.nodes) {
      json_t *nodeJ = json_object();
      json_t *edges = json_array();
      int j = 0;
      for(auto& edge : node.edges){
        json_t *edgeJ = json_object();
        json_object_set_new(edgeJ, "note", json_integer(edge.note));
        json_object_set_new(edgeJ, "count", json_integer(edge.count));
        json_array_insert_new(edges, j, edgeJ);
        j++;
      }
      json_object_set_new(nodeJ, "edges", edges);
      json_object_set_new(nodeJ, "note", json_integer(node.note));
      json_object_set_new(nodeJ, "count", json_integer(node.count));
      json_array_insert_new(nodesJ, i, nodeJ);
      i++;
    }
    json_object_set_new(rootJ, "nodes", nodesJ);
    json_object_set_new(rootJ, "current", json_integer(chain.current));
    json_object_set_new(rootJ, "learning", json_boolean(learning));
    json_object_set_new(rootJ, "theme", theme.to_json());
    json_object_set_new(rootJ, "face_wiggle", json_boolean(face_wiggle));
    json_object_set_new(rootJ, "keep_clothes", json_boolean(keep_clothes));
    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
    json_t *nodesJ = json_object_get(rootJ, "nodes");
    for (int i = 0; i>=0; i++){
      if(json_array_get(nodesJ, i) != NULL){
        json_t *node = json_array_get(nodesJ, i);
        json_t *edges = json_object_get(node, "edges");
        Node n = Node(json_integer_value(json_object_get(node, "note")), json_integer_value(json_object_get(node, "count")));
        for (int j = 0; j>=0; j++) {
          if(json_array_get(edges, j) != NULL){
            std::vector<Edge> es;
            json_t *edge = json_array_get(edges, j);
            n.edges.push_back(Edge(json_integer_value(json_object_get(edge, "note")), json_integer_value(json_object_get(edge, "count"))));
          }else{
            break;
          }
        }
        chain.nodes.push_back(n);
      }else{
        break;
      }
    }
    chain.hasNodes = (int)chain.nodes.size() > 0;
    chain.current = json_integer_value(json_object_get(rootJ, "current"));
    learning = json_boolean_value(json_object_get(rootJ, "learning"));
    theme.load_json(json_object_get(rootJ, "theme"));
    face_wiggle = json_boolean_value(json_object_get(rootJ, "face_wiggle"));
    keep_clothes = json_boolean_value(json_object_get(rootJ, "keep_clothes"));
  }
  Chainkov(){
    // unless_colors::loadConfig();
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(LEARN_PARAM, 0.0f, 1.0f, 0.0f, "toggle learning mode");
    configParam(CLEAR_PARAM, 0.0f, 1.0f, 0.0f, "clear current note");
    configParam(RANDOMNESS_PARAM, -1.0f, 1.0f, 0.0f, "randomness");

    configInput(LEARN_INPUT, "learn");
    configInput(CLEAR_INPUT, "clear");
    configInput(RANDOMNESS_INPUT, "randomness");
    configInput(CV_INPUT, "cv");
    configInput(GATE_INPUT, "gate");
    configInput(TRIGGER_INPUT, "trigger");
    configOutput(CV_OUTPUT, "cv");
    configOutput(GATE_OUTPUT, "gate");
    
    onReset();
  }
};
struct ChainkovKnob :  SVGKnob{
   ChainkovKnob(){
    box.size = Vec(48,48);
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/knobs/MrChainkov.svg")));
    minAngle = -3.1415f * 0.5f + 3.1415f * 0.1f;
    maxAngle = 3.1415f * 0.5f - 3.1415f * 0.1f;
  }
};


struct ChainkovDisplay : _less::Widget {
  Chainkov *module;
  NVGcolor black = nvgRGBA(0x22, 0x22, 0x22, 0xee);
  NVGcolor white = nvgRGBA(0xf9, 0xfa, 0xea, 0xff);
  NVGcolor grey = nvgRGBA(0xee, 0xee, 0xee, 0xdd);
  float hue = 0.0f;
  Vec relpos, parentsize;

  unless::ChainkovTheme *chainkov_theme;
  
  ChainkovDisplay(Vec p, Chainkov *m) {
    module = m;
    box.pos = p;
    box.size = Vec(96,96);
  }
  int cvToMidi(float v){
    return round((clamp(v + 5.0f, 0.0f, 10.58f) / 10.58f) * 127.0f);
  }

  void init(NVGcontext *vg) override{

    set_box(box.pos.x, box.pos.y, box.size.x, box.size.y);
    color_mode(HSLA);
  }
  int posToNote(int x, int y){
    const int w = 8;
    return floor((y / w) * 12) + ((x / w) % 12);
  }
  void onButton(const event::Button &e) override{
    if(e.action == GLFW_PRESS && module){
      int note = posToNote(e.pos.x, e.pos.y);
      if(module->learning){
        module->chain.add(note, 0.0f);
      }else
        module->chain.setNote(note);
      module->set_dirty();
    }
  }


  void render_lights() override {
    int w = 8;
    fill(chainkov_theme->shirt);
    rect(0, 0, width, height, FILL);
    fill(nvgHSLA(0.0f, 0.0f, 0.5f, 90));
    rect(0, 0, width, height, FILL);
    fill(nvgHSLA(0.0f, 0.0f, 0.5f, 125));
    rect(0, w * 5, width, w, FILL);
    if(module && module->chain.hasNodes){
      //NODES
      for(auto& node : module->chain.nodes){
        int n = node.note;
        fill(module->blackkeys[n % 12] ? black : white);
        rect((n % 12) * w, (n / 12) * w, w, w);
      }

      int current = module->chain.current;
      if(current >= 0 && current < (int) module->chain.nodes.size()){
        int n = module->chain.nodes.at(current).note;
        int nx = (n % 12) * w + 4;
        int ny = (n / 12) * w + 4;
        int es = (int) module->chain.nodes.at(current).edges.size();
        Vec pos1 = Vec(nx, ny);
        for(int j = es - 1; j>=0; j--){
          float weight = 1.0f - ((float)j / (float)es);
          int next = module->chain.nodes.at(current).edges.at(j).note;
          // HIGHLIGHT
          fill(nvgHSLA(hue + (1.0f - weight) * 0.3f, 0.5f, 0.5f, 50 + weight * 130));
          rect((next % 12) * w, (next / 12) * w, w, w);

          // EDGE
          // slightly modified version of the wire drawing code from Rack/src/app/WireWidget.cpp
          Vec pos2 = Vec((next % 12) * w + 4, (next / 12) * w + 4);
          float dist = pos1.minus(pos2).norm();
          Vec slump;
          slump.y = (1.0 - 0.5) * 0.4 * (150.0 + 1.0*dist);
          // slump.y = (1.0 - gToolbar->wireTensionSlider->value) * 0.4 * (150.0 + 1.0*dist);
          Vec pos3 = pos1.plus(pos2).div(2).plus(slump);
          nvgLineJoin(_vg, NVG_ROUND);
          Vec pos4 = pos3.plus(slump.mult(0.08));
          nvgBeginPath(_vg);
          nvgMoveTo(_vg, pos1.x, pos1.y);
          nvgQuadTo(_vg, pos4.x, pos4.y, pos2.x, pos2.y);
          nvgStrokeColor(_vg, nvgHSLA(hue + (1.0f - weight) * 0.3f, 0.5f, 0.5f, weight * 255));
          nvgStrokeWidth(_vg, 2);
          nvgStroke(_vg);
        }
        //CURRENT
        fill(nvgHSL(hue, 0.7f, 0.5f));
        rect((n % 12) * w, (n / 12) * w, w, w);
      }

    }
  }
};

struct ChainkovSetting : MenuItem {
  bool *setting = NULL;
  Chainkov *module;
  void onAction(const event::Action &e) override {
    *setting = !*setting;
  }
  void step() override {
    rightText = *setting ? "✔" : "";
    MenuItem::step();
  }
};

struct ChainkovThemeSwitch : MenuItem {
  unless::ChainkovTheme *chainkov_theme;
  unless::ChainkovPanel *panel;
  ChainkovThemeSwitch(std::string t, unless::ChainkovTheme *th, unless::ChainkovPanel *p){
    text = t;
    chainkov_theme = th;
    panel = p;
  }
  void onAction(const event::Action &e) override {
    chainkov_theme->minimal_skin = !chainkov_theme->minimal_skin;
    panel->set_dirty();
  }
};

struct ChainkovWidget : ModuleWidget {
  _less::Panel *pp = NULL;
  Chainkov *moduleInstance = NULL;
  unless::ChainkovPanel *chainkov_panel = NULL;
  unless::ChainkovTheme *chainkov_theme = NULL;
  ChainkovWidget(Chainkov *module){
    setModule(module);
    moduleInstance = module;
    box.size = Vec(120, 380);
    chainkov_theme = module ? &module->theme : new unless::ChainkovTheme();

    pp = new _less::Panel(box.size, nvgHSLA(0.5,0.0,0.5,0));
    addChild(pp);
    ChainkovDisplay *d = new ChainkovDisplay(Vec(12,84), module);
    d->chainkov_theme = chainkov_theme;
    d->relpos = Vec(- 12, - 76);
    d->parentsize = Vec(box.size.x, box.size.y);
    chainkov_panel = new unless::ChainkovPanel(box, chainkov_theme, pp, module ? &module->keep_clothes : NULL);
    pp->fb->addChild(chainkov_panel);
    pp->fb->addChild(d);

    addChild(createWidget<PeaceScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<PeaceScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    //HEAD
    int x = (int)(box.size.x * 0.5f);
    int y = 215;
    int spacey = 27;
    addParam(createParam<ChainkovKnob>(Vec(x - 24, y), module, Chainkov::RANDOMNESS_PARAM));

    unless::ChainkovHead *head = new unless::ChainkovHead(x - 24, y, chainkov_theme);
    head->box.size = Vec(48, 48);
    if(module){
      head->minimal_skin = &module->theme.minimal_skin;
      head->randomness = &module->randomness;
      head->face_wiggle = &module->face_wiggle;
    }
    addChild(head);
    addInput(
      new _less::Port<unless::CvInput>(
        Vec(box.size.x * 0.5f, y - 19),
        module, 
        Chainkov::RANDOMNESS_INPUT
    ));

    // LEARN / FORGET
    x = 13;
    y = 34.0f;
    spacey = 28;
    
    float centerx = box.size.x * 0.5f;

    x = box.size.x / 3.6f;
    float w2 = 12.0f;

    addInput(
      new _less::Port<unless::GateInput>(
        Vec(centerx - x, y),
        module, 
        Chainkov::LEARN_INPUT
    ));

    addInput(
      new _less::Port<unless::GateInput>(
        Vec(centerx + x, y),
        module, 
        Chainkov::CLEAR_INPUT
    ));
    y += 18.0f;
    addChild(createLight<MediumLight<RedLight>>(Vec(centerx - x - w2 + 7.5f, y + 6.5f), module, Chainkov::LEARN_LIGHT));
    addParam(
      createParam<RecordButton>(Vec(centerx - x - w2, y), module, Chainkov::LEARN_PARAM)
    );
    addParam(createParam<StopButton>(Vec(centerx + x - w2, y), module, Chainkov::CLEAR_PARAM));

    // PORTS
    y = 300;
    spacey = 42;

    x = box.size.x / 3.6f;


    addInput(
      new _less::Port<unless::CvInput>(
        Vec(centerx - x, y),
        module, 
        Chainkov::CV_INPUT
    ));

    addOutput(
      new _less::Port<unless::CvOutput>(
        Vec(centerx - x, y + spacey),
        module, 
        Chainkov::CV_OUTPUT
    ));
    addInput(
      new _less::Port<unless::GateInput>(
        Vec(centerx, y + 13.0f),
        module, 
        Chainkov::TRIGGER_INPUT
    ));
    addInput(
      new _less::Port<unless::GateInput>(
        Vec(centerx + x, y),
        module, 
        Chainkov::GATE_INPUT
    ));

    addOutput(
      new _less::Port<unless::GateOutput>(
        Vec(centerx + x, y + spacey),
        module, 
        Chainkov::GATE_OUTPUT
    ));
  }
  void set_dirty(){
    pp->set_dirty();
    moduleInstance->dirty = false;
  }
  void step() override{
    ModuleWidget::step();
    if(theme.dirty || (moduleInstance && moduleInstance->dirty)){
      set_dirty();
    }
  }
  void appendContextMenu(Menu *menu) override {
    Chainkov *markov = dynamic_cast<Chainkov*>(module);
    assert(markov);
    menu->addChild(construct<MenuLabel>());
    ChainkovThemeSwitch *ms = new ChainkovThemeSwitch("dehumanize", chainkov_theme, chainkov_panel);
    menu->addChild(ms);
    menu->addChild(construct<ChainkovSetting>(&MenuItem::text, "face wiggle", &ChainkovSetting::setting, &markov->face_wiggle, &ChainkovSetting::module, markov));
    menu->addChild(construct<ChainkovSetting>(&MenuItem::text, "never change clothes", &ChainkovSetting::setting, &markov->keep_clothes, &ChainkovSetting::module, markov));
  }
};


Model *modelChainkov = createModel<Chainkov, ChainkovWidget>("markov");
/*
  TODO

  ties

  reverse weights option
  
  OPTIMIZE

  calculate all edge counts beforehand

*/
