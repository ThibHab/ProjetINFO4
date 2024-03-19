namespace unless{

struct ChainkovTheme{
  NVGcolor suit,
  outline,
  shirt,
  tie,
  skin,
  chain,
  back_bg,
  back_fg,
  footer_fg,
  back_fg_text;

  bool minimal_skin = false;
  bool dark = false;

  static NVGcolor random_color(){
    return nvgHSLA(random::uniform(), random::uniform(), random::uniform(), 255);
  }

  json_t *color_to_json(NVGcolor c){
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "r", json_real(c.r));
    json_object_set_new(rootJ, "g", json_real(c.g));
    json_object_set_new(rootJ, "b", json_real(c.b));
    json_object_set_new(rootJ, "a", json_real(c.a));
    return rootJ;
  }
  NVGcolor color_from_json(json_t *c){
    return nvgRGBAf(json_number_value(json_object_get(c, "r")), json_number_value(json_object_get(c, "g")), json_number_value(json_object_get(c, "b")), json_number_value(json_object_get(c, "a")));
  }
  void load_json(json_t *rootJ){
    outline = color_from_json(json_object_get(rootJ, "outline"));
    suit = color_from_json(json_object_get(rootJ, "suit"));
    shirt = color_from_json(json_object_get(rootJ, "shirt"));
    tie = color_from_json(json_object_get(rootJ, "tie"));
    skin = color_from_json(json_object_get(rootJ, "skin"));
    chain = color_from_json(json_object_get(rootJ, "chain"));
    back_bg = color_from_json(json_object_get(rootJ, "back_bg"));
    back_fg = color_from_json(json_object_get(rootJ, "back_fg"));
    back_fg_text = color_from_json(json_object_get(rootJ, "back_fg_text"));
    minimal_skin = json_boolean_value(json_object_get(rootJ, "minimal_skin"));
    dark = json_boolean_value(json_object_get(rootJ, "dark"));
  }
  json_t *to_json(){
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "outline", color_to_json(outline));
    json_object_set_new(rootJ, "suit", color_to_json(suit));
    json_object_set_new(rootJ, "shirt", color_to_json(shirt));
    json_object_set_new(rootJ, "tie", color_to_json(tie));
    json_object_set_new(rootJ, "skin", color_to_json(skin));
    json_object_set_new(rootJ, "chain", color_to_json(chain));
    json_object_set_new(rootJ, "back_bg", color_to_json(back_bg));
    json_object_set_new(rootJ, "back_fg", color_to_json(back_fg));
    json_object_set_new(rootJ, "back_fg_text", color_to_json(back_fg_text));
    json_object_set_new(rootJ, "minimal_skin", json_boolean(minimal_skin));
    json_object_set_new(rootJ, "dark", json_boolean(dark));
    return rootJ;
  }
  float lightness(NVGcolor c){
    return (c.r + c.g + c.b) / 3.0f;
  }
  void randomize(){
    suit = random_color();
    if(lightness(suit) > 0.4f)
      outline = nvgHSLA(0,0,0,255);
    else
      outline = nvgHSLA(0,0,0.6f,255);
    shirt = random_color();
    tie = random_color();
    skin = nvgRGBA(241, 220, 169, 255);
    chain = random_color();
  }
  ChainkovTheme(){
    randomize();
  }
};
struct ChainkovHead : _less::Widget {
  float _randomness = 0.1f;
  bool _face_wiggle = true;
  bool _minimal_skin = false;

  float *randomness = NULL;
  bool *minimal_skin = NULL;
  bool *face_wiggle = NULL;

  Vec face[6] = {Vec(-0.28, -0.2), Vec(-0.07, -0.22), Vec(-0.01, 0.13), Vec(0.032, 0.12), Vec(0.1, -0.26), Vec(0.28, -0.23)};
  Vec mouth[4] = {Vec(-0.07, 0.25), Vec(0.07, 0.25)};//{Vec(-0.3, 0.1),Vec(-0.3, 0.1),Vec(-0.3, 0.1),Vec(-0.3, 0.1)};
  NVGcolor outline = nvgRGBA(0x11, 0x11, 0x10, 0xff);
  ChainkovTheme *chainkov_theme;
  float minAngle, maxAngle;
  ChainkovHead(float x, float y, ChainkovTheme *t){
    face_wiggle = &_face_wiggle;
    minimal_skin = &_minimal_skin;
    randomness = &_randomness;
    chainkov_theme = t;
    box.pos = Vec(x, y);
    minAngle = -3.1415f * 0.5f + 3.1415f * 0.1f;
    maxAngle = 3.1415f * 0.5f - 3.1415f * 0.1f;
  }
  float sbr(float s){
    s *= 0.1f;
    return -s + random::uniform() * s;
  }
  void init(NVGcontext *vg)override {
    
  }
  void render() override{
    // if(module){
    //   minimal_skin = module->theme.minimal_skin; 
    //   r = module->randomness;
    // }
    float r = *randomness;
    float s = box.size.x;
    float half = s * 0.5f;
    if(*minimal_skin){
      translate(half, half);
      fill(nvgRGBA(0,0,0,70));
      circle(0, half * 0.2, half);

      fill(theme.output);
      circle(0, 0, half);

      translate(0.05f, 0.0f);
      rotate(1.0f * (-3.1415f * 1.2 + minAngle + (maxAngle - minAngle) * -1.0f * ilerpf(1.0f, -1.0f, r)));
      scale(s,s);

      stroke(theme.fg_light);
      stroke_width(1.0f / half);
      line(0.0f, -0.1f, 0.0f, -0.5f);
    }else{ 
      translate(half, half);
      fill(chainkov_theme->skin);
      circle(0, 0, half);

      translate(0.05f,0.0f);
      rotate(-3.1415f * 1.21 + minAngle + (maxAngle - minAngle) * -1.0f * ilerpf(1.0f, -1.0f, r));
      scale(s, s);
      
      r = rescale(r, -1.0f, 1.0f, 0.0f, 1.0f);
      r *= r;
      stroke(outline);
      stroke_width(1.0f / s);


      begin();
      if(*face_wiggle){
        point(face[0].x + sbr(r), face[0].y + sbr(r));
        for(int i = 1; i<6; i++)
          point(face[i].x + sbr(r), face[i].y + sbr(r));
        move(mouth[0].x + sbr(r), mouth[0].y + sbr(r));
        for(int i = 1; i<2; i++)
          point(mouth[i].x + sbr(r), mouth[i].y + sbr(r));
      }else{
        point(face[0].x, face[0].y);
        for(int i = 1; i<6; i++)
          point(face[i].x, face[i].y);
        move(mouth[0].x, mouth[0].y);
        for(int i = 1; i<2; i++)
          point(mouth[i].x, mouth[i].y);
      }
      end();
    }
  }
  void render_lights() override{
    float b = rack::settings::rackBrightness;
    if(b < 1.0){
      float s = box.size.x;
      float half = s * 0.5f;
      float y = 0.04f;
      float eyer = 2.0f;
      translate(half, half);
      translate(0.05f,0.0f);
      rotate(-3.1415f * 1.21 + minAngle + (maxAngle - minAngle) * -1.0f * ilerpf(1.0f, -1.0f, *randomness));
      // scale(s, s);
      
      fill(lerp(theme.rainbow_color(0.0f), theme.transparent, b));
      circle(lerpf(face[0].x, face[1].x, 0.5f) * s, (face[0].y + y) * s, eyer);
      circle(lerpf(face[4].x, face[5].x, 0.5f) * s, (face[4].y + y) * s, eyer);
    }
  }
};

struct ChainkovPanel : _less::Widget{
  NVGcolor suit = hsl(0.42f,0.4f,0.4f);
  NVGcolor outline = hsl(0.42f,0.4f,0.4f);
  NVGcolor footer_color = hsl(0.1f,0.05f,0.3f);

  unless::ChainkovTheme *chainkov_theme;
  _less::Panel *panel;
  bool minimal_skin = false;
  float footer = 20.0f;
  float shoulder_y = 262.f;
  bool _keep_clothes = false;
  bool *keep_clothes = NULL;
  ChainkovPanel(Rect b, unless::ChainkovTheme *t, _less::Panel *p, bool *kc){
    if(kc){
      keep_clothes = kc;
    }else{
      keep_clothes = &_keep_clothes;
    }

    box = b;
    chainkov_theme = t;
    panel = p;
  }
  void set_dirty(){
    panel->set_dirty();
  }
  void onButton(const event::Button &e) override{
    if(e.action == GLFW_PRESS && e.button == 0){
      if(e.pos.y > box.size.y - footer || e.pos.y < footer){
        if(!*keep_clothes){
          chainkov_theme->randomize();
          panel->set_dirty();
        }
      }
    }
  }
  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
  }
  void drawPanel(bool fg = true){
    push();
    fill(theme.bg_dark);
    rect(box);
    if(fg){
      fill(theme.bg);
      float y = 80.0f;
      rect(8.0f, y, 104.0f, height - y);
    }
  }
  void drawLabels(){
    std::string order = "order";
    std::string chaos = "chaos";
    std::string learn = "LEARN";
    std::string clear = "CLEAR";
    float s = 16.0f;
    fill(theme.fg);
    float x = 42.0f;
    float xs = 9.0f;
    float xfs = 10.0f;
    float xsy = -156.0f;
    float cm = chainkov_theme->minimal_skin ? -1.0f : 1.0f;
    translate(center.x, center.y - 3.0f);
    for(int i = 0; i<5; i++){
      text(chaos.substr(i,1).c_str(), -x * cm, s * i * 0.9f, s);
      text(order.substr(i,1).c_str(), x * cm, s * i * 0.9f, s);
      text(learn.substr(i,1).c_str(), -xs, xsy + xfs * i * 0.9f, xfs);
      text(clear.substr(i,1).c_str(), xs, xsy + xfs * i * 0.9f, xfs);
    }
    fill(theme.input);
    text("mr. chainkov", 0, - center.y + 12.0f, s * 0.9f);
    pop();
  }
  void drawPortLabels(NVGcolor c){
    push();
    float x = width / 3.6f;
    float s = 16.0f;
    translate(center.x, height - 60.0f);
    fill(c);
    text("cv", -x, 0, s);
    text("gt",  x, 0, s);
    float y = 15.0f;
    text("tr",  0, y, s);
    text("gt",  0, y + s, s);
    pop();
  }
  void drawArm(Vec pit, float side = 1.0f){
    nvgBeginPath(_vg);
    nvgMoveTo(_vg, pit.x * side, pit.y);
    nvgBezierTo(_vg, pit.x * side, pit.y + 0.1f * (height - pit.y), width * 0.5f * side, height, (pit.x - 30.0f) * side, height);
    end();
  }

  void drawShirt(){
    push();
    float w2 = width * 0.5f;
    float shoulder_h = 24.f;
    translate(center.x, shoulder_y);
    fill(chainkov_theme->shirt);
    point(-w2, 0);
    point(0, -shoulder_h);
    point(w2, 0);
    point(w2, height - shoulder_y);
    point(-w2, height - shoulder_y);
    close(FILL);
    stroke(chainkov_theme->outline);
    line(-4.0f, 0, -4.0f, height - shoulder_y - 60.0f);
    fill(chainkov_theme->outline);
    circle(0, 10.0f, 2.f, FILL);
    circle(0, 25.0f, 2.f, FILL);
    pop();

  }

  void drawSuit(){
    push();
    stroke(chainkov_theme->outline);
    fill(chainkov_theme->suit);
    translate(center.x, 0.f);
    float w2 = width * 0.5f;
    float shoulder_h = 24.f;

    float collar_t = 0.63f;
    float collar_w = 0.05f;
    float x = lerpf(-w2, 0, collar_t);
    float y = lerpf(shoulder_y, shoulder_y - shoulder_h, collar_t);
    float nx = lerpf(-w2, 0, collar_t + collar_w);
    float ny = lerpf(shoulder_y, shoulder_y - shoulder_h, collar_t + collar_w);
    float suit_close = height - 60.0f;

    point(-w2, shoulder_y);
    point(nx, ny);
    point(0, suit_close);
    point(-nx, ny);
    point(w2, shoulder_y);
    point(w2, height);
    point(-w2, height);
    close(BOTH);


    std::vector<Vec> collar = {
      Vec(x, y),
      Vec(x - 5.0f, y + 27.0f),
      Vec(x + 4.0f, y + 30.0f),
      Vec(x - 4.0f, y + 35.0f),
      Vec(0, suit_close),
      Vec(nx, ny)
    };

    for(auto v : collar){
      point(v);
    }
    close();

    for(auto v : collar){
      point(Vec(v.x * -1.0f, v.y));
    }
    close();

    Vec armpit = Vec(w2 * 0.85f, suit_close - 30.f);
    drawArm(armpit);
    drawArm(armpit, -1.0f);

    fill(chainkov_theme->shirt);

    pop();
  }

  void drawMinimalLines(){
    push();
    float w2 = width * 0.53f;
    translate(center.x, shoulder_y + 14.0f);
    fill(theme.bg_dark);
    float y = -18.0f;
    rect(-w2, y, w2 * 2.0f, width);
    fill(theme.bg_dark);
    stroke_width(10.0f);
    stroke(theme.bg);

    line(0, -20, 0, 40);
    line(0, 40, 40, 70);
    line(0, 40, -40, 70);
    pop();
  }
  void drawFooter(){
    fill(theme.fg);
    rect(0, height - footer, width, footer);
    fill(theme.bg);
    text("unless", width * 0.5f, height - footer * 0.5f, footer * 0.8f);
  }
  void render() override{
    drawPanel(!chainkov_theme->minimal_skin);
    drawLabels();
    if(!chainkov_theme->minimal_skin){
      stroke(nvgRGB(255,0,0));
      drawShirt();
      drawSuit();
      drawPortLabels(chainkov_theme->outline);
    }else{
      drawMinimalLines();
      drawPortLabels(theme.fg);
    }
    drawFooter();
  }
};
}