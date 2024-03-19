
enum TowersViewMode{
  SLIDER,
  STAFF,
  OMRI_CVENSIS,
  WINDOWS,
  // NOTES,
  TOWERS_DRAW_MODES
};

struct BipolarSlider : _less::Widget {
  float scaled_value = 0.0f;
  float *modded_value = NULL;

  bool *bipolar = NULL;

  bool last_bipolar = false;

  ParamQuantity *paramQuantity = NULL;

  float *color_pos = NULL;

  NVGcolor neg_color;
  NVGcolor pos_color;

  TowersViewMode *view_mode;

  float wid = 2.0f;

  int visible_octaves = 4;
  int staves = ((visible_octaves * 7) / 2);
  int index = 1;
  BipolarSlider(){
    load_font("font/Terminus.ttf");

    generateSegments();
  }

  void update_polarity(bool bi){
    paramQuantity->displayOffset = bi ? -5.0f : 0.0f;
  }

  void drawBipolar(float v){
    float hp = height - wid * 2.0f;
    fill(theme.output);
    path(1, center.y);
    float y = center.y + (v - 0.5f) * - hp;
    point(1, y);
    point(width - 1, y);
    point(width - 1, center.y);
    close(FILL);
    if(v > 0.5f)
      stroke(lerp(theme.output, pos_color, (v - 0.5f) * 2.0f));
    else
      stroke(lerp(neg_color, theme.output, v * 2.0f));
    line(1, y, width - 1, y);

    if(modded_value){
      if(*modded_value > 0.0f)
        stroke(lerp(theme.output, pos_color, rescale(*modded_value, 0.0f, 5.0f, 0.0, 1.0f)));
      else
        stroke(lerp(neg_color, theme.output, rescale(*modded_value, -5.0f, 0.0f, 0.0, 1.0f)));

      y = center.y + (rescale(*modded_value, -5.0f, 5.0f, 0.0, 1.0f) - 0.5f) * -hp;
      line(1, y, width - 1, y);
    }
  }

  void drawUnipolar(float v){
    fill(theme.output);
    float y = rescale(v, 0.0f, 1.0f, height - wid, wid);
    point(1, height - 1);
    point(1, y);
    point(width - 1, y);
    point(width - 1, height - 1);
    close(FILL);
    stroke(lerp(theme.output, pos_color, v));
    line(1, y, width - 1, y);

    if(modded_value){
      stroke(lerp(theme.output, pos_color, rescale(*modded_value, 0.0f, 10.0f, 0.0, 1.0f)));
      y = rescale(rescale(*modded_value, 0.0f, 10.0f, 0.0, 1.0f), 0.0f, 1.0f, height - wid, wid);
      line(1, y, width - 1, y);
    }
  }

  #define SHROOMSEGMENTS 8
  Vec segments[SHROOMSEGMENTS];
  void generateSegments(){
    for(int i = 0; i < SHROOMSEGMENTS; i++)
      segments[i] = Vec(random::uniform(), ((float) i / (float) SHROOMSEGMENTS));

  }
  void drawPsilocybeOmrinsisUnipolar(float v){

    // STEM
    fill(theme.transparent);
    stroke(pos_color);
    float y = rescale(v, 0.0f, 1.0f, height - wid, wid);

    int l = ceil(v * (float)SHROOMSEGMENTS);
    if(l > 0){
      for(int i = 0; i<l; i++){
        if(i == l - 1)
          point(center.x + width * (segments[i].x - 0.5f) * 0.2f, height * (1 - v) + 5.0f);
        else
          point(center.x + width * (segments[i].x - 0.5f) * 0.2f, height * (1 - segments[i].y));

      }
      end(STROKE);
    }
    fill(pos_color);
    stroke(lerp(theme.output, pos_color, v));

    // CAP
    float h = 4.20f + 10.0f * (1.0f - v);
    float w = v * width * 0.5;
    float x = center.x + width * (segments[l - 1].x - 0.5f) * 0.2f;
    point(x, y);
    point(x - w, y + h);
    point(x + w, y + h);
    close(FILL);
    
    // if(modded_value){
    //   stroke(lerp(theme.output, pos_color, rescale(*modded_value, 0.0f, 10.0f, 0.0, 1.0f)));
    //   y = rescale(rescale(*modded_value, 0.0f, 10.0f, 0.0, 1.0f), 0.0f, 1.0f, height - wid, wid);
    // }
  }
  void drawPsilocybeOmrinsisBipolar(float v){

    // STEM
    fill(theme.transparent);
    fill(theme.transparent);
    stroke(pos_color);
    
    NVGcolor c = v > 0.5f 
      ? lerp(theme.output, pos_color, (v - 0.5f) * 2.0f)
      : lerp(neg_color, theme.output, v * 2.0f);


    stroke(c);
    float y = height * 0.5f;
    line(1, 0.5f * box.size.y, width - 1, 0.5f * box.size.y);

    fill(theme.bg_dark);
    point(1, 1);
    point(1, y);
    point(width - 1, y);
    point(width - 1, 1);
    close(FILL);
    fill(c);

    y = rescale(v, 0.0f, 1.0f, height - wid, wid);

    int l = ceil(v * (float)SHROOMSEGMENTS);
    if(l > 0){
      for(int i = 0; i<l; i++){
        if(i == l - 1)
          point(center.x + width * (segments[i].x - 0.5f) * 0.2f, height * (1 - v) + 5.0f);
        else
          point(center.x + width * (segments[i].x - 0.5f) * 0.2f, height * (1 - segments[i].y));

      }
      end(STROKE);
    }
    // fill(pos_color);
    // stroke(lerp(theme.output, pos_color, v));

    // CAP
    v = v < 0.5f ? 0.1f : (v - 0.5f) * 2.0f * 0.9f + 0.1f;
    float h = 4.20f + 10.0f * (1.0f - v);
    float w = v * width * 0.5;
    float x = center.x + width * (segments[l - 1].x - 0.5f) * 0.2f;
    point(x, y);
    point(x - w, y + h);
    point(x + w, y + h);
    close(FILL);
    
    // if(modded_value){
    //   stroke(lerp(theme.output, pos_color, rescale(*modded_value, 0.0f, 10.0f, 0.0, 1.0f)));
    //   y = rescale(rescale(*modded_value, 0.0f, 10.0f, 0.0, 1.0f), 0.0f, 1.0f, height - wid, wid);
    // }
  }
  float staffy = 30.0f;
  float staffwd = 13.0f;
  float staffx = 30.0f;
  // float staffw2 = w * 0.5f;

  float staffh = 9.0f;
  float note_width = staffh * 0.5f;

  float legh = 30.0f;

  float sy(){
    return height - 5 * staffh;
  }

  float sw(){
    return box.size.x;
  }

  float middleCY(){
    return staffy + (5 * staffh);
  }
  
  int scale[12] = { 0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6 };
  int sharps[12] = { false, true, false, true, false, false, true, false, true, false, true, false };

  int octave(int n){
    return n / 12;
  }

  int noteToStaff(int n){
    int key = n % 12;
    int oct = octave(n);
    return oct * 7 - (12 - scale[key]) - 13;
  }
  bool flatSharp(int n){
    return sharps[(n % 12)];
  }
  float noteToY(float v){
    int m = Midi::cvToMidiRound(v);
    float h = box.size.y / staves;
    int i = noteToStaff(m) - 6;
    return center.y + (height - 12.0f * h) - (i * h * 0.5f);
  }

  void drawStave(){
    float h = box.size.y / (float)staves;
    for(int i = 0; i< staves + 1; i++){
      if(i == 0 || i == staves || (i >= 2 && i < 7) || (i >= 8 && i < 13))
        line(0, h * i, box.size.x, h * i);
    }
  }

  void drawSharp(){
    float w = 4.0f;
    // float w2 = w * 0.4f;
    // float o = 0.5f;
    round_cap();
    stroke_width(2.0f);
    line(-w, 0, w, 0);
    line(0, -w, 0, w);

    // line(-w, w2, w, w2 - o);
    // line(-w, -w2, w, -w2 - o);
    // line(-w2, -w, -w2, w);
    // line(w2, -w + o, w2, w + o);
  }

  void drawTuneLeg(float v, float w, float py){
    float x = w - 2.115f;

    float h = (height / (float)staves) * 3.0f - v * 2.0f;
    stroke_width(2.5f);
    float cents = Midi::midiRoundCents(v);    
    float y = py < height * 0.25 ? h : -h;
    // float y = v > 0.9f ? h : -h;

    round_cap();
    line(x, 0, x, y);

    push();
    stroke(cents < 0.0f ? lerp(neg_color, theme.input, 1.0f + cents * 2.0f) : lerp(theme.input, pos_color, cents * 2.0f));
    stroke_width(2.5f);
    line(x, y, x + clampf(width - center.x - x * 1.85f, 0.0f, 10.0f), y - (15.0f * cents));
    pop();

  }
  const std::string note_names[7] = {
    "C", "D", "E", "F", "G", "A", "B"
  };

  void drawNoteGlyph(float v, bool sharp){
    float y = noteToY(v);
    int offset = 0;
    if(y < -1.0f){
      y = noteToY(v - 4.0f);
      if(v > 5.58){
        float _y = height - v * 0.1f * height + 1.0f;
        stroke(pos_color);
        line(0, _y, width, _y);
        // TODO show out of midi range
      }
      offset = 1;
    }else if(y > height){
      y = noteToY(v + 4.0f);
      offset = -1;
    }


    fill(theme.input);
    float tp = 10.0f;
    text(note_names[scale[Midi::cvToMidiRound(v) % 12]], center.x - 2.0f, (y < 20.0f ? center.y : tp), 11.0f);

    push();
    translate(center.x, y);
    float w = clamp(width * 0.25f, 2.0f, 6.0f);
    float rx = w * 0.95f;
    float ry = w * 0.60f;
    stroke(offset == 0 ? theme.input : (offset == 1 ? pos_color : neg_color));
    fill(offset == 0 ? theme.input : (offset == 1 ? pos_color : neg_color));

    drawTuneLeg(v, w, y);

    ellipse(-2.0f, 0, rx,ry, BOTH);

    translate(-3.0f, - ry * 2.7f * (y < w * 2.5f ? -1.0f : 1.0f));
    if(sharp)
      drawSharp();
    pop();
  }
  void drawNote(float v){
    
    fill(theme.input);
    drawNoteGlyph(v, flatSharp(Midi::cvToMidiRound(v)));
  }

  void drawWindows(float v){
    alpha(0.5f);
    if(modded_value)
      v = *modded_value;
    fill(v > 0.1f ? pos_color : theme.bg_dark);
    center_rect(center.x, center.y, width - 2.0f, height);
  }

  void step()override{
  }

  void render_lights() override{
    float v = modded_value ? scaled_value : random::uniform();

    if(bipolar && *bipolar != last_bipolar){
      last_bipolar = *bipolar;
      update_polarity(last_bipolar);
    }

    float c = color_pos ? *color_pos : 0.0f;
    neg_color = theme.rainbow_color(c);
    pos_color = theme.rainbow_color(float_wrap(c + 0.5f));

    fill(theme.fg);

    if(false && *view_mode == STAFF){
      float p = 3.0f;
      text(std::to_string(index % 10), center.x - p, -8.5f, 11.0f);
      text(note_names[scale[Midi::cvToMidiRound(v) % 12]], center.x + p, -8.5f, 11.0f);
    }else{
      text(std::to_string(index % 10), center.x, -8.5f, 11.0f);
    }
    // fill(theme.fg);
    scissor();
    stroke_width(wid);
    stroke(theme.fg);
    fill(theme.output_dark);
    rect(0,0,width, height, BOTH);


    fill(theme.output);


    if(view_mode){
      switch(*view_mode){
        case SLIDER : {
          if(bipolar && *bipolar)
            drawBipolar(v);
          else
            drawUnipolar(v);
        }break;

        case STAFF : {
          float _v = bipolar && *bipolar ? *modded_value : *modded_value;


          stroke(theme.input_dark);
          stroke_width(1.0f);
          drawStave();
          drawNote(_v);
        }break;

        case OMRI_CVENSIS : {
          if(bipolar && *bipolar)
            drawPsilocybeOmrinsisBipolar(v);
          else
            drawPsilocybeOmrinsisUnipolar(v);
        }break;

        case WINDOWS : {
          if(bipolar && *bipolar)
            drawBipolar(v);
          else
            drawUnipolar(v);
          drawWindows(v);
        }break;

        default : 
        break;
      }
    }else{
      drawUnipolar(random::uniform());
    }
  }
};
