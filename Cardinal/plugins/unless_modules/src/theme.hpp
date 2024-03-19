#include "rack.hpp"
#include "utils.hpp"
namespace _less{

struct Theme{
  int version = 2;
  std::string name;
  float hue = 0.1f;
  float sat = 0.03f;
  NVGcolor bg = nvgHSLA(hue,sat,0.45f, 255);
  NVGcolor bg_light = nvgHSLA(hue, sat, 0.49f, 255);
  NVGcolor bg_dark = nvgHSLA(hue, sat, 0.38f, 255);
  NVGcolor fg = nvgHSLA(hue, 0.01f, 0.1f, 255);
  NVGcolor fg_light = nvgHSLA(hue, sat, 0.2f, 255);
  NVGcolor fg_dark = nvgHSLA(hue, sat, 0.17f, 255);
  NVGcolor input = nvgHSLA(0.1,sat * 5.0f, 0.67f, 255);
  NVGcolor output = nvgHSLA(0.1,sat, 0.3f, 255);
  // NVGcolor output = nvgRGB(42,41,39);
  NVGcolor input_dark = nvgRGB(142,140,131);
  NVGcolor output_dark = nvgRGB(36,35,34);
  NVGcolor darker =  nvgRGB(60,60,60);
  NVGcolor shadow = nvgRGBA(0,0,0,45);
  NVGcolor black = nvgRGB(0,0,0);
  NVGcolor white = nvgRGB(255,255,255);
  NVGcolor energy[4] = {
    nvgTransRGBAf(nvgRGBA(255,255,255,255), 0.42f),
    nvgTransRGBAf(nvgRGBA(0,0,0,255), 0.42f),
    nvgTransRGBAf(nvgRGBA(255,255,255,255), 0.24f),
    nvgTransRGBAf(nvgRGBA(0,0,0,255), 0.24f)
  };
  NVGcolor transparent = nvgRGBA(255,255,255,0);

  NVGcolor rainbow[6] = {
    nvgHSL(0,0.8f,0.6f),
    nvgHSL(0.07,0.8f,0.6f),
    nvgHSL(0.15f,0.8f,0.6f),
    nvgHSL(0.3f,0.65f,0.6f),
    nvgHSL(0.45f,0.8f,0.6f),
    nvgHSL(0.69f,0.9f,0.7f)
  };

  NVGcolor layers[2];

  bool dirty = false;

  Theme(std::string n = "custom"){
    name = n;
    layers[0] = input; layers[1] = output;
  }
  NVGcolor rainbow_color(float t){
    t = float_wrap(t);
    int i = floor(t * 6.0f);
    float w = 1.0f / 6.0f;
    float tt = rack::rescale(fmod(t, w), 0.0f, w, 0.0f, 1.0f);
    int ni = (i + 1) % 6;
    return nvgLerpRGBA(rainbow[i], rainbow[ni], tt);
  }

  json_t *toJson(){
    json_t *rootJ = json_object();
    
    json_object_set_new(rootJ, "bg", rgbToJson(bg));
    json_object_set_new(rootJ, "fg", rgbToJson(fg));
    json_object_set_new(rootJ, "in", rgbToJson(input));
    json_object_set_new(rootJ, "out", rgbToJson(output));
    json_object_set_new(rootJ, "version", json_integer(version));

    return rootJ;
  }

  int colorVal(float x){
    return floor(x * 255.0f);
  }

  // void colorHex(char s[8], NVGcolor c){
  //   snprintf(s, 8, "%02x%02x%02x%02x", colorVal(c.rgba[0]), colorVal(c.rgba[1]), colorVal(c.rgba[2]), colorVal(c.rgba[3]));
  // }
  
  std::string chars(char s[8]){
    std::string r = std::to_string(s[0]);
    for(int i = 1; i<8; i++)
      r += s[i];
    return r;

  }
  int charVal(int x){
    return x < 97 ? x - 48 : x - 97 + 10;
  }
  int hexVal(int a, int b){
    return charVal(a) * 16 + charVal(b);
  }

  NVGcolor hexColor(const char s[8]){
    return nvgRGB(hexVal(s[0], s[1]), hexVal(s[2], s[3]), hexVal(s[4], s[5]));
  }

  // json_t *colorToJson(NVGcolor c){
  //   char s[8];
  //   colorHex(s, c);
  //   return json_string(s);
  // }

  json_t *rgbToJson(NVGcolor c){
    json_t *rootJ = json_object();

    json_object_set_new(rootJ, "r", json_integer(colorVal(c.rgba[0])));
    json_object_set_new(rootJ, "g", json_integer(colorVal(c.rgba[1])));
    json_object_set_new(rootJ, "b", json_integer(colorVal(c.rgba[2])));

    return rootJ;
  }

  void calculate_colors(NVGcolor _bg, NVGcolor _fg, NVGcolor _i, NVGcolor _o){
    NVGcolor black = nvgRGB(0,0,0);
    NVGcolor white = nvgRGB(255,255,255);
    float contrast = 0.3f;


    bg = _bg;
    bg_dark = nvgLerpRGBA(bg, black, contrast);
    bg_light = nvgLerpRGBA(bg, white, contrast);

    fg = _fg;
    fg_dark = nvgLerpRGBA(fg, black, contrast);
    fg_light = nvgLerpRGBA(fg, white, contrast);

    input = _i;
    input_dark = nvgLerpRGBA(input, black, contrast);

    output = _o;
    output_dark = nvgLerpRGBA(output, black, contrast * 2.0f);

    layers[0] = input; layers[1] = output;
  }

  NVGcolor layer(int i){
    if(i == 0) return input;
    else return output;
  }
  NVGcolor layer_dark(int i){
    if(i == 0) return input_dark;
    else return output_dark;
  }

  NVGcolor colorFromJson(json_t *rootJ){
    return nvgRGB(
      json_integer_value(json_object_get(rootJ, "r")),
      json_integer_value(json_object_get(rootJ, "g")),
      json_integer_value(json_object_get(rootJ, "b"))
    );
  }

  void fromJson(json_t * rootJ){
    bg = colorFromJson(json_object_get(rootJ, "bg"));
    fg = colorFromJson(json_object_get(rootJ, "fg"));
    input = colorFromJson(json_object_get(rootJ, "in"));
    output = colorFromJson(json_object_get(rootJ, "out"));
    calculate_colors(bg, fg, input, output);
  }

  void set_color(NVGcolor *c, NVGcolor b){
    for(int i = 0; i<4; i++)
      c->rgba[i] = b.rgba[i];
  }
};



struct ThemeFile{
  bool loaded = false;
  const std::string file_path(){
    return rack::asset::user("unlessgames") + "/theme.json";
  }
  void save(const std::string filename, json_t *rootJ){
    if (rootJ) {
      FILE *file = fopen(filename.c_str(), "w");
      if (!file){
        WARN("[ unless ] cannot open '%s' to write\n", filename.c_str());
        return;
      }
      json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
      json_decref(rootJ);
      fclose(file);
      INFO("[ unless ] saving to %s\n", filename.c_str());
    }else{
      INFO("[ unless ] json to %s is empty\n", filename.c_str());
    }
  }
  void save(_less::Theme *theme){
    save(file_path(), theme->toJson());
  }
  void generate(_less::Theme *theme){
    save(file_path(), theme->toJson());
  }
  json_t *load(const std::string filename){
    FILE *file = fopen(filename.c_str(), "r");
    json_error_t error;
    json_t *rootJ = json_loadf(file, 0, &error);
    if (rootJ == NULL) {
      WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
    }else{
    }

    fclose(file);

    return rootJ;
  }
  void check(_less::Theme *theme){
    if(!loaded){
      if(!rack::system::isDirectory(rack::asset::user("unlessgames"))){
        rack::system::createDirectory(rack::asset::user("unlessgames"));
        generate(theme);
      }else{
        if(!rack::system::isFile(file_path())){
          generate(theme);
          return;
        }else{
          INFO("[ unless ] loading color theme from ./unlessgames/theme.json");
          
          json_t *rootJ = load(file_path());

          json_t *version = json_object_get(rootJ, "version");
          if(!version || json_integer_value(version) < theme->version){
            INFO("[ unless ] theme created with previous version");
            generate(theme);
            return;
          }

          if(rootJ){
            theme->fromJson(rootJ);
            loaded = true;
          }else{
            WARN("problem loading file from ./unlessgames/theme.json");
            return;
          }
        }
      }
      return;
    }else{
      return;
    }
  }
};

}
