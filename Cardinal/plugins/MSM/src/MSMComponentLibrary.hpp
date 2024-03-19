#pragma once

#include "rack.hpp"

/////////////////////////////////////////////////////////////////////////////////////////
//
// COMPONENTS LIBRARY
//
/////////////////////////////////////////////////////////////////////////////////////////

////////////////////
// Knobs
////////////////////

struct MSMLargeKnob : SvgKnob {
	MSMLargeKnob() {
		minAngle = -0.78*M_PI;
		maxAngle = 0.78*M_PI;
		shadow->blurRadius = 4.0;
		shadow->opacity = 0.8;
	}
};


struct GreyLargeKnob : MSMLargeKnob {
	GreyLargeKnob() {
		box.size = Vec(47, 47);
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Knobs/GreyLargeKnob.svg")));
	}
};

struct BlueLargeKnob : MSMLargeKnob {
	BlueLargeKnob() {
		box.size = Vec(47, 47);
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Knobs/BlueLargeKnob.svg")));
	}
};

struct GreenLargeKnob : MSMLargeKnob {
	GreenLargeKnob() {
		box.size = Vec(47, 47);
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Knobs/GreenLargeKnob.svg")));
	}
};

struct RedLargeKnob : MSMLargeKnob {
	RedLargeKnob() {
		box.size = Vec(47, 47);
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Knobs/RedLargeKnob.svg")));
	}
};

struct VioLargeKnob : MSMLargeKnob {
	VioLargeKnob() {
		box.size = Vec(47, 47);
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Knobs/VioLargeKnob.svg")));
	}
};

struct MSMSmallKnob : SvgKnob {
	MSMSmallKnob() {
		minAngle = -0.78*M_PI;
		maxAngle = 0.78*M_PI;
		shadow->blurRadius = 2.5;
		shadow->opacity = 1.0;
	}
};

struct GreySmallKnob : MSMSmallKnob {
	GreySmallKnob() {
		box.size = Vec(32, 32);
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Knobs/GreySmallKnob.svg")));
	}
};

struct BlueSmallKnob : MSMSmallKnob {
	BlueSmallKnob() {
		box.size = Vec(32, 32);
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Knobs/BlueSmallKnob.svg")));
	}
};

struct GreenSmallKnob : MSMSmallKnob {
	GreenSmallKnob() {
		box.size = Vec(32, 32);
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Knobs/GreenSmallKnob.svg")));
	}
};

struct RedSmallKnob : MSMSmallKnob {
	RedSmallKnob() {
		box.size = Vec(32, 32);
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Knobs/RedSmallKnob.svg")));
	}
};

struct YellowSmallKnob : MSMSmallKnob {
	YellowSmallKnob() {
		box.size = Vec(32, 32);
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Knobs/YellowSmallKnob.svg")));
	}
};

struct GreenTinyKnob : MSMSmallKnob {
	GreenTinyKnob() {
		box.size = Vec(25, 25);
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Knobs/GreenTinyKnob.svg")));
	}
};

struct MSMToggleKnob : SvgKnob {
	MSMToggleKnob() {
		minAngle = -0.78*M_PI;
		maxAngle = 0.78*M_PI;
		shadow->blurRadius = 2.5;
		shadow->opacity = 1.0;
	}

	void onChange(const event::Change &e) override {
    getParamQuantity()->setValue(roundf(getParamQuantity()->getValue()));
    SvgKnob::onChange(e);
  }
};

struct GreenToggleKnobSmall : MSMToggleKnob {
	GreenToggleKnobSmall() {
		box.size = Vec(32, 32);
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Knobs/GreenSmallKnob.svg")));
		snap = true;
		smooth = false;
	}
};

struct RedSmallToggleKnob : MSMToggleKnob {
	RedSmallToggleKnob() {
		box.size = Vec(32, 32);
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Knobs/RedSmallKnob.svg")));
		snap = true;
		smooth = false;
	}
};

struct BlueSmallToggleKnob : MSMToggleKnob {
	BlueSmallToggleKnob() {
		box.size = Vec(32, 32);
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Knobs/BlueSmallKnob.svg")));
		snap = true;
		smooth = false;
	}
};

struct MSMToggle2Knob : SvgKnob {
	MSMToggle2Knob() {
		minAngle = -0.78*M_PI;
		maxAngle = 0.78*M_PI;
		shadow->blurRadius = 4.0;
		shadow->opacity = 0.8;
		snap = true;
		smooth = false;
	}

	void onChange(const event::Change &e) override {
    getParamQuantity()->setValue(roundf(getParamQuantity()->getValue()));
    SvgKnob::onChange(e);
  }
};


struct RedLargeToggleKnob : MSMToggle2Knob {
	RedLargeToggleKnob() {
		box.size = Vec(47, 47);
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Knobs/RedLargeKnob.svg")));
		shadow->blurRadius = 4.0;
		shadow->opacity = 0.8;
	}
};

/////////////////////////
// Slider
/////////////////////////

struct MSMSlider : SvgSlider {
	MSMSlider() {
		Vec margin = Vec(3.6, 3.6);
		maxHandlePos = Vec(11.0, 0.0).plus(margin);
		minHandlePos = Vec(11.0, 140.0).plus(margin);
		setBackgroundSvg(APP->window->loadSvg(asset::plugin(pluginInstance,"res/Slider/slider.svg")));
		setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Slider/sliderhandle.svg")));
		background->box.pos = margin;
		box.size = background->box.size.plus(margin.mult(2));
	}
};

struct MSMSlidePot : SvgSlider {
	MSMSlidePot() {
		Vec margin = Vec(4, 4);
		maxHandlePos = Vec(0.0, -5.0).plus(margin);
		minHandlePos = Vec(0.0, 90.0).plus(margin);
		setBackgroundSvg(APP->window->loadSvg(asset::plugin(pluginInstance,"res/Slider/SlidePot.svg")));
		setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Slider/SlidePotHandle.svg")));
		background->box.pos = margin;
		box.size = background->box.size.plus(margin.mult(2));
	}
};

/////////////////////////
// Jacks
/////////////////////////

struct MSMPort : SvgPort {
	MSMPort() {
	}
};

struct SilverSixPort : MSMPort {
	SilverSixPort() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Port/SilverSixPort.svg")));
		shadow->blurRadius = 5.0;
		shadow->opacity = 0.75;
	}

};

struct SilverSixPortA : MSMPort {
	SilverSixPortA() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Port/SilverSixPortA.svg")));
		shadow->blurRadius = 5.0;
		shadow->opacity = 0.75;
	}

};

struct SilverSixPortB : MSMPort {
	SilverSixPortB() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Port/SilverSixPortB.svg")));
		shadow->blurRadius = 5.0;
		shadow->opacity = 0.75;
	}

};

struct SilverSixPortC : MSMPort {
	SilverSixPortC() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Port/SilverSixPortC.svg")));
		shadow->blurRadius = 5.0;
		shadow->opacity = 0.75;
	}

};

struct SilverSixPortD : MSMPort {
	SilverSixPortD() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Port/SilverSixPortD.svg")));
		shadow->blurRadius = 5.0;
		shadow->opacity = 0.75;
	}

};

struct SilverSixPortE : MSMPort {
	SilverSixPortE() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Port/SilverSixPortE.svg")));
		shadow->blurRadius = 5.0;
		shadow->opacity = 0.75;
	}

};

/////////////////////////
// Switch
/////////////////////////

struct ToggleMe : SvgSwitch {
	ToggleMe()
	{

	}
};

struct ToggleMe2 : SvgSwitch {
	ToggleMe2()
	{

	}
};

struct MThree : ToggleMe {
	MThree() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Switch/CKSSThree_2.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Switch/CKSSThree_1.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Switch/CKSSThree_0.svg")));
	}
};

struct MThree2 : ToggleMe2 {
	MThree2() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Switch/CKSSThree_2.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Switch/CKSSThree_1.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Switch/CKSSThree_0.svg")));
	}
};

struct VioMSwitch : ToggleMe {
	VioMSwitch() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Switch/VioMSwitch_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Switch/VioMSwitch_1.svg")));
	}
};

struct VioM2Switch : ToggleMe2 {
	VioM2Switch() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Switch/VioMSwitch_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Switch/VioMSwitch_1.svg")));
	}
};

struct FMSM : ToggleMe {
	FMSM() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Switch/FMSM_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Switch/FMSM_1.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Switch/FMSM_2.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Switch/FMSM_3.svg")));
	}
};

/////////////////////////
// Button
/////////////////////////

struct MToggleGrey : SvgSwitch {
	MToggleGrey() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button/GreyRoundToggle_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button/GreyRoundToggle_1.svg")));
	}
};

struct OSCiXEGG : SvgSwitch {
	OSCiXEGG() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button/Easteregg_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button/Easteregg_1.svg")));
	}
};

struct MButton : SvgSwitch {
	MButton() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button/ButtonGreen0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button/ButtonGreen1.svg")));
	}
};

struct ThreeColorButton : SvgSwitch {
	ThreeColorButton() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button/ColorButton0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button/ColorButton1.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button/ColorButton2.svg")));
	}
};

struct FiveColorButton : SvgSwitch {
	FiveColorButton() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button/ColorButton0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button/ColorButton1.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button/ColorButton2.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button/ColorButton3.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button/ColorButton4.svg")));
	}
};

struct SixColorButton : SvgSwitch {
	SixColorButton() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button/ColorButton0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button/ColorButton1.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button/ColorButton2.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button/ColorButton3.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button/ColorButton4.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Button/ColorButton5.svg")));
	}
};

////////////////////////
// Screws
////////////////////////

struct MScrewA : SvgScrew {
		MScrewA() {
		sw->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Screws/MScrewA.svg")));
		box.size = sw->box.size;
	}
};

struct MScrewB : SvgScrew {
		MScrewB() {
		sw->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Screws/MScrewB.svg")));
		box.size = sw->box.size;
	}
};

struct MScrewC : SvgScrew {
		MScrewC() {
		sw->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Screws/MScrewC.svg")));
		box.size = sw->box.size;
	}
};

struct MScrewD : SvgScrew {
		MScrewD() {
		sw->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Screws/MScrewD.svg")));
		box.size = sw->box.size;
	}
};

////////////////////////
// Light
////////////////////////

static const NVGcolor MCOLOR_BLUE = nvgRGB(0x16, 0xBC, 0xEB);

struct MGrayModuleLightWidget : ModuleLightWidget {
	MGrayModuleLightWidget() {
		bgColor = nvgRGB(0x5a, 0x5a, 0x5a);
		borderColor = nvgRGBA(0, 0, 0, 0x60);
	}
};

template <typename BASE>
struct MLargeLight : BASE {
	MLargeLight() {
		this->box.size = mm2px(Vec(6.0, 6.0));
	}
};

struct MBlueLight : MGrayModuleLightWidget {
	MBlueLight() {
		addBaseColor(MCOLOR_BLUE);
	}
};

/////////////////////////////////////////////
// Panel
// Panel widget forward ported from Rack 0.6
// Widget was eliminated in v1
/////////////////////////////////////////////

struct MSMPanel : TransparentWidget {
  NVGcolor backgroundColor;
  std::string imagePath;
	void draw(const DrawArgs &args) override {
      std::shared_ptr<Image> backgroundImage = APP->window->loadImage(imagePath);
	  nvgBeginPath(args.vg);
	  nvgRect(args.vg, 0.0, 0.0, box.size.x, box.size.y);

	  // Background color
	  if (backgroundColor.a > 0) {
	    nvgFillColor(args.vg, backgroundColor);
	    nvgFill(args.vg);
	  }

	  // Background image
	  if (backgroundImage) {
	    int width, height;
	    nvgImageSize(args.vg, backgroundImage->handle, &width, &height);
	    NVGpaint paint = nvgImagePattern(args.vg, 0.0, 0.0, width, height, 0.0, backgroundImage->handle, 1.0);
	    nvgFillPaint(args.vg, paint);
	    nvgFill(args.vg);
	  }

	  // Border
	  NVGcolor borderColor = nvgRGBAf(0.5, 0.5, 0.5, 0.5);
	  nvgBeginPath(args.vg);
	  nvgRect(args.vg, 0.5, 0.5, box.size.x - 1.0, box.size.y - 1.0);
	  nvgStrokeColor(args.vg, borderColor);
	  nvgStrokeWidth(args.vg, 1.0);
	  nvgStroke(args.vg);

	  Widget::draw(args);
	}
};

struct MSMLightPanel : MSMPanel {
  MSMLightPanel() {
    backgroundColor = componentlibrary::SCHEME_LIGHT_GRAY;
  }
};

/////////////////////////////////////////////////////////////////////////////////////
