#include "unless.hpp"
#include "widgets.hpp"
#include "dsp/digital.hpp"
#include "utils.hpp"
#include <algorithm>
#include "../art/components.hpp"

#define MODULE_WIDTH 15

#define MAX_WIDTH 64
#define MAX_HEIGHT 32


#define MIN_WIDTH 3
#define MIN_HEIGHT 3
enum SnakeCellState{
  EMPTY,
  FOOD,
  SNAKE,
  SNAKECELL_STATES
};

struct Snake : Module {
  enum ParamIds {
    WIDTH_PARAM,
    HEIGHT_PARAM,
    WALLS_PARAM,
    UP_PARAM,
    RIGHT_PARAM,
    DOWN_PARAM,
    LEFT_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    CLOCK_INPUT,
    FOODX_INPUT,
    FOODY_INPUT,
    WIDTH_INPUT,
    HEIGHT_INPUT,
    WALLS_INPUT,
    UP_INPUT,
    RIGHT_INPUT,
    DOWN_INPUT,
    LEFT_INPUT,
    FOOD_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    LEFT_EYE_OUTPUT,
    FORWARD_EYE_OUTPUT,
    RIGHT_EYE_OUTPUT,

    FOOD_GATE_OUTPUT,
    FOODX_OUTPUT,
    FOODY_OUTPUT,

    DIRECTION_OUTPUT,
    HEADX_OUTPUT,
    HEADY_OUTPUT,

    LENGTH_OUTPUT,
    TAILX_OUTPUT,
    TAILY_OUTPUT,

    ENDING_OUTPUT,
    UP_CV_OUTPUT,
    RIGHT_CV_OUTPUT,
    DOWN_CV_OUTPUT,
    LEFT_CV_OUTPUT,
    UP_GATE_OUTPUT,
    RIGHT_GATE_OUTPUT,
    DOWN_GATE_OUTPUT,
    LEFT_GATE_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  PersistentOutput persistent_outputs[NUM_OUTPUTS];

  SnakeCellState map[MAX_WIDTH][MAX_HEIGHT];

  TriggerSwitch direction_triggers[4];

  struct Snakey{
    std::vector<Vec> body;
    int direction = 1;
    int next_dir = -1;
    bool alive = false;


    void born(int l, int x, int y, int d = 1){
      body.clear();
      for(int i = 0; i<l; i++){
        body.insert(body.begin(), Vec(x + i, y));
      }
      alive = true;
      direction = d;
    }
    bool can_turn(int d){
      return d != ((direction + 2) % 4);
    }
    Vec next_pos(){
      return body.at(0).plus(direction_vectors[direction]);
    }
  };

  bool dirty = true;

  Snakey snake;

  TriggerSwitch clock_trigger, walls_trigger, food_trigger;

  int map_width = MAX_WIDTH, map_height = MAX_HEIGHT;


  int start_length = 8;
  int death_frame_count = 10;
  bool rainbow_snake = false;

  bool walls_enabled = true;

  bool first_frame = true;

  // CHEATS
  bool ghost_snake, multifood, keep_foods, relative_turn, no_grow, no_eat, random_spawn, bouncer, paint, buffered_multifood, grid;


  void set_dirty(){
    dirty = true;
  }
  void clear_map(int t = -1){
    if(t == -1){
      for(int x = 0; x < MAX_WIDTH; x++){
        for(int y = 0; y < MAX_HEIGHT; y++){
          if(!keep_foods || map[x][y] != FOOD)
            map[x][y] = EMPTY;
        }
      }
    }else{
      for(int x = 0; x < MAX_WIDTH; x++){
        for(int y = 0; y < MAX_HEIGHT; y++){
          if((int)map[x][y] == t)
            map[x][y] = EMPTY;
        }
      }
    }
  }
  void give_birth_to_snake(int l, int x, int y, int d = 1){
    snake.born(l, x, y, d);
    for(auto v : snake.body)
      map[(int)v.x][(int)v.y] = SNAKE;
    output_direction();
  }
  void onReset() override{
    color = 0.42f;
    ghost_snake = false;
    multifood = false;
    keep_foods = false;
    relative_turn = false;
    no_grow = false;
    no_eat = false;
    random_spawn = false;
    bouncer = false;
    paint = false;
    buffered_multifood = false;
    grid = false;
    clear_map();
    foods.clear();
    setup_persistent_outputs();
    give_birth_to_snake(start_length, 0, map_height - 1);
    set_dirty();
  }

  const uint8_t spawn_border = 1;
  const uint8_t spawn_border2 = spawn_border * 2;

  float ray_distance(Vec p, Vec d){
    int i = 0;
    int c = d.x != 0 ? map_width : map_height;
    while(i < c){
      p = p.plus(d);
      if(wallhit(map_width, map_height, p).x != -1 || map[(int)p.x][(int)p.y] == SNAKE){
        return rescale((float) i, 0.0f, (float)(c - 1), 0.0f, 10.0f);
      }
      i++;
    }
    return -1;
  }
  
  std::vector<Vec> foods;
  int death_frames = 0;

  float color = 0.42f;

  int food_index(Vec p){
    int i = 0;
    for(auto f : foods){
      if(f.isEqual(p))
        return i;
      i++;
    }
    return -1;
  }
  Vec pos_outside_snake(float x = -1, float y = -1){
    int remaining_tiles = map_width * map_height;
    Vec p = Vec( x == -1 ? spawn_border + floor(random::uniform() * (map_width - spawn_border2)) : x, 
                 y == -1 ? spawn_border + floor(random::uniform() * (map_height - spawn_border2)) : y);
    if(ghost_snake)
      return p;
    for(int i = 0; i < remaining_tiles; i++){
      if(map[(int)p.x][(int)p.y] == EMPTY){
        return p;
      }else{
        p.x = ((int)p.x + 1) % map_width;
        p.y = (((int)p.x + 1) % map_width == 0) ? ((int)p.y + 1) % map_height : p.y;
      }
    }
    return p;
  }
  void offset_color(){
    color = fmod(color + 0.087f, 1.0f);
    set_dirty();
  }
  void add_food(int x, int y){
    Vec v = pos_outside_snake(x, y);
    int food_count = foods.size();
    if(multifood || food_count == 0){
      if(food_count < map_height * map_width - 1){
        foods.push_back(v);
        map[(int) v.x][(int) v.y] = FOOD;
      }

    }else{
      if(foods.size() > 1)
        clear_map(FOOD);
      else
        map[(int) foods.at(0).x][(int) foods.at(0).y] = EMPTY;

      foods.clear();
      foods.push_back(v);
      map[(int) v.x][(int) v.y] = FOOD;
    }
  }

  void spawn_food(int x, int y){
    add_food(x, y);
    persistent_outputs[FOODY_OUTPUT].setVoltage(rescale(foods.at(0).y, 0, map_height - 1, output_scale.x, output_scale.y));
    persistent_outputs[FOODX_OUTPUT].setVoltage(rescale(foods.at(0).x, 0, map_width - 1, output_scale.x, output_scale.y));
  }

  void toggle_food(int x, int y){
    if(map[x][y] == FOOD){
      foods.erase(foods.begin() + food_index(Vec(x, y)));
      map[x][y] = EMPTY;
    }else
      spawn_food(x, y);
  }

  bool inside_snake(Vec p){
    for(Vec v : snake.body){
      if(v.isEqual(p))
        return true;
    }
    return false;
  }


  Vec output_scale = Vec(0.0f, 10.0f);
  // Vec output_scale = Vec(-5.0f, 5.0f);

  bool over_food(Vec p){
    return (map[(int)p.x][(int)p.y] == FOOD);
  }
  void try_respawn_food(){
    if((foods.size() == 0 || !inside(map_width, map_height, foods.at(0))) && !inputs[FOOD_INPUT].isConnected())
      spawn_food_with_inputs();
  }
  void restart(){
    set_dirty();
    die();
    clear_map();
    try_respawn_food();
  }
  void update_width(int w){
    if(map_width != w){
      map_width = w;
      restart();
    }
  }
  void update_height(int h){
    if(map_height != h){
      map_height = h;
      restart();
    }
  }

  Vec mod_vec(Vec a, int w, int h){
    return Vec(wrap(a.x, w), wrap(a.y, h));
  }
  bool try_turning_snake(int d){
    if(death_frames != 0)
      return false;
    if(relative_turn)
      d = wrap(snake.direction + (d > 0 && d < 3 ? 1 : -1), 4);
    if(ghost_snake || snake.can_turn(d)){
      snake.next_dir = d;
      return true;
    }else{
      return false;
    }
  }
  void clear_wall_outputs(){
  }
  void output_wall_crossings(){
    Vec last = snake.body.at(snake.body.size() - 1);
    bool outs[4] = {false, false, false, false};
    float outsv[4];
    for(int i = snake.body.size() - 2; i >= 0; i--){
      float x = snake.body.at(i).x - last.x;
      if(x > 1.5f){
        outs[3] = true;
        outsv[3] = rescale(last.y, 0, map_height - 1, output_scale.y, output_scale.x);
      }else if(x < -1.5f){
        outsv[1] = rescale(last.y, 0, map_height - 1, output_scale.x, output_scale.y);
        outs[1] = true;
      }else{
        float y = snake.body.at(i).y - last.y;
        if(y > 1.5f){
          outs[0] = true;
          outsv[0] = rescale(last.x, 0, map_width - 1, output_scale.x, output_scale.y);
        }else if(y < -1.5f){
          outs[2] = true;
          outsv[2] = rescale(last.x, 0, map_width - 1, output_scale.y, output_scale.x);
        }
      }
      last = snake.body.at(i);
    }

    for(int i = 0; i<4; i++){
      if(outs[i]){
        outputs[UP_GATE_OUTPUT + i].setVoltage(outs[i] ? 10.0f : 0.0f);
        outputs[UP_CV_OUTPUT + i].setVoltage(outsv[i]);
      }else{

        outputs[UP_GATE_OUTPUT + i].setVoltage(outs[i] ? 10.0f : 0.0f);
      }
    }
  }

  Vec wallhit(int w, int h, Vec p){
    if(p.y < 0)
      return Vec(0, rescale(p.x, 0, map_width - 1, output_scale.x, output_scale.y));
    else if(p.x >= w)
      return Vec(1, rescale(p.y, 0, map_height - 1, output_scale.x, output_scale.y));
    else if(p.y >= h)
      return Vec(2, rescale(p.x, 0, map_width - 1, output_scale.y, output_scale.x));
    else if(p.x < 0)
      return Vec(3, rescale(p.y, 0, map_height - 1, output_scale.y, output_scale.x));
    else
      return Vec(-1,-1);
  }
  void output_direction(){
    persistent_outputs[DIRECTION_OUTPUT].setVoltage(rescale((float)snake.direction, 0.0f, 4.0f, 0.0f, 10.0f));  
  }
  void die(){
    int max_length = map_height * map_width;
    if(rainbow_snake)
      rainbow_snake = false;

    if((int)snake.body.size() == max_length){
      if(!ghost_snake)
        rainbow_snake = true;
    }
    snake.alive = false;
    outputs[ENDING_OUTPUT].setVoltage(10.0f);
    if(!keep_foods)
      foods.clear();
    death_frames = death_frame_count;
    clear_map();
  }
  void remove_tail(){
    Vec tp = snake.body.at(snake.body.size() - 1);
    if(map[(int)tp.x][(int)tp.y] == SNAKE)
      map[(int)tp.x][(int)tp.y] = EMPTY;

    snake.body.erase(snake.body.end() - 1);
  }
  void step_snake(Vec p){
    int max_length = map_height * map_width;
    p = mod_vec(p, map_width, map_height);
    snake.body.insert(snake.body.begin(), p);
    if(over_food(p)){
      if(!no_eat){
        map[(int) p.x][(int)p.y] = SNAKE;
        foods.erase(foods.begin() + food_index(p));

        if(!inputs[FOOD_INPUT].isConnected())
          spawn_food_with_inputs();
      }
      outputs[FOOD_GATE_OUTPUT].setVoltage(10.0f);
      outputs[LENGTH_OUTPUT].setVoltage(rescale(snake.body.size(), 0, map_width * map_height, output_scale.x, output_scale.y));

      if(no_grow || ((int)snake.body.size() > max_length) )
        remove_tail();
    }else{

      map[(int) p.x][(int)p.y] = SNAKE;
      
      if(!paint)   
        remove_tail();

      outputs[FOOD_GATE_OUTPUT].setVoltage(0.0f);
    }
    if(ghost_snake && ((int)snake.body.size() == max_length))
      remove_tail();
  }
  bool try_bounce(int d){
    int _dir = snake.direction;
    snake.direction = d;
    Vec p = snake.next_pos();
    Vec hit = wallhit(map_width, map_height, p);
    bool empty_cell = false;
    if(hit.x == -1){
      if(ghost_snake || map[(int)p.x][(int)p.y] != SNAKE){
        return true;
      }
    }
    if(!empty_cell)
      snake.direction = _dir;
    return empty_cell;
  }
  void update_snake(){
    if(snake.next_dir != -1){
      snake.direction = snake.next_dir;
      output_direction();
      snake.next_dir = -1;
    }
    Vec p = snake.next_pos();
    Vec hit = wallhit(map_width, map_height, p);

    bool hit_the_wall = !(!walls_enabled || hit.x == -1);
    if(!hit_the_wall && (!inside_snake(p) || ghost_snake)){
      step_snake(p);
    }else{
      if(walls_enabled && bouncer){
        if (try_bounce(wrap(snake.direction - 1, 4)))
          step_snake(snake.next_pos());
        else if(try_bounce(wrap(snake.direction + 1, 4)))
          step_snake(snake.next_pos());
        else if(ghost_snake)
          snake.direction = wrap(snake.direction + 2, 4);
        else 
          die();
      }else
        die();
    }

    if(hit.x != -1){
      outputs[UP_GATE_OUTPUT + hit.x].setVoltage(10.0f);
      outputs[UP_CV_OUTPUT + hit.x].setVoltage(hit.y);
    }

    if(!walls_enabled)
      output_wall_crossings();

    outputs[HEADX_OUTPUT].setVoltage(rescale(snake.body.at(0).x, 0, map_width - 1, output_scale.x, output_scale.y));
    outputs[HEADY_OUTPUT].setVoltage(rescale(snake.body.at(0).y, 0, map_height - 1, output_scale.x, output_scale.y));
    outputs[TAILX_OUTPUT].setVoltage(rescale(snake.body.at(snake.body.size() - 1).x, 0, map_width - 1, output_scale.x, output_scale.y));
    outputs[TAILY_OUTPUT].setVoltage(rescale(snake.body.at(snake.body.size() - 1).y, 0, map_height - 1, output_scale.x, output_scale.y));

    if(outputs[FORWARD_EYE_OUTPUT].isConnected())
      outputs[FORWARD_EYE_OUTPUT].setVoltage(ray_distance(snake.body.at(0), direction_vectors[snake.direction]));

    if(outputs[LEFT_EYE_OUTPUT].isConnected())
      outputs[LEFT_EYE_OUTPUT].setVoltage(ray_distance(snake.body.at(0), direction_vectors[wrap(snake.direction - 1, 4)]));

    if(outputs[RIGHT_EYE_OUTPUT].isConnected())
      outputs[RIGHT_EYE_OUTPUT].setVoltage(ray_distance(snake.body.at(0), direction_vectors[wrap(snake.direction + 1, 4)]));


  }
  void clear_wall_gates(){
    for(int i = 0; i<4; i++){
      outputs[UP_GATE_OUTPUT + i].setVoltage(0);      
      // outputs[UP_CV_OUTPUT + i].setVoltage(0);      
    }
  }
  void spawn_food_with_inputs(){
    spawn_food(
      (inputs[FOODX_INPUT].isConnected() ? (rescale(inputs[FOODX_INPUT].getVoltage(), output_scale.x, output_scale.y, 0, map_width - 1)) : -1),
      (inputs[FOODY_INPUT].isConnected() ? (rescale(inputs[FOODY_INPUT].getVoltage(), output_scale.x, output_scale.y, 0, map_height - 1)) : -1)
    );
  }
  void setup_persistent_outputs(){
   for(int i = 0; i<NUM_OUTPUTS; i++)
      persistent_outputs[i].set_port(&outputs.at(i));
  }

  void process(const ProcessArgs &args) override {
    if(first_frame){
      first_frame = false;
      setup_persistent_outputs();
    }
    update_width(params[WIDTH_PARAM].value);
    update_height(params[HEIGHT_PARAM].value);

    if(inputs[WALLS_INPUT].isConnected()){
      walls_trigger.update(inputs[WALLS_INPUT].getVoltage());
      if(walls_trigger.state == PRESSED){
        walls_enabled = !walls_enabled;
        set_dirty();
      }
    }else if(params[WALLS_PARAM].value != (walls_enabled ? 1 : 0)){
      walls_enabled = params[WALLS_PARAM].value == 1;
      set_dirty();
    }

    food_trigger.update(inputs[FOOD_INPUT].getVoltage());
    if(food_trigger.state == PRESSED){
      spawn_food_with_inputs();
    }


    if(map_height != params[HEIGHT_PARAM].value){
      map_height = params[HEIGHT_PARAM].value;
      set_dirty();
    }

    if(snake.next_dir == -1){
      for(int i = 0; i<4; i++){
        direction_triggers[i].update(params[UP_PARAM + i].value + inputs[UP_INPUT + i].value);
        if(direction_triggers[i].state == PRESSED){
          if(try_turning_snake(i))
            break;
        }
      }
    }

    clock_trigger.update(inputs[CLOCK_INPUT].value);
    if(clock_trigger.state == PRESSED){
      persistent_outputs[FOODX_OUTPUT].update();
      persistent_outputs[FOODY_OUTPUT].update();
      persistent_outputs[DIRECTION_OUTPUT].update();
      if(buffered_multifood != multifood){
        multifood = buffered_multifood;
        if(!multifood){
          foods.clear();
          clear_map(FOOD);
          spawn_food_with_inputs();
        }  
      }
      dirty = true;


      if(!snake.alive){
        death_frames--;
        if(death_frames == 0){
          outputs[ENDING_OUTPUT].setVoltage(0.0f);
          clear_wall_gates();
          if(!inputs[FOOD_INPUT].isConnected())
            spawn_food_with_inputs();
          // spawn_food();
          if(random_spawn){
            give_birth_to_snake(clamp(start_length, 1, map_width - 4), 0, floor(random::uniform() * (float) map_height));
          }else
            give_birth_to_snake(clamp(start_length, 1, map_width - 4), 0, map_height - 1);
          outputs[LENGTH_OUTPUT].setVoltage(rescale(snake.body.size(), 0, map_width * map_height, output_scale.x, output_scale.y));
        }
      }else{
        update_snake();
      }
    }
  }

  json_t *dataToJson() override {
    json_t *rootJ = json_object();
    json_object_set(rootJ, "color", json_real(color));
    json_object_set(rootJ, "death_frames", json_integer(death_frame_count));
    json_object_set(rootJ, "start_length", json_integer(start_length));
    json_object_set(rootJ, "no_grow", json_boolean(no_grow));
    json_object_set(rootJ, "ghost_snake", json_boolean(ghost_snake));
    json_object_set(rootJ, "relative_turn", json_boolean(relative_turn));
    json_object_set(rootJ, "multifood", json_boolean(multifood));
    json_object_set(rootJ, "no_eat", json_boolean(no_eat));
    json_object_set(rootJ, "random_spawn", json_boolean(random_spawn));
    json_object_set(rootJ, "bouncer", json_boolean(bouncer));
    json_object_set(rootJ, "paint", json_boolean(paint));
    json_object_set(rootJ, "grid", json_boolean(grid));
    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
    grid = json_boolean_value(json_object_get(rootJ, "grid"));
    paint = json_boolean_value(json_object_get(rootJ, "paint"));
    bouncer = json_boolean_value(json_object_get(rootJ, "bouncer"));
    random_spawn = json_boolean_value(json_object_get(rootJ, "random_spawn"));
    no_eat = json_boolean_value(json_object_get(rootJ, "no_eat"));
    multifood = json_boolean_value(json_object_get(rootJ, "multifood"));
    buffered_multifood = multifood;
    death_frame_count = json_integer_value(json_object_get(rootJ, "death_frames"));
    start_length = json_integer_value(json_object_get(rootJ, "start_length"));
    ghost_snake = json_boolean_value(json_object_get(rootJ, "ghost_snake"));
    relative_turn = json_boolean_value(json_object_get(rootJ, "relative_turn"));
    no_grow = json_boolean_value(json_object_get(rootJ, "no_grow"));
    color = json_number_value(json_object_get(rootJ, "color"));
  }
  Snake(){
    
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(WIDTH_PARAM, MIN_WIDTH, MAX_WIDTH, 24, "screen width");
    configParam(HEIGHT_PARAM, MIN_HEIGHT, MAX_HEIGHT, 13, "screen height");
    configSwitch(WALLS_PARAM, 0, 1, 1, "walls", {"wrapping", "solid"});
    configButton(UP_PARAM, "move up [W]");
    configButton(RIGHT_PARAM, "move right [D]");
    configButton(DOWN_PARAM, "move down [S]");
    configButton(LEFT_PARAM, "move left [A]");

    configInput(CLOCK_INPUT, "clock");
    configInput(FOODX_INPUT, "foodx");
    configInput(FOODY_INPUT, "foody");
    configInput(WIDTH_INPUT, "width");
    configInput(HEIGHT_INPUT, "height");
    configInput(WALLS_INPUT, "walls");
    configInput(UP_INPUT, "up");
    configInput(RIGHT_INPUT, "right");
    configInput(DOWN_INPUT, "down");
    configInput(LEFT_INPUT, "left");
    configInput(FOOD_INPUT, "food");

    configOutput(LEFT_EYE_OUTPUT, "left_eye");
    configOutput(FORWARD_EYE_OUTPUT, "forward_eye");
    configOutput(RIGHT_EYE_OUTPUT, "right_eye");
    configOutput(FOOD_GATE_OUTPUT, "food_gate");
    configOutput(FOODX_OUTPUT, "foodx");
    configOutput(FOODY_OUTPUT, "foody");
    configOutput(DIRECTION_OUTPUT, "direction");
    configOutput(HEADX_OUTPUT, "headx");
    configOutput(HEADY_OUTPUT, "heady");
    configOutput(LENGTH_OUTPUT, "length");
    configOutput(TAILX_OUTPUT, "tailx");
    configOutput(TAILY_OUTPUT, "taily");
    configOutput(ENDING_OUTPUT, "ending");
    configOutput(UP_CV_OUTPUT, "up_cv");
    configOutput(RIGHT_CV_OUTPUT, "right_cv");
    configOutput(DOWN_CV_OUTPUT, "down_cv");
    configOutput(LEFT_CV_OUTPUT, "left_cv");
    configOutput(UP_GATE_OUTPUT, "up_gate");
    configOutput(RIGHT_GATE_OUTPUT, "right_gate");
    configOutput(DOWN_GATE_OUTPUT, "down_gate");
    configOutput(LEFT_GATE_OUTPUT, "left_gate");
    onReset();
  }
};
struct SnakePanel : _less::Widget {
  Snake* module;
  Rect display_box = Rect();
  Vec pixel_size = Vec(1,1);
  Rect color_button;
  SnakePanel(){
  }
  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
    float w = RACK_GRID_WIDTH * 3.0f;
    float h = RACK_GRID_WIDTH;
    color_button = Rect(box.size.x * 0.5f - w - 2.0f, box.size.y - h, w, h);
  }
  void onButton(const event::Button &e) override{
    if(e.action == GLFW_PRESS && e.button == 0){
      if(color_button.isContaining(e.pos)){
        module->offset_color();
      }
    }
  }
  void update_pixel_size(float w, float h){
    pixel_size.x = display_box.size.x / w;
    pixel_size.y = display_box.size.y / h;
  }

  Vec port_pos(int i, bool input){
    float padding = 30.0f;
    float h = (box.size.x - padding) / 6.0f;
    float y = box.size.y * 0.44f;

    if(!input)
      y = box.size.y * 0.72f;
    int yy = i / 6;
    int xx = i % 6;
    Vec s = Vec(padding, y + yy * h);
    Vec e = Vec(box.size.x - padding, y + yy * h);
    Vec p = equaldist(s, e, xx, 6);
    return p;
  }

  void draw_connect(int a, int b, bool input_bank, bool input_color){
    stroke(input_color ? theme.input : theme.output);
    Vec ap = port_pos(a, input_bank);
    Vec bp = port_pos(b, input_bank);
    line(ap, bp);
  }

  void draw_grid(){
    stroke(theme.output);

    update_pixel_size(module->map_width, module->map_height);

    for(int i = 0; i < module->map_width; i++)
      line(display_box.pos.x + i * pixel_size.x, display_box.pos.y, display_box.pos.x + i * pixel_size.x, display_box.pos.y + display_box.size.y);
    for(int i = 0; i < module->map_height; i++)
      line(display_box.pos.x, display_box.pos.y + pixel_size.y * i, display_box.pos.x + display_box.size.x, display_box.pos.y + pixel_size.y * i);
  }

  void render() override{
    stroke(theme.output);
    fill(theme.bg_dark);
    round_rect(0,0,box.size.x, box.size.y, 23.0f, BOTH);
    stroke(theme.bg_dark);
    fill(theme.bg);
    round_rect(0,0,box.size.x, box.size.y - 5.0f, 23.0f, BOTH);

    fill(lerp(theme.input, theme.bg, 0.8f));
    circle(center.x, RACK_GRID_WIDTH * 3, 75.0f);
    fill(lerp(theme.input, theme.bg, 0.6f));
    circle(center.x, RACK_GRID_WIDTH * 2.5, 65.0f);

    fill(theme.bg);
    rect(display_box, FILL);
    if(module && module->grid)
      draw_grid();
    
    fill(theme.fg);
    std::string snakia = "----SNAKIA----";
    if(module){


      std::string b = module->bouncer ? "<" : "-";
      std::string ng = module->no_grow ? "=" : "-";
      std::string p = module->paint ? "@" : "-";
      std::string rt = module->relative_turn ? "%" : "-";

      std::string s = module->ghost_snake ? "$" : "S";

      std::string ne = module->no_eat ? "_" : "-";
      std::string gr = module->grid ? "#" : "A";

      std::string rs = module->random_spawn ? "?" : "-";
      std::string m = module->multifood ? "*" : "-";
      std::string kf = module->keep_foods && module->multifood ? "+" : "-";
      snakia = b + ng + p + rt + s + "NAKI" + gr + ne + m + kf + rs;
    }

    text(snakia, center.x, RACK_GRID_WIDTH - 2.0f, 14.0f);

    text("unless games", center.x, box.size.y - RACK_GRID_WIDTH, 12.0f);

    Vec p = port_pos(12, true);

    float y = RACK_GRID_WIDTH * 1.4f;
    fill(theme.input);

    text("F", p.x, p.y + y, 12.0f);
    p = port_pos(13, true);
    text("X", p.x, p.y + y, 12.0f);
    p = port_pos(14, true);
    text("Y", p.x, p.y + y, 12.0f);

    p = port_pos(1, true);
    stroke(theme.input);
    center_round_rect(p.x, p.y, 96.0f, 35.0f, 5.0f, STROKE);
    center_round_rect(p.x, p.y + 55.0f, 96.0f, 75.0f, 5.0f, STROKE);


    fill(lerp(theme.input, theme.bg, 0.6f));
    stroke(lerp(theme.input, theme.bg, 0.6f));
    p = port_pos(10, true);
    center_round_rect(p.x, p.y + 1.0f, 98.0f, 100.0f, 5.0f, FILL);
    stroke_width(2.0f);
    stroke(lerp(theme.rainbow_color(0.42f), theme.bg, 0.4f));
    center_round_rect(p.x, p.y + 1.0f, 28.0f, 29.0f, 5.0f, STROKE);

    p = port_pos(10, false);
    fill(lerp(theme.output, theme.bg, 0.5f));
    stroke(lerp(theme.output, theme.bg, 0.5f));
    center_round_rect(p.x, p.y + 1.0f, 98.0f, 100.0f, 5.0f, BOTH);

    stroke(lerp(theme.rainbow_color(0.02f), theme.bg, 0.3f));
    center_round_rect(p.x, p.y + 1.0f, 28.0f, 29.0f, 5.0f, BOTH);

    p = port_pos(13, false);
    stroke(lerp(theme.output, theme.bg, 0.5f));
    center_round_rect(p.x, p.y + 2.0f, 90.0f, 34.0f, 5.0f, BOTH);

    fill(theme.input);
    y = RACK_GRID_WIDTH * 1.4f;
    text("^", p.x, p.y + y, 16.0f);
    y = 0; //RACK_GRID_WIDTH * 0.6f;
    float w = 18.0f;
    text("<", p.x - w, p.y + y, 10.0f);    
    text(">", p.x + w, p.y + y, 10.0f);

    p = port_pos(1, false);

    fill(theme.output);
    center_rect(p.x - 17.0f, p.y + 18.0f, 3.0f, 52.0f, FILL);
    center_rect(p.x - 30.0f, p.y + 48.0f, 30.0f, 3.0f, FILL);

    fill(theme.input);
    center_rect(p.x - 17.0f, p.y - 8.0f, 3.0f, 4.0f, FILL);

    stroke_width(4.0f);
    draw_connect(3, 4, true, true);
    draw_connect(5, 11, true, true);
    draw_connect(16, 17, true, true);
    draw_connect(15, 9, true, true);

    draw_connect(6, 8, true, true);
    draw_connect(12, 14, true, false);
    // draw_connect(1, 7, true);
    // draw_connect(2, 8, true);

    draw_connect(1, 2, false, false);
    draw_connect(7, 8, false, false);
    // draw_connect(12, 14, false, false);

    draw_connect(3, 4, false, false);
    draw_connect(5, 11, false, false);
    draw_connect(9, 15, false, false);
    draw_connect(16, 17, false, false);
  }

};


struct HCVSKnob : unless::DrawKnob{
  bool is_input = true;
  void render() override{
    bg = &theme.input;
    fg = &theme.input_dark;
    thickness = 2.f;
    unless::DrawKnob::render();
  }
};

struct SnakeDisplay : _less::Widget {
  Snake* module;
  int drag_state = 0;
  Vec pixel_size = Vec(1, 1);
  float gapped_scale = 0.9f;
  Vec default_size = Vec(24, 16);
  SnakeDisplay(){
  }

  void onButton(const event::Button &e) override{
    if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
      module->toggle_food(floor(rescale(e.pos.x, 0, width, 0, module->map_width)), floor(rescale(e.pos.y, 0, height, 0, module->map_height)));
  }
  void update_pixel_size(float w, float h){
    pixel_size.x = width / w;
    pixel_size.y = height / h;
  }
  void set_box(float x, float y, float w, float h){
    _less::Widget::set_box(x, y, w, h);
    update_pixel_size(MAX_WIDTH, MAX_HEIGHT);
  }
  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
  }
  void draw_snake(const std::vector<Vec> &snake, NVGcolor color){
    if(module && module->ghost_snake)
      fill(transp(color, 0.42f));
    else
      fill(color);

    for(auto p : snake)
      center_rect(p.mult(pixel_size).plus(pixel_size.mult(0.5)), pixel_size.mult(gapped_scale));
  }
  void draw_rainbow_snake(const std::vector<Vec> &snake, NVGcolor color){
    int s = snake.size();
    for(int i = 0; i<s; i++){
      float t = (float) i / (float) snake.size();
      color = theme.rainbow_color(t);
      if(module && module->ghost_snake)
        fill(transp(color, 0.42f));
      else
        fill(color);
      center_rect(snake.at(i).mult(pixel_size).plus(pixel_size.mult(0.5)), pixel_size.mult(gapped_scale));
    }
  }

  bool inside_snake(Vec p, const std::vector<Vec> &snake){
    for(Vec v : snake){
      if(v.isEqual(p))
        return true;
    }
    return false;
  }
  void draw_food(const Vec &p){
    fill(theme.output);
    Vec s = Vec(clamp(pixel_size.x * 0.7f, 2.0f, 300.0f), clamp(pixel_size.y * 0.7f, 2.0f, 300.0f));
    rect(p.plus(pixel_size.mult(0.15f)), s);
  }
  std::vector<Vec> random_snake(){
    int l = default_size.x * default_size.y;
    std::vector<Vec> r;

    int d = 1;

    for(int i = 0; i<l; i++){
      if(i == 0)
        r.push_back(Vec(4, default_size.y - 3));
      else{
        Vec p = r.at(i - 1).plus(direction_vectors[d]);
        bool inside_level = p.x >= 0 && p.x <= default_size.x && p.y >= 0 && p.y <= default_size.y;
        if(inside_level && !inside_snake(p, r)){
          r.push_back(p);
          if(random::uniform() < 0.2f)
            d = wrap(d + floor(random::uniform() * 2.0f - 1.0f), 4);
        }else
          break;
      }
    }
    return r;
  }
  Vec random_pos(){
    return Vec(floor(random::uniform() * pixel_size.x * default_size.x), floor(random::uniform() * pixel_size.y * default_size.y));
  }
  void render() override{
  }
  void render_lights() override{
    if(module && module->rainbow_snake)
      fill(theme.bg);
    else
      fill(transp(theme.rainbow_color((module ? module->color : 0.42f)), 0.5f));

    round_rect(0,0,box.size.x, box.size.y, 5.0f);


    push();
    scissor(0.0f, 0.0f, width, height);

    if(module){
      update_pixel_size(module->map_width, module->map_height);
      if(module->rainbow_snake)
        draw_rainbow_snake(module->snake.body, module->death_frames % 2 == 0 ? theme.output : theme.output_dark);
      else
        draw_snake(module->snake.body, module->death_frames % 2 == 0 ? theme.output : theme.output_dark);
      if(module->foods.size() > 0){
        for(auto f : module->foods){
          draw_food(f.mult(pixel_size));
        }
      }
    }else{
      update_pixel_size(default_size.x, default_size.y);
      draw_snake(random_snake(), theme.output);
      draw_food(random_pos());
    }
    pop();
  }
  void render_light_cover() override{
    float bw = 3.0f;
    if(module && !module->walls_enabled)
      stroke(lerp(nvgRGB(0,0,0), lerp(theme.bg, theme.input, 0.8f), settings::rackBrightness));
    else
      stroke(lerp(nvgRGB(0,0,0), lerp(theme.bg_dark, theme.bg, 0.5f), settings::rackBrightness));
    stroke_width(bw * 2.0f);
    round_rect(-bw, -bw, width + bw * 2, height + bw * 2, 8.0f, STROKE);
  }
};


struct SnakeDirectionButton : unless::TextButtonWidget{
  int state = 0;
  int direction = 0;

  std::string dirs[4] = {"2abc", "6mno", "8tuv", "4ghi"};
  
  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
  }

  void symbol() override{
    round_cap();
    push();
    rotate(direction * 3.141592f * 0.5f);
    float w = width * 0.25f;
    float h = width * 0.15f;
    stroke(theme.output);
    stroke_width(2.5f);
    line(-w, h, 0, -h);
    line(w, h, 0, -h);
    pop();
  }
  // void render() override{
  //   // float roundness = 2.0f;

  //   // fill(theme.input_dark);
  //   // round_rect(0, 2, width, height - 2, roundness, FILL);
  //   // // if(state == 1){
  //   // //   fill(theme.input_dark);
  //   // //   round_rect(0, 0, width, height, roundness, FILL);
  //   // // }else{
  //   // fill(theme.input);
  //   // round_rect(0, 0, width, height - 3, roundness, FILL);
  //   // // }
  //   // arrow();
  // }
};

struct SnakeWallsButton : unless::TextButtonWidget{
  int state = 0;
  void init(NVGcontext *vg) override{
    load_font("font/Terminus.ttf");
  }
  void render() override{
    if(state == 1){
      bg = &theme.input;
      fg = &theme.output;
    }else{
      bg = &theme.output;
      fg = &theme.input;
    }
    unless::TextButtonWidget::render();
  }
  void symbol() override{
    stroke(fg);
    stroke_width(2.0f);
    center_round_rect(0.0f, 0.0f, width * 0.6f, height * 0.4f, 2.0f, STROKE);
  }
};

struct SnakeWidget : ModuleWidget {
  _less::Panel *panelWidget = NULL;
  _less::Panel *displayPanel = NULL;
  Snake *moduleInstance = NULL;

  int arrow_key(int k){
    switch(k){
      case GLFW_KEY_UP : return 0;
      case GLFW_KEY_RIGHT : return 1;
      case GLFW_KEY_DOWN : return 2;
      case GLFW_KEY_LEFT : return 3;

      case GLFW_KEY_W : return 0;
      case GLFW_KEY_D : return 1;
      case GLFW_KEY_S : return 2;
      case GLFW_KEY_A : return 3;
      default : return -1;
    }
  }
  void onHoverKey(const event::HoverKey &e) override{
    int k = arrow_key(e.key);
    if(!((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) && !((e.mods & RACK_MOD_MASK) == (RACK_MOD_CTRL | GLFW_MOD_SHIFT)) && k != -1){
      e.consume(this);
      e.stopPropagating();
      if(e.action == GLFW_PRESS)
        moduleInstance->try_turning_snake(k);
    }else
      ModuleWidget::onHoverKey(e);
  }
  SnakeWidget(Snake *module){
    setModule(module);
    moduleInstance = module;
    box.size = Vec(MODULE_WIDTH * RACK_GRID_WIDTH, 380);

    panelWidget = new _less::Panel(box.size, nvgHSLA(0, 0, 0, 0), false);
    SnakePanel *panel = new SnakePanel();
    panel->module = module;
    panel->set_box(0.0f,0.0f,box.size.x, box.size.y);

    panelWidget->fb->addChild(panel);
    addChild(panelWidget);
    
    addChild(createWidget<PeaceScrew>(Vec(RACK_GRID_WIDTH * 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH * 1.4f)));
    addChild(createWidget<PeaceScrew>(Vec(box.size.x - 3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH * 1.4f )));

    addChild(createWidget<PeaceScrew>(Vec(RACK_GRID_WIDTH * 2, 0)));
    addChild(createWidget<PeaceScrew>(Vec(box.size.x - 3 * RACK_GRID_WIDTH, 0)));

    displayPanel = new _less::Panel(box.size, nvgHSLA(0, 0, 0, 0), false);
    SnakeDisplay *display = new SnakeDisplay();
    display->module = module;
    display->set_box(
      RACK_GRID_WIDTH * 1.25f, RACK_GRID_WIDTH * 2, 
      (box.size.x - 2.5f * RACK_GRID_WIDTH), (box.size.x - 2 * RACK_GRID_WIDTH) * (48.0f / 84.0f));
    panel->display_box = display->box;
    addChild(displayPanel);
    displayPanel->fb->addChild(display);

    // float x = box.size.x * 0.5f;
    float padding = 30.0f;
    float y = box.size.y * 0.44f;

    float h = (box.size.x - padding) / 6.0f;

    int top_io[18] = {
      Snake::WALLS_PARAM,
      Snake::WIDTH_PARAM,
      Snake::HEIGHT_PARAM,
      Snake::UP_INPUT,
      Snake::UP_PARAM,
      Snake::RIGHT_INPUT,

      
      Snake::FOOD_INPUT,
      Snake::FOODX_INPUT,
      Snake::FOODY_INPUT,
      Snake::LEFT_PARAM,
      Snake::CLOCK_INPUT,
      Snake::RIGHT_PARAM,

      -1,
      -1,
      -1,
      Snake::LEFT_INPUT,
      Snake::DOWN_PARAM,
      Snake::DOWN_INPUT,
    };

    int top_type[18] = {
      2, 2, 2, 1, 2, 1,
      1, 0, 0, 2, 1, 2,
      -1,-1,-1,1, 2, 1
    };


    for(int i = 0; i < 18; i++){
      int yy = i / 6;
      int xx = i % 6;
      Vec s = Vec(padding, y + yy * h);
      Vec e = Vec(box.size.x - padding, y + yy * h);
      Vec p = equaldist(s, e, xx, 6);
      Vec button_size = Vec(25.0f, 25.0f);
      switch(top_type[i]){
        case 0 : 
        addInput(
          new _less::Port<unless::CvInput>(
            p, module, top_io[i]
        ));
        break;

        case 1 : 
        addInput(
          new _less::Port<unless::GateInput>(
            p, module, top_io[i]
        ));
        break;

        case 2 : {
          int pa = top_io[i];
          if(pa == Snake::UP_PARAM || pa == Snake::DOWN_PARAM || pa == Snake::LEFT_PARAM || pa == Snake::RIGHT_PARAM){
            int d = pa - Snake::UP_PARAM;
            _less::Button<SnakeDirectionButton> *b = new _less::Button<SnakeDirectionButton>(
                p,
                button_size,
                module,
                pa
            );
            b->sw->direction = d;
            addParam(b);
          }else if(pa == Snake::WIDTH_PARAM || pa == Snake::HEIGHT_PARAM){
            addParam(new _less::Knob<HCVSKnob>(
              p, 
              Vec(23.0f, 23.0f), 
              module, 
              pa, 
              true
            ));
          }else if(pa == Snake::WALLS_PARAM){
            _less::Button<SnakeWallsButton> * wb = new _less::Button<SnakeWallsButton>(
                p,
                Vec(23.0f, 23.0f), 
                module,
                pa,
                false
            );
            addParam(wb);
          }
        }break;
      }
    }

    int pv = Snake::FOOD_GATE_OUTPUT;
    for(int i = 0; i<3; i++){
      int yy = 2;
      int xx = i % 6;
      Vec s = Vec(padding, y + yy * h);
      Vec e = Vec(box.size.x - padding, y + yy * h);
      Vec p = equaldist(s, e, xx, 6);
      if(i == 0)
        addOutput(
          new _less::Port<unless::GateOutput>(
            p, module, pv + i
        ));
      else
        addOutput(
          new _less::Port<unless::CvOutput>(
            p, module, pv + i
        ));
    }

    y = box.size.y * 0.72f;

    int bottom_outputs[18] = {
      Snake::DIRECTION_OUTPUT,      
      Snake::HEADX_OUTPUT,
      Snake::HEADY_OUTPUT,

      Snake::UP_CV_OUTPUT,
      Snake::UP_GATE_OUTPUT,
      Snake::RIGHT_CV_OUTPUT,


      Snake::LENGTH_OUTPUT,
      Snake::TAILX_OUTPUT,
      Snake::TAILY_OUTPUT,

      Snake::LEFT_GATE_OUTPUT,
      Snake::ENDING_OUTPUT,
      Snake::RIGHT_GATE_OUTPUT,
      

      Snake::LEFT_EYE_OUTPUT,
      Snake::FORWARD_EYE_OUTPUT,
      Snake::RIGHT_EYE_OUTPUT,


      Snake::LEFT_CV_OUTPUT,
      Snake::DOWN_GATE_OUTPUT,
      Snake::DOWN_CV_OUTPUT,
    };

    bool port_is_gate[18] = {
      false, false, false,
      false, true, false, 
      false, false, false,
      true, true, true, 
      false, false, false,
      false, true, false, 
    };

    for(int i = 0; i < 18; i++){
      int yy = i / 6;
      int xx = i % 6;
      Vec s = Vec(padding, y + yy * h);
      Vec e = Vec(box.size.x - padding, y + yy * h);
      Vec p = equaldist(s, e, xx, 6);
      if(port_is_gate[i]){
        addOutput(
          new _less::Port<unless::GateOutput>(
            p, module, bottom_outputs[i]
        ));
      }else{
        addOutput(
          new _less::Port<unless::CvOutput>(
            p, module, bottom_outputs[i]
        ));
      }
    }

  }
  void step() override{
    ModuleWidget::step();
    if(moduleInstance && moduleInstance->dirty){
      // panelWidget->set_dirty();
      displayPanel->set_dirty();
      moduleInstance->dirty = false;
    }
  }

  std::vector<std::string> lines = {
    "connect a clock into the green gate input",
    "(at the center of the directional buttons)",
    "this steps the game forward by 1 frame",
    "",
    "control your snake",
    "ARROWS or WASD",
    "(only works if your cursor is over the module)"
    "",
  };
  void appendContextMenu(Menu *menu) override {
    Snake *view = dynamic_cast<Snake*>(module);
    assert(view);
    
    add_color_slider(menu, moduleInstance);

    menu->addChild(construct<MenuLabel>());
    
    menu->addChild(new _less::HelpItem(&lines));

    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "SETTINGS"));

    menu->addChild(construct<_less::IntMenuItem>(&MenuItem::text, "start length", &_less::IntMenuItem::setting, &view->start_length, &_less::IntMenuItem::min_value, 1, &_less::IntMenuItem::panel, panelWidget));
    menu->addChild(construct<_less::IntMenuItem>(&MenuItem::text, "steps to wait after game over", &_less::IntMenuItem::setting, &view->death_frame_count, &_less::IntMenuItem::min_value, 1, &_less::IntMenuItem::panel, panelWidget));
    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "!!! CHEATS !!!"));
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "snake"));

    menu->addChild(construct<_less::BoolMenuItem>(&MenuItem::text, "< bouncer", &_less::BoolMenuItem::setting, &view->bouncer, &_less::BoolMenuItem::panel, panelWidget));
    menu->addChild(construct<_less::BoolMenuItem>(&MenuItem::text, "= lock snake length", &_less::BoolMenuItem::setting, &view->no_grow, &_less::BoolMenuItem::panel, panelWidget));
    menu->addChild(construct<_less::BoolMenuItem>(&MenuItem::text, "@ leave trail", &_less::BoolMenuItem::setting, &view->paint, &_less::BoolMenuItem::panel, panelWidget));
    menu->addChild(construct<_less::BoolMenuItem>(&MenuItem::text, "% relative turning", &_less::BoolMenuItem::setting, &view->relative_turn, &_less::BoolMenuItem::panel, panelWidget));
    menu->addChild(construct<_less::BoolMenuItem>(&MenuItem::text, "$ ghost snake", &_less::BoolMenuItem::setting, &view->ghost_snake, &_less::BoolMenuItem::panel, panelWidget));

    menu->addChild(construct<_less::BoolMenuItem>(&MenuItem::text, "? random start row", &_less::BoolMenuItem::setting, &view->random_spawn, &_less::BoolMenuItem::panel, panelWidget));

    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "food"));
    menu->addChild(construct<_less::BoolMenuItem>(&MenuItem::text, "_ don't eat food", &_less::BoolMenuItem::setting, &view->no_eat, &_less::BoolMenuItem::panel, panelWidget));
    menu->addChild(construct<_less::BoolMenuItem>(&MenuItem::text, "* multiple foods", &_less::BoolMenuItem::setting, &view->buffered_multifood, &_less::BoolMenuItem::panel, panelWidget));
    menu->addChild(construct<_less::BoolMenuItem>(&MenuItem::text, "+ keep foods (needs *)", &_less::BoolMenuItem::setting, &view->keep_foods, &_less::BoolMenuItem::panel, panelWidget));
    menu->addChild(construct<_less::BoolMenuItem>(&MenuItem::text, "# show grid", &_less::BoolMenuItem::setting, &view->grid, &_less::BoolMenuItem::panel, panelWidget));
  }
};


Model *modelSnake = createModel<Snake, SnakeWidget>("snake");
/*
  TODO
  cheats
    cv turn

  polyphony
    dirs, outs, clock

*/
