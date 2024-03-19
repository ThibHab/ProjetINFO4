#include "plugin.hpp"
#include "components/Knobs.hpp"
#include "ui/OverlayMessageWidget.hpp"

namespace StoermelderPackOne {
namespace Sail {

enum class IN_MODE {
	DIFF = 0,
	ABSOLUTE = 1
};

enum class OUT_MODE {
	REDUCED = 0,
	FULL = 1
};

struct SailModule : Module {
	enum ParamIds {
		PARAM_SLEW,
		PARAM_STEP,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_VALUE,
		INPUT_FINE,
		INPUT_SLEW,
		INPUT_INC,
		INPUT_DEC,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		LIGHT_ACTIVE,
		NUM_LIGHTS
	};

	const float FINE = 0.1f;

	/** [Stored to JSON] */
	int panelTheme = 0;
	/** [Stored to JSON] */
	IN_MODE inMode;
	/** [Stored to JSON] */
	OUT_MODE outMode;

	dsp::RingBuffer<int, 8> overlayQueue;
	/** [Stored to Json] */
	bool overlayEnabled;
	uint16_t overlayMessageId = 0;

	bool fineMod;

	float inVoltBase;
	float inVoltTarget;
	float incdecTarget;

	float valueBaseOut;
	float valuePrevious;

	bool hoveredWidgetNull = true;
	WeakPtr<ParamWidget> hoveredWidget;
	int64_t hoveredModuleId = -1;
	int hoveredParamId = -1;

	dsp::SchmittTrigger incTrigger;
	dsp::SchmittTrigger decTrigger;

	dsp::ClockDivider processDivider;
	dsp::ClockDivider lightDivider;
	dsp::ExponentialSlewLimiter slewLimiter;

	SailModule() {
		panelTheme = pluginSettings.panelThemeDefault;
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configInput(INPUT_VALUE, "Parameter value");
		configInput(INPUT_FINE, "Fine adjustment gate");
		inputInfos[INPUT_FINE]->description = "When high the INC/DEC-inputs apply fine adjustments on the hovered parameter.";
		configInput(INPUT_SLEW, "Slew CV");
		configInput(INPUT_INC, "Increment parameter");
		configInput(INPUT_DEC, "Decrement parameter");
		configOutput(OUTPUT, "Parameter value");
		configParam(PARAM_SLEW, 0.f, 5.f, 0.f, "Slew limiting", "s");
		configParam(PARAM_STEP, 0.f, 2.f, 0.2f, "Stepsize", "%", 0.f, 10.f);
		configLight(LIGHT_ACTIVE, "Adjustable parameter indication");
		processDivider.setDivision(32);
		lightDivider.setDivision(512);
		onReset();
	}

	void onReset() override {
		Module::onReset();
		inMode = IN_MODE::DIFF;
		outMode = OUT_MODE::REDUCED;
		slewLimiter.reset();
		overlayEnabled = true;
	}

	void process(const ProcessArgs& args) override {
		if (incTrigger.process(inputs[INPUT_INC].getVoltage())) {
			float step = params[PARAM_STEP].getValue() / 10.f;
			if (fineMod || inputs[INPUT_FINE].getVoltage() >= 1.f) step *= FINE;
			incdecTarget += step;
		}
		if (decTrigger.process(inputs[INPUT_DEC].getVoltage())) {
			float step = params[PARAM_STEP].getValue() / 10.f;
			if (fineMod || inputs[INPUT_FINE].getVoltage() >= 1.f) step *= FINE;
			incdecTarget -= step;
		}

		ParamQuantity* paramQuantity = NULL;
		// paramQuantity is guaranteed to be existing after this point as we are in the middle of a sample
		if (!hoveredWidgetNull && hoveredWidget.get() != nullptr) {
			paramQuantity = hoveredWidget->getParamQuantity();
		}

		if (processDivider.process() && paramQuantity && paramQuantity->module != this) {
			if (paramQuantity->module->id != hoveredModuleId || paramQuantity->paramId != hoveredParamId) {
				hoveredModuleId = paramQuantity->module->id;
				hoveredParamId = paramQuantity->paramId;
				overlayMessageId++;
				// Current parameter value
				valuePrevious = paramQuantity->getScaledValue();
				inVoltTarget = incdecTarget = slewLimiter.out = valuePrevious;
				inVoltBase = clamp(inputs[INPUT_VALUE].getVoltage() / 10.f, 0.f, 1.f);
			}

			if (paramQuantity->isBounded()) {
				float valueNext = valuePrevious;

				if (inputs[INPUT_VALUE].isConnected()) {
					// IN-port
					float inVolt = clamp(inputs[INPUT_VALUE].getVoltage() / 10.f, 0.f, 1.f);
					switch (inMode) {
						case IN_MODE::DIFF: {
							// Change since last time
							float d1 = inVolt - inVoltBase;
							inVoltBase = inVolt;
							if (fineMod || inputs[INPUT_FINE].getVoltage() >= 1.f) d1 *= FINE;
							// Actual change of parameter after slew limiting
							float d2 = inVoltTarget - valuePrevious;
							// Reapply the sum of both
							valueNext = clamp(valuePrevious + d1 + d2, 0.f, 1.f);
							inVoltTarget = valueNext;
							break;
						}
						case IN_MODE::ABSOLUTE: {
							// Only move on input voltage change
							if (inVolt != inVoltBase) {
								valueNext = inVolt;
								// Detach when target value has been reached
								if (valuePrevious == inVolt) inVoltBase = inVolt;
							}
							break;
						}
					}
				}
				else {
					// INC/DEC-ports
					incdecTarget = clamp(incdecTarget, 0.f, 1.f);
					valueNext = incdecTarget;
				}

				// Apply slew limiting
				float slew = inputs[INPUT_SLEW].isConnected() ? clamp(inputs[INPUT_SLEW].getVoltage(), 0.f, 5.f) : params[PARAM_SLEW].getValue();
				if (slew > 0.f) {
					slew = (1.f / slew) * 10.f;
					slewLimiter.setRiseFall(slew, slew);
					valueNext = slewLimiter.process(args.sampleTime * processDivider.getDivision(), valueNext);
				}

				// Determine the relative change
				float delta = valueNext - valuePrevious;
				if (delta != 0.f) {
					paramQuantity->moveScaledValue(delta);
					valueBaseOut = paramQuantity->getScaledValue();
					if (overlayEnabled && overlayQueue.capacity() > 0) overlayQueue.push(overlayMessageId);
				}

				valuePrevious = valueNext;

				if (outputs[OUTPUT].isConnected()) {
					switch (outMode) {
						case OUT_MODE::REDUCED: {
							float v = paramQuantity->getScaledValue();
							if (v != valueBaseOut) {
 								outputs[OUTPUT].setVoltage(v * 10.f);
							}
							break;
						}
						case OUT_MODE::FULL: {
							outputs[OUTPUT].setVoltage(paramQuantity->getScaledValue() * 10.f);
							break;
						}
					}
				}
			}
		}

		if (lightDivider.process()) {
			bool active = paramQuantity && paramQuantity->isBounded() && paramQuantity->module != this;
			lights[LIGHT_ACTIVE].setSmoothBrightness(active ? 1.f : 0.f, args.sampleTime * lightDivider.getDivision());
		}
	}

	void setHoveredWidget(ParamWidget* pw) {
		if (pw) {
			hoveredWidgetNull = false;
			if (hoveredModuleId != pw->module->id || hoveredParamId != pw->paramId) {
				hoveredWidget = pw;
			}
		}
		else {
			hoveredWidgetNull = true;
			hoveredModuleId = -1;
			hoveredParamId = -1;
		}
	}

	json_t* dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "panelTheme", json_integer(panelTheme));
		json_object_set_new(rootJ, "inMode", json_integer((int)inMode));
		json_object_set_new(rootJ, "outMode", json_integer((int)outMode));
		json_object_set_new(rootJ, "overlayEnabled", json_boolean(overlayEnabled));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		panelTheme = json_integer_value(json_object_get(rootJ, "panelTheme"));
		inMode = (IN_MODE)json_integer_value(json_object_get(rootJ, "inMode"));
		outMode = (OUT_MODE)json_integer_value(json_object_get(rootJ, "outMode"));
		json_t* overlayEnabledJ = json_object_get(rootJ, "overlayEnabled");
		if (overlayEnabledJ) overlayEnabled = json_boolean_value(overlayEnabledJ);
	}
};


struct SailWidget : ThemedModuleWidget<SailModule>, OverlayMessageProvider {
	SailWidget(SailModule* module)
		: ThemedModuleWidget<SailModule>(module, "Sail") {
		setModule(module);

		addChild(createWidget<StoermelderBlackScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<StoermelderBlackScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addChild(createLightCentered<TinyLight<WhiteLight>>(Vec(22.5f, 38.0f), module, SailModule::LIGHT_ACTIVE));

		addInput(createInputCentered<StoermelderPort>(Vec(22.5f, 69.0f), module, SailModule::INPUT_FINE));

		addInput(createInputCentered<StoermelderPort>(Vec(22.5f, 113.3f), module, SailModule::INPUT_SLEW));
		addParam(createParamCentered<StoermelderTrimpot>(Vec(22.5f, 137.9f), module, SailModule::PARAM_SLEW));

		addParam(createParamCentered<StoermelderTrimpot>(Vec(22.5f, 178.6f), module, SailModule::PARAM_STEP));
		addInput(createInputCentered<StoermelderPort>(Vec(22.5f, 203.5f), module, SailModule::INPUT_INC));
		addInput(createInputCentered<StoermelderPort>(Vec(22.5f, 239.2f), module, SailModule::INPUT_DEC));

		addInput(createInputCentered<StoermelderPort>(Vec(22.5f, 283.5f), module, SailModule::INPUT_VALUE));

		addOutput(createOutputCentered<StoermelderPort>(Vec(22.5f, 327.7f), module, SailModule::OUTPUT));

		if (module) {
			OverlayMessageWidget::registerProvider(this);
		}
	}

	~SailWidget() {
		if (module) {
			OverlayMessageWidget::unregisterProvider(this);
		}
	}

	void step() override {
		ThemedModuleWidget<SailModule>::step();
		if (!module) return;
		module->fineMod = APP->window->getMods() & GLFW_MOD_SHIFT;

		Widget* w = APP->event->getHoveredWidget();
		ParamWidget* pw = dynamic_cast<ParamWidget*>(w);
		module->setHoveredWidget(pw);
	}

	int nextOverlayMessageId() override {
		if (module->overlayQueue.empty())
			return -1;
		return module->overlayQueue.shift();
	}

	void getOverlayMessage(int id, Message& m) override {
		if (module->hoveredWidgetNull || module->overlayMessageId != id) return;
		ParamWidget* pw = module->hoveredWidget.get();
		if (pw == nullptr) return;
		ParamQuantity* paramQuantity = pw->getParamQuantity();
		if (!paramQuantity) return;

		m.title = paramQuantity->getDisplayValueString() + paramQuantity->getUnit();
		m.subtitle[0] = paramQuantity->module->model->name;
		m.subtitle[1] = paramQuantity->name;
	}

	void appendContextMenu(Menu* menu) override {
		ThemedModuleWidget<SailModule>::appendContextMenu(menu);

		menu->addChild(new MenuSeparator());
		menu->addChild(StoermelderPackOne::Rack::createMapPtrSubmenuItem<IN_MODE>("IN-mode",
			{
				{ IN_MODE::DIFF, "Differential" },
				{ IN_MODE::ABSOLUTE, "Absolute" }
			},
			&module->inMode
		));
		menu->addChild(StoermelderPackOne::Rack::createMapPtrSubmenuItem<OUT_MODE>("OUT-mode",
			{
				{ OUT_MODE::REDUCED, "Reduced" },
				{ OUT_MODE::FULL, "Continuous" }
			},
			&module->outMode
		));
		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Status overlay", "", &module->overlayEnabled));
	}
};

} // namespace Sail
} // namespace StoermelderPackOne

Model* modelSail = createModel<StoermelderPackOne::Sail::SailModule, StoermelderPackOne::Sail::SailWidget>("Sail");