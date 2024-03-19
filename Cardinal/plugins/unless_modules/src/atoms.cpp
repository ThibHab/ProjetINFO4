#include "unless.hpp"
#include "widgets.hpp"
#include "utils.hpp"
#include "../art/components.hpp"

#define SLUG "atoms"

#define ELECTRONS 8
#define LAYERS 2

#define NONE -1
#define INNER 0
#define OUTER 1

#define PADDING 12
#define INPUT_X 21
#define CHANNEL_SPACING 31
#define LINE_WIDTH 5.0f

#define CENTER_OFFSET 0.515f

#define RADIUS_ATOM 0.06f
#define RADIUS_INNER 0.14f
#define RADIUS_OUTER 0.23f
#define RADIUS_ELECTRON 0.028f

enum AtomMode{
  ATTACK,
  TRIGGER,
  MIX,
  SEQ,
  ATOMMODES
};
#define INPUT_MODES 3

// enum LayerIndex{
//   INNER,
//   OUTER
// };

enum ModMode{
  NO_MOD,
  POS_MOD,
  ENERGY_MOD,
  TOGGLE_MOD,
  DISTANCE_OUT,
  POS_OUT,
  TWIN_DISTANCE
};
struct ModData{
  ModMode mode;
  int layer;
  int index;
  ModData(ModMode m = NO_MOD, int l = NONE, int i = NONE){
    mode = m; layer = l; index = i;
  }
  void copy(ModData m){
    mode = m.mode; layer = m.layer; index = m.index;
  }
  void set(ModMode m, int l, int i){
    mode = m; layer = l; index = i;
  }
  bool equals(ModData d){
    return d.mode != NO_MOD && d.mode == mode && d.layer == layer && d.index == index;
  }
  void print(){
    printf("m %d\n", mode);
    printf("l %d\n", layer);
    printf("i %d\n\n", index);
  }
};


struct DualPort : Widget{
  bool is_input;
  _less::Port<unless::CvInput> *input;
  _less::Port<unless::CvOutput> *output;
  bool input_layer = true;
  DualPort(_less::Port<unless::CvInput> *i, _less::Port<unless::CvOutput> *o, bool isin = true, bool l = true, bool is_electron = false){
    input = i; output = o; is_input = isin; input_layer = l;
    update_ports(is_input, is_electron);
  }
  void update_ports(bool ii, bool is_electron){
    // if(input != NULL && output != NULL){
      is_input = ii;
      if(input_layer ? is_electron || is_input : !is_electron && is_input){
        output->visible = false;
        input->visible = true;
        APP->scene->rack->clearCablesOnPort(output);
      }else{
        output->visible = true;
        input->visible = false;
        APP->scene->rack->clearCablesOnPort(input);
      }
      // }
  }
};

struct ModPort{
  ModData data;
  TriggerSwitch trigger;
  DualPort *port;
  ModPort(){
    port = NULL;
  }
  ~ModPort(){
    delete port;
  }
  void swap(int t = -1){
    if(t == -1){
      port->is_input = !port->is_input;
      port->update_ports(port->is_input, false);
      data.mode = NO_MOD;
    }
  }
  void load(ModData d){
    data.copy(d);
  }
  void print(){
    data.print();
  }
};

struct Electron {
  bool active = true;
  float pos;
  float offset = 0.0f;
  float energy = 0.1f;
  float energy_mod = 0.0f;
  Electron(float p = 0.0f){
    pos = p;
  }
  void load(Electron *e){
    pos = e->pos;
    active = e->active;
    energy = e->energy;
    offset = 0.0f;
    energy_mod = 0.0f;
  }
  float p(){
    return fmod(offset + pos, 1.0f);
  }
  float e(){
    return clamp(energy + energy_mod, 0.0f, 1.0f);
  }
  void toggle(){
    active = !active;
  }
};

struct Layer {
  int count = 1;
  float offset = 0.0f;
  float input_offset = 0.0f;
  Electron electrons[ELECTRONS];
  Layer(){
    equalize(true, true);
  }
  void load(Layer *l){
    offset = l->offset;    
    count = l->count;
    for(int e = 0; e<ELECTRONS; e++)
      electrons[e].load(&l->electrons[e]);
  }
  void equalize(bool energy = false, bool active = false){
    float d = 1.0f / (float) count;
    for(int i = 0; i<count; i++){
      electrons[i].pos = (float) i * d;
      electrons[i].offset = 0.0;
      if(energy){
        electrons[i].energy = d * 0.5f;
        electrons[i].energy_mod = 0.0f;
      }
      if(active)
        electrons[i].active = true;
    }
  }
  bool update_count(int c, bool eq = true){
    if(c != count){
      count = c;
      if(eq)
        equalize(true);
      return true;
    }else
      return false;
  }
  float pos(int index){
    float p = electrons[index].pos + electrons[index].offset + offset + input_offset;
    return std::fmod((p < 0 ? 1.0f - abs(p) : p), 1.0f);  
  }
  float pos(int index, bool animated){
    if(animated){
      return std::fmod(
        electrons[index].pos + electrons[index].offset 
        + offset + input_offset
        , 1.0f);  
    }
    else
      return std::fmod(electrons[index].pos + offset, 1.0f);
  }
  void reset(){
    offset = 0.0f;
    input_offset = 0.0f;
    update_count(count);
    equalize(true, true);
  }
};

struct ElectronPair{
  float dist = 1.0f;
  bool connected = false;
  float time = 0.0f;
};

struct OutputElectron{
  ElectronPair pairs[ELECTRONS];
  int closest = NONE;

  void pair(int index, Layer *in, Layer *out){
    float d = 1.0f;
    for(int i = 0; i<in->count; i++){
      pairs[i].dist = circle_dist(out->pos(index), in->pos(i));
      bool nt = in->electrons[i].active && pairs[i].dist < in->electrons[i].e() + out->electrons[index].e();
      pairs[i].connected = nt;

      if(pairs[i].connected && pairs[i].dist < d){
        d = pairs[i].dist;
        closest = i;
      }
    }

    if(!pairs[closest].connected)
      closest = NONE;
  }
  void update_times(int input_count, float dt){
    for(int i = 0; i<input_count; i++){
      if(pairs[i].time > 0.0f)
          pairs[i].time -= dt;
    }
    if(closest != NONE && pairs[closest].time < 1.0f)
      pairs[closest].time += dt + dt;
  }
};

struct Atom {
  Layer layers[2];
  std::string name;
  int atom = 0;
  ModData mods[ELECTRONS * 2];
  Atom(int a = 0, int ic = 5, int oc = 3){
    atom = a;
    layers[INNER].count = ic;
    layers[OUTER].count = oc;
    layers[INNER].reset();
    layers[OUTER].reset();
  }
  void load(Atom *a){
    for(int l = 0; l<LAYERS; l++){
      layers[l].load(&a->layers[l]);
    }
    for(int i = 0; i < ELECTRONS * 2; i++)
      mods[i].copy(a->mods[i]);
  }
};


struct Atoms : Module {
  enum ParamIds {
    ENUMS(COUNT_PARAM, LAYERS),
    NUM_PARAMS
  };
  enum InputIds {
    ENUMS(INS, ELECTRONS * 2),
    ENUMS(COUNT_INPUT, LAYERS),
    ENUMS(OFFSET_INPUT, LAYERS),
    ATOM_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    ENUMS(OUTS, ELECTRONS * 2),
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  NVGcolor rainbow[6] = {
    nvgHSL(0,0.8f,0.6f),
    nvgHSL(0.07,0.8f,0.6f),
    nvgHSL(0.15f,0.8f,0.6f),
    nvgHSL(0.3f,0.6f,0.6f),
    nvgHSL(0.45f,0.8f,0.6f),
    nvgHSL(0.69f,0.9f,0.7f)
  };

  _less::Theme theme = _less::Theme();

  bool auto_equalize = true;
  bool show_ids = false;
  bool show_hints = true;
  bool animate = false;
  bool audio_rate_pairing = false;
  int color_offset = 0;

  AtomMode mode = ATTACK;
  std::vector<Atom> atoms = {Atom(0)};
  bool dirty = true;
  bool unsaved_atom = false;

  OutputElectron out_electrons[ELECTRONS];

  float lerp_speed = 20.0f;
  float selected_atom = 0.0f;
  int current_atom = 0;

  std::vector<Layer*> layers = { &atoms.at(0).layers[INNER], &atoms.at(0).layers[OUTER]};

  ModPort mods[ELECTRONS * 2];

  int counter = 0;
  int pair_cycle = 128;


  void clear_mod(int &index){
    mods[index].data.mode = NO_MOD;
    atoms.at(current_atom).mods[index].mode = NO_MOD;
  }

  void fix_mods(int layer){
    for(int i = 0; i<ELECTRONS; i++){
      int index = layer * ELECTRONS + i;
      int count = layers[layer]->count;
      if(i <  count 
        || (mods[index].data.layer > NONE && layers[mods[index].data.layer]->count <= mods[index].data.index)
        )
        clear_mod(index);
    }
    set_dirty();
  }

  void update_counts(int layer){
    int c = inputs[COUNT_INPUT + layer].active 
      ? round(clamp(params[COUNT_PARAM + layer].value + (7.0f * inputs[COUNT_INPUT + layer].value * 0.1f), 1.0f, 8.0f))
      : params[COUNT_PARAM + layer].value;

    if(layers[layer]->update_count(c, auto_equalize)){
      set_dirty();
      fix_mods(layer);
    }
  }


  bool layer_mod(int layer){
    return inputs[OFFSET_INPUT + layer].active;
  }

  void update_offset(const int &i){
    layers[i]->input_offset = 
      inputs[OFFSET_INPUT + i].active 
      ? rescale(inputs[OFFSET_INPUT + i].value, -10.0f, 10.0f, -1.0f, 1.0f)
      : 0.0;
  }

  void update_mod(const int &i){
    if(inputs[INS + i].active){
      switch(mods[i].data.mode){
        case NO_MOD : break;
        case POS_MOD : {
          float o = clamp(inputs[INS + i].value, -10.0f, 10.0f) * 0.1f;
          layers[mods[i].data.layer]->electrons[mods[i].data.index].offset = o < 0 ? 1.f + o : o;
        }break;

        case ENERGY_MOD :
        layers[mods[i].data.layer]->electrons[mods[i].data.index].energy_mod = inputs[INS + i].value * 0.1;
        break;

        case TOGGLE_MOD :
        mods[i].trigger.update(inputs[INS + i].value);
        if(mods[i].trigger.state == PRESSED)
          layers[mods[i].data.layer]->electrons[mods[i].data.index].toggle();
        break;

        default:
        break;
      }
    }
  }

  void update_mods(){
    int start = layers[INNER]->count;
    int end = start + (8 - start);
    for(int i = start; i < end; i++)
      update_mod(i);
    start = layers[OUTER]->count;
    end = ELECTRONS + start + (8 - start);
    for(int i = ELECTRONS + start; i< end; i++)
      update_mod(i);
  }
  void output_mods(const int &l){
    for(int i = layers[l]->count; i<ELECTRONS; i++){
      switch(mods[l * ELECTRONS + i].data.mode){
        case NO_MOD : break;

        case DISTANCE_OUT :{
        const int index = mods[l * ELECTRONS + i].data.index;
        const int closest = out_electrons[index].closest;

        outputs[OUTS + ((1 - l) * ELECTRONS + i)].value = closest == NONE ? 0.0f :
        rescale(out_electrons[index].pairs[closest].dist, 0.0f, layers[OUTER]->electrons[index].e() + layers[INNER]->electrons[closest].e(), 10.0f, 0.0f);
        }break;
        
        case TWIN_DISTANCE :{
          outputs[OUTS + i].value = circle_dist(
            layers[INNER]->pos( i % layers[INNER]->count),
            layers[OUTER]->pos( i % layers[OUTER]->count)) * 10.0f;
        }break;
        // case ENERGY_OVERLAP :
        // case TRIGGER_FROM_INPUT :
        case POS_OUT :{
          int layer = mods[l * ELECTRONS + i].data.layer;
          int index = mods[l * ELECTRONS + i].data.index;
          outputs[OUTS + ((1 - l) * ELECTRONS + i)].value = layers[layer]->pos(index) * 10.f;
        }
        break;
        
        default:
        break;
      } 
    }
  }
  void output_as_trigger(const int &i){
    const int p = out_electrons[i].closest;
    outputs[OUTS + i].value = 
      (p == NONE || !layers[OUTER]->electrons[i].active) ? 0.0 
      : (inputs[INS + p].active && layers[INNER]->electrons[p].active ? inputs[INS + p].value : layers[INNER]->electrons[p].active ? 10.0 : 0.0);
  }

  void output_as_sequencer(const int &i){
    const int p = out_electrons[i].closest;
    if(p != NONE){
      if(layers[OUTER]->electrons[p].active && layers[INNER]->electrons[p].active){
          outputs[OUTS + i].value = inputs[INS + p].value;
      }
    }
  }

  void output_as_mixer(const int &index){
    float v = 0.0f;
    const float e = layers[OUTER]->electrons[index].e();
    for(int i = 0; i<layers[INNER]->count; i++){
      if(inputs[INS + i].active && layers[INNER]->electrons[i].active){
        float ie = e + layers[INNER]->electrons[i].e();
        if(out_electrons[index].pairs[i].dist < ie){
          v += inputs[INS + i].value * 2.0f * (1.f - (out_electrons[index].pairs[i].dist / (ie)));
        }
      }
    }
    outputs[OUTS + index].value = sigmoid(v * 0.2f) * 12.f - 6.f;
  }

  void output_as_attack(const int &index){
    float v = 0.0;
    for(int i = 0; i<layers[INNER]->count; i++){
      if(inputs[INS + i].active && out_electrons[index].pairs[i].time > 0.0){
        v += inputs[INS + i].value * out_electrons[index].pairs[i].time;
      }
      outputs[OUTS + index].value = v;
    }
  }

  void step() override{
    if(inputs[ATOM_INPUT].active){
      selected_atom = clamp(inputs[ATOM_INPUT].value * 0.1f, 0.0f, 0.999f);
      select_atom(0);
    }


    if(!audio_rate_pairing){
      counter ++;
      if(counter > pair_cycle){
        counter = 0;
        for(int i = 0; i<layers[OUTER]->count; i++){
          out_electrons[i].pair(i, layers[INNER], layers[OUTER]);
        }
        update_mods();

        update_counts(INNER);
        update_counts(OUTER);

        update_offset(INNER);
        update_offset(OUTER);
      }
    }else{
      for(int i = 0; i<layers[OUTER]->count; i++)
        out_electrons[i].pair(i, layers[INNER], layers[OUTER]);    

      update_mods();

      update_counts(INNER);
      update_counts(OUTER);

      update_offset(INNER);
      update_offset(OUTER);
    }

    int out_count = layers[OUTER]->count;
    switch(mode){
      case ATTACK :{
      float dt = APP->engine->getSampleTime() * lerp_speed;
      for(int i = 0; i<out_count; i++){
        out_electrons[i].update_times(layers[INNER]->count, dt);

        if(outputs[OUTS + i].active)
          output_as_attack(i);
      }
      }break;

      case TRIGGER:
      for(int i = 0; i<out_count; i++){
        if(outputs[OUTS + i].active)
          output_as_trigger(i);
      }break;

      case MIX :
      for(int i = 0; i<out_count; i++){
        if(outputs[OUTS + i].active)
          output_as_mixer(i); 
      }break;

      case SEQ : 
      for(int i = 0; i<out_count; i++){
        if(outputs[OUTS + i].active)
          output_as_sequencer(i); 
      }break;

      default: break;
    }

    output_mods(INNER);
    output_mods(OUTER);
  }

// UI FUNCTIONS

  void set_dirty(bool d = true){
    dirty = d;
    save_atom();
  }

  void switch_to_atom(int i){
    current_atom = i;
    for(int l = 0; l<LAYERS; l++){
      layers[l] = &atoms.at(i).layers[l];
      update_counts(l);
    }
    for(int j = 0; j < ELECTRONS * 2; j++)
      mods[j].load(atoms.at(i).mods[j]);

    fix_mods(INNER);
    fix_mods(OUTER);
    set_dirty();
  }

  int select_atom(float f){
    f = fmod(selected_atom + f, 1.0f);
    if(f < 0.0)
      f = 1.0f + f;
    selected_atom = f;
    float s = (float) atoms.size();
    current_atom = (int) floor(s * f);
    switch_to_atom(current_atom);
    set_dirty();
    return current_atom;
  }

  bool add_new_atom(){
    int i = atoms.size();
    if(i < 118){
      atoms.push_back(Atom(i));
      atoms.at(i).load(&atoms.at(current_atom));
      selected_atom = (float) i / (float) (i + 1);
      current_atom = i;
      switch_to_atom(i);
      set_dirty();
      return true;
    }else
      return false;

  }

  void delete_atom(){
    if(atoms.size() > 1){
      atoms.erase(atoms.begin() + current_atom);
      current_atom = current_atom == 0 ? 0 : current_atom - 1;
      selected_atom = (float) current_atom / (float) atoms.size();
      select_atom(0.0f);
    }
    set_dirty();
  }

  void save_atom(){
    for(int i = 0; i<ELECTRONS*2; i++){
      atoms[current_atom].mods[i].copy(mods[i].data);
    }
  }

  void switch_mode(AtomMode a){
    mode = a;
    set_dirty();
  }

  void reset_atom(){
    for(int l = 0; l<LAYERS; l++){
      layers[l]->reset();
    }
    set_dirty();
  }
  void equalize_layer(int layer){
    layers[layer]->equalize(true);
    set_dirty();
  }
  void move_electron(int l, int e, float f){
    float x = layers[l]->electrons[e].pos + f;
    layers[l]->electrons[e].pos = x < 0 ? 1.0f + fmod(x, 1.0f) : fmod(x, 1.0f);
    set_dirty();
  }
  void adjust_energy(int l, int e, float f){
    layers[l]->electrons[e].energy = clamp(layers[l]->electrons[e].energy + f, 0.0f, 1.0f);
    set_dirty();
  }
  void spin_layer(int l, float f){
    layers[l]->offset = f;  
    set_dirty();
  }
  void toggle_port(int l, int i){
    if(l == INNER || layers[l]->count > i){
      layers[l]->electrons[i].active = !layers[l]->electrons[i].active;
    }
    set_dirty();
  }
  void clear_swap_mods(int target, ModData data){
    for(int l = 0; l<2; l++){
      for(int j = layers[l]->count; j<ELECTRONS; j++){
        int mj = l * ELECTRONS + j;
        if(mj != target && mods[mj].data.equals(data)){
          mods[mj].data.copy(mods[target].data);
          atoms[current_atom].mods[mj].copy(mods[mj].data);
        }
      }
    }
  }
  void pick_mod(bool input, int modlayer, int modindex, ModData mod_data){
    int i = modlayer * ELECTRONS + modindex;
    clear_swap_mods(i, mod_data);
    switch(mods[i].data.mode){
      case POS_MOD : 
      layers[mods[i].data.layer]->electrons[mods[i].data.index].offset = 0.0f;
      break;
      case ENERGY_MOD : 
      layers[mods[i].data.layer]->electrons[mods[i].data.index].energy_mod = 0.0f;
      break;
      default:
      break;
    }
    mods[i].load(mod_data);
    atoms[current_atom].mods[i].copy(mod_data);
    fix_mods(modlayer);
    set_dirty();
  }
  void offset_color(){
    color_offset++;
    set_dirty();
  }
  json_t *dataToJson() override {
    json_t *rootJ = json_object();

    json_t *settings = json_object();
    json_object_set_new(settings, "show_ids", json_boolean(show_ids));
    json_object_set_new(settings, "show_hints", json_boolean(show_hints));
    json_object_set_new(settings, "auto_equalize", json_boolean(auto_equalize));
    json_object_set_new(settings, "animate", json_boolean(animate));
    json_object_set_new(settings, "audio_rate_pairing", json_boolean(audio_rate_pairing));
    json_object_set_new(settings, "color_offset", json_integer(color_offset));
    json_object_set_new(settings, "mode", json_integer(mode));
    json_object_set_new(settings, "current_atom", json_integer(current_atom));
    json_object_set_new(rootJ, "settings", settings);


    json_t *atomsJ = json_array();
    for(int a = 0; a<(int)atoms.size(); a++){
      json_t *atomJ = json_object();
      json_t *amodsJ = json_array();
      json_t *layersJ = json_array();
      json_t *layerz[2] = {json_object(), json_object()};

      for(int i = 0; i<ELECTRONS * 2; i++){
        json_t *m = json_object();
        json_object_set_new(m, "mode", json_integer((int)atoms[a].mods[i].mode));
        json_object_set_new(m, "layer", json_integer(atoms[a].mods[i].layer));
        json_object_set_new(m, "index", json_integer(atoms[a].mods[i].index));
        json_object_set_new(m, "is_input", json_boolean(atoms[a].mods[i].mode > INPUT_MODES));
        json_array_insert_new(amodsJ, i, m);
      }
      json_object_set_new(atomJ, "mods", amodsJ);

      for(int i = 0; i<2; i++){
        json_object_set_new(layerz[i], "offset", json_real(atoms[a].layers[i].offset));
        json_object_set_new(layerz[i], "count", json_integer(atoms[a].layers[i].count));
        json_t *es = json_array();
        for(int e = 0; e<ELECTRONS; e++){
          json_t *el = json_object();
          json_object_set_new(el, "pos", json_real(atoms[a].layers[i].electrons[e].pos));
          json_object_set_new(el, "energy", json_real(atoms[a].layers[i].electrons[e].energy));
          json_object_set_new(el, "active", json_boolean(atoms[a].layers[i].electrons[e].active));
          json_array_insert_new(es, e, el);
        }
        json_object_set_new(layerz[i], "electrons", es);
        json_array_insert_new(layersJ, i, layerz[i]);
      }
      json_object_set_new(atomJ, "layers", layersJ);


      json_array_insert_new(atomsJ, a, atomJ);
    }
    json_object_set_new(rootJ, "atoms", atomsJ);
    return rootJ;
  }
  void reset(){
    mode = ATTACK;
    audio_rate_pairing = false;
    show_ids = false;
    show_hints = true;
    auto_equalize = true;
    animate = false;
    switch_to_atom(0);
    for(int i = 0; i<LAYERS; i++){
      atoms.at(0).layers[i].equalize();
    }
    for(int e = 0; e < ELECTRONS * 2; e++){
      clear_mod(e);
    }
    set_dirty();
    // fix_ports();
  }
  void onReset()override{
    atoms.clear();
    atoms.push_back(Atom(0, 3, 5));
    reset();
  }
  
  void dataFromJson(json_t *rootJ) override {
    json_t *atomsJ = json_object_get(rootJ, "atoms");
    int size = json_array_size(atomsJ);

    json_t *settingsJ = json_object_get(rootJ, "settings");

    show_hints = json_boolean_value(json_object_get(settingsJ, "show_hints"));
    auto_equalize = json_boolean_value(json_object_get(settingsJ, "auto_equalize"));
    show_ids = json_boolean_value(json_object_get(settingsJ, "show_ids"));
    animate = json_boolean_value(json_object_get(settingsJ, "animate"));
    audio_rate_pairing = json_boolean_value(json_object_get(settingsJ, "audio_rate_pairing"));
    color_offset = json_integer_value(json_object_get(settingsJ, "color_offset"));
    mode = (AtomMode) json_integer_value(json_object_get(settingsJ, "mode"));

    atoms.clear();
    for(int a = 0; a<size; a++){

      json_t *atomJ = json_array_get(atomsJ, a);
      json_t *layersJ = json_object_get(atomJ, "layers");
      json_t *modsJ = json_object_get(atomJ, "mods");

      Atom _atom = Atom(a);

      for (int i = 0; i<2; i++){
        json_t *layerJ = json_array_get(layersJ, i);
        _atom.layers[i].offset = json_number_value(json_object_get(layerJ, "offset"));
        _atom.layers[i].count = json_integer_value(json_object_get(layerJ, "count"));
        json_t *electrons = json_object_get(layerJ, "electrons");
        for (int e = 0; e<ELECTRONS; e++){
          json_t *el = json_array_get(electrons, e);
          _atom.layers[i].electrons[e].energy = json_number_value(json_object_get(el, "energy"));
          _atom.layers[i].electrons[e].pos = json_number_value(json_object_get(el, "pos"));
          _atom.layers[i].electrons[e].active = json_boolean_value(json_object_get(el, "active"));
        }
      }
      for (int i = 0; i<ELECTRONS * 2; i++){
        json_t *m = json_array_get(modsJ, i);
        int mm = json_integer_value(json_object_get(m, "mode"));
        _atom.mods[i].mode = mm == NONE ? NO_MOD : (ModMode) mm;
        _atom.mods[i].layer = json_integer_value(json_object_get(m, "layer"));
        _atom.mods[i].index = json_integer_value(json_object_get(m, "index"));
      }
      atoms.push_back(_atom);
    }
    current_atom = json_integer_value(json_object_get(settingsJ, "current_atom"));
    selected_atom = (1.0f / (float)atoms.size()) * (float)current_atom;
    switch_to_atom(current_atom);
    set_dirty();
  }
  NVGcolor atom_color(int i, float offset = 0.0f){
    float tt = ((float) (i) / (float)atoms.size());
    float f = tt * 6.0f;
    int ci = floor(f);
    f = fmod(f + offset, 1.0);
    return nvgLerpRGBA(nvgLerpRGBA(rainbow[(ci + color_offset) % 6], rainbow[(ci + 1 + color_offset) % 6], f), theme.bg_dark, 0.2f);
  }
  bool active_port(char i){
    return inputs[INS + i].active;
  }
  Atoms() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(COUNT_PARAM + INNER, 1.f, 8.f, 3.f, "input layer count");
    configParam(COUNT_PARAM + OUTER, 1.f, 8.f, 5.f, "output layer count");
    color_offset = (int)floor(random::uniform() * 5.0f);
    for(int i = 0; i<LAYERS; i++){
      atoms.at(current_atom).layers[i].equalize();
    }
    configInput(ATOM_INPUT, "atom");
    configInput(OFFSET_INPUT, "input layer offset");
    configInput(OFFSET_INPUT + 1, "output layer offset");
    configInput(COUNT_INPUT, "input count");
    configInput(COUNT_INPUT + 1, "output count");
    for(int i = 0; i<ELECTRONS * 2; i++){
      configInput(INS + i, "#" + std::to_string(i + 1));
      configInput(OUTS + i, "#" + std::to_string(i + 1));
    }
  }
};

enum SelectionType{
  NOTHING,
  CHANNEL,
  MOD,
  LAYER,
  ELECTRON,
  ATOM,
  MODE,
  SIGNO
};
struct Selection{
  SelectionType type;
  signed char index, layer;
  Selection(SelectionType t = NOTHING, signed char l = NONE, signed char i = NONE){
    type = t; index = i; layer = l;
  }
  bool equals(Selection s){
    return type == s.type && index == s.index && layer == s.layer;
  }
};
struct Letter{
  std::string text;
  Vec pos;
  float rot;
  Letter(std::string t, Vec p, float r){
    text = t; pos = p; rot = r;
  }
};

struct AtomsDisplay : _less::Widget {
  Atoms *module;
  Vec center;
  float radius[4];
  float pi = 3.141592f;
  float pi2 = pi * 2.0f;
  float rotate_speed = 0.0006f;
  float line_width = 5.0f;
  float port_pad = 3.3f;
  float port_spacing = 30.75f;
  bool selecting_atom = false;
  bool param_adjust = false;
  bool audio_rate_pairing = false;
  Vec mouse = Vec(0,0);
  std::vector<Letter> letters;
  float view_offset = pi * -0.5f;


  AtomsDisplay(Rect r, Atoms *m) {
    module = m;
    box = r;
    center = Vec(r.size.x * 0.5f, r.size.y * CENTER_OFFSET);
    radius[INNER] = box.size.y * RADIUS_INNER;
    radius[OUTER] = box.size.y * RADIUS_OUTER;
    radius[2] = box.size.y * RADIUS_ELECTRON;
    radius[3] = box.size.y * RADIUS_ATOM;
    float rr = 9.0f * 1.4f;
    float y = center.y - radius[OUTER] - 34.0f;
    letters.push_back(Letter("A", Vec(center.x - 52.0f, y + rr + 14), 0.0f));
    letters.push_back(Letter("T", Vec(center.x - 28.0f, y + rr + 6), 0.24f));
    letters.push_back(Letter("M", Vec(center.x + 30.0f, y + rr + 6), 0.78f));
    letters.push_back(Letter("S", Vec(center.x + 55.0f, y + rr + 14), 1.0f));
  }
  void init(NVGcontext *vg) override{
    load_font("font/CuteFont-Regular.ttf");
  }
  NVGcolor atom_color(int i, float offset = 0.0f){
    float tt = ((float) (i) / (float)module->atoms.size());
    float f = fmod(tt + offset, 1.0f) * 6.0f;
    int ci = floor(f);
    f = fmod(f, 1.0);
    return nvgLerpRGBA(nvgLerpRGBA(module->rainbow[(ci + module->color_offset) % 6], module->rainbow[(ci + 1 + module->color_offset) % 6], f), theme.bg_dark, 0.2f);
  }
  void drawAtomMenu(NVGcolor c, NVGcolor bg, int s = 1, int i = 0, bool digits = false){
    fill(bg);
    circle(center.x, center.y, radius[3], FILL);

    
    stroke(theme.bg_dark);
    circle(center.x, center.y, radius[3], STROKE);
    float a = pi2 / (float) s;
    float o = pi2 * 0.25f;
    if(s > 1){
      float t = o + i * a;
      stroke(c);
      arc(center.x, center.y, radius[3], t, t+a, NVG_CW);
    }
    if(digits){
      text(std::to_string(i + 1), center.x, center.y, (i > 8 ? (i > 98 ? 27.0f : 36.f) : 42.f), theme.darker);
    }
  }
  
  void drawLetters(float y = 0.0f){
    for(int i = 0; i<4; i++)
      nvgText(_vg, letters[i].pos.x, letters[i].pos.y + y, letters[i].text.c_str(), NULL);
  }
  void drawTitle(NVGcolor c1, NVGcolor c2, int mode = 0){

    nvgFontSize(_vg, 48.0f);
    nvgFontFaceId(_vg, font->handle);
    nvgTextAlign(_vg, NVG_ALIGN_CENTER);

    //selected mode
    float t = view_offset - 0.5f + letters[mode].rot;

    stroke(nvgTransRGBAf(c2, 0.5f));
    round_cap();
    stroke_width(LINE_WIDTH * 3);

    push();
    translate(center);
    rotate(t);    
    line(0,0, radius[OUTER] * 1.27f, 0);
    pop();
    butt_cap();
    
    //title shadow
    NVGcolor c = theme.darker;
    nvgFillColor(_vg, c);
    nvgBeginPath(_vg);
    drawLetters();
    nvgFill(_vg);
    //title
    nvgBeginPath(_vg);
    nvgFillColor(_vg, c1);
    nvgStrokeWidth(_vg, LINE_WIDTH + 2);
    drawLetters(-2.0f);
    nvgFill(_vg);

  }
  void drawWires(){
    float y = center.y - radius[INNER] + 6;
    // float r = 9.0f;

    stroke_width(LINE_WIDTH + 1);
    stroke(theme.input);
    line(
      INPUT_X + 6, y,
      INPUT_X + 6 + 20.0f, center.y - radius[OUTER]
    );
    line(center.x, y, box.size.x - PADDING - INPUT_X + 6, y);

    y = center.y + radius[INNER] - 6;

    stroke_width(LINE_WIDTH + 2);
    stroke(theme.output);

    line(center.x, y,INPUT_X + 6, y);
    line(
      box.size.x - PADDING - INPUT_X + 6, y,
      box.size.x - INPUT_X - 6 - 20.0f, center.y + radius[OUTER]
    );
  }
  void drawLayer(NVGcontext *vg, float offset, float r, float r2, NVGcolor bc, NVGcolor fc){
    fill(theme.shadow);
    stroke_width(LINE_WIDTH);
    circle(center.x, center.y + 4.0f, r + 2.0f, FILL);

    circle(center.x, center.y, r, fc);

    push();
    translate(center.x, center.y);
    rotate(offset * pi2 + pi);
    line(0.0f,0.0f, 0.0f, r, bc, LINE_WIDTH);
    pop();

    if(!param_adjust){
      circle(center.x, center.y, r, bc, STROKE);
    }
  }

  void drawPortButton(Atom *atom, int l, int i, float w, float x, float y){
    if(l == OUTER){
      i = ELECTRONS - 1 - i;
    }
    NVGcolor inactive = l == INNER ? theme.input_dark : theme.output;
    NVGcolor color = l == INNER ? theme.input : theme.output_dark;
    if(i < atom->layers[l].count){
      NVGcolor foreground = theme.layer(1 - l);
      if(atom->layers[l].electrons[i].active){
        circle(x, y + 3.0f, radius[2], inactive); 
        circle(x, y, radius[2], color); 
        text(std::to_string(i + 1), x, y, 25, foreground);
      }else{
        circle(x, y + 3.0f, radius[2], inactive); 
        circle(x, y, radius[2] * 0.8f, inactive, color); 
      }
    }else{
      int mod_id = ELECTRONS * l + i;
      NVGcolor c = theme.layer(atom->mods[mod_id].layer);
      std::string n = std::to_string(atom->mods[mod_id].index + 1);
      switch(atom->mods[mod_id].mode){
        case POS_MOD : 
        circle(x, y, radius[2] * 0.8f, theme.bg, theme.bg);
        text(n, x, y, 25, c);
        break;

        case ENERGY_MOD : 
        rect(x - radius[2], y - radius[2],radius[2] * 2, radius[2] * 2, theme.bg);
        text(n, x, y, 25, c);
        break;
        
        case TOGGLE_MOD : 
        rect(x - radius[2], y - radius[2],radius[2] * 2, radius[2] * 2, color);  
        text(n, x, y, 25, theme.layer(1 - atom->mods[mod_id].layer));
        break;


        case DISTANCE_OUT : 
        stroke_width(2.0f);
        circle(x, y, radius[2], theme.bg_dark, theme.output);
        stroke_width(LINE_WIDTH);
        text(n, x, y, 25, theme.output);
        break;

        case POS_OUT : 
        stroke_width(2.0f);
        circle(x, y, radius[2], theme.bg_dark, theme.output);
        stroke_width(LINE_WIDTH);
        text(n, x, y, 25, theme.input);
        break;

        case TWIN_DISTANCE : 
        stroke_width(2.0f);
        rect(x - radius[2] + 1, y - radius[2] + 1,radius[2] * 2 - 2, radius[2] * 2 - 2, color, STROKE);  
        text(n, x, y, 25, theme.output);
        stroke_width(LINE_WIDTH);
        break;
        
        default :
        circle(x, y, radius[2] * 0.8f, theme.bg, theme.bg);
        break;

      }
    }
  }
  void drawSigno(){
    nvgTextLetterSpacing(_vg, 1.0f);
    float y = center.y + radius[OUTER] + 21;
    text("unless games", center.x, y, 23, theme.fg);
    nvgTextLetterSpacing(_vg, 0.0f);
  }
  void drawPanel(Atom *atom, NVGcolor c1, NVGcolor c2){
    rect(0, 0, box.size.x, box.size.y, theme.bg_dark);

    fill(nvgLerpRGBA(c2, theme.bg_dark, 0.9f));
    circle(center.x, center.y, radius[OUTER] * (1.53f));
    
    fill(nvgLerpRGBA(c1, theme.bg_dark, 0.8f));
    circle(center.x, center.y, radius[OUTER] * (1.3f));

    // rect(0, 0, PADDING * 0.5f, box.size.y, theme.darker);
    // rect(box.size.x - PADDING * 0.5f, 0, PADDING * 0.5f, box.size.y, theme.darker);

    drawWires();

  }
  void drawLayers(Atom *atom){
    drawLayer(_vg, atom->layers[OUTER].offset, radius[OUTER], radius[OUTER] * 0.85f, theme.output, nvgLerpRGBA(theme.bg, theme.output, 0.2f));
    drawLayer(_vg, atom->layers[INNER].offset, radius[INNER], radius[INNER] * 0.8f, theme.input, nvgLerpRGBA(theme.bg, theme.input, 0.4f));
  }
  void drawPortButtons(Atom *atom){
    float iy = PADDING + 35;
    float oy = box.size.y - 20 - PADDING * 2.5f;
    float bw = 24.0f + port_pad;
    for(int i = 0; i<ELECTRONS; i++){
      float xx = PADDING + i * port_spacing + port_spacing * 0.5f;
      drawPortButton(atom, INNER, i, bw, xx, iy);
      drawPortButton(atom, OUTER, i, bw, xx, oy);
    }
  }
  void drawEnergies(Atom *atom, bool animate) {
    for(int l = 1; l>=0; l--){
      for(int e = 0; e<atom->layers[l].count; e++){
        float t = view_offset + pi2 * atom->layers[l].pos(e, animate);
        float en = atom->layers[l].electrons[e].energy;
        stroke_width(LINE_WIDTH);
        stroke(theme.energy[(param_adjust ? 0 : 2) + l]);
        arc(center.x, center.y, radius[OUTER] * (l == INNER ? 0.78f : 0.83f), t - pi * en, t + pi * en, NVG_CW);
      }
    }
  }
  void drawElectrons(Atom *atom, bool animate = false, bool digits = false){
    if(!digits){
      push();
      translate(center.x, center.y);
      for(int l = 1; l>=0; l--){
        NVGcolor c = l == INNER ? theme.input : theme.output_dark;
        NVGcolor c3 = l == INNER ? theme.bg : theme.darker;
        float r = radius[2];
        float x = radius[l];
        float y = 0.0f;

        for(int e = 0; e<atom->layers[l].count; e++){
          float t = view_offset + pi2 * atom->layers[l].pos(e, animate);
          rotate(t);
          stroke_width(line_width - 0.5f);
          circle(x, y, r, (atom->layers[l].electrons[e].active ? c : c3), c);
          rotate(-t);
        }
      }
      pop();
    }
  }
  void render_lights() override{
    if(module){
      Atom *a = &module->atoms.at(module->current_atom);
      if(param_adjust){
        drawEnergies(a, module->animate);
        drawElectrons(a, module->animate, module->show_ids);
      }else{
        drawEnergies(a, module->animate);
        drawElectrons(a, module->animate, module->show_ids);
      }

    }
  }
  void render() override{
    if(module){
      Atom *a = &module->atoms.at(module->current_atom);
      NVGcolor c1 = atom_color(module->current_atom);
      NVGcolor c2 = atom_color(module->current_atom, 0.6f);
      drawPanel(a, c1, c2);
      drawTitle(c1, c2, module->mode);
      drawLayers(a);
      drawAtomMenu(c1, (module->audio_rate_pairing ? c2 : theme.bg), module->atoms.size(), module->current_atom, module->show_ids);
      drawPortButtons(a);

      if(param_adjust){
        drawEnergies(a, module->animate);
        drawElectrons(a, module->animate, module->show_ids);
      }else{
        drawEnergies(a, module->animate);
        drawElectrons(a, module->animate, module->show_ids);
      }
    }else{
      // Atom at = Atom(0, floor(2.0f + rack::random::uniform() * 6.0f), floor(2.0f + rack::random::uniform() * 6.0f));
      Atom at = Atom(0, 3, 5);
      Atom *a = &at;
      NVGcolor c1 = nvgHSLA(0.1f, 0.8f, 0.6f, 255);
      NVGcolor c2 = nvgHSLA(0.7f, 0.6f, 0.6f, 255);
      drawPanel(a, c1, c2);
      drawTitle(c1, c2);
      drawLayers(a);
      drawAtomMenu(c1, theme.bg);
      drawPortButtons(a);
      drawEnergies(a, false);
      drawElectrons(a, false, false);
      drawSigno();
    }
  }
};
struct AtomsControls : _less::Widget {
  Atoms *module;
  AtomsDisplay *display;
  Vec center;
  Selection selection;
  Selection hover;
  float radius[4];
  float pi = 3.141592f;
  float pi2 = pi * 2.0f;
  float rotate_speed = 0.0006f;
  float line_width = 5.0f;
  float port_pad = 3.3f;
  float port_spacing = 30.75f;
  bool selecting_atom = false;
  bool param_adjust = false;
  Vec mouse = Vec(0,0);
  std::vector<Letter> letters;
  std::string info_text = "";
  float info_time = 0.0f;
  float info_fade_speed = 0.02f;
  float view_offset = pi * -0.5f;
  AtomsControls(Rect r, Atoms *m, AtomsDisplay *d) {
    display = d;

    module = m;
    box = r;
    center = Vec(r.size.x * 0.5f, r.size.y * CENTER_OFFSET);
    radius[INNER] = box.size.y * RADIUS_INNER;
    radius[OUTER] = box.size.y * RADIUS_OUTER;
    radius[2] = box.size.y * RADIUS_ELECTRON;
    radius[3] = box.size.y * RADIUS_ATOM;
    float rr = 9.0f * 1.4f;
    float y = center.y - radius[OUTER] - 34.0f;
    letters.push_back(Letter("A", Vec(center.x - 52.0f, y + rr + 14), -0.02f));
    letters.push_back(Letter("T", Vec(center.x - 28.0f, y + rr + 6), 0.24f));
    letters.push_back(Letter("M", Vec(center.x + 30.0f, y + rr + 6), 0.8f));
    letters.push_back(Letter("S", Vec(center.x + 55.0f, y + rr + 14), 1.04f));
  }
  void init(NVGcontext *vg) override{
    set_box(box.pos.x, box.pos.y, box.size.x, box.size.y);
    load_font("font/CuteFont-Regular.ttf");
  }
  NVGcolor atom_color(int i, float offset = 0.0f){
    float tt = ((float) (i) / (float)module->atoms.size());
    float f = tt * 6.0f;
    int ci = floor(f);
    f = fmod(f + offset, 1.0);
    return nvgLerpRGBA(nvgLerpRGBA(module->rainbow[(ci + module->color_offset) % 6], module->rainbow[(ci + 1 + module->color_offset) % 6], f), theme.bg_dark, 0.2f);
  }
  int inside_circle(float cx, float cy, float x, float y, float r){
    float dx = abs(x - cx);
    float dy = abs(y - cy);
    float d = dx * dx + dy * dy;
    return  d < r * r;
  }
  int inside_disc(float x, float y){
    if(inside_circle(center.x, center.y, x, y, radius[3]))
      return 2;
    else if(inside_circle(center.x, center.y, x, y, radius[INNER]))
      return INNER;
    else if(inside_circle(center.x, center.y, x, y, radius[OUTER]))
      return OUTER;
    else
      return NONE;
  }
  int inside_port_button(float x, int l){
    int i = floor(
      (x - (float)PADDING) / (box.size.x - (float)PADDING * 2.0f) 
      * (float) ELECTRONS
    );
    return l == OUTER ? 7 - i : i;
  }
  bool inside_electron(int l, int i, float x, float y){
    float _o = pi2 * -0.25f;
    float t = _o + pi2 * module->layers[l]->pos(i, module->animate);
    float _x = cos(t) * radius[l];
    float _y = sin(t) * radius[l];
    return inside_circle(center.x + _x, center.y + _y, x, y, radius[2] * 1.2f);
  }
  Selection select_stuff(float x, float y){
    // electrons
    for(int l = 0; l < 2; l++){
      for(int i = 0; i < module->layers[l]->count; i++){
        if(inside_electron(l, i, x, y))
          return Selection(ELECTRON, l, i);
      }
    }
    // layers, atoms
    if(y > center.y - radius[OUTER] && y < center.y + radius[OUTER]){
      int d = inside_disc(x, y);
      if(d != NONE){
        if(d < 2)
          return Selection(LAYER, d, NONE);
        else
          return Selection(ATOM);
      }
    }else{
      // port buttons
      int l = 
        y > PADDING + 20 && y < PADDING + 50 ? INNER 
        : ( y > box.size.y - 60 && y < box.size.y - 30 ? OUTER : NONE);
      if(l > NONE && x > PADDING && x < box.size.x - PADDING){
        int i = inside_port_button(x, l);
        if(i > NONE)
          return Selection(module->layers[l]->count > i ? CHANNEL : MOD, l, i);
      }else{
      // mode || color
        for(int i = 0; i<4; i++){
          if(x > letters[i].pos.x - 10.0f && x < letters[i].pos.x + 10.0f && y > letters[i].pos.y - 25.0f && y < letters[i].pos.y)
            return Selection(MODE, i);
        }

        if( y < box.size.y - 60 && y > box.size.y - 90 && x > 70.0f && x < box.size.x - 70.0f)
          return Selection(SIGNO, floor((x - 70.0f) / (box.size.x - 140.0f) * 3.0f));
      }
    }
    return Selection(NOTHING);
  }
  void release(){
    if(selection.type == MOD){
      if(module->mods[selection.layer * ELECTRONS + selection.index].port->is_input){
        ModMode m = hover.type == CHANNEL ? TOGGLE_MOD 
          : (hover.type == ELECTRON ? (param_adjust ? ENERGY_MOD : POS_MOD ) 
          : NO_MOD );
        module->pick_mod(true, selection.layer, selection.index, ModData(m, hover.layer, hover.index));
      }else{
        ModMode m = NO_MOD;
        if(hover.type == ELECTRON){
          if(hover.layer == OUTER)
            m = param_adjust ? TWIN_DISTANCE : DISTANCE_OUT;
          else
            m = param_adjust ? NO_MOD : POS_OUT;
        }
        if(m != NO_MOD)
          module->pick_mod(false, selection.layer, selection.index, ModData(m, hover.layer, hover.index));
      }
    }
    selection = Selection(NOTHING);
  }

  void switch_adjust_mode(){
    param_adjust = !param_adjust;
    display->param_adjust = param_adjust;
    module->set_dirty();
  }

  bool is_mod_port(int &i){
    return (i >= module->layers[INNER]->count && i < ELECTRONS) 
        || (i >= ELECTRONS + module->layers[OUTER]->count && i < ELECTRONS * 2);
  }
  void toggle_mod_type(const int &layer, const int &index){
    int i = layer * ELECTRONS + index;
    if(is_mod_port(i))
      module->mods[i].swap();
    module->fix_mods(layer);
    module->set_dirty();
  }



  void onButton(const event::Button &e) override{
    if(e.action == GLFW_PRESS && e.button != GLFW_MOUSE_BUTTON_MIDDLE){
      selection = select_stuff(e.pos.x, e.pos.y);
      if(selection.type != NOTHING && selection.type != SIGNO){
        e.consume(this);
      }
      switch(selection.type){
        case MOD :
        if(e.button == 1){
          toggle_mod_type(selection.layer, selection.index);
        }
        break;
        case CHANNEL : 
        if(e.button == 0 && selection.type != MOD)
          module->toggle_port(selection.layer, selection.index);
        break;

        case ELECTRON :
        if(e.button == 1)
          switch_adjust_mode();
        break;

        case LAYER : 
        if(e.button == 1)
          switch_adjust_mode();
        break;

        case ATOM : 
        break;

        case MODE :
        module->switch_mode((AtomMode)selection.layer);
        break;

        case SIGNO :
        if(e.button == 0){
          switch(selection.layer){
            case 0 : 
            module->offset_color();
            break;

            case 1: 
            module->animate = !module->animate;
            break;

            case 2: 
            module->show_ids = !module->show_ids;
            break;


          }
        }break;

        default : break;
      }
      if(selection.type == MOD && e.button == 1)
        selection = Selection(NOTHING);
    }else if(e.action == GLFW_RELEASE){
      release();
    }
  }
  void onDragStart(const event::DragStart &e) override{
    if(e.button != GLFW_MOUSE_BUTTON_MIDDLE){
      e.consume(this);
      if(selection.type == LAYER || 
         selection.type == ELECTRON || selection.type == ATOM){
        APP->window->cursorLock();
      }
    }
  }
  void onDragMove(const event::DragMove &e) override{
    if(e.button != GLFW_MOUSE_BUTTON_MIDDLE){
    e.consume(this);
    int mods = APP->window->getMods();
    float delta = 1.0f;
    if ((mods & RACK_MOD_MASK) == RACK_MOD_CTRL)
      delta /= 8.f;
    if ((mods & RACK_MOD_MASK) == (RACK_MOD_CTRL | GLFW_MOD_SHIFT))
      delta /= 32.f;

    float f = e.mouseDelta.y * - rotate_speed * delta;
    
    switch(selection.type){ 
      case LAYER :
      f = fmod(f + module->layers[selection.layer]->offset, 1.0f);
      if(f < 0.0)
        f = 1.0f + f;
      module->spin_layer(selection.layer, f);
      break;

      case ELECTRON : 
      if(param_adjust)
        module->adjust_energy(selection.layer, selection.index, f);
      else
        module->move_electron(selection.layer, selection.index, f);
      break;

      case ATOM : 
      module->select_atom(f);
      break;

      case MOD :{      
        Selection h = select_stuff(mouse.x, mouse.y);
        if(!hover.equals(h)){
          if(h.type == ELECTRON || h.type == NOTHING){
            show_hint(get_hint(h));
            hover = h;
          }
        }
      }break;

      default : break;
    }
    }
  }
  Selection reverse_outselect(Selection s){
    if(s.type != NONE && s.layer == OUTER)
      s.index = 7 - s.index;
    return s;
  }
  void onDragHover(const event::DragHover &e) override{
    if(e.button != GLFW_MOUSE_BUTTON_MIDDLE){
    e.consume(this);
    mouse.x = e.pos.x;
    mouse.y = e.pos.y;
    }
  }
  void onDragEnd(const event::DragEnd &e) override{
    APP->window->cursorUnlock();    
    release();
  }

  const char* get_hint(Selection s){
    switch(s.type){
      case MODE : 
      switch(s.layer){
        case ATTACK : return "attack mode";
        case TRIGGER : return "trigger mode";
        case MIX : return "mixer mode";
        case SEQ : return "sample mode"; 
        default : return "";
      }
      case CHANNEL : {
        return s.layer == INNER ? "input channel" : "output channel";
      }
      case MOD : {
        int index = s.layer * ELECTRONS + s.index;
        switch(module->mods[index].data.mode){
          case NO_MOD : return "assign mod";
          case POS_MOD : return "position mod";
          case ENERGY_MOD : return "range mod";
          case TOGGLE_MOD : return "mute trigger";
          case DISTANCE_OUT : return "distance output";
          case TWIN_DISTANCE : return "twin?";
          case POS_OUT : return "angle out";
          default : return "";
        }
      }
      case LAYER : return s.layer == INNER ? "input layer" : "output layer";
      case ELECTRON : return s.layer == INNER 
          ? (param_adjust ? "input range" : "input position") 
          : (param_adjust ? "output range" : "output position");
      case ATOM : return "atom selector";
      case SIGNO : {
        switch(s.layer){
          case 0 : return "change color";
          case 1 : return (module->animate ? "freeze" : "animate mods");
          default : return (module->show_ids ? "hide digits" : "show digits");
        }
      }break;
      default : return "";

    }
  }

  void onHover(const event::Hover &e) override{

    if(selection.type == NOTHING || selection.type == MOD){
      mouse.x = e.pos.x;
      mouse.y = e.pos.y;
    }

    if(selection.type == NOTHING){
      Selection h = select_stuff(mouse.x, mouse.y);
      if(!hover.equals(h)){
        show_hint(get_hint(h));
      }
      hover = h;
    }
  }
  void show_hint(const char* t){
    info_text = t;
  }
  void drawSigno(NVGcontext *vg){
    nvgTextLetterSpacing(vg, 1.0f);
    float y = center.y + radius[OUTER] + 21;
    float fs = 23;
    if(module->show_hints && hover.type != NOTHING){
      info_time -= info_fade_speed;
      text(get_hint(hover), center.x, y, fs, theme.fg);
    }else
      text("unless games", center.x, y, fs, theme.fg);
    nvgTextLetterSpacing(vg, 0.0f);
  }

  void drawEnergy(int l, int i, float alpha = 0.4f, bool with_mod = false){
    push();
    translate(center.x,center.y);
    rotate(view_offset + module->layers[l]->pos(i, module->animate) * pi2);
    NVGcolor ac = atom_color(module->current_atom);
    stroke(nvgLerpRGBA(ac, nvgRGBA(0,0,0,0), alpha));
    stroke_width(LINE_WIDTH + 1);
    float en = with_mod ? module->layers[l]->electrons[i].e() : module->layers[l]->electrons[i].energy;
    arc(0, 0, radius[OUTER] * (l == INNER ? 0.78f : 0.83f), -pi * en, pi * en, NVG_CW);
    pop();
  }
  NVGcolor inverted(NVGcolor c){
    return nvgRGBAf(c.b, c.r, c.g, 1.0f);
  }
  void drawConnectedLine(int &ie, int &oe){
    round_cap();
    float ox = module->outputs[module->OUTS + oe].value;
    NVGcolor pc = ox < 0.0f ? nvgLerpRGBA(atom_color(module->current_atom), theme.output, rescale(ox, -7.0f, 0.0f, 0.0f, 1.0f)) : nvgLerpRGBA(theme.output, inverted(atom_color(module->current_atom)), ox * 0.1f);
    float t = view_offset + pi2 * module->layers[INNER]->pos(ie, module->animate);
    float pt = view_offset + pi2 * module->layers[OUTER]->pos(oe, module->animate);

    stroke(pc);
    stroke_width((LINE_WIDTH / 3) * 4);
    fill(pc);
    push();


    translate(center);


    rotate(t);
    float x = radius[INNER];
    circle(x,0,radius[2] * 0.7f, FILL);
    rotate(pt - t);
    float _x = radius[OUTER];
    circle(_x,0,radius[2] * 0.7f, FILL);
    


    path(_x, 0);
    rotate(- (pt - t));
    point(x, 0);
    end();
    
    pop();
    butt_cap();
  }
  void drawPairs(NVGcontext *vg){
    if(module->mode == MIX){
      for(int i = 0; i<module->layers[OUTER]->count; i++){
        for(int p = 0; p < ELECTRONS; p++){
          if(
             module->layers[INNER]->count > p
             && module->out_electrons[i].pairs[p].connected 
             && module->layers[OUTER]->electrons[i].active 
             && module->layers[INNER]->electrons[p].active
             ){
            drawConnectedLine(p, i);
          }
        }
      }
    }else{
      for(int i = 0; i<module->layers[OUTER]->count; i++){
        int p = module->out_electrons[i].closest;
        if(p != NONE 
           && module->layers[OUTER]->electrons[i].active 
           && module->layers[INNER]->electrons[p].active){
          drawConnectedLine(p, i);
        }
      }
    }
    stroke_width(LINE_WIDTH);
  }
  void drawElectron(int l, int i, bool hover = false){
    push();
    float r = module->layers[l]->pos(i, module->animate) * pi * 2.0f;
    translate(center.x, center.y);
    rotate(r);
    translate(0.0f, -radius[l]);
    rotate(-r);
    stroke_width(line_width - 0.5f);    
    if(hover){
      NVGcolor ac = atom_color(module->current_atom);
      circle(0,0,radius[2] * 1.01, ac, ac);
      fill(theme.output);
    }else{
      NVGcolor ac = theme.layer_dark(l);
      circle(0,0,radius[2], ac, ac);
      fill(theme.layer_dark(1 - l));
    }
    text(std::to_string(i + 1), -1, 0, radius[2] * 2.5f);
    pop();
  }
  void drawElectrons(){
    for(int l = 0; l<2; l++){
      for(int i = 0; i<module->layers[l]->count; i++)
        drawElectron(l, i);
    }
  }

  void drawLayerMod(int l){
    push();
    translate(center.x, center.y);
    NVGcolor ac = atom_color(module->current_atom);
    stroke(nvgLerpRGBA(ac, nvgRGBA(100,100,100, 200), 0.4f));

    rotate(module->layers[l]->offset * pi2 + view_offset);
    stroke_width(LINE_WIDTH);
    float r = 
      l == OUTER 
      ? radius[l] + radius[2] + LINE_WIDTH
      : radius[3] + LINE_WIDTH;
    float a = module->layers[l]->input_offset * pi2;
    float offs = -pi*0.028f;
    if(a >= 0 && a < pi){
      arc(0.0f,0.0f, r, offs, a, NVG_CW);
    }
    else
      arc(0.0f,0.0f, r, -offs, a, NVG_CCW);
    pop();
  }

  void drawPortMod(int i){
    ModData *d = &module->mods[i].data;
    NVGcolor ac = atom_color(module->current_atom);
    NVGcolor faded = nvgLerpRGBA(ac, nvgRGBA(100,100,100, 20), 0.4f);

    if(module->active_port(i)){
      switch(d->mode){
        case NO_MOD: break;

        case POS_MOD :{
        push();
        butt_cap();
        translate(center.x, center.y);
        rotate(module->layers[d->layer]->pos(d->index, true) * pi2 + view_offset);
        stroke(faded);
        fill(faded);
        stroke_width(LINE_WIDTH);
        float r = radius[d->layer] - LINE_WIDTH;
        float a = module->layers[d->layer]->electrons[d->index].offset * pi2;
        
        if(a > pi)
          arc(0.0f,0.0f, r, 0.0f, -a, NVG_CW);
        else
          arc(0.0f,0.0f, r, 0.0f, -a, NVG_CCW);

        circle(r,0.0f,  4.0f, BOTH);
        pop();
        }break;      

        case ENERGY_MOD : 
        butt_cap();
        drawEnergy(d->layer, d->index, 0.3f, true);
        break;

        default: break;
      }
    }
  }

  Vec port_button_pos(int l, int i){
    return Vec(
      PADDING + (l == OUTER ? 7 - i : i) * port_spacing + port_spacing * 0.5f,
      (l == INNER ? PADDING + 35 : box.size.y - 20 - PADDING * 2.5f));
  }

  void render_lights() override{
    butt_cap();
    NVGcolor ac = module->atom_color(module->current_atom);
    drawPairs(_vg);
    if(module->show_ids)
      drawElectrons();
    for(int i = 0; i<2; i++){
      if(module->layer_mod(i))
        drawLayerMod(i);
    }
    for(int i = 0; i<ELECTRONS * 2; i++){
      drawPortMod(i);
    }
    if(selection.type != NOTHING){
      switch(selection.type){ 
        case ATOM : 
        break;
        case LAYER :
        break;

        case ELECTRON :
        drawElectron(selection.layer, selection.index, true);
        butt_cap();
        drawEnergy(selection.layer, selection.index, 0.0f);
        break;


        case MOD :{
          Vec p = port_button_pos(selection.layer, selection.index);
          stroke(ac);
          fill(ac);
          round_cap();
          line(p.x, p.y, mouse.x, mouse.y, ac, LINE_WIDTH);
          butt_cap();
          circle(p.x, p.y, radius[2] * 0.5f, BOTH);
        }break;

        default : break;
      }  
    }
    switch(hover.type){ 
      case CHANNEL :
      drawElectron(hover.layer, hover.index, true);

      break;
      case LAYER :
      break;
      case MOD :{
      ModData *d = &module->mods[hover.layer * ELECTRONS + hover.index].data;
      if(d->mode != NO_MOD)
        drawElectron(d->layer, d->index, true);
      }break;


      case ELECTRON :
      butt_cap();
      if(selection.type == MOD){
        if(param_adjust)
          drawEnergy(hover.layer, hover.index, 0.0f);
        else
          drawElectron(hover.layer, hover.index, true);
      }else{
        drawElectron(hover.layer, hover.index, true);      
        drawEnergy(hover.layer, hover.index, 0.0f);
      }
      break;

      case ATOM : {
      int sai = module->current_atom;
      text(std::to_string(sai + 1), center.x, center.y, (sai > 8 ? (sai > 98 ? 27.0f : 36.f) : 42.f), theme.darker);
      }break;

      default : break;
    }

    drawSigno(_vg);
  }

};
struct AtomButton : _less::Widget {
  std::string button_text;
  bool write;
  Atoms *module;
  bool down = false;
  // NVGcolor theme.layers[2] = {nvgRGB(180,176,166), nvgRGB(8,4,0)};  
  AtomButton(Vec p, bool w, std::string t, Atoms *m) {
    box.pos = p;
    module = m;
    write = w;
    box.size = Vec(40,40);
    button_text = t;
  }
  void init(NVGcontext *vg)override{
    load_font("font/CuteFont-Regular.ttf");
  }
  void render() override{
    float half = box.size.x * 0.5f;
    translate(half, half);
    fill(nvgRGBA(0,0,0,100));
    circle(0, 4, half, FILL);
    NVGcolor fg = write ? theme.output : theme.input;
    NVGcolor bg = write ? theme.input : theme.output_dark;

    fill(bg);
    circle(0, (down ? 2 : 0), half, FILL);
    text(button_text, 0, (down ? 2 : 0), 32.0f, fg);
  }
  void onButton(const event::Button &e) override{
    if(e.button != GLFW_MOUSE_BUTTON_MIDDLE){
    e.consume(this);
    if(e.action == GLFW_PRESS){
      if(write){
        if(e.button == 0)
          module->equalize_layer(INNER);
        else
          module->add_new_atom();
      }else{
        if(e.button == 0)
          module->equalize_layer(OUTER);
        else
          module->delete_atom();
      }
      down = true;
    }else{
      down = false;
    }
  }
  }
};

struct AtomsDarkInput : unless::CvOutput {
  bool input = true;
};

struct AtomSetting : MenuItem {
  bool *setting = NULL;
  Atoms *module = NULL;
  void onAction(const event::Action &e) override {
    *setting = !*setting;
    if(module)
      module->set_dirty();
  }
  void step() override {
    rightText = *setting ? "✔" : "";
    MenuItem::step();
  }
};


struct AtomsWidget : ModuleWidget {
  _less::Panel *pp;
  Atoms *moduleInstance;
  _less::Port<unless::ColoredPort> *atom_port;
  AtomsWidget(Atoms *module){
    moduleInstance = module;
    setModule(module);

    box.size = Vec(RACK_GRID_WIDTH * 18, RACK_GRID_HEIGHT);

    float radius[4] = {box.size.y * RADIUS_INNER, box.size.y * RADIUS_OUTER, RADIUS_ELECTRON, box.size.y * RADIUS_ATOM};
    
    // _less::Theme theme = _less::Theme();


    {
      pp = new _less::Panel(box.size, nvgHSLA(0.5,0.0,0.5,255));
      addChild(pp);
      AtomsDisplay *d = new AtomsDisplay(box, module);
      pp->fb->addChild(d);
      if(module){
        AtomsControls *c = new AtomsControls(box, module, d);
        addChild(c);
      }
    }
    Vec center = Vec(box.size.x * 0.5f, box.size.y * CENTER_OFFSET);
    float r1 = radius[OUTER] * 1.1f;
    {
      _less::Knob<unless::InputKnob> *ik = new _less::Knob<unless::InputKnob>(
          Vec(center.x - r1, center.y - r1), 
          Vec(40.0f, 40.0f),
          module, 
          Atoms::COUNT_PARAM + INNER,
          true
      );
      addParam(ik);

      _less::Knob<unless::OutputKnob> *ok = new _less::Knob<unless::OutputKnob>(
          Vec(center.x + r1, center.y + r1), 
          Vec(40.0f, 40.0f),
          module, 
          Atoms::COUNT_PARAM + OUTER,
          true
      );
      addParam(ok);
    }

    {
      AtomButton *wb = new AtomButton(Vec(center.x + r1 - 20, center.y - r1 - 20), true, "e+", module);
      AtomButton *rb = new AtomButton(Vec(center.x - r1 - 20, center.y + r1 - 20), false, "e-", module);
      addChild(rb);
      addChild(wb);
    }

    addInput(
      new _less::Port<unless::CvInput>(
        Vec(INPUT_X + 7.f, center.y - radius[INNER] + 5.5f), 
        module, 
        Atoms::COUNT_INPUT + INNER
    ));

    addInput(
      new _less::Port<AtomsDarkInput>(
        Vec(box.size.x - PADDING - INPUT_X + 7.f, center.y + radius[INNER] - 6.0f),
        module, 
        Atoms::COUNT_INPUT + OUTER
    ));
    
    addInput(
      new _less::Port<unless::CvInput>(
        Vec(box.size.x - PADDING - INPUT_X + 7.f, center.y - radius[INNER] + 6.f), 
        module, 
        Atoms::OFFSET_INPUT + INNER
    ));


    addInput(
      new _less::Port<AtomsDarkInput>(
        Vec(INPUT_X + 7.0f, center.y + radius[INNER] - 5.5f), 
        module, 
        Atoms::OFFSET_INPUT + OUTER
    ));

    float w = box.size.x - (2 * PADDING);
    float pw = w / 8;
    float pp = 12.f + 6.0f;
    for(int e = 0; e<ELECTRONS; e++){
      int x = e * pw + PADDING + pp - 2.0f;

      {
        Vec pos = Vec(x, pp);
        _less::Port<unless::CvInput> *i = new _less::Port<unless::CvInput>(pos, module, Atoms::INS + e);
        _less::Port<unless::CvOutput> *o = new _less::Port<unless::CvOutput>(pos, module, Atoms::OUTS + ELECTRONS + e);
        addInput(i);
        if(module){
          addOutput(o);
          module->mods[e].port = new DualPort(i, o, true, true, e < module->layers[INNER]->count);
        }
      }

      {      
        int index = 7 - e;
        Vec pos = Vec(x, box.size.y - pp);
        _less::Port<unless::CvInput> *i = new _less::Port<unless::CvInput>(pos, module, Atoms::INS + ELECTRONS + index);
        _less::Port<unless::CvOutput> *o = new _less::Port<unless::CvOutput>(pos, module, Atoms::OUTS + index);
        addOutput(o);
        if(module){
          addInput(i);
          module->mods[ELECTRONS + index].port = new DualPort(i, o, false, false, e < module->layers[OUTER]->count);
        }
      }
    }


    {
      atom_port = new _less::Port<unless::ColoredPort>(
        Vec(center.x + 1.0f, center.y - radius[OUTER] - 31.0f), 
        module, 
        Atoms::ATOM_INPUT
      );
      atom_port->sw->bg = module ? module->atom_color(module->current_atom) : nvgHSLA(0.1f, 0.8f, 0.5f, 255);
      addInput(atom_port);
    }
    if(module != NULL){
      module->set_dirty();
      moduleInstance->update_counts(INNER);
      moduleInstance->update_counts(OUTER);
    }

  }

  void fix_mods(int layer){
    for(int i = 0; i<ELECTRONS; i++){
      int index = layer * ELECTRONS + i;
      int count = moduleInstance->layers[layer]->count;
      if(i <  count 
        || (moduleInstance->mods[index].data.mode > TOGGLE_MOD && moduleInstance->mods[index].port && moduleInstance->mods[index].port->is_input)
        || (moduleInstance->mods[index].data.mode <= TOGGLE_MOD && moduleInstance->mods[index].port && !moduleInstance->mods[index].port->is_input)
        )
        moduleInstance->clear_mod(index);
    } 
  }
  void fix_ports(){
    for(int i = 0; i<ELECTRONS * 2; i++){
      ModMode mode = moduleInstance->atoms.at(moduleInstance->current_atom).mods[i].mode;
      char layer = i >= 8 ? OUTER : INNER;
      char index = layer == INNER ? i : i - 8;
      if(mode != NO_MOD && moduleInstance->mods[i].port){
        moduleInstance->mods[i].port->update_ports(mode <= TOGGLE_MOD, index < moduleInstance->layers[layer]->count);
      }
    }
  }

  void update_ports(){
    for(int i = 0; i < ELECTRONS; i++){
      moduleInstance->mods[i].port->update_ports(moduleInstance->mods[i].port->is_input, i < moduleInstance->layers[INNER]->count);
      moduleInstance->mods[ELECTRONS + i].port->update_ports(moduleInstance->mods[ELECTRONS + i].port->is_input, i < moduleInstance->layers[OUTER]->count);
    }
  }
  void step() override{
    ModuleWidget::step();
    if(moduleInstance && (moduleInstance->animate || moduleInstance->dirty)){
      pp->set_dirty();
      atom_port->sw->bg = moduleInstance->atom_color(moduleInstance->current_atom);
      atom_port->set_dirty();
      update_ports();
      fix_ports();
      fix_mods(INNER);
      fix_mods(OUTER);
      moduleInstance->dirty = false;
    }
  }
  void appendContextMenu(Menu *menu) override {
    Atoms *atoms = dynamic_cast<Atoms*>(module);
    assert(atoms);
  
    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "HELP"));
    
    menu->addChild(construct<AtomSetting>(&MenuItem::text, "hints", &AtomSetting::setting, &atoms->show_hints, &AtomSetting::module, atoms));


    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "LOOKS "));
    menu->addChild(construct<AtomSetting>(&MenuItem::text, "digits", &AtomSetting::setting, &atoms->show_ids, &AtomSetting::module, atoms));
    menu->addChild(construct<AtomSetting>(&MenuItem::text, "animate", &AtomSetting::setting, &atoms->animate));
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, ""));
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "SETTINGS "));
    menu->addChild(construct<AtomSetting>(&MenuItem::text, "audio-rate pairing", &AtomSetting::setting, &atoms->audio_rate_pairing));
    menu->addChild(construct<AtomSetting>(&MenuItem::text, "auto-equalize", &AtomSetting::setting, &atoms->auto_equalize, &AtomSetting::module, atoms));
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, ""));
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "alpha version!"));
  }
};

Model *modelAtoms = createModel<Atoms, AtomsWidget>(SLUG);

/*
TODO
  hint on e+- buttons

  antipop output toggle
  
  colors, remove calls to nvgRGB

  calibrate colors

  write manual, example patches, video


SEQUEL
  third value on electrons A : attack time, T : division/pulsewidth/chance, M : gain, S : offset
  morph between atoms when switching??
  colored electrons??
  flip inner-outer??
*/
