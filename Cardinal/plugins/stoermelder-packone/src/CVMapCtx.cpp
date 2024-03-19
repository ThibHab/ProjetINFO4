#include "plugin.hpp"
#include "CVMap.hpp"
#include "components/LedTextField.hpp"

namespace StoermelderPackOne {
namespace CVMap {

struct CVMapCtxModule : CVMapCtxBase {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	/** [Stored to JSON] */
	int panelTheme = 0;
	/** [Stored to JSON] */
	std::string cvMapId;

	CVMapCtxModule() {
		panelTheme = pluginSettings.panelThemeDefault;
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		onReset();
	}

	void onReset() override {
		Module::onReset();
		cvMapId = "";
	}

	std::string getCVMapId() override {
		return cvMapId;
	}

	json_t* dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "panelTheme", json_integer(panelTheme));
		json_object_set_new(rootJ, "cvMapId", json_string(cvMapId.c_str()));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		panelTheme = json_integer_value(json_object_get(rootJ, "panelTheme"));
		cvMapId = json_string_value(json_object_get(rootJ, "cvMapId"));
	}
};


struct IdTextField : StoermelderTextField {
	CVMapCtxModule* module;
	void step() override {
		StoermelderTextField::step();
		if (!module) return;
		if (isFocused) module->cvMapId = text;
		else text = module->cvMapId;
	}
};

struct CVMapCtxWidget : ThemedModuleWidget<CVMapCtxModule> {
	CVMapCtxWidget(CVMapCtxModule* module)
		: ThemedModuleWidget<CVMapCtxModule>(module, "CVMapCtx", "CVMap.md#ctx-expander") {
		setModule(module);

		addChild(createWidget<StoermelderBlackScrew>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<StoermelderBlackScrew>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		IdTextField* textField = createWidget<IdTextField>(Vec());
		textField->textSize = 13.f;
		textField->maxTextLength = 8;
		textField->module = module;
		textField->box.size = Vec(54.f, 13.f);

		TransformWidget* tw = new TransformWidget;
		tw->addChild(textField);
		tw->box.pos = Vec(-12.f, 305.f);
		tw->box.size = Vec(120.f, 13.f);
		addChild(tw);

		math::Vec center = textField->box.getCenter();
		tw->identity();
		tw->translate(center);
		tw->rotate(-M_PI / 2);
		tw->translate(center.neg());
	}
};

} // namespace CVMap
} // namespace StoermelderPackOne

Model* modelCVMapCtx = createModel<StoermelderPackOne::CVMap::CVMapCtxModule, StoermelderPackOne::CVMap::CVMapCtxWidget>("CVMapCtx");