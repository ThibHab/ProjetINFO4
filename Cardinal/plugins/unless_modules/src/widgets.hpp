#pragma once
#include "utils.hpp"
// #include "_ui.hpp"

// using namespace unless_colors;

struct OnOffSwitch : SVGSwitch {
  OnOffSwitch() {
    // setSvg(APP->window->loadSvg(asset::system("art/svg/ComponentLibrary/RoundLargeBlackKnob.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/OffSwitch.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/OnSwitch.svg")));
  }
};
struct LoopSwitch : SVGSwitch {
  LoopSwitch() {
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/LoopOffButton.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/LoopOnButton.svg")));
  }
  // void randomize() override{}
};
struct CvGateSwitch : SVGSwitch {
  CvGateSwitch() {
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/CvButton.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/GateButton.svg")));
  }
  // void randomize() override{}
};

struct BlankButton : SVGSwitch{
  BlankButton() {
    momentary = true;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/BlankButton.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/BlankButtonDown.svg")));
  }
};

struct AddButton : SVGSwitch{
  AddButton() {
    momentary = true;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/AddButton.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/AddButtonDown.svg")));
  }
};
struct PlayButton : SVGSwitch{
  PlayButton() {
    momentary = true;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/PlayButton.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/PlayButtonDown.svg")));
  }
};
struct DotButton : SVGSwitch{
  DotButton() {
    momentary = true;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/DotButton.svg")));
  }
};
struct DotDotButton : SVGSwitch{
  DotDotButton() {
    momentary = true;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/DotDotButton.svg")));
  }
};
struct StopButton : SVGSwitch{
  StopButton() {
    momentary = true;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/StopButton.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/StopButtonDown.svg")));
  }
};
struct RecordButton : SVGSwitch{
  RecordButton() {
    momentary = true;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/RecordButton.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/RecordButtonDown.svg")));
  }
};
struct UpButton : SVGSwitch{
  UpButton() {
    momentary = true;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/UpButton.svg")));
  }
};
struct DownButton : SVGSwitch{
  DownButton() {
    momentary = true;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/DownButton.svg")));
  }
};
struct LeftButton : SVGSwitch{
  LeftButton() {
    momentary = true;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/LeftButton.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/LeftButtonDown.svg")));
  }
};
struct RightButton : SVGSwitch{
  RightButton() {
    momentary = true;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/RightButton.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/RightButtonDown.svg")));
  }
};

struct SunKnob : SVGKnob {
  SunKnob(){
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/knobs/SunKnob.svg")));
    minAngle = -3.1414f * 0.8f;
    maxAngle = 3.1415f * 0.8f;
  }
};

struct RotaryEncoderKnob : SVGKnob{
  RotaryEncoderKnob(){
    minAngle = -99999.f;
    maxAngle = 99999.f;
    speed = 0.00001f;
    smooth = false;
  }
};
struct StartEncoder : RotaryEncoderKnob {
  StartEncoder() {
    // minAngle = -99999.f;
    // maxAngle = 99999.f;
    // speed = 0.00004f;
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/knobs/StartEncoder.svg")));
  }
  // void randomize() override{}
};
struct StopEncoder : RotaryEncoderKnob {
  StopEncoder() {
    // minAngle = -99999.f;
    // maxAngle = 99999.f;
    // speed = 0.00004f;
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/knobs/StopEncoder.svg")));
  }
  // void randomize() override{}
};
struct ArrowSnapKnob : SVGKnob {
  ArrowSnapKnob() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/knobs/ArrowKnob.svg")));
    // snap = true;
    // smooth = false;
    minAngle = -3.1414f;
    maxAngle = 3.1415f - (3.1415f * 0.25f);
    // shadow->opacity = 0.0;
  }
};
struct SmallKnob : SVGKnob {
  SmallKnob(){
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/knobs/SmallKnob.svg")));
    minAngle = -3.1414f + (3.1415f * 0.25f);
    maxAngle = 3.1415f - (3.1415f * 0.25f);
  }
  // void randomize() override{}
};


struct PiongKnob : SVGKnob {
  PiongKnob(){
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/knobs/PiongKnob.svg")));
    minAngle = -3.1414f + (3.1415f * 0.25f);
    maxAngle = 3.1415f - (3.1415f * 0.25f);
  }
};
struct PiongKnobSmall : SVGKnob {
  PiongKnobSmall(){
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/knobs/PiongKnobSmall.svg")));
    minAngle = -3.1414f + (3.1415f * 0.25f);
    maxAngle = 3.1415f - (3.1415f * 0.25f);
  }
};

struct TransparentSnapKnob : SVGKnob {
  TransparentSnapKnob() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/knobs/TransparentKnob.svg")));
    // snap = true;
    // smooth = false;
    shadow->opacity = 0.0;
  }
};
// struct RoundSmallBlackSnapKnob : RoundSmallBlackKnob {
//   RoundSmallBlackSnapKnob() {
//     // snap = true;
//     // smooth = false;
//   }
// };
struct SmallOutputPort : SVGPort {
  SmallOutputPort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/ports/SmallDarkPort.svg")));    
    shadow->opacity = 0.0;
  }
};
struct SmallDarkPort : SVGPort {
  SmallDarkPort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/ports/SmallDarkPort.svg")));    
    // shadow->opacity = 0.0;
  }
};
struct FlatDarkPort : SVGPort {
  FlatDarkPort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/ports/FlatDarkPort.svg")));    
    // shadow->opacity = 0.0;
  }
};
struct DarkHole : SVGPort {
  DarkHole() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/ports/DarkHole.svg")));    
    shadow->opacity = 0.0;
  }
};
struct SmallHole : SVGPort {
  SmallHole() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/ports/SmallHole.svg")));    
    shadow->opacity = 0.0;
  }
};
struct SmallBrightPort : SVGPort {
  SmallBrightPort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/ports/SmallBrightPort.svg")));    
    // shadow->opacity = 0.0;
  }
};
struct FlatInputPort : SVGPort {
  FlatInputPort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/ports/FlatInputPort.svg")));    
  }
};
struct InputPort : SVGPort {
  InputPort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/ports/InputPort.svg")));    
  }
};
struct InputGatePort : SVGPort {
  InputGatePort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/ports/InputGatePort.svg")));    
    shadow->opacity = 0.0;
  }
};
struct VOctOutputPort : SVGPort {
  VOctOutputPort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/ports/VOctOutputPort.svg")));    
    shadow->opacity = 0.0;
  }
};
struct VOctInputPort : SVGPort {
  VOctInputPort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/ports/VOctInputPort.svg")));    
    shadow->opacity = 0.0;
  }
};
struct OutputGatePort : SVGPort {
  OutputGatePort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/ports/OutputGatePort.svg")));    
    shadow->opacity = 0.0;
  }
};
struct OutputPort : SVGPort {
  OutputPort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/ports/OutputPort.svg")));    
  }
};
struct PeaceScrewButton : SVGSwitch{
  PeaceScrewButton() {
    momentary = true;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/misc/PeaceScrew.svg")));
  }
};
struct PeaceScrew : SVGScrew {
  PeaceScrew() {
    sw->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/misc/PeaceScrew.svg")));
    box.size = sw->box.size;
    // sw->rotate(randomUniform() * 360.f);
  }
};
struct TreeScrew : SVGScrew {
  TreeScrew() {
    sw->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/misc/TreeScrew.svg")));
    box.size = sw->box.size;
    // rotate(randomUniform() * 360.f);
  }
};
// struct OrangeLight : GrayModuleLightWidget {
//   OrangeLight(){
//     addBaseColor(nvgRGB(0xff, 0xaa, 0x44));
//   }
// };
// struct WhiteLight : GrayModuleLightWidget {
//   WhiteLight(){
//     addBaseColor(nvgRGB(0xff, 0xff, 0xfa));
//   }
// };
struct AutoOrGate : SVGSwitch {
  AutoOrGate() {
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/AG_Gate.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "art/svg/buttons/AG_Auto.svg")));
  }
};


// ========================================================================================
// GLOBALLY COLORED WIDGETS
// ========================================================================================



// struct GtOutput24 : SVGPort {
//   int border = C_OUTB, main = C_OUT;
//   GtOutput24(){
//     box.size = Vec(24,24);
//   }
//   void draw(NVGcontext *vg) override{
//     float s = box.size.x * 0.82f;
//     float offs = box.size.x * 0.09f;
//     nvgBeginPath(vg);
//     nvgFillColor(vg, ColorScheme::get(C_SHADOW));
//     nvgRect(vg, offs, offs + s * 0.1f, s, s);
//     nvgFill(vg);

//     nvgBeginPath(vg);
//     nvgFillColor(vg, ColorScheme::get(border));
//     nvgRect(vg, offs, offs, s, s);
//     nvgFill(vg);

//     nvgBeginPath(vg);
//     nvgFillColor(vg, ColorScheme::get(main));
//     nvgRect(vg, offs + s * 0.05, offs + s * 0.05, s * 0.9, s * 0.9);
//     nvgFill(vg);

//     nvgTranslate(vg, box.size.x * 0.5, box.size.y * 0.5);
//     nvgBeginPath(vg);
//     nvgFillColor(vg, ColorScheme::get(C_SHADOW));
//     nvgCircle(vg, 0, -1, 5);
//     nvgFill(vg);

//     nvgBeginPath(vg);
//     nvgFillColor(vg, ColorScheme::get(C_HOLE));
//     nvgCircle(vg, 0, 0, 5);
//     nvgFill(vg);
//   }
// };

// struct GtInput24 : GtOutput24 {
//   GtInput24(){
//     border = C_INB;
//     main = C_IN;
//   }
// };

// struct CvOutput24 : SVGPort {
//   int border = C_OUTB, main = C_OUT;
//   CvOutput24(){
//     box.size = Vec(24,24);
//   }
//   void draw(NVGcontext *vg) override{
//     float half = box.size.x * 0.5f;
//     nvgTranslate(vg, half, half);
//     nvgBeginPath(vg);
//     nvgFillColor(vg, ColorScheme::get(C_SHADOW));
//     nvgCircle(vg, 0, box.size.y * 0.1, half);
//     nvgFill(vg);

//     nvgBeginPath(vg);
//     nvgFillColor(vg, ColorScheme::get(border));
//     nvgCircle(vg, 0, 0, half);
//     nvgFill(vg);

//     nvgBeginPath(vg);
//     nvgFillColor(vg, ColorScheme::get(main));
//     nvgCircle(vg, 0, 0, half * 0.86);
//     nvgFill(vg);

//     nvgBeginPath(vg);
//     nvgFillColor(vg, ColorScheme::get(C_SHADOW));
//     nvgCircle(vg, 0, -1, 5);
//     nvgFill(vg);

//     nvgBeginPath(vg);
//     nvgFillColor(vg, ColorScheme::get(C_HOLE));
//     nvgCircle(vg, 0, 0, 5);
//     nvgFill(vg);
//   }
// };


// struct CvInput24 : CvOutput24 {
//   CvInput24(){
//     border = C_INB;
//     main = C_IN;
//     // box.size = Vec(28,28);
//   }
// };

