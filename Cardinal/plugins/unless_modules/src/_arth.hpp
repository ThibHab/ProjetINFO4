namespace arth{

enum ComponentType{
  INPUT,
  OUTPUT,
  PARAM,
  CUSTOM,
  COMPONENT_TYPES
};

struct Filer{
  bool loaded = false;
  std::string path;
  Filer(std::string p){
    path = p;
  }
  void save(const std::string filename, json_t *rootJ){
    if (rootJ) {
      FILE *file = fopen(filename.c_str(), "w");
      if (!file){
        WARN("Cannot open '%s' to write", filename.c_str());
        return;
      }
      json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
      json_decref(rootJ);
      fclose(file);
      INFO("Saving art to %s", filename.c_str());
    }else{
      WARN("json to %s is empty", filename.c_str());
    }
  }
  void save(json_t *root){
    save(path, root);
  }
  json_t *load(const std::string filename){
    json_t *rootJ = NULL;
    FILE *file = fopen(filename.c_str(), "r");
    if(file){
      json_error_t error;
      rootJ = json_loadf(file, 0, &error);
      if (rootJ == NULL) {
        WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
      }else{
      }
      fclose(file);
    }else{
      WARN("no art file at %s", filename.c_str());
    }
    return rootJ;
  }
  void check(){}
};

struct Helper{
  template <typename TData>
  static void json_array_to_vector(json_t *a, std::vector<TData> *vs){
    int c = json_array_size(a);
    vs->clear();
    for(int i=0; i<c; i++){
      TData d = TData();
      d.fromJson(json_array_get(a,i));
      vs->push_back(d);
    }
  }
  template <typename TData>
  static json_t *vector_to_json_array(const std::vector<TData> &ls){
    json_t *rootJ = json_array();
    for(auto wd : ls){
      json_array_append(rootJ, wd.toJson());
    }
    return rootJ;
  }

  static json_t *vec_to_json(Vec v){
    // json_t *rootJ = json_object();
    // json_object_set_new(rootJ, "x", json_real(v.x));
    // json_object_set_new(rootJ, "y", json_real(v.y));
    
    json_t *rootJ = json_array();
    json_array_append(rootJ,json_real(v.x));
    json_array_append(rootJ,json_real(v.y));
    return rootJ;
  }
  static Vec vec_from_json(json_t *rootJ){
    // return Vec(json_number_value(json_object_get(rootJ, "x")), json_number_value(json_object_get(rootJ, "y")));
    return Vec(json_number_value(json_array_get(rootJ, 0)), json_number_value(json_array_get(rootJ, 1)));
  }
  static rack::Rect rect_from_json(json_t *rootJ){
    return rack::Rect(
      Helper::vec_from_json(json_object_get(rootJ,"pos")),
      Helper::vec_from_json(json_object_get(rootJ,"size"))
    );
  }

  static json_t *rect_to_json(Rect r){

    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "pos", Helper::vec_to_json(r.pos));
    json_object_set_new(rootJ, "size", Helper::vec_to_json(r.size));
    return rootJ;
  }

  static Rect cornered_rect(Rect r){
    return Rect(r.pos.minus(r.size.mult(0.5f)), r.size);
  }
};


struct EditorLayer : _less::Widget {
  bool enabled = false;
  Vec mouse_pos;

  void printv(Vec v){
    WARN("( %f, %f )", v.x, v.y);
  }
  void onButton(const event::Button &e) override{
    if(enabled){
      e.consume(this);
      if(e.action == GLFW_PRESS){
        click(e.pos, e.button);
      }
    }
  }
  void onDragMove(const event::DragMove & e)override{
    if(enabled){
      // TODO fix
      // float zoom = std::pow(2.f, settings::zoom);
      // Vec p = e.mouseDelta.mult(1.0f / zoom);
      // drag(p);
    }
  }
  void onDragHover(const event::DragHover & e)override{
    mouse_pos = e.pos;
  }
  void onHover(const event::Hover & e)override{
    mouse_pos = e.pos;
  }
  virtual void click(Vec pos, int button){};
  virtual void drag(Vec delta){};
  virtual void undo(){};
};

struct WidgetData {
  int id = -1;
  ComponentType type = CUSTOM;
  rack::Rect box = rack::Rect(0,0,30,30);
  std::string name = "";
  void set_id(int i, std::string n = ""){
    id = i;
    name = n == "" ? std::to_string(i) : n;
  }
  WidgetData(ComponentType t = CUSTOM, rack::Vec p = rack::Vec(0,0), rack::Vec s = rack::Vec(10,10), int i = -1, std::string n = ""){
    type = t;
    box.pos = p;
    box.size = s;
    set_id(i, n);
  }
  template <typename TWidget>
  WidgetData(ComponentType t, TWidget *w, int i){
    box.size = w->box.size.mult(1.0f);
    box.pos = w->box.pos.plus(box.size.mult(0.5f));
    set_id(i);
  }
  WidgetData(json_t *rootJ){
    fromJson(rootJ);
  }
  json_t *toJson(){
    json_t *rootJ = json_object();

    json_object_set_new(rootJ,"box", Helper::rect_to_json(box));
    json_object_set_new(rootJ, "id", json_integer(id));
    json_object_set_new(rootJ, "type", json_integer((int) type));
    json_object_set_new(rootJ, "name", json_string(name.c_str()));

    return rootJ;
  }
  void fromJson(json_t * rootJ){
    id = json_integer_value(json_object_get(rootJ, "id"));
    type = (ComponentType) json_integer_value(json_object_get(rootJ, "type"));
    box = Helper::rect_from_json(json_object_get(rootJ, "box"));
    name = json_string_value(json_object_get(rootJ, "name"));
  }
};

struct ComponentId{
  ComponentType type;
  int id;
  ComponentId(ComponentType t = CUSTOM, int i = -1){
    type = t; id = i;
  }
  bool operator== ( const ComponentId &c){
    return type == c.type && id == c.id;
  }
  static ComponentId of(const WidgetData &wd){
    return ComponentId(wd.type, wd.id);
  }
};
struct Component : ComponentId {
  ComponentType type;
  int id = -1;
  rack::Widget *w;
  Component(ComponentType t, int i, rack::Widget * _w) : ComponentId(t, i){
    id = i;
    type = t;
    w = _w;
  }
  static Component from(rack::Widget *w, int id = -1){
    if(w){
      rack::ParamWidget *pa = dynamic_cast<rack::ParamWidget*>(w);
      if(pa)
        return Component(PARAM, pa->getParamQuantity()->paramId, w);
      rack::PortWidget *pw = dynamic_cast<rack::PortWidget*>(w);
      if(pw)
        return Component(((int)pw->type == 1 ? INPUT : OUTPUT), pw->portId, w);
      else
        return Component(CUSTOM, id, w);
    }
    return Component(CUSTOM, id, NULL);
  }
};
struct AnyComponent {
  WidgetData *data;
  ComponentId id;
  AnyComponent(ComponentId i, WidgetData *w = NULL){
    id = i;
    data = w;
  }
};

struct LayoutData{
  std::string slug = "";
  int width = 10;
  std::vector<WidgetData> params = {};
  std::vector<WidgetData> inputs = {};
  std::vector<WidgetData> outputs = {};
  std::vector<WidgetData> widgets = {};
  LayoutData(){

  }
  LayoutData(json_t* rootJ){
    fromJson(rootJ);
  }
  json_t *toJsonArray(const std::vector<WidgetData> &ls){
    json_t *rootJ = json_array();
    for(auto wd : ls){
      json_array_append(rootJ, wd.toJson());
    }
    return rootJ;
  }
  void sanitize(int n, std::vector<WidgetData> *ls, float x_offset){
    float pad = 50.0f;
    float w = 30.0f;
    int l = ls->size();
    int h = (380.0f - pad * 2.0f) / (float) n;
    if(l < n){
      // WARN("sanitizing widgets");
      for(int i = 0; i< n - l; i++)
        ls->push_back(WidgetData(CUSTOM, Vec(pad + w * x_offset, pad + h * (l + i)), Vec(w, w), l+i));
    }
  }
  void sanitize(int is, int os, int ps, int ws = 0){
    sanitize(is, &inputs, 0.0f);
    sanitize(os, &outputs, 1.0f);
    sanitize(ps, &params, 2.0f);
    sanitize(ws, &widgets, 3.0f);
  }
  json_t *toJson(){
    json_t *rootJ = json_object();

    json_object_set_new(rootJ, "slug", json_string(slug.c_str()));
    json_object_set_new(rootJ, "width", json_integer(width));
    json_object_set_new(rootJ, "params", toJsonArray(params));
    json_object_set_new(rootJ, "inputs", toJsonArray(inputs));
    json_object_set_new(rootJ, "outputs", toJsonArray(outputs));
    json_object_set_new(rootJ, "widgets", toJsonArray(widgets));

    return rootJ;
  }
  void load_widget_vectors(json_t *rootJ, std::string type, std::vector<WidgetData> *ls){
    ls->clear();
    json_t *ps = json_object_get(rootJ, type.c_str());
    int pc = json_array_size(ps);
    for(int i = 0; i<pc; i++){
      json_t *wd = json_array_get(ps, i);
      WidgetData d = WidgetData(wd);
      ls->push_back(d);
    }
  }
  void fromJson(json_t *rootJ){
    slug = json_string_value(json_object_get(rootJ, "slug"));
    width = json_integer_value(json_object_get(rootJ, "width"));
    load_widget_vectors(rootJ, "params", &params);
    load_widget_vectors(rootJ, "inputs", &inputs);
    load_widget_vectors(rootJ, "outputs", &outputs);
    load_widget_vectors(rootJ, "widgets", &widgets);

  }

  WidgetData *data_of(ComponentId c){
    switch(c.type){
      case INPUT : return &inputs.at(c.id); break;
      case OUTPUT : return &outputs.at(c.id); break;
      case PARAM : return &params.at(c.id); break;
      case CUSTOM : return &widgets.at(c.id); break;
      default : return NULL;
    }
  }

  struct component_by_id{
    bool operator()(WidgetData a, WidgetData b)const{
      return a.id < b.id;
    }
  };
  static LayoutData from(rack::ModuleWidget *mw){
    LayoutData data;
    for(auto p : mw->getParams()){
      // printf("( %f, %f )\n", p->box.size.x, p->box.size.y);
      data.params.push_back(WidgetData(
        PARAM,
        p->box.pos.plus(p->box.size.mult(0.5)),
        p->box.size,
        p->getParamQuantity()->paramId,
        p->getParamQuantity()->getLabel()
      ));
    }
    for(auto i : mw->getInputs()){
      std::string n = "input : " + std::to_string(i->portId);
      data.inputs.push_back(WidgetData(
        INPUT,
        i->box.pos.plus(i->box.size.mult(0.5)),
        i->box.size,
        i->portId,
        n
      ));
    }
    for(auto o : mw->getOutputs()){
      std::string n = "output : " + std::to_string(o->portId);
      data.outputs.push_back(WidgetData(
        OUTPUT,
        o->box.pos.plus(o->box.size.mult(0.5)),
        o->box.size,
        o->portId,
        n
      ));
    }
    int ci = 0;
    for(auto w : mw->children){

      PortWidget *po = dynamic_cast<PortWidget*>(w);
      ParamWidget *pa = dynamic_cast<ParamWidget*>(w);
      if(ci == 0 || po || pa){

      }else{
        EditorLayer *ed = dynamic_cast<EditorLayer*>(w);
        if(!ed){
          data.widgets.push_back(WidgetData(CUSTOM, w,data.widgets.size()));

          // WARN("widget child\n");
        }
      }
      ci++;
    }
    data.width = floor(mw->box.size.x / rack::app::RACK_GRID_WIDTH);
    data.slug = mw->module->model->slug;


    std::sort(data.inputs.begin(), data.inputs.end(), component_by_id());
    std::sort(data.outputs.begin(), data.outputs.end(), component_by_id());
    std::sort(data.params.begin(), data.params.end(), component_by_id());
    return data;
  }
};

struct LabelData : WidgetData {
  std::string text = "unless";
  float font_size = 14.0f;
  LabelData(){

  }
  LabelData(Vec p, std::string t){
    box.pos = p;
    text = t;
  }
};

struct DrawnWidgetData{
  int color = 0;
  int width = 0;
  _less::Widget::PathMode fill_mode;

  void fromJson(json_t *rootJ){
    color = json_integer_value(json_object_get(rootJ, "color"));
    fill_mode = (_less::Widget::PathMode) json_integer_value(json_object_get(rootJ, "fill_mode"));
    width = json_integer_value(json_object_get(rootJ, "width"));
  }
  json_t *toJson(){
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "color", json_integer(color));
    json_object_set_new(rootJ, "width", json_integer(width));
    json_object_set_new(rootJ, "fill_mode", json_integer(fill_mode));
    return rootJ;
  }
};

struct LineShapesData : DrawnWidgetData{
  std::vector<Vec> points;
  json_t *toJson(){
    json_t *rootJ = json_object();
    json_t *ps = json_array();
    for(auto p : points)
      json_array_append(ps, Helper::vec_to_json(p));
    json_object_set_new(rootJ, "points", ps);
    return rootJ;
    // return Helper::vector_to_json_array(points)
  }
  void fromJson(json_t *rootJ) {
    DrawnWidgetData::fromJson(rootJ);
    json_t *ps = json_object_get(rootJ, "points");
    int c = json_array_size(ps);
    points.clear();
    for(int i=0; i<c; i++)
      points.push_back(Helper::vec_from_json(json_array_get(ps,i)));
  }
};

struct RectangleData{
  Rect rect;
  json_t *toJson(){
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "rect", Helper::rect_to_json(rect));
    return rootJ;
  }
  void fromJson(json_t *rootJ){
    rect = Helper::rect_from_json(json_object_get(rootJ, "rect"));
  }
};

struct RectangleShapesData : DrawnWidgetData{
  std::vector<RectangleData> rects;
  json_t *toJson(){
    json_t *rootJ = DrawnWidgetData::toJson();
    json_t *rs = json_array();
    for(auto r : rects){
      json_array_append(rs, r.toJson());
    }
    json_object_set_new(rootJ, "rects", rs);
    return rootJ;
  }
  void fromJson(json_t *rootJ){
    DrawnWidgetData::fromJson(rootJ);
    json_t *ps = json_object_get(rootJ, "rects");
    int c = json_array_size(ps);
    rects.clear();
    for(int i=0; i<c; i++){
      RectangleData d = RectangleData();
      d.fromJson(json_array_get(ps,i));
      rects.push_back(d);
    }
  }
};

struct LayerData{
  std::vector<LineShapesData> lineshapes;
  std::vector<RectangleShapesData> rectangles;

  std::vector<LabelData> labels;

  json_t *toJson(){
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "labels", Helper::vector_to_json_array(labels));
    json_object_set_new(rootJ, "lineshapes", Helper::vector_to_json_array(lineshapes));
    json_object_set_new(rootJ, "rectangles", Helper::vector_to_json_array(rectangles));

    return rootJ;
  }
  void fromJson(json_t * rootJ){
    Helper::json_array_to_vector(json_object_get(rootJ, "lineshapes"), &lineshapes);
    Helper::json_array_to_vector(json_object_get(rootJ, "rectangles"), &rectangles);
    Helper::json_array_to_vector(json_object_get(rootJ, "labels"), &labels);
  }
};
struct LayerWidget : _less::Widget{
  LayerData data;
  LayerWidget(LayerData d){
    data = d;
  }
  void render() override{
    stroke(theme.fg);
    begin();
    for(auto lsd : data.lineshapes){
      for(auto p : lsd.points)
        point(p);
    }
    end();
  }
};
struct ArtData{
  // art file loader
  std::vector<LayerData> layers = { LayerData() };
  LayoutData layout;
  template <typename ArtT>
  static void save(ArtT d){
    Filer filer = Filer(rack::asset::plugin(pluginInstance, "art/"+ d.layout.slug + ".art"));
    json_t *rootJ = json_object();
    json_object_set_new(rootJ,"layout", d.layout.toJson());
    json_object_set_new(rootJ,"layers", Helper::vector_to_json_array(d.layers));
    filer.save(rootJ);
  }
  static ArtData load(std::string slug){
    ArtData art;
    std::string path = rack::asset::plugin(pluginInstance, "art/"+ slug + ".art");
    Filer filer = Filer(path);
    json_t *art_json = filer.load(path);
    if(art_json){
      art.layout.fromJson(json_object_get(art_json, "layout"));
      INFO(("Loaded ART " + slug + ".art").c_str());
      json_decref(art_json);
    }else{
      // WARN(".art data not found!");
    }
    return art;
  }
};

struct Art : ArtData{
  std::shared_ptr<Font> font = NULL;  
  ModuleWidget *mw;

  int widget_count = 0;

  template <typename TModuleWidget>
  static Art load(TModuleWidget *m, std::string slug, int is, int os, int ps, int ls = 0){
    ModuleWidget *mw = dynamic_cast<ModuleWidget*>(m);
    assert(mw);
    Art art;
    art.widget_count = 0;
    ArtData a = ArtData::load(slug);
    art.mw = mw;
    art.layout = a.layout;
    art.layers = a.layers;
    art.layout.sanitize(is, os, ps, ls);
    m->box.size = Vec(art.layout.width * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    if(art.layout.slug == ""){
      art.layout.slug = slug;
    }
    // ArtData::save(art);
    return art;
  }
    // if(set_panel){
    //   // set width from layout
    //   box.size = Vec(RACK_GRID_WIDTH * art.layout.width, RACK_GRID_HEIGHT);
    //   // create layer widgets
    //   for(auto l : art.layers){
    //     LayerWidget *lw = new LayerWidget(l);
    //     layers.push_back(lw);
    //     addChild(lw);
    //   }
    // }

  // void load_font(std::string fp){
  //   font = APP->window->loadFont(asset::plugin(pluginInstance, fp.c_str()));
  //   // font = nvgCreateFont(_vg, "default", fp.c_str());
  // }
  // void add_labels(std::string f){
  //   for(auto l : layers){
  //   }
  // }
  template <typename TWidget>
  TWidget *panel(TWidget *p){
    p->box = Rect(0.0f, 0.0f, layout.width * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    mw->addChild(p);
    return p;
  }

  template <typename TPortWidget>
  void input(int id){
    mw->addInput(new TPortWidget(layout.inputs[id].box.pos, mw->module, id));
  }

  template <typename TPortWidget>
  void output(int id){
    mw->addOutput(new TPortWidget(layout.outputs[id].box.pos, mw->module, id));
  }

  template <typename TParamWidget>//snap = momentary
  TParamWidget *param(int id, bool snap = false, bool centered = true){
    TParamWidget *p = new TParamWidget(layout.params[id].box.pos, layout.params[id].box.size, mw->module, id, snap, centered);
    mw->addParam(p);
    return p;
  }

  template <typename TWidget>
  void widget(TWidget *w, bool add = true){
    if(widget_count < (int)layout.widgets.size()){
      // WARN("setting widget size from layout id : %d", widget_count);
      w->box = layout.widgets[widget_count].box;
      w->box.pos = w->box.pos.minus(w->box.size.mult(0.5f));
    }else{
      WARN("widget [%d] does not exist, adding new...", widget_count);
      layout.widgets.push_back(WidgetData(CUSTOM, w, widget_count));
    }
    widget_count++;

    if(add)
      mw->addChild(w);  
  }

};
}
