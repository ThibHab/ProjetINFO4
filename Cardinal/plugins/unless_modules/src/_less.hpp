#pragma once
// base Widget wrapper for procedurally drawn widgets
// less typing when drawing with nvg functions
  /*
  struct ExampleWidget : _less::Widget{
    void init(NVGcontext *vg) override{
      set_box(0,0,100,100);
      color_mode(HSLA);
    }

    void render() override{
      fill(0, 0, 0, 1);
      back();
      fill(0, 0, 0.8f, 0.5f);
      stroke(0, 0, 0.8f, 0.5f);
      circle(center.x, center.y, box.size.x * 0.5f, BOTH);
    }
  };

  */
namespace _less{
  struct Color{
    float h, s, l, a;
    Color(float _h = 0.01f, float _s = 0.02f, float _l = 0.4f, float _a = 1.0f){
      h = _h; s = _s; l = _l; a = _a;
    }
    NVGcolor val(){
      return nvgHSLA(h, s, l, a * 255);
    }
    json_t *save(){
      json_t *rootJ = json_object();
      json_object_set_new(rootJ, "h", json_real(h));
      json_object_set_new(rootJ, "s", json_real(s));
      json_object_set_new(rootJ, "l", json_real(l));
      json_object_set_new(rootJ, "a", json_real(a));
      return rootJ;
    }
    Color(json_t *c){
      h = json_number_value(json_object_get(c, "h"));
      s = json_number_value(json_object_get(c, "s"));
      l = json_number_value(json_object_get(c, "l"));
      a = json_number_value(json_object_get(c, "a"));
    }
  };

  struct Circle{
    float x, y, r;
    Circle(float _x = 0.0f, float _y = 0.0f, float _r = 1.0f){
      x = _x; y = _y; r = _r;
    }
    bool inside(float px, float py){
      return sqrt((x - px) * (x - px) + (y - py) * (y - py)) < r;
    }
  };
  struct Rect{
    float x, y, w, h;
    Rect(float _x = 0.0f, float _y = 0.0f, float _w = 1.0f, float _h = 1.0f){
      x = _x; y = _y; w = _w; h = _h;
    }
    bool inside(float px, float py){
      return px > x && px < x + w && py > y && py < y + h;
    }
  };

  struct Plane : rack::TransparentWidget {
    NVGcolor bg;
    void draw(const DrawArgs &args) override{
      NVGcontext *vg = args.vg;
      nvgBeginPath(vg);
      nvgRect(vg, 0,0,box.size.x, box.size.y);
      nvgFillColor(vg, bg);
      nvgFill(vg);
    }
    Plane(Vec s, NVGcolor c){
      box.size = s;
      bg = c;
    }
  };


  struct Gradient{
    const NVGcolor missing_color = nvgRGB(255,0,255);
    std::vector<NVGcolor> colors;
    Gradient(std::vector<NVGcolor> cs){
      colors = cs;
    }
    int get_index(float c, float t){
      if(c > 0)
        return floor((t < 0 ? 1.0f - fmodf(abs(t), 1.0f) : fmodf(t, 1.0f)) * c);
      return -1;
    }

    NVGcolor at(float t){
      int size = colors.size();
      int i = get_index(size, t);
      switch(size){
        case 0 : return missing_color;
        case 1 : return colors[0];
        default : 
        if(i > -1){
          float st = 1.0f / (float) size;
          float tt = (t - st * i) / st;
          return nvgLerpRGBA(colors[i], colors[(i + 1) % size], tt);
        }
        return missing_color;
      }
    }
    NVGcolor get(int i){
      int size = colors.size();
      if(size > 0){
        if(i < 0)
          return colors[size + (i % size)];
        else
          return colors[i % size];
      }
      return missing_color;
    }
  };

  
  struct Widget : rack::Widget{
    enum ColorMode{
      RGBA,
      HSLA,
      RGBAF
    };
    enum PathMode{
      STROKE,
      FILL,
      BOTH
    };

    NVGcontext *_vg;
    bool draggable = false;
    // Theme theme = Theme();
    ColorMode color_mode_setting = RGBA;
    bool started_path = false;
    bool first_frame = true;

    float width, height;
    bool dirty = true;
    Vec center = Vec(0,0);

    std::shared_ptr<Font> font = NULL;
    std::string font_path;
    bool using_font = false;

    Vec mouse = Vec();

    NVGcolor white = nvgRGB(255,255,255);
    NVGcolor black = nvgRGB(0,0,0);
    NVGcolor red = nvgRGB(255,0,0);
    NVGcolor green = nvgRGB(0,255,0);
    NVGcolor blue = nvgRGB(0,0,255);
    static const NVGalign center_align = (NVGalign) 0b00010010;
    static const NVGalign right_align = (NVGalign) 0b00010100;
    static const NVGalign left_align = (NVGalign) 0b00010001;


    void set_box(rack::Rect b){
      box = b;
      set_box();
    }

    template <typename T>
    void set_box(T x = 0, T y = 0, T w = 141, T h = 141){
      width = w;
      height = h;
      center = Vec(width * 0.5f,height * 0.5f);
      box.pos = Vec(x, y);
      box.size = Vec(width, height);
    }
    template <typename T>
    void set_width(T w = 4.0f){
      set_box(box.pos.x, box.pos.y, RACK_GRID_WIDTH * w, box.size.y);
    }
    template <typename T>
    void set_box_center(T x = 0, T y = 0, T w = 141, T h = 141){
      width = w;
      height = h;
      center = Vec(x, y);
      box.size = Vec(width, height);
      box.pos = center.minus(box.size.mult(0.5f));
    }
    void set_box(){
      width = box.size.x;
      height = box.size.y;
      center = Vec(width * 0.5f,height * 0.5f);
    }

    void set_box(Vec v){
      box.size = v;
      set_box();
    }
    virtual void init(NVGcontext *vg){
      // printf("no init function\n");
    }
    virtual void render(){
      // printf("no render function\n");
    };
    virtual void render_lights(){

    }
    virtual void render_light_cover(){

    }
    virtual void frame(int i){};


    virtual void click(){};
    virtual void right(){};
    virtual void middle(){};



    void fill(const NVGcolor &c){
      nvgFillColor(_vg, c);
    }
    void fill(NVGcolor *c){
      nvgFillColor(_vg, *c);
    }
    void fill(NVGpaint p){
      nvgFillPaint(_vg,p);
    }
    void stroke(NVGpaint p){
      nvgStrokePaint(_vg,p);
    }
    void stroke(const NVGcolor &c){
      nvgStrokeColor(_vg, c);
    }
    void stroke(NVGcolor *c){
      nvgStrokeColor(_vg, *c);
    }
    template <typename T>
    void fill(T a, T b, T c, T _a = 1){
      switch(color_mode_setting){
        case RGBA : 
        nvgFillColor(_vg, nvgRGBA(a,b,c,_a));
        break;
        case HSLA : 
        nvgFillColor(_vg, nvgHSLA(a,b,c,_a * 255.0f));
        break;
        case RGBAF : 
        nvgFillColor(_vg, nvgRGBAf(a,b,c,_a));
        break;
      }
    };
    template <typename T>
    void stroke(T a, T b, T c, T _a = 1){
      switch(color_mode_setting){
        case RGBA : 
        nvgStrokeColor(_vg, nvgRGBA(a,b,c,_a));
        break;
        case HSLA : 
        nvgStrokeColor(_vg, nvgHSLA(a,b,c,_a * 255.0f));
        break;
        case RGBAF : 
        nvgStrokeColor(_vg, nvgRGBAf(a,b,c,_a));
        break;
      }
    };

    template <typename T>
    void rgb(T r, T g, T b, T a = 255){
      NVGcolor c = nvgRGBA(r,g,b,a);
      nvgFillColor(_vg, c);
      nvgStrokeColor(_vg, c);
    };
    NVGcolor lerp(NVGcolor a, NVGcolor b, float t){
      return nvgLerpRGBA(a, b, t);
    }

    NVGcolor transp(NVGcolor a, float t){
      return nvgTransRGBA(a, t * 255.0f);
    }
    template <typename T>
    NVGcolor color(T a, T b, T c, T _a = 255){
      switch(color_mode_setting){
        case RGBA : 
        return nvgRGBA(a,b,c,_a);
        break;
        case HSLA : 
        return nvgHSLA(a,b,c,_a * 255);
        break;
        case RGBAF : 
        return nvgRGBAf(a,b,c,_a);
        break;
        default :
        return nvgRGB(255,0,255);
      }
    };

    void color(NVGcolor c, PathMode mode = BOTH){
      switch(mode){
        case STROKE : stroke(c); break;
        case FILL : fill(c); break;
        case BOTH : stroke(c); fill(c); break;
      }
    }

    template <typename T>
    NVGcolor hsl(T h, T s, T l, T a = 1.0){
      return nvgHSLA(h,s,l,a * 255);
      // nvgFillColor(_vg, c);
      // nvgStrokeColor(_vg, c);
    };

    void alpha(float a){
      nvgGlobalAlpha(_vg, a);
    };

    void color_mode(ColorMode c){
      color_mode_setting = c;
    }

    template <typename T>
    void translate(T x, T y){
      nvgTranslate(_vg, x, y);
    };


    void translate(Vec v){
      nvgTranslate(_vg, v.x, v.y);
    };


    template <typename T>
    void scale(T x, T y){
      nvgScale(_vg, x, y);
    };


    void scale(Vec v){
      nvgScale(_vg, v.x, v.y);
    };

    template <typename T>
    void rotate(T a){
      nvgRotate(_vg, a);
    };

    template <typename T>
    void rotate_pi(T a){
      nvgRotate(_vg, a * 3.14159265358979323846f);
    };

    void push(){
      nvgSave(_vg);
    }
    void pop(){
      nvgRestore(_vg);
    }

    void load_font(std::string fp){
      font_path = asset::plugin(pluginInstance, fp.c_str());
      using_font = true;
      // font = APP->window->loadFont();
      // font = nvgCreateFont(_vg, "default", fp.c_str());
    }

    void text(std::string t, float x, float y, float s, NVGalign a = center_align){
      nvgTextAlign(_vg, a);
      if(font)
        nvgFontFaceId(_vg, font->handle);
      nvgFontSize(_vg, s);
      nvgText(_vg, x, y, t.c_str(), NULL);
    }
    void font_face(int handle){
      nvgFontFaceId(_vg, handle);
    }
    void text(std::string t, float x, float y, float s, NVGcolor c, NVGalign a = center_align){
      nvgTextAlign(_vg, a);
      if(font)
        nvgFontFaceId(_vg, font->handle);
      nvgFontSize(_vg, s);
      fill(c);
      nvgText(_vg, x, y, t.c_str(), NULL);
    }

    void text(std::string t, Vec p, float s, NVGalign a = center_align){
      text(t, p.x, p.y, s, a);
    }



    void move(float x, float y){
      nvgMoveTo(_vg, x, y);
    }


    void move(Vec p){
      nvgMoveTo(_vg, p.x, p.y);
    }

    void path(float x = 0.0f, float y = 0.0f){
      nvgBeginPath(_vg);
      move(x, y);
      started_path = true;
    }
    void begin(){
      nvgBeginPath(_vg);
    }

    void point(float x, float y){
      if(started_path)
        nvgLineTo(_vg, x, y);
      else
        path(x, y);
    };

    void point(Vec p){
      point(p.x, p.y);
    }

    void end(PathMode mode = STROKE){
      finish(mode);
      started_path = false;
    }

    void close(PathMode mode = STROKE){
      nvgClosePath(_vg);
      end(mode);
    }

    void rect(rack::Rect r, PathMode mode = FILL){
      rect(r.pos.x, r.pos.y, r.size.x, r.size.y, mode);
    }

    void rect(rack::Vec v, Vec s, PathMode mode = FILL){
      rect(v.x, v.y, s.x, s.y, mode);
    }

    void rect(rack::Vec v, float w, float h, PathMode mode = FILL){
      rect(v.x, v.y, w, h, mode);
    }
    void rect(float x, float y, float w, float h, PathMode mode = FILL){
      nvgBeginPath(_vg);
      nvgRect(_vg, x, y, w, h);
      finish(mode);
    };
    void rect(float x, float y, float w, float h, NVGcolor c, PathMode mode = FILL){
      nvgBeginPath(_vg);
      color(c, mode);
      nvgRect(_vg, x, y, w, h);
      finish(mode);
    };
    void center_rect(float x, float y, float w, float h, PathMode mode = FILL){
      nvgTranslate(_vg, (float) -w * 0.5f, (float) -h * 0.5f);
      rect(x, y, w, h, mode);    
      nvgTranslate(_vg, (float) w * 0.5f, (float) h * 0.5f);
    }
    void center_rect(rack::Vec v, Vec s, PathMode mode = FILL){
      center_rect(v.x, v.y, s.x, s.y, mode);
    }
    void center_rect(const rack::Rect &r, PathMode mode = FILL){
      center_rect(r.getCenter(), r.size, mode);
    }
    void round_rect(const rack::Rect &r, float roundness = 5.0f, PathMode mode = FILL){
      nvgBeginPath(_vg);
      nvgRoundedRect(_vg, r.pos.x, r.pos.y, r.size.x, r.size.y, roundness);
      finish(mode);
    }
    void round_rect(const rack::Vec &p, const rack::Vec &s, float roundness = 5.0f, PathMode mode = FILL){
      nvgBeginPath(_vg);
      nvgRoundedRect(_vg, p.x, p.y, s.x, s.y, roundness);
      finish(mode);
    }
    void round_rect(float x, float y, float w, float h, float r, PathMode mode = FILL){
      nvgBeginPath(_vg);
      nvgRoundedRect(_vg, x, y, w, h, r);
      finish(mode);
    };
    void center_round_rect(const float &x, const float &y, const float &w, const float &h, float roundness, PathMode mode = FILL){
      nvgBeginPath(_vg);
      nvgTranslate(_vg, -w * 0.5f, -h * 0.5f);
      nvgRoundedRect(_vg, x, y, w, h, roundness);
      nvgTranslate(_vg, w * 0.5f, h * 0.5f);
      finish(mode);
    }
    void center_round_rect(rack::Vec p, rack::Vec s, float roundness, PathMode mode = FILL){
      nvgBeginPath(_vg);
      nvgTranslate(_vg, -s.x * 0.5f, -s.y * 0.5f);
      nvgRoundedRect(_vg, p.x, p.y, s.x, s.y, roundness);
      nvgTranslate(_vg, s.x * 0.5f, s.y * 0.5f);
      finish(mode);
    }

    void stretch_rect(float x, float y, float x2, float y2, PathMode mode = FILL){
      rect(x, y, x2 - x, y2 - y, mode);
    }
    void stretch_round_rect(float x, float y, float x2, float y2, float roundness, PathMode mode = FILL){
      round_rect(x, y, x2 - x, y2 - y, roundness, mode);
    }

    void ellipse(float x, float y, float rx, float ry, PathMode mode = FILL){
      nvgBeginPath(_vg);
      nvgEllipse(_vg, x, y, rx, ry);
      finish(mode);
    };
    void circle(float x, float y, float r, PathMode mode = FILL){
      nvgBeginPath(_vg);
      nvgCircle(_vg, x, y, r);
      finish(mode);
    };
    void circle(float x, float y, float r, NVGcolor c, PathMode mode = FILL){
      color(c, mode);
      nvgBeginPath(_vg);
      nvgCircle(_vg, x, y, r);
      finish(mode);
    };
    void circle(Vec p, float r, NVGcolor c, PathMode mode = FILL){
      circle(p.x, p.y, r, c, mode);
    }
    void circle(Vec p, float r, PathMode mode = FILL){
      circle(p.x, p.y, r, mode);
    }
    void circle(float x, float y, float r, NVGcolor fill_c, NVGcolor stroke_c){
      fill(fill_c);
      stroke(stroke_c);
      nvgBeginPath(_vg);
      nvgCircle(_vg, x, y, r);
      finish(BOTH);
    };
    void line(float x1, float y1, float x2, float y2){
      nvgBeginPath(_vg);
      nvgMoveTo(_vg, x1, y1);
      nvgLineTo(_vg, x2, y2);
      nvgStroke(_vg);
    };
    void line(Vec a, Vec b){
      nvgBeginPath(_vg);
      nvgMoveTo(_vg, a.x, a.y);
      nvgLineTo(_vg, b.x, b.y);
      nvgStroke(_vg);
    };
    void stroke_width(float w){
      nvgStrokeWidth(_vg, w);
    }
    void line(float x1, float y1, float x2, float y2, float w){
      nvgBeginPath(_vg);
      nvgStrokeWidth(_vg, w);
      nvgMoveTo(_vg, x1, y1);
      nvgLineTo(_vg, x2, y2);
      nvgStroke(_vg);
    };
    template <typename T>
    void line(T x1, T y1, T x2, T y2, NVGcolor c, float w = 1.0f){
      nvgBeginPath(_vg);
      nvgStrokeWidth(_vg, w);
      nvgMoveTo(_vg, x1, y1);
      nvgLineTo(_vg, x2, y2);
      nvgStrokeColor(_vg, c);
      nvgStroke(_vg);
    };
    template <typename T>
    void line(T x1, T y1, T x2, T y2, T w, NVGcolor c){
      nvgBeginPath(_vg);
      nvgStrokeWidth(_vg, w);
      nvgMoveTo(_vg, x1, y1);
      nvgLineTo(_vg, x2, y2);
      nvgStrokeColor(_vg, c);
      nvgStroke(_vg);
    };
    void arc(float x, float y, float r, float a1, float a2, int d = NVG_CW, PathMode mode = STROKE){
      nvgBeginPath(_vg);
      nvgArc(_vg, x, y, r, a1, a2, d);
      finish(mode);
    }
    void back(){
      rect(0, 0, width, height);
    }

    void hole(float r = 6.0f){
      color_mode(HSLA);
      fill(0.f,0.f,0.f, 0.5f);
      nvgBeginPath(_vg);
      nvgEllipse(_vg, center.x, center.y, r, r - 0.15f);
      finish(FILL);
      // circle(center.x, center.y - 0.5f, 6.f, FILL);
      fill(0,0,0);
      nvgBeginPath(_vg);
      nvgEllipse(_vg, center.x, center.y + 0.5f, r - 1.0f, r - 1.25f);
      finish(FILL);
      // circle(center.x, center.y + 0.5f, 5.0f, FILL);
    }


    int create_image(int w, int h, const unsigned char* data, int flags = NVG_IMAGE_GENERATE_MIPMAPS){
      return nvgCreateImageRGBA(_vg, w, h, flags, data);
    }
    void update_image(int image, const unsigned char* data){
      nvgUpdateImage(_vg, image, data);
    }
    int load_image(std::string image_path, bool local = true, int flags = NVG_IMAGE_GENERATE_MIPMAPS){
      return nvgCreateImage(_vg, (local ? asset::plugin(pluginInstance, image_path).c_str() : image_path.c_str()), flags);
    }

    void round_cap(){
      nvgLineCap(_vg, NVG_ROUND);
    }

    void butt_cap(){
      nvgLineCap(_vg, NVG_BUTT);
    }

    template <typename T>
    void image(int image_handle, T x, T y, T w, T h, bool _draw = true){
      nvgFillPaint(_vg, nvgImagePattern(_vg, x, y, w, h, 0, image_handle, 1.0f));
      if(_draw){
        nvgBeginPath(_vg);
        nvgRect(_vg, x, y, w, h);
        nvgFill(_vg);
      }
    }

    template <typename T>
    void scissor(T x, T y, T w, T h){
      nvgScissor(_vg, x, y, w, h);
    }

    void scissor(rack::Rect r){
      nvgScissor(_vg, r.pos.x, r.pos.y, r.size.x,  r.size.y);
    }
    void scissor(){
      nvgScissor(_vg, 0, 0, width, height);
    }
    void finish(PathMode mode){
      switch(mode){
        case BOTH : nvgFill(_vg); nvgStroke(_vg); break;
        case STROKE : nvgStroke(_vg); break;
        case FILL : nvgFill(_vg); break;
      }
    }
    void letter_spacing(float s){
      nvgTextLetterSpacing(_vg, s);
    }

    float slider(float w, float x, float pad = 7.0f){
      if(x < pad)
        return 0.0f;
      else if(x > w - pad)
        return 1.0f;
      else
        return rescale(x, pad, w - pad, 0.0f, 1.0f);
    }

    void draw(const DrawArgs& args) override{
      _vg = args.vg;
      if(first_frame){
        nvgFillColor(_vg, green);
        nvgStrokeColor(_vg, red);
        set_box();
        first_frame = false;
        init(_vg);
      }
      if(using_font)
        font = APP->window->loadFont(font_path);
      // nvgScissor(_vg, 0, 0, width, height);
      render();
    };

    void drawLayer(const DrawArgs& args, int layer) override{
      _vg = args.vg;
      if(first_frame){
        nvgFillColor(_vg, green);
        nvgStrokeColor(_vg, red);
        set_box();
        first_frame = false;
        init(_vg);
      }
      if(layer == 1){
        if(using_font)
          font = APP->window->loadFont(font_path);
        render_lights();
        render_light_cover();
      }
    }

    // void onHover(const event::Hover &e) override{
    //   e.consume(this);
    //   mouse.x = e.pos.x / width;
    //   mouse.y = e.pos.y / height;
    // }


    // void onButton(const event::Button &e)override{
    //   // if(draggable){
    //   //   e.consume(this);
    //   // }
    //   // mouse.x = e.pos.x / width;
    //   // mouse.y = e.pos.y / height;
    //   // if(e.action == GLFW_PRESS){
    //   //   if(e.button == GLFW_MOUSE_BUTTON_LEFT)
    //   //     click();
    //   //   else if(e.button == GLFW_MOUSE_BUTTON_RIGHT)
    //   //     right();
    //   //   else
    //   //     middle();
    //   // }
    // }
    // void onDragMove(const event::DragMove &e) override {
    //   if(draggable)
    //     box.pos = box.pos.plus(e.mouseDelta);
    // }
  };
  struct BufferedWidget : rack::Widget {
    widget::FramebufferWidget *fb;
    Plane *dp;
    // Border *border;
    BufferedWidget(rack::Rect b){
      box = b;

      fb = new widget::FramebufferWidget;
      fb->box.size = b.size;
      fb->box.pos = Vec(0, 0);
      addChild(fb);

      set_dirty();

    }
    void set_dirty(){
      fb->dirty = true;
    }
  };
  struct Panel : rack::Widget {
    widget::FramebufferWidget *fb;
    Plane *dp;
    // Border *border;
    Panel(Vec s, NVGcolor c = nvgHSLA(0, 0, 0, 0), bool border = true){
      fb = new widget::FramebufferWidget;
      addChild(fb);

      dp = new Plane(s, c);
      fb->addChild(dp);

      box.size = s;
      fb->box.size = s;
      set_dirty();

      // border = new Border();
      // border->set_box(box.size);
      // addChild(border);
        
      if(border){
        PanelBorder* pb = new PanelBorder;
        pb->box.size = box.size;
        addChild(pb);
      }
    }
    void set_dirty(){
      fb->dirty = true;
    }
  };
}
