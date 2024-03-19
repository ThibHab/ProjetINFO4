
#include "CmpDist.hpp"

bool CmpDist::active() {
	return (
		outputs[MIX_OUTPUT].isConnected() ||
		outputs[GT_OUTPUT].isConnected() ||
		outputs[EQ_OUTPUT].isConnected() ||
		outputs[LT_OUTPUT].isConnected()
	);
}

int CmpDist::channels() {
	return inputs[A_INPUT].getChannels();
}

void CmpDist::addChannel(int c) {
	_engines[c] = new Engine();
}

void CmpDist::removeChannel(int c) {
	delete _engines[c];
	_engines[c] = NULL;
}

void CmpDist::modulate() {
	float aLevel = clamp(params[A_DRY_PARAM].getValue(), 0.0f, 1.0f);
	aLevel = 1.0f - aLevel;
	aLevel *= Amplifier::minDecibels;
	_aDryAmplifier.setLevel(aLevel);

	float bLevel = clamp(params[B_DRY_PARAM].getValue(), 0.0f, 1.0f);
	bLevel = 1.0f - bLevel;
	bLevel *= Amplifier::minDecibels;
	_bDryAmplifier.setLevel(bLevel);
}

void CmpDist::modulateChannel(int c) {
	Engine& e = *_engines[c];

	e.aScale = clamp(params[A_PARAM].getValue(), -1.0f, 1.0f);
	if (inputs[A_SCALE_INPUT].isConnected()) {
		e.aScale *= clamp(inputs[A_SCALE_INPUT].getPolyVoltage(c) / 5.0f, -1.0f, 1.0f);
	}
	e.bScale = clamp(params[B_PARAM].getValue(), -1.0f, 1.0f);
	if (inputs[B_SCALE_INPUT].isConnected()) {
		e.bScale *= clamp(inputs[B_SCALE_INPUT].getPolyVoltage(c) / 5.0f, -1.0f, 1.0f);
	}

	e.window = clamp(params[WINDOW_PARAM].getValue(), 0.0f, 1.0f);
	if (inputs[WINDOW_INPUT].isConnected()) {
		e.window *= clamp(inputs[WINDOW_INPUT].getPolyVoltage(c) / 10.0f, 0.0f, 1.0f);
	}
	e.window *= 10.0f;

	e.gtMix = clamp(params[GT_MIX_PARAM].getValue(), -1.0f, 1.0f);
	if (inputs[GT_MIX_INPUT].isConnected()) {
		e.gtMix *= clamp(inputs[GT_MIX_INPUT].getPolyVoltage(c) / 5.0f, -1.0f, 1.0f);
	}
	e.eqMix = clamp(params[EQ_MIX_PARAM].getValue(), -1.0f, 1.0f);
	e.ltMix = clamp(params[LT_MIX_PARAM].getValue(), -1.0f, 1.0f);
	if (inputs[LT_MIX_INPUT].isConnected()) {
		e.ltMix *= clamp(inputs[LT_MIX_INPUT].getPolyVoltage(c) / 5.0f, -1.0f, 1.0f);
	}

	float dw = clamp(params[DRY_WET_PARAM].getValue(), -1.0f, 1.0f);
	if (inputs[DRY_WET_INPUT].isConnected()) {
		e.ltMix *= clamp(inputs[DRY_WET_INPUT].getPolyVoltage(c) / 5.0f, -1.0f, 1.0f);
	}
	e.dryWet.setParams(dw, 0.9f, false);
}

void CmpDist::processAlways(const ProcessArgs& args) {
	outputs[GT_OUTPUT].setChannels(_channels);
	outputs[EQ_OUTPUT].setChannels(_channels);
	outputs[LT_OUTPUT].setChannels(_channels);
	outputs[MIX_OUTPUT].setChannels(_channels);
}

void CmpDist::processChannel(const ProcessArgs& args, int c) {
	const float v = 5.0f;
	Engine& e = *_engines[c];

	float a = inputs[A_INPUT].getPolyVoltage(c);
	a *= e.aScale;
	float b = v;
	if (inputs[B_INPUT].isConnected()) {
		b = inputs[B_INPUT].getPolyVoltage(c);
	}
	b *= e.bScale;

	float gt = 0.0f;
	float eq = 0.0f;
	float lt = 0.0f;
	if (fabsf(a - b) <= e.window) {
		gt = -v;
		eq = v;
		lt = -v;
	}
	else if (a > b) {
		gt = v;
		eq = -v;
		lt = -v;
	}
	else {
		gt = -v;
		eq = -v;
		lt = v;
	}
	lt = -lt;

	float dry = _aDryAmplifier.next(a) + _bDryAmplifier.next(b);
	float wet = e.gtMix*gt + e.eqMix*eq + e.ltMix*lt;
	float mix = e.dryWet.next(dry, wet);
	outputs[GT_OUTPUT].setVoltage(gt, c);
	outputs[EQ_OUTPUT].setVoltage(eq, c);
	outputs[LT_OUTPUT].setVoltage(lt, c);
	outputs[MIX_OUTPUT].setVoltage(mix, c);
}

struct CmpDistWidget : BGModuleWidget {
	static constexpr int hp = 10;

	CmpDistWidget(CmpDist* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);
		setPanel(box.size, "CmpDist");
		createScrews();

		// generated by svg_widgets.rb
		auto aParamPosition = Vec(14.0, 46.0);
		auto windowParamPosition = Vec(60.5, 46.0);
		auto bParamPosition = Vec(107.0, 46.0);
		auto gtMixParamPosition = Vec(14.0, 111.0);
		auto eqMixParamPosition = Vec(60.5, 111.0);
		auto ltMixParamPosition = Vec(107.0, 111.0);
		auto dryWetParamPosition = Vec(60.5, 176.0);
		auto aDryParamPosition = Vec(20.5, 182.5);
		auto bDryParamPosition = Vec(113.5, 182.5);

		auto gtMixInputPosition = Vec(15.0, 230.0);
		auto ltMixInputPosition = Vec(47.0, 230.0);
		auto windowInputPosition = Vec(79.0, 230.0);
		auto dryWetInputPosition = Vec(111.0, 230.0);
		auto aInputPosition = Vec(15.0, 274.0);
		auto aScaleInputPosition = Vec(47.0, 274.0);
		auto bInputPosition = Vec(15.0, 318.0);
		auto bScaleInputPosition = Vec(47.0, 318.0);

		auto gtOutputPosition = Vec(79.0, 274.0);
		auto ltOutputPosition = Vec(111.0, 274.0);
		auto eqOutputPosition = Vec(79.0, 318.0);
		auto mixOutputPosition = Vec(111.0, 318.0);
		// end generated by svg_widgets.rb

		addParam(createParam<Knob29>(aParamPosition, module, CmpDist::A_PARAM));
		addParam(createParam<Knob29>(windowParamPosition, module, CmpDist::WINDOW_PARAM));
		addParam(createParam<Knob29>(bParamPosition, module, CmpDist::B_PARAM));
		addParam(createParam<Knob29>(gtMixParamPosition, module, CmpDist::GT_MIX_PARAM));
		addParam(createParam<Knob29>(eqMixParamPosition, module, CmpDist::EQ_MIX_PARAM));
		addParam(createParam<Knob29>(ltMixParamPosition, module, CmpDist::LT_MIX_PARAM));
		addParam(createParam<Knob29>(dryWetParamPosition, module, CmpDist::DRY_WET_PARAM));
		addParam(createParam<Knob16>(aDryParamPosition, module, CmpDist::A_DRY_PARAM));
		addParam(createParam<Knob16>(bDryParamPosition, module, CmpDist::B_DRY_PARAM));

		addInput(createInput<Port24>(gtMixInputPosition, module, CmpDist::GT_MIX_INPUT));
		addInput(createInput<Port24>(ltMixInputPosition, module, CmpDist::LT_MIX_INPUT));
		addInput(createInput<Port24>(windowInputPosition, module, CmpDist::WINDOW_INPUT));
		addInput(createInput<Port24>(dryWetInputPosition, module, CmpDist::DRY_WET_INPUT));
		addInput(createInput<Port24>(aInputPosition, module, CmpDist::A_INPUT));
		addInput(createInput<Port24>(aScaleInputPosition, module, CmpDist::A_SCALE_INPUT));
		addInput(createInput<Port24>(bInputPosition, module, CmpDist::B_INPUT));
		addInput(createInput<Port24>(bScaleInputPosition, module, CmpDist::B_SCALE_INPUT));

		addOutput(createOutput<Port24>(gtOutputPosition, module, CmpDist::GT_OUTPUT));
		addOutput(createOutput<Port24>(ltOutputPosition, module, CmpDist::LT_OUTPUT));
		addOutput(createOutput<Port24>(eqOutputPosition, module, CmpDist::EQ_OUTPUT));
		addOutput(createOutput<Port24>(mixOutputPosition, module, CmpDist::MIX_OUTPUT));
	}
};

Model* modelCmpDist = createModel<CmpDist, CmpDistWidget>("Bogaudio-CmpDist", "CMPDIST", "Comparator-based distortion", "Distortion", "Effect", "Polyphonic");
