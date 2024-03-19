#include "components.hpp"


using simd::float_4;


template <typename T>
static T clip(T x) {
	// return std::tanh(x);
	// Pade approximant of tanh
	x = simd::clamp(x, -3.f, 3.f);
	return x * (27 + x * x) / (27 + 9 * x * x);
}


template <typename T>
struct LadderFilter {
	T omega0;
	T resonance = 1;
	T state[4];
	T input;

	LadderFilter() {
		reset();
		setCutoff(0);
	}

	void reset() {
		for (int i = 0; i < 4; i++) {
			state[i] = 0;
		}
	}

	void setCutoff(T cutoff) {
		omega0 = 2 * T(M_PI) * cutoff;
	}

	void process(T input, T dt) {
		dsp::stepRK4(T(0), dt, state, 4, [&](T t, const T x[], T dxdt[]) {
			T inputt = crossfade(this->input, input, t / dt);
			T inputc = clip(inputt - resonance * x[3]);
			T yc0 = clip(x[0]);
			T yc1 = clip(x[1]);
			T yc2 = clip(x[2]);
			T yc3 = clip(x[3]);

			dxdt[0] = omega0 * (inputc - yc0);
			dxdt[1] = omega0 * (yc0 - yc1);
			dxdt[2] = omega0 * (yc1 - yc2);
			dxdt[3] = omega0 * (yc2 - yc3);
		});

		this->input = input;
	}

	T lowpass() {
		return state[3];
	}
	T highpass() {
		return clip((input - resonance * state[3]) - 4 * state[0] + 6 * state[1] - 4 * state[2] + state[3]);
	}
};


static const int UPSAMPLE = 2;

struct VCF : Module {
	enum ParamIds {
		FREQ_PARAM,
		FINE_PARAM, // removed in 2.0
		RES_PARAM,
		FREQ_CV_PARAM,
		DRIVE_PARAM,
		// Added in 2.0
		RES_CV_PARAM,
		DRIVE_CV_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		FREQ_INPUT,
		RES_INPUT,
		DRIVE_INPUT,
		IN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		LPF_OUTPUT,
		HPF_OUTPUT,
		NUM_OUTPUTS
	};

	LadderFilter<float_4> filters[4];
	// Upsampler<UPSAMPLE, 8> inputUpsampler;
	// Decimator<UPSAMPLE, 8> lowpassDecimator;
	// Decimator<UPSAMPLE, 8> highpassDecimator;

	VCF() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		// To preserve backward compatibility with <2.0, FREQ_PARAM follows
		// freq = C4 * 2^(10 * param - 5)
		// or
		// param = (log2(freq / C4) + 5) / 10
		const float minFreq = (std::log2(dsp::FREQ_C4 / 8000.f) + 5) / 10;
		const float maxFreq = (std::log2(8000.f / dsp::FREQ_C4) + 5) / 10;
		const float defaultFreq = (0.f + 5) / 10;
		configParam(FREQ_PARAM, minFreq, maxFreq, defaultFreq, "Cutoff frequency", " Hz", std::pow(2, 10.f), dsp::FREQ_C4 / std::pow(2, 5.f));
		configParam(RES_PARAM, 0.f, 1.f, 0.f, "Resonance", "%", 0.f, 100.f);
		configParam(RES_CV_PARAM, -1.f, 1.f, 0.f, "Resonance CV", "%", 0.f, 100.f);
		configParam(FREQ_CV_PARAM, -1.f, 1.f, 0.f, "Cutoff frequency CV", "%", 0.f, 100.f);
		// gain(drive) = (1 + drive)^5
		// gain(0) = 1
		// gain(-1) = 0
		// gain(1) = 5
		configParam(DRIVE_PARAM, -1.f, 1.f, 0.f, "Drive", "%", 0, 100, 100);
		configParam(DRIVE_CV_PARAM, -1.f, 1.f, 0.f, "Drive CV", "%", 0, 100);

		configInput(FREQ_INPUT, "Frequency");
		configInput(RES_INPUT, "Resonance");
		configInput(DRIVE_INPUT, "Drive");
		configInput(IN_INPUT, "Audio");

		configOutput(LPF_OUTPUT, "Lowpass filter");
		configOutput(HPF_OUTPUT, "Highpass filter");

		configBypass(IN_INPUT, LPF_OUTPUT);
		configBypass(IN_INPUT, HPF_OUTPUT);
	}

	void onReset() override {
		for (int i = 0; i < 4; i++)
			filters[i].reset();
	}

	void process(const ProcessArgs& args) override {
		if (!outputs[LPF_OUTPUT].isConnected() && !outputs[HPF_OUTPUT].isConnected()) {
			return;
		}

		float driveParam = params[DRIVE_PARAM].getValue();
		float driveCvParam = params[DRIVE_CV_PARAM].getValue();
		float resParam = params[RES_PARAM].getValue();
		float resCvParam = params[RES_CV_PARAM].getValue();
		float freqParam = params[FREQ_PARAM].getValue();
		// Rescale for backward compatibility
		freqParam = freqParam * 10.f - 5.f;
		float freqCvParam = params[FREQ_CV_PARAM].getValue();

		int channels = std::max(1, inputs[IN_INPUT].getChannels());

		for (int c = 0; c < channels; c += 4) {
			auto& filter = filters[c / 4];

			float_4 input = inputs[IN_INPUT].getVoltageSimd<float_4>(c) / 5.f;

			// Drive gain
			float_4 drive = driveParam + inputs[DRIVE_INPUT].getPolyVoltageSimd<float_4>(c) / 10.f * driveCvParam;
			drive = clamp(drive, -1.f, 1.f);
			float_4 gain = simd::pow(1.f + drive, 5);
			input *= gain;

			// Add -120dB noise to bootstrap self-oscillation
			input += 1e-6f * (2.f * random::uniform() - 1.f);

			// Set resonance
			float_4 resonance = resParam + inputs[RES_INPUT].getPolyVoltageSimd<float_4>(c) / 10.f * resCvParam;
			resonance = clamp(resonance, 0.f, 1.f);
			filter.resonance = simd::pow(resonance, 2) * 10.f;

			// Get pitch
			float_4 pitch = freqParam + inputs[FREQ_INPUT].getPolyVoltageSimd<float_4>(c) * freqCvParam;
			// Set cutoff
			float_4 cutoff = dsp::FREQ_C4 * simd::pow(2.f, pitch);
			// Without oversampling, we must limit to 8000 Hz or so @ 44100 Hz
			cutoff = clamp(cutoff, 1.f, args.sampleRate * 0.18f);
			filter.setCutoff(cutoff);

			// Upsample input
			// float dt = args.sampleTime / UPSAMPLE;
			// float inputBuf[UPSAMPLE];
			// float lowpassBuf[UPSAMPLE];
			// float highpassBuf[UPSAMPLE];
			// inputUpsampler.process(input, inputBuf);
			// for (int i = 0; i < UPSAMPLE; i++) {
			// 	// Step the filter
			// 	filter.process(inputBuf[i], dt);
			// 	if (outputs[LPF_OUTPUT].isConnected())
			// 		lowpassBuf[i] = filter.lowpass();
			// 	if (outputs[HPF_OUTPUT].isConnected())
			// 		highpassBuf[i] = filter.highpass();
			// }

			// // Set outputs
			// if (outputs[LPF_OUTPUT].isConnected()) {
			// 	outputs[LPF_OUTPUT].setVoltage(5.f * lowpassDecimator.process(lowpassBuf));
			// }
			// if (outputs[HPF_OUTPUT].isConnected()) {
			// 	outputs[HPF_OUTPUT].setVoltage(5.f * highpassDecimator.process(highpassBuf));
			// }

			// Set outputs
			filter.process(input, args.sampleTime);
			if (outputs[LPF_OUTPUT].isConnected()) {
				outputs[LPF_OUTPUT].setVoltageSimd(5.f * filter.lowpass(), c);
			}
			if (outputs[HPF_OUTPUT].isConnected()) {
				outputs[HPF_OUTPUT].setVoltageSimd(5.f * filter.highpass(), c);
			}
		}

		outputs[LPF_OUTPUT].setChannels(channels);
		outputs[HPF_OUTPUT].setChannels(channels);
	}

	void paramsFromJson(json_t* rootJ) override {
		// These attenuators didn't exist in version <2.0, so set to 1 in case they are not overwritten.
		params[RES_CV_PARAM].setValue(1.f);
		params[DRIVE_CV_PARAM].setValue(1.f);

		Module::paramsFromJson(rootJ);
	}
};

struct VCFWidget : ModuleWidget {
	typedef FundamentalBlackKnob<40> BigKnob;
	typedef FundamentalBlackKnob<18> SmallKnob;

	static constexpr const int kWidth = 7;
	static constexpr const float kPosLeft = 25.5f;
	static constexpr const float kPosCenter = kRACK_GRID_WIDTH * kWidth * 0.5f;
	static constexpr const float kPosRight = 78.5f;

	static constexpr const float kVerticalPos1 = kRACK_GRID_HEIGHT - 307.f - kRACK_JACK_HALF_SIZE;
	static constexpr const float kVerticalPos2 = kRACK_GRID_HEIGHT - 250.f - BigKnob::kHalfSize;
	static constexpr const float kVerticalPos3 = kRACK_GRID_HEIGHT - 220.f - SmallKnob::kHalfSize;
	static constexpr const float kVerticalPos4 = kRACK_GRID_HEIGHT - 190.f - kRACK_JACK_HALF_SIZE;
	static constexpr const float kVerticalPos5 = kRACK_GRID_HEIGHT - 131.f - BigKnob::kHalfSize;
	static constexpr const float kVerticalPos6 = kRACK_GRID_HEIGHT - 108.f - SmallKnob::kHalfSize;
	static constexpr const float kVerticalPos7 = kRACK_GRID_HEIGHT - 77.5f - kRACK_JACK_HALF_SIZE;
	static constexpr const float kVerticalPos8 = kRACK_GRID_HEIGHT - 26.f - kRACK_JACK_HALF_SIZE;

	VCFWidget(VCF* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/VCF.svg")));

		addChild(createWidget<ThemedScrew>(Vec(kRACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * kRACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(kRACK_GRID_WIDTH, kRACK_GRID_HEIGHT - kRACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * kRACK_GRID_WIDTH, kRACK_GRID_HEIGHT - kRACK_GRID_WIDTH)));

		addInput(createInputCentered<FundamentalPort>(Vec(kPosCenter, kVerticalPos1), module, VCF::IN_INPUT));

		addParam(createParamCentered<BigKnob>(Vec(kPosLeft, kVerticalPos2), module, VCF::FREQ_PARAM));
		addParam(createParamCentered<BigKnob>(Vec(kPosRight, kVerticalPos2), module, VCF::RES_PARAM));

		addParam(createParamCentered<SmallKnob>(Vec(kPosLeft, kVerticalPos3), module, VCF::FREQ_CV_PARAM));
		addParam(createParamCentered<SmallKnob>(Vec(kPosRight, kVerticalPos3), module, VCF::RES_CV_PARAM));

		addInput(createInputCentered<FundamentalPort>(Vec(kPosLeft, kVerticalPos4), module, VCF::FREQ_INPUT));
		addInput(createInputCentered<FundamentalPort>(Vec(kPosRight, kVerticalPos4), module, VCF::RES_INPUT));

		addParam(createParamCentered<BigKnob>(Vec(kPosCenter, kVerticalPos5), module, VCF::DRIVE_PARAM));
		addParam(createParamCentered<SmallKnob>(Vec(kPosCenter, kVerticalPos6), module, VCF::DRIVE_CV_PARAM));
		addInput(createInputCentered<FundamentalPort>(Vec(kPosCenter, kVerticalPos7), module, VCF::DRIVE_INPUT));

		addOutput(createOutputCentered<FundamentalPort>(Vec(12.5f + 18.f, kVerticalPos8), module, VCF::LPF_OUTPUT));
		addOutput(createOutputCentered<FundamentalPort>(Vec(56.5f + 18.f, kVerticalPos8), module, VCF::HPF_OUTPUT));
	}
};


Model* modelVCF = createModel<VCF, VCFWidget>("VCF");
