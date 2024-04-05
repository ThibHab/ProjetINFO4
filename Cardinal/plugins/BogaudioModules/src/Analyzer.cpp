
#include "Analyzer.hpp"

void Analyzer::reset() {
	_core.resetChannels();
}

void Analyzer::sampleRateChange() {
	_sampleRate = APP->engine->getSampleRate();
}

json_t* Analyzer::saveToJson(json_t* root) {
	frequencyPlotToJson(root);
	amplitudePlotToJson(root);
	return root;
}

void Analyzer::loadFromJson(json_t* root) {
	frequencyPlotFromJson(root);
	amplitudePlotFromJson(root);
}

void Analyzer::modulate() {
	float range = params[RANGE2_PARAM].getValue();
	_rangeMinHz = 0.0f;
	_rangeMaxHz = 0.5f * _sampleRate;
	if (range < 0.0f) {
		range *= 0.9f;
		_rangeMaxHz *= 1.0f + range;
	}
	else if (range > 0.0f) {
		range *= range;
		range *= 0.8f;
		_rangeMinHz = range * _rangeMaxHz;
	}

	const float maxTime = 0.5;
	float smooth = params[SMOOTH_PARAM].getValue() * maxTime;
	smooth /= _core.size() / (_core._overlap * _sampleRate);
	int averageN = std::max(1, (int)roundf(smooth));

	AnalyzerCore::Quality quality = AnalyzerCore::QUALITY_GOOD;
	if (params[QUALITY_PARAM].getValue() > 2.5) {
		quality = AnalyzerCore::QUALITY_ULTRA;
	}
	else if (params[QUALITY_PARAM].getValue() > 1.5) {
		quality = AnalyzerCore::QUALITY_HIGH;
	}

	AnalyzerCore::Window window = AnalyzerCore::WINDOW_KAISER;
	if (params[WINDOW_PARAM].getValue() > 2.5) {
		window = AnalyzerCore::WINDOW_NONE;
	}
	else if (params[WINDOW_PARAM].getValue() > 1.5) {
		window = AnalyzerCore::WINDOW_HAMMING;
	}

	_core.setParams(_sampleRate, averageN, quality, window);
}

void Analyzer::processAll(const ProcessArgs& args) {
	for (int i = 0; i < 4; ++i) {
		_core.stepChannel(i, inputs[SIGNALA_INPUT + i]);

		outputs[SIGNALA_OUTPUT + i].setChannels(inputs[SIGNALA_INPUT + i].getChannels());
		outputs[SIGNALA_OUTPUT + i].writeVoltages(inputs[SIGNALA_INPUT + i].getVoltages());
	}

	lights[QUALITY_ULTRA_LIGHT].value = _core._quality == AnalyzerCore::QUALITY_ULTRA;
	lights[QUALITY_HIGH_LIGHT].value = _core._quality == AnalyzerCore::QUALITY_HIGH;
	lights[QUALITY_GOOD_LIGHT].value = _core._quality == AnalyzerCore::QUALITY_GOOD;
	lights[WINDOW_NONE_LIGHT].value = _core._window == AnalyzerCore::WINDOW_NONE;
	lights[WINDOW_HAMMING_LIGHT].value = _core._window == AnalyzerCore::WINDOW_HAMMING;
	lights[WINDOW_KAISER_LIGHT].value = _core._window == AnalyzerCore::WINDOW_KAISER;
}

struct AnalyzerWidget : AnalyzerBaseWidget {
	static constexpr int hp = 20;

	AnalyzerWidget(Analyzer* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);
		setPanel(box.size, "Analyzer");
		createScrews();

		{
			auto inset = Vec(10, 25);
			auto size = Vec(box.size.x - 2*inset.x, 230);
			auto display = new AnalyzerDisplay(module, size, true);
			display->box.pos = inset;
			display->box.size = size;
			addChild(display);
		}

		// generated by svg_widgets.rb
		auto range2ParamPosition = Vec(30.0, 271.0);
		auto smoothParamPosition = Vec(103.0, 271.0);
		auto qualityParamPosition = Vec(179.0, 306.0);
		auto windowParamPosition = Vec(250.0, 306.0);

		auto signalaInputPosition = Vec(13.5, 323.0);
		auto signalbInputPosition = Vec(86.0, 323.0);
		auto signalcInputPosition = Vec(158.5, 323.0);
		auto signaldInputPosition = Vec(230.5, 323.0);

		auto signalaOutputPosition = Vec(44.5, 323.0);
		auto signalbOutputPosition = Vec(117.0, 323.0);
		auto signalcOutputPosition = Vec(189.5, 323.0);
		auto signaldOutputPosition = Vec(261.5, 323.0);

		auto qualityUltraLightPosition = Vec(170.0, 267.0);
		auto qualityHighLightPosition = Vec(170.0, 281.0);
		auto qualityGoodLightPosition = Vec(170.0, 295.0);
		auto windowNoneLightPosition = Vec(241.0, 267.0);
		auto windowHammingLightPosition = Vec(241.0, 281.0);
		auto windowKaiserLightPosition = Vec(241.0, 295.0);
		// end generated by svg_widgets.rb

		addParam(createParam<Knob38>(range2ParamPosition, module, Analyzer::RANGE2_PARAM));
		addParam(createParam<Knob38>(smoothParamPosition, module, Analyzer::SMOOTH_PARAM));
		addParam(createParam<StatefulButton9>(qualityParamPosition, module, Analyzer::QUALITY_PARAM));
		addParam(createParam<StatefulButton9>(windowParamPosition, module, Analyzer::WINDOW_PARAM));

		addInput(createInput<Port24>(signalaInputPosition, module, Analyzer::SIGNALA_INPUT));
		addInput(createInput<Port24>(signalbInputPosition, module, Analyzer::SIGNALB_INPUT));
		addInput(createInput<Port24>(signalcInputPosition, module, Analyzer::SIGNALC_INPUT));
		addInput(createInput<Port24>(signaldInputPosition, module, Analyzer::SIGNALD_INPUT));

		addOutput(createOutput<Port24>(signalaOutputPosition, module, Analyzer::SIGNALA_OUTPUT));
		addOutput(createOutput<Port24>(signalbOutputPosition, module, Analyzer::SIGNALB_OUTPUT));
		addOutput(createOutput<Port24>(signalcOutputPosition, module, Analyzer::SIGNALC_OUTPUT));
		addOutput(createOutput<Port24>(signaldOutputPosition, module, Analyzer::SIGNALD_OUTPUT));

		addChild(createLight<BGSmallLight<GreenLight>>(qualityUltraLightPosition, module, Analyzer::QUALITY_ULTRA_LIGHT));
		addChild(createLight<BGSmallLight<GreenLight>>(qualityHighLightPosition, module, Analyzer::QUALITY_HIGH_LIGHT));
		addChild(createLight<BGSmallLight<GreenLight>>(qualityGoodLightPosition, module, Analyzer::QUALITY_GOOD_LIGHT));
		addChild(createLight<BGSmallLight<GreenLight>>(windowNoneLightPosition, module, Analyzer::WINDOW_NONE_LIGHT));
		addChild(createLight<BGSmallLight<GreenLight>>(windowHammingLightPosition, module, Analyzer::WINDOW_HAMMING_LIGHT));
		addChild(createLight<BGSmallLight<GreenLight>>(windowKaiserLightPosition, module, Analyzer::WINDOW_KAISER_LIGHT));
	}

	void contextMenu(Menu* menu) override {
		addFrequencyPlotContextMenu(menu);
		addAmplitudePlotContextMenu(menu);
	}
};

Model* modelAnalyzer = bogaudio::createModel<Analyzer, AnalyzerWidget>("Bogaudio-Analyzer", "ANALYZER", "4-channel spectrum analyzer", "Visual");