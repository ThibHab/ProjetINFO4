#include "MSM.hpp"
#include "MSMComponentLibrary.hpp"
#include "Additional/Tables.hpp"
#include "Additional/Wave.hpp"

class OSiXFold {
	public:
		OSiXFold()
		{

		}

		void Shape(double in, double sc, double sccv, double up, double down, bool folderactive) {
			_sc = sc + sccv;
			_up = up;
			_down = down;
			_in = in;
			_folderactive = folderactive;
		};

		void process() {
				if(_folderactive) {
					double s = 1.0;
					double j = -1.0;
					double combine = 0, sig1 = 0, sig2 = 0, X = 0, Y = 0, Z = 0, W = 0, A1 = 0, A2 = 0, result = 0;

					combine = (_in) / (1.0f - _sc + 1.0f);

					sig1 = (-(combine - 1.0f) * 2.0f);
					X = std::isgreater(combine, s);
					Y =	sig1 * X;
					A1 = fastSin(Y * _up) * 2.0f;

					sig2 = (-(combine + 1.0f) * 2.0f);
					Z = std::isless(combine, j);
					W = sig2 * Z;
					A2 = fastSin(W * _down) * 2.0f;

					result = combine + A1 + A2;

					folderBuffer = (2.0f * fastSin(tanh_noclip(fastSin(result * M_PI / 2.0f)) / M_PI)) * 10.0f;
				}
		};

		inline double Output() {
			return folderBuffer;
		};

	private:

		double folderBuffer = 0.0f;

		double _sc = 0.0f;
		double _up = 0.0f;
		double _down = 0.0f;
		double _in = 0.0f;

		bool _folderactive = false;
};

extern float sawTable[2048];
extern float triTable[2048];

template <int OVERSAMPLE, int QUALITY>
class Boscillator {
	public:
		Boscillator()
		{
			freq = 0.0f;
			pitch = 0.0f;
		}

		bool analog = false;
		bool soft = false;
		bool syncEnabled = false;
		bool syncDirection = false;

		void setPitch(float pitchKnob, float pitchCv, char _lfomode) {
			// Compute frequency
			pitch = pitchKnob;
			if (analog) {
				// Apply pitch slew
				const float pitchSlewAmount = 3.0f;
				pitch += pitchSlew * pitchSlewAmount;
			}
			else {
				// Quantize coarse knob if digital mode
				pitch = roundf(pitch);
			}
			pitch += pitchCv;
			// Note C3
			switch(_lfomode) {
				case 0:
					freq = (261.626f * std::pow(2.0f, pitch / 12.0f)) / 100.0f;
				break;
				case 1:
					freq = 261.626f * std::pow(2.0f, pitch / 12.0f);
				break;
			}
		}

		bool Tri_isAct = false;
		bool Saw_isAct = false;
		bool Sqr_isAct = false;
		bool F_isAct = false;
		bool H_isAct = false;
		bool Ramp_isAct = false;

		void Tri_Active(bool act) {
			Tri_isAct = act;
		}
		void Saw_Active(bool act) {
			Saw_isAct = act;
		}
		void Sqr_Active(bool act) {
			Sqr_isAct = act;
		}
		void H_Active(bool act) {
			H_isAct = act;
		}
		void F_Active(bool act) {
			F_isAct = act;
		}
		void Ramp_Active(bool act) {
			Ramp_isAct = act;
		}

		void setPulseWidth(float pulseWidth) {
			const float pwMin = 0.01f;
			pw = clamp(pulseWidth, pwMin, 1.0f - pwMin);
		}

		void process(float deltaTime, float syncValue) {
			if (analog) {
				// Adjust pitch slew
				if (++pitchSlewIndex > 32) {
					const float pitchSlewTau = 100.0f; // Time constant for leaky integrator in seconds
					pitchSlew += (clamp(random::normal(), -3.0f, 3.0f) - pitchSlew / pitchSlewTau) / APP->engine->getSampleRate();
					pitchSlewIndex = 0;
				}
			}

			// Advance phase
			float deltaPhase = clamp(freq * deltaTime, 1e-6, 0.5f);

			// Detect sync
			int syncIndex = -1.0f; // Index in the oversample loop where sync occurs [0, OVERSAMPLE)
			float syncCrossing = 0.0f; // Offset that sync occurs [0.0, 1.0)
			if (syncEnabled) {
				syncValue -= 0.01;
				if (syncValue > 0.0f && lastSyncValue <= 0.0f) {
					float deltaSync = syncValue - lastSyncValue;
					syncCrossing = 1.0f - syncValue / deltaSync;
					syncCrossing *= OVERSAMPLE;
					syncIndex = (int)syncCrossing;
					syncCrossing -= syncIndex;
				}
				lastSyncValue = syncValue;
			}

			if (syncDirection)
				deltaPhase *= -1.0f;

			sqrFilter.setCutoff(40.0f * deltaTime);

			for (int i = 0; i < OVERSAMPLE; i++) {
				if (syncIndex == i) {
					if (soft) {
						syncDirection = !syncDirection;
						deltaPhase *= -1.0f;
					}
					else {
						// phase = syncCrossing * deltaPhase / OVERSAMPLE;
						phase = 0.0f;
					}
				}

				if (analog) {
					// Quadratic approximation of sine, slightly richer harmonics
					if (phase < 0.5f)
						sinBuffer[i] = 1.f - 16.f * std::pow(phase - 0.25f, 2.0f);
					else
						sinBuffer[i] = -1.f + 16.f * std::pow(phase - 0.75f, 2.0f);
					sinBuffer[i] *= 1.08f;
				}
				else {
					sinBuffer[i] = fastSin(2.f*M_PI * phase);
				}

				if(analog) {
					sinsinBuffer[i] = (2.0f * Sine(phase) + 1.0f * fastSin(4.0f * M_PI * phase)) / 2.5f;
				}
				else {
					sinsinBuffer[i] = (2.0f * Sine(phase) + 1.0f * fastSin(4.0f * M_PI * phase)) / 2.5f;
				}

				if(Tri_isAct) {
					if (analog) {
						triBuffer[i] = 1.25f * interpolateLinear(triTable, phase * 2047.f);
					}
					else {
						triBuffer[i] = ((phase < 0.5) * (4.0 * phase - 1.0)) + ((phase >= 0.5) * (1.0 - 4.0 * (phase - 0.5)));
					}
				}

				if(Saw_isAct) {
					if (analog) {
						sawBuffer[i] = 1.66f * interpolateLinear(sawTable, phase * 2047.f);
					}
					else {
						if (phase < 0.5f)
							sawBuffer[i] = 2.f * phase;
						else
							sawBuffer[i] = -2.f + 2.f * phase;
					}
				}

				if(Sqr_isAct) {
					sqrBuffer[i] = (phase < pw) ? 1.f : -1.f;
					if (analog) {
						// Simply filter here
						sqrFilter.process(sqrBuffer[i]);
						sqrBuffer[i] = 1.05f * sqrFilter.highpass();
					}
				}

				if(H_isAct) {
					if (analog) {
						if (phase < 0.5f)
							halfBuffer[i] = (phase < 0.5f ? 2.0f * fastSin(2.0f * M_PI * phase) - 2.0f / M_PI : -2.0f / M_PI) - 0.5f;
						else
							halfBuffer[i] = (phase < - 0.5f ? 2.0f * fastSin(2.0f * M_PI * phase) + 2.0f / M_PI : -2.0f/ M_PI) - 0.5f;
						halfBuffer[i] *= 1.08f;
					}
					else {
						halfBuffer[i] = (phase < 0.5f ? 2.0f * fastSin(2.0f * M_PI * phase) - 2.0f / M_PI : - 2.0f / M_PI) - 0.3f;
					}
				}

				if(F_isAct) {
					if (analog) {
						if (phase < 0.25f)
							fullBuffer[i] = ((1.0f * fastSin(M_PI * phase) - 2.0f / M_PI) + 0.2f) * 1.5f;
						else
							fullBuffer[i] = ((1.0f * fastSin(M_PI * phase) - 2.0f / M_PI) + 0.2f) * 1.5f;
					}
					else {
						fullBuffer[i] = (2.0f * fastSin(M_PI * phase) - 4.0f / M_PI) + 0.3f;
					}
				}

				if(Ramp_isAct) {
					if (analog) {
						rampBuffer[i] = 1.0f - (1.66f * interpolateLinear(sawTable, (1.0f - phase) * 2047.f));
					}
					else {
						if (phase < 0.5f)
							rampBuffer[i] = 0.1f + (1.0f - (2.f * phase));
						else
							rampBuffer[i] = 0.1f + (1.0f - (-2.f + 2.f * phase));
					}
				}

				if (analog) {
					// Quadratic approximation of sine, slightly richer harmonics
					if (phase < 0.5f)
						cosBuffer[i] = 1.f - 16.f * std::pow(phase - 0.25f, 2);
					else
						cosBuffer[i] = -1.f + 16.f * std::pow(phase - 0.75f, 2);
					cosBuffer[i] *= 1.08f;
				}
				else {
					cosBuffer[i] = cosf(2.f*M_PI * phase);
				}




				// Advance phase
				phase += deltaPhase / OVERSAMPLE;
				phase = math::eucMod(phase, 1.0f);
			}
		}

		inline float sin() {
			return sinDecimator.process(sinBuffer);
		}
		inline float ramp() {
			return rampDecimator.process(rampBuffer);
		}
		inline float cos() {
			return cosDecimator.process(cosBuffer);
		}
		inline float tri() {
			return triDecimator.process(triBuffer);
		}
		inline float saw() {
			return sawDecimator.process(sawBuffer);
		}
		inline float sqr() {
			return sqrDecimator.process(sqrBuffer);
		}
		inline float half() {
			return halfDecimator.process(halfBuffer);
		}
		inline float full() {
			return fullDecimator.process(fullBuffer);
		}
		inline float sinsin() {
			return sinsinDecimator.process(sinsinBuffer);
		}
		inline float light() {
			return sinf(2*M_PI * phase);
		}

	private:
		float lastSyncValue = 0.0f;
		float phase = 0.0f;
		float freq;
		float pw = 0.5f;
		float pitch;
		// For analog detuning effect
		float pitchSlew = 0.0f;
		int pitchSlewIndex = 0;

		dsp::Decimator<OVERSAMPLE, QUALITY> sinDecimator;
		dsp::Decimator<OVERSAMPLE, QUALITY> cosDecimator;
		dsp::Decimator<OVERSAMPLE, QUALITY> rampDecimator;
		dsp::Decimator<OVERSAMPLE, QUALITY> triDecimator;
		dsp::Decimator<OVERSAMPLE, QUALITY> sawDecimator;
		dsp::Decimator<OVERSAMPLE, QUALITY> sqrDecimator;
		dsp::Decimator<OVERSAMPLE, QUALITY> halfDecimator;
		dsp::Decimator<OVERSAMPLE, QUALITY> fullDecimator;
		dsp::Decimator<OVERSAMPLE, QUALITY> sinsinDecimator;
		dsp::RCFilter sqrFilter;

		float sinBuffer[OVERSAMPLE] = {};
		float cosBuffer[OVERSAMPLE] = {};
		float rampBuffer[OVERSAMPLE] = {};
		float triBuffer[OVERSAMPLE] = {};
		float sawBuffer[OVERSAMPLE] = {};
		float sqrBuffer[OVERSAMPLE] = {};
		float halfBuffer[OVERSAMPLE] = {};
		float fullBuffer[OVERSAMPLE] = {};
		float sinsinBuffer[OVERSAMPLE] = {};
};


struct VCO : Module {
	enum ParamIds {
		MODE1_PARAM,
		SYNC1_PARAM,
		SYNC2_PARAM,
		FREQ_1_PARAM,
		FREQ_2_PARAM,
		FINE_1_PARAM,
		FINE_2_PARAM,
		WAVE_1_PARAM,
		WAVE_2_PARAM,
		SYNCFREQ_PARAM,
		PHASE_1_PARAM,
		PHASE_2_PARAM,
		PHASE_1_CV_PARAM,
		PHASE_2_CV_PARAM,
		FMLIN_1_PARAM,
		FMEXP_1_PARAM,
		FMLIN_2_PARAM,
		FMEXP_2_PARAM,
		PW_1_PARAM,
		PW_2_PARAM,
		SHAPE_1_PARAM,
		INPUT_GAIN_PARAM,
		LFOMODE1_PARAM,
		LFOMODE2_PARAM,
		SYNCSWITCHA_PARAM,
		SYNCSWITCHB_PARAM,
		FMBUS_PARAM,
		SHAPE_CV_PARAM,
		UP_PARAM,
		DOWN_PARAM,
		EASTEREGG_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		VOCT_1_INPUT,
		VOCT_2_INPUT,
		PW_1_INPUT,
		PW_2_INPUT,
		LIN_1_INPUT,
		EXP_1_INPUT,
		LIN_2_INPUT,
		EXP_2_INPUT,
		PHASE_1_CV_INPUT,
		PHASE_2_CV_INPUT,
		SYNC1_INPUT,
		SYNC2_INPUT,
		EXT_SRC_INPUT,
		SHAPE_1_CV_INPUT,
		SYNCSWITCHA_CV_INPUT,
		SYNCSWITCHB_CV_INPUT,
		FMBUS_INPUT,
		SHAPE_CV_INPUT,
		UP_INPUT,
		DOWN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_1_OUTPUT,
		OUT_2_OUTPUT,
		SIN_OUTPUT,
		TRI_OUTPUT,
		SAW_OUTPUT,
		SIN_2_OUTPUT,
		SQR_2_OUTPUT,
		FOLD_OUTPUT,
		NUM_OUTPUTS
	};

	//Oscillators
	Boscillator<2, 4> oscillator1;
	Boscillator<2, 4> oscillator2;

	//Folder
	OSiXFold folder;

	// Panel Theme
	int Theme = THEME_DEFAULT;

	bool LFOMODE1 = 1;
	bool LFOMODE2 = 1;
	bool easteregg = 0;
//261.626f * std::pow(2.0f, pitch / 12.0f)
	VCO() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		configParam(VCO::EASTEREGG_PARAM, 0.0, 1.0, 0.0, "OSCiX");
		configParam(VCO::MODE1_PARAM, 0.0, 1.0, 1.0, "Anlg/Digi");
		configParam(VCO::SYNCSWITCHA_PARAM, 0.0, 1.0, 0.0, "Sync");
		configParam(VCO::SYNCSWITCHB_PARAM, 0.0, 1.0, 0.0, "Sync");
		configParam(VCO::LFOMODE1_PARAM, 0.0, 1.0, 1.0, "VCO/LFO");
		configParam(VCO::LFOMODE2_PARAM, 0.0, 1.0, 1.0, "VCO/LFO");
		configParam(VCO::FREQ_1_PARAM, -54.0, 54.0, 0.0, "Frequency", "Hz", 1.05946309436f, 261.626f, 0.f);
		configParam(VCO::FINE_1_PARAM, -1.0, 1.0, 0.0, "Fine Frequency");
		configParam(VCO::FREQ_2_PARAM, -54.0, 54.0, 0.0, "Frequency", "Hz", 1.05946309436f, 261.626f, 0.f);
		configParam(VCO::FINE_2_PARAM, -1.0, 1.0, 0.0, "Fine Frequency");
		configParam(VCO::FMLIN_1_PARAM, -1.0, 1.0, 0.0, "Linear FM", "%", 0.f, 100);
		configParam(VCO::FMLIN_2_PARAM, -1.0, 1.0, 0.0, "Linear FM", "%", 0.f, 100);
		configParam(VCO::FMEXP_1_PARAM, -1.0, 1.0, 0.0, "Exponential FM", "%", 0.f, 100);
		configParam(VCO::FMEXP_2_PARAM, -1.0, 1.0, 0.0, "Exponential FM", "%", 0.f, 100);
		configParam(VCO::PW_2_PARAM, 0.0, 1.0, 0.5, "Pulse Width", "%", 0.f, 100);
		configParam(VCO::FMBUS_PARAM, 0.0, 1.0, 0.0, "FM Master Index");
		configParam(VCO::INPUT_GAIN_PARAM, 0.0, 1.0, 0.0, "Fold Gain");
		configParam(VCO::SHAPE_1_PARAM, 0.0, 1.0, 0.0, "Fold Shape");
		configParam(VCO::SHAPE_CV_PARAM, -1.0, 1.0, 0.0, "Fold Shape CV");
		configParam(VCO::UP_PARAM, 0.0, 1.0, 0.0, "Upper Shape");
		configParam(VCO::DOWN_PARAM, 0.0, 1.0, 0.0, "Lower Shape");
		configInput(EXP_1_INPUT, "(A) Exponential FM Attenuator Control Voltage");
		configInput(VOCT_1_INPUT, "(A) Volt/Octave");
		configInput(SYNC1_INPUT, "(A) Start of Cycle Sync");
		configInput(VOCT_2_INPUT, "(B) Volt/Octave");
		configInput(SYNC2_INPUT, "(B) Start of Cycle Sync");
		configInput(LIN_1_INPUT, "(A) Linear FM Attenuator Control Voltage");
		configInput(LIN_2_INPUT, "(B) Linear FM Attenuator Control Voltage");
		configInput(EXP_2_INPUT, "(B) Exponential FM Attenuator Control Voltage");
		configInput(PW_2_INPUT, "(B) Pulse Width Control Voltage");
		configInput(EXT_SRC_INPUT, "External Fold Source");
		configInput(SHAPE_1_CV_INPUT, "Fold Index Control Voltage");
		configInput(SHAPE_CV_INPUT, "Fold Control Voltage");
		configInput(FMBUS_INPUT, "FM Index Control Voltage");
		configInput(SYNCSWITCHA_CV_INPUT, "Hardsync (A) Control Voltage");
		configInput(SYNCSWITCHB_CV_INPUT, "Hardsync (B) Control Voltage");
		configInput(UP_INPUT, "Asymmetric Up Folding Control Voltage");
		configInput(DOWN_INPUT, "Asymmetric Down Folding Control Voltage");
		configOutput(SIN_OUTPUT, "(A) Sin");
		configOutput(TRI_OUTPUT, "(A) Triangle");
		configOutput(SAW_OUTPUT, "(A) Sawtooth");
		configOutput(SIN_2_OUTPUT, "(B) Sin");
		configOutput(SQR_2_OUTPUT, "(B) Square");
		configOutput(FOLD_OUTPUT, "Fold");
		getParamQuantity(LFOMODE1_PARAM)->randomizeEnabled = false;
		getParamQuantity(LFOMODE2_PARAM)->randomizeEnabled = false;
	}

	void process(const ProcessArgs& args) override;

	//Json for Panel Theme
	json_t *dataToJson() override	{
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Theme", json_integer(Theme));
		return rootJ;
	}
	void dataFromJson(json_t *rootJ) override	{
		json_t *ThemeJ = json_object_get(rootJ, "Theme");
		if (ThemeJ)
			Theme = json_integer_value(ThemeJ);
	}

};


void VCO::process(const ProcessArgs& args) {

	// OSC1
	float pitchFine1, pitchFine2, pitchCv1, pitchCv2;

	oscillator1.analog = params[MODE1_PARAM].getValue() > 0.0f;

	pitchFine1 = 3.0f * dsp::quadraticBipolar(params[FINE_1_PARAM].getValue());
	pitchCv1 = (12.0f * inputs[VOCT_1_INPUT].getVoltage());

	if(inputs[LIN_1_INPUT].isConnected()) {
		pitchCv1 += (dsp::quadraticBipolar(params[FMLIN_1_PARAM].getValue()) * 12.0f * inputs[LIN_1_INPUT].getVoltage()) * (params[FMBUS_PARAM].getValue() + inputs[FMBUS_INPUT].getVoltage());
	}
	else {
		if(easteregg) {
			pitchCv1 += (dsp::quadraticBipolar(params[FMLIN_1_PARAM].getValue()) * 12.0f * (5.0f * oscillator2.sinsin())) * (params[FMBUS_PARAM].getValue() + inputs[FMBUS_INPUT].getVoltage());
		}
		else {
			pitchCv1 += (dsp::quadraticBipolar(params[FMLIN_1_PARAM].getValue()) * 12.0f * (5.0f * oscillator2.sin())) * (params[FMBUS_PARAM].getValue() + inputs[FMBUS_INPUT].getVoltage());
		}
	}
	float FM1 = params[FMEXP_1_PARAM].getValue() * 12.0f * inputs[EXP_1_INPUT].getVoltage();
	const float expBase = 25.0f;
	if(inputs[EXP_1_INPUT].isConnected()) {
		pitchCv1 += (rescale(powf(expBase, clamp(FM1 / 10.0f, 0.0f, 2.0f)), 1.0, expBase, 0.0f, 1.0f)) * (params[FMBUS_PARAM].getValue() + inputs[FMBUS_INPUT].getVoltage());
	}
	else {
		if(easteregg) {
			pitchCv1 += (rescale(powf(expBase, clamp(params[FMEXP_1_PARAM].getValue() * 12.0f *(5.0f * oscillator2.sinsin()) / 10.0f, 0.0f, 2.0f)), 1.0f, expBase, 0.0f, 1.0f)) * (params[FMBUS_PARAM].getValue() + inputs[FMBUS_INPUT].getVoltage());
		}
		else {
			pitchCv1 += (rescale(powf(expBase, clamp(params[FMEXP_1_PARAM].getValue() * 12.0f *(5.0f * oscillator2.sin()) / 10.0f, 0.0f, 2.0f)), 1.0f, expBase, 0.0f, 1.0f)) * (params[FMBUS_PARAM].getValue() + inputs[FMBUS_INPUT].getVoltage());
		}
	}

	LFOMODE1 = params[LFOMODE1_PARAM].getValue();

	oscillator1.setPitch(params[FREQ_1_PARAM].getValue(), pitchCv1 + pitchFine1, LFOMODE1);

	easteregg = params[EASTEREGG_PARAM].getValue();

	if(easteregg == 1) {
		oscillator1.H_Active(outputs[TRI_OUTPUT].isConnected());
		oscillator1.F_Active(outputs[SAW_OUTPUT].isConnected());
	}
	else {
		oscillator1.Tri_Active(outputs[TRI_OUTPUT].isConnected());
		oscillator1.Saw_Active(outputs[SAW_OUTPUT].isConnected());
	}

	char SYNCA = params[SYNCSWITCHA_PARAM].getValue() + inputs[SYNCSWITCHA_CV_INPUT].getVoltage() > 0.0f;
	switch(SYNCA) {
		case 1:
		oscillator1.syncEnabled = true;
		oscillator1.process(1.0f / args.sampleRate, (5.0f  * oscillator2.sin()));
		break;
		default:
		oscillator1.syncEnabled = inputs[SYNC1_INPUT].isConnected();
		oscillator1.process(1.0f / args.sampleRate, inputs[SYNC1_INPUT].getVoltage());
		break;
	}

	if(easteregg == 1) {
		if (outputs[SIN_OUTPUT].isConnected()) {
			outputs[SIN_OUTPUT].setVoltage(6.0f * oscillator1.sinsin());
		}
		if (outputs[TRI_OUTPUT].isConnected()) {
			outputs[TRI_OUTPUT].setVoltage(6.0f * oscillator1.half() + 0.5f);
		}
		if (outputs[SAW_OUTPUT].isConnected()) {
			outputs[SAW_OUTPUT].setVoltage(6.0f * oscillator1.full() - 1.0f);
		}
	}
	else {
		if (outputs[SIN_OUTPUT].isConnected()) {
			outputs[SIN_OUTPUT].setVoltage(6.0f * oscillator1.sin());
		}
		if (outputs[TRI_OUTPUT].isConnected()) {
			outputs[TRI_OUTPUT].setVoltage(6.0f * oscillator1.tri());
		}
		if (outputs[SAW_OUTPUT].isConnected()) {
			outputs[SAW_OUTPUT].setVoltage(6.0f * oscillator1.saw());
		}
	}

	// OSC2

	oscillator2.analog = params[MODE1_PARAM].getValue() > 0.0f;

	pitchFine2 = 3.0f * dsp::quadraticBipolar(params[FINE_2_PARAM].getValue());
	pitchCv2 = 12.0f * inputs[VOCT_2_INPUT].getVoltage();

	if(inputs[LIN_2_INPUT].isConnected()) {
		pitchCv2 += (dsp::quadraticBipolar(params[FMLIN_2_PARAM].getValue()) * 12.0f * inputs[LIN_2_INPUT].getVoltage()) * (params[FMBUS_PARAM].getValue() + inputs[FMBUS_INPUT].getVoltage());
	}
	else {
		pitchCv2 += (dsp::quadraticBipolar(params[FMLIN_2_PARAM].getValue()) * 12.0f * (5.0f * oscillator1.sin())) * (params[FMBUS_PARAM].getValue() + inputs[FMBUS_INPUT].getVoltage());
	}
	float FM2 = params[FMEXP_2_PARAM].getValue() * 12.0f * inputs[EXP_2_INPUT].getVoltage();
	const float expBase1 = 25.0f;
	if(inputs[EXP_2_INPUT].isConnected()) {
		pitchCv2 += (rescale(powf(expBase1, clamp(FM2 / 10.0f, 0.0f, 2.0f)), 1.0f, expBase1, 0.0f, 1.0f)) * (params[FMBUS_PARAM].getValue() + inputs[FMBUS_INPUT].getVoltage());
	}
	else {
		pitchCv2 += (rescale(powf(expBase1, clamp((params[FMEXP_2_PARAM].getValue() * 12.0f * (5.0f * oscillator1.sin())) / 10.0f, 0.0f, 2.0f)), 1.0f, expBase1, 0.0f, 1.0f)) * (params[FMBUS_PARAM].getValue() + inputs[FMBUS_INPUT].getVoltage());
	}

	LFOMODE2 = params[LFOMODE2_PARAM].getValue();

	oscillator2.setPitch(params[FREQ_2_PARAM].getValue(), pitchCv2 + pitchFine2, LFOMODE2);

	oscillator2.setPulseWidth(params[PW_2_PARAM].getValue() + inputs[PW_2_INPUT].getVoltage() / 10.0f);

	char SYNCB = params[SYNCSWITCHB_PARAM].getValue() + inputs[SYNCSWITCHB_CV_INPUT].getVoltage() > 0.0f;
	switch(SYNCB) {
		case 1:
		oscillator2.syncEnabled = true;
		oscillator2.process(1.0f / args.sampleRate, (5.0f * oscillator1.sin()));
		break;
		default:
		oscillator2.syncEnabled = inputs[SYNC2_INPUT].isConnected();
		oscillator2.process(1.0f / args.sampleRate, inputs[SYNC2_INPUT].getVoltage());
		break;
	}

	oscillator2.Sqr_Active(outputs[SQR_2_OUTPUT].isConnected());

	if (outputs[SIN_2_OUTPUT].isConnected()) {
		outputs[SIN_2_OUTPUT].setVoltage(6.0f * oscillator2.sin());
	}
	if (outputs[SQR_2_OUTPUT].isConnected()) {
		outputs[SQR_2_OUTPUT].setVoltage(6.0f * oscillator2.sqr());
	}


	// SHAPER

	float IN_1;

	if(inputs[EXT_SRC_INPUT].isConnected()) {
		IN_1 = inputs[EXT_SRC_INPUT].getVoltage();
	}
	else {
		IN_1 = 6.0f * oscillator2.sin();
	}

	float SHAPE_MOD = params[SHAPE_1_PARAM].getValue();

	float SHAPE_CV = inputs[SHAPE_CV_INPUT].getVoltage() * params[SHAPE_CV_PARAM].getValue();

	float upW = clamp(params[UP_PARAM].getValue() + inputs[UP_INPUT].getVoltage() / 2.5f, 0.0f, 4.0f);

	float downW = clamp(params[DOWN_PARAM].getValue() + inputs[DOWN_INPUT].getVoltage() / 2.5f, 0.0f, 4.0f);

	folder.Shape(IN_1, SHAPE_MOD, SHAPE_CV, upW, downW, outputs[FOLD_OUTPUT].isConnected());

	folder.process();

	float FoldAmp = clamp(params[INPUT_GAIN_PARAM].getValue() + inputs[SHAPE_1_CV_INPUT].getVoltage() / 10.f, 0.0f, 1.0f);
	float FoldOut = folder.Output() * FoldAmp;
	outputs[FOLD_OUTPUT].setVoltage(saturate(FoldOut / 1.5f));
};

struct VCOClassicMenu : MenuItem {
	VCO *vco;
	void onAction(const event::Action &e) override {
		vco->Theme = 0;
	}
	void step() override {
		rightText = (vco->Theme == 0) ? "✔" : "";
		MenuItem::step();
	}
};

struct VCONightModeMenu : MenuItem {
	VCO *vco;
	void onAction(const event::Action &e) override {
		vco->Theme = 1;
	}
	void step() override {
		rightText = (vco->Theme == 1) ? "✔" : "";
		MenuItem::step();
	}
};

struct VCOWidget : ModuleWidget {
	// Panel Themes
	SvgPanel *panelClassic;
	SvgPanel *panelNightMode;

#ifndef USING_CARDINAL_NOT_RACK
	void appendContextMenu(Menu *menu) override {
		VCO *vco = dynamic_cast<VCO*>(module);
		assert(vco);
		menu->addChild(construct<MenuEntry>());
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Theme"));
		menu->addChild(construct<VCOClassicMenu>(&VCOClassicMenu::text, "Classic (default)", &VCOClassicMenu::vco, vco));
		menu->addChild(construct<VCONightModeMenu>(&VCONightModeMenu::text, "Night Mode", &VCONightModeMenu::vco, vco));
	}
#endif

	VCOWidget(VCO *module);
	void step() override;
};


VCOWidget::VCOWidget(VCO *module) {
	setModule(module);
	box.size = Vec(27 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	// Classic Theme
	panelClassic = new SvgPanel();
	panelClassic->box.size = box.size;
	panelClassic->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/VCO.svg")));
	panelClassic->visible = !THEME_DEFAULT;
	addChild(panelClassic);
	// Night Mode Theme
	panelNightMode = new SvgPanel();
	panelNightMode->box.size = box.size;
	panelNightMode->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/VCO-Dark.svg")));
	panelNightMode->visible = !!THEME_DEFAULT;
	addChild(panelNightMode);

	addChild(createWidget<MScrewA>(Vec(15, 0)));
	addChild(createWidget<MScrewC>(Vec(box.size.x-30, 0)));
	addChild(createWidget<MScrewD>(Vec(15, 365)));
	addChild(createWidget<MScrewA>(Vec(box.size.x-30, 365)));

	addParam(createParam<OSCiXEGG>(Vec(125, 259.5), module, VCO::EASTEREGG_PARAM));

	addParam(createParam<VioMSwitch>(Vec(152.5, 30), module, VCO::MODE1_PARAM));
	addParam(createParam<VioMSwitch>(Vec(152.5, 80), module, VCO::SYNCSWITCHA_PARAM));
	addParam(createParam<VioMSwitch>(Vec(152.5, 130), module, VCO::SYNCSWITCHB_PARAM));
	addParam(createParam<VioM2Switch>(Vec(16, 109), module, VCO::LFOMODE1_PARAM));
	addParam(createParam<VioM2Switch>(Vec(289, 109), module, VCO::LFOMODE2_PARAM));

	addParam(createParam<RedLargeKnob>(Vec(43, 94.5), module, VCO::FREQ_1_PARAM));
	addParam(createParam<RedSmallKnob>(Vec(97, 131), module, VCO::FINE_1_PARAM));

	addParam(createParam<RedLargeKnob>(Vec(230, 94.5), module, VCO::FREQ_2_PARAM));
	addParam(createParam<RedSmallKnob>(Vec(192, 131), module, VCO::FINE_2_PARAM));

	addParam(createParam<BlueSmallKnob>(Vec(63.5, 183.5), module, VCO::FMLIN_1_PARAM));
	addParam(createParam<BlueSmallKnob>(Vec(225.5, 183.5), module, VCO::FMLIN_2_PARAM));
	addParam(createParam<BlueSmallKnob>(Vec(32, 231), module, VCO::FMEXP_1_PARAM));
	addParam(createParam<BlueSmallKnob>(Vec(257.5, 231), module, VCO::FMEXP_2_PARAM));
	addParam(createParam<BlueSmallKnob>(Vec(272, 158), module, VCO::PW_2_PARAM));
	addParam(createParam<GreenSmallKnob>(Vec(144.5, 183.5), module, VCO::FMBUS_PARAM));

	addParam(createParam<GreenSmallKnob>(Vec(349, 202), module, VCO::INPUT_GAIN_PARAM));
	addParam(createParam<RedLargeKnob>(Vec(341, 35), module, VCO::SHAPE_1_PARAM));
	addParam(createParam<GreenSmallKnob>(Vec(349, 154), module, VCO::SHAPE_CV_PARAM));
	addParam(createParam<GreenSmallKnob>(Vec(332, 105), module, VCO::UP_PARAM));
	addParam(createParam<GreenSmallKnob>(Vec(367, 105), module, VCO::DOWN_PARAM));

	addInput(createInput<SilverSixPortA>(Vec(55, 327.5), module, VCO::EXP_1_INPUT));
	addInput(createInput<SilverSixPortA>(Vec(14, 289.5), module, VCO::VOCT_1_INPUT));
	addInput(createInput<SilverSixPortD>(Vec(14, 327.5), module, VCO::SYNC1_INPUT));
	addInput(createInput<SilverSixPortC>(Vec(200.5, 289.5), module, VCO::VOCT_2_INPUT));
	addInput(createInput<SilverSixPort>(Vec(281.5, 327.5), module, VCO::SYNC2_INPUT));
	addInput(createInput<SilverSixPort>(Vec(55, 289.5), module, VCO::LIN_1_INPUT));
	addInput(createInput<SilverSixPortC>(Vec(240.5, 289.5), module, VCO::LIN_2_INPUT));
	addInput(createInput<SilverSixPortE>(Vec(240.5, 327.5), module, VCO::EXP_2_INPUT));
	addInput(createInput<SilverSixPortC>(Vec(281.5, 289.5), module, VCO::PW_2_INPUT));
	addInput(createInput<SilverSixPortA>(Vec(352, 327.5), module, VCO::EXT_SRC_INPUT));
	addInput(createInput<SilverSixPortB>(Vec(332, 289.5), module, VCO::SHAPE_1_CV_INPUT));
	addInput(createInput<SilverSixPort>(Vec(332, 249.5), module, VCO::SHAPE_CV_INPUT));
	addInput(createInput<SilverSixPortD>(Vec(95, 289.5), module, VCO::FMBUS_INPUT));
	addInput(createInput<SilverSixPortA>(Vec(95, 327.5), module, VCO::SYNCSWITCHA_CV_INPUT));
	addInput(createInput<SilverSixPortE>(Vec(200.5, 327.5), module, VCO::SYNCSWITCHB_CV_INPUT));
	addInput(createInput<SilverSixPortA>(Vec(372, 249.5), module, VCO::UP_INPUT));
	addInput(createInput<SilverSixPortC>(Vec(372, 289.5), module, VCO::DOWN_INPUT));

	addOutput(createOutput<SilverSixPortA>(Vec(14, 29), module, VCO::SIN_OUTPUT));
	addOutput(createOutput<SilverSixPort>(Vec(55, 29), module, VCO::TRI_OUTPUT));
	addOutput(createOutput<SilverSixPortD>(Vec(95, 29), module, VCO::SAW_OUTPUT));
	addOutput(createOutput<SilverSixPortC>(Vec(200, 29), module, VCO::SIN_2_OUTPUT));
	addOutput(createOutput<SilverSixPortB>(Vec(240.5, 29), module, VCO::SQR_2_OUTPUT));
	addOutput(createOutput<SilverSixPort>(Vec(281.5, 29), module, VCO::FOLD_OUTPUT));

};

void VCOWidget::step() {
#ifdef USING_CARDINAL_NOT_RACK
	panelClassic->visible = !settings::preferDarkPanels;
	panelNightMode->visible = settings::preferDarkPanels;
#else
	if (module) {
		VCO *vco = dynamic_cast<VCO*>(module);
		assert(vco);

		panelClassic->visible = (vco->Theme == 0);
		panelNightMode->visible = (vco->Theme == 1);
	}
#endif
	ModuleWidget::step();
}

Model *modelVCO = createModel<VCO, VCOWidget>("OSCiX");


struct BVCO : Module {

	enum ParamIds {
		FREQ_PARAM,
		FINE_PARAM,
		FMEXP_PARAM,
		FMLIN_PARAM,
		MODE1_PARAM,
		PW_PARAM,
		LFOMODE1_PARAM,
		NUM_PARAMS
	};

	enum InputIds {
		SYNC_INPUT,
		VOCT_INPUT,
		LIN_INPUT,
		EXP_INPUT,
		PW_INPUT,
		NUM_INPUTS
	};

	enum OutputIds {
		SIN_OUTPUT,
		RAMP_OUTPUT,
		TRI_OUTPUT,
		SAW_OUTPUT,
		SQR_OUTPUT,
		HALF_OUTPUT,
		FULL_OUTPUT,
		NUM_OUTPUTS
	};

	Boscillator<2, 4> oscillator;

	// Panel Theme
	int Theme = THEME_DEFAULT;

	bool LFOMOD = 1;

	BVCO() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		configParam(BVCO::MODE1_PARAM, 0.0, 1.0, 1.0, "Anlg/Digi");
		configParam(BVCO::LFOMODE1_PARAM, 0.0, 1.0, 1.0, "VCO/LFO");
		configParam(BVCO::FREQ_PARAM, -54.0, 54.0, 0.0, "Frequency", "Hz", 1.05946309436f, 261.626f, 0.f);
		configParam(BVCO::FINE_PARAM, -1.0, 1.0, 0.0, "Fine Frequency");
		configParam(BVCO::FMLIN_PARAM, 0.0, 1.0, 0.0, "Linear FM", "%", 0.f, 100);
		configParam(BVCO::FMEXP_PARAM, 0.0, 1.0, 0.0, "Exponential FM", "%", 0.f, 100);
		configParam(BVCO::PW_PARAM, 0.0, 1.0, 0.5, "Pulse Width", "%", 0.f, 100);
		configInput(VOCT_INPUT, "Volt/Octave");
		configInput(SYNC_INPUT, "Start of Cycle Sync");
		configInput(PW_INPUT, "Pulse Width Control Voltage");
		configInput(LIN_INPUT, "Linear FM Attenuator Control Voltage");
		configInput(EXP_INPUT, "Exponential FM Attenuator Control Voltage");
		configOutput(RAMP_OUTPUT, "Ramp");
		configOutput(SIN_OUTPUT, "Sin");
		configOutput(TRI_OUTPUT, "Triangle");
		configOutput(SAW_OUTPUT, "Sawtooth");
		configOutput(SQR_OUTPUT, "Square");
		configOutput(HALF_OUTPUT, "Half-Wave Rectified Sin");
		configOutput(FULL_OUTPUT, "Full-Wave Rectified Sin");
		getParamQuantity(LFOMODE1_PARAM)->randomizeEnabled = false;
	}

	void process(const ProcessArgs& args) override;

	//Json for Panel Theme
	json_t *dataToJson() override	{
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Theme", json_integer(Theme));
		return rootJ;
	}
	void dataFromJson(json_t *rootJ) override	{
		json_t *ThemeJ = json_object_get(rootJ, "Theme");
		if (ThemeJ)
			Theme = json_integer_value(ThemeJ);
	}

};

void BVCO::process(const ProcessArgs& args) {

	float pitchFine, pitchCv;

	oscillator.analog = params[MODE1_PARAM].getValue() > 0.0f;

	pitchFine = 3.0f * dsp::quadraticBipolar(params[FINE_PARAM].getValue());
	pitchCv = 12.0f * inputs[VOCT_INPUT].getVoltage();

	if(inputs[LIN_INPUT].isConnected()) {
		pitchCv += dsp::quadraticBipolar(params[FMLIN_PARAM].getValue()) * 12.0f * inputs[LIN_INPUT].getVoltage();
	}

	float FM = params[FMEXP_PARAM].getValue() * 12.0f * inputs[EXP_INPUT].getVoltage();
	const float expBase = 50.0f;
	if(inputs[EXP_INPUT].isConnected()) {
		pitchCv += rescale(powf(expBase, clamp(FM / 10.0f, 0.0f, 2.0f)), 1.0, expBase, 0.0f, 1.0f);
	}


	LFOMOD = params[LFOMODE1_PARAM].getValue();

	oscillator.setPitch(params[FREQ_PARAM].getValue(), pitchCv + pitchFine, LFOMOD);


	oscillator.setPulseWidth(params[PW_PARAM].getValue() + inputs[PW_INPUT].getVoltage() / 10.0f);
	oscillator.syncEnabled = inputs[SYNC_INPUT].isConnected();
	oscillator.process(1.0f / args.sampleRate, inputs[SYNC_INPUT].getVoltage());

	oscillator.Tri_Active(outputs[TRI_OUTPUT].isConnected());
	oscillator.Saw_Active(outputs[SAW_OUTPUT].isConnected());
	oscillator.Sqr_Active(outputs[SQR_OUTPUT].isConnected());
	oscillator.H_Active(outputs[HALF_OUTPUT].isConnected());
	oscillator.F_Active(outputs[FULL_OUTPUT].isConnected());
	oscillator.Ramp_Active(outputs[RAMP_OUTPUT].isConnected());

	if (outputs[SIN_OUTPUT].isConnected())
		outputs[SIN_OUTPUT].setVoltage(6.0f * oscillator.sin());
	if (outputs[RAMP_OUTPUT].isConnected())
		outputs[RAMP_OUTPUT].setVoltage((6.0f * oscillator.ramp()) - 6.0f);
	if (outputs[TRI_OUTPUT].isConnected())
		outputs[TRI_OUTPUT].setVoltage(6.0f * oscillator.tri());
	if (outputs[SAW_OUTPUT].isConnected())
		outputs[SAW_OUTPUT].setVoltage(6.0f * oscillator.saw());
	if (outputs[SQR_OUTPUT].isConnected())
		outputs[SQR_OUTPUT].setVoltage(6.0f * oscillator.sqr());
	if (outputs[HALF_OUTPUT].isConnected())
		outputs[HALF_OUTPUT].setVoltage(6.0f * oscillator.half());
	if (outputs[FULL_OUTPUT].isConnected())
		outputs[FULL_OUTPUT].setVoltage(6.0f * oscillator.full());

};

struct BVCOClassicMenu : MenuItem {
	BVCO *bvco;
	void onAction(const event::Action &e) override {
		bvco->Theme = 0;
	}
	void step() override {
		rightText = (bvco->Theme == 0) ? "✔" : "";
		MenuItem::step();
	}
};

struct BVCONightModeMenu : MenuItem {
	BVCO *bvco;
	void onAction(const event::Action &e) override {
		bvco->Theme = 1;
	}
	void step() override {
		rightText = (bvco->Theme == 1) ? "✔" : "";
		MenuItem::step();
	}
};

struct BVCOWidget : ModuleWidget {
	// Panel Themes
	SvgPanel *pClassic;
	SvgPanel *pNightMode;

#ifndef USING_CARDINAL_NOT_RACK
	void appendContextMenu(Menu *menu) override {
		BVCO *bvco = dynamic_cast<BVCO*>(module);
		assert(bvco);
		menu->addChild(construct<MenuEntry>());
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Theme"));
		menu->addChild(construct<BVCOClassicMenu>(&BVCOClassicMenu::text, "Classic (default)", &BVCOClassicMenu::bvco, bvco));
		menu->addChild(construct<BVCONightModeMenu>(&BVCONightModeMenu::text, "Night Mode", &BVCONightModeMenu::bvco, bvco));
	}
#endif

	BVCOWidget(BVCO *module);
	void step() override;
};

BVCOWidget::BVCOWidget(BVCO *module) {
	setModule(module);
	box.size = Vec(9 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	pClassic = new SvgPanel();
	pClassic->box.size = box.size;
	pClassic->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/BVCO.svg")));
	pClassic->visible = !THEME_DEFAULT;
	addChild(pClassic);

	pNightMode = new SvgPanel();
	pNightMode->box.size = box.size;
	pNightMode->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Panels/BVCO-Dark.svg")));
	pNightMode->visible = !!THEME_DEFAULT;
	addChild(pNightMode);

	addChild(createWidget<MScrewA>(Vec(15, 0)));
	addChild(createWidget<MScrewC>(Vec(box.size.x-30, 0)));
	addChild(createWidget<MScrewD>(Vec(15, 365)));
	addChild(createWidget<MScrewB>(Vec(box.size.x-30, 365)));

	addParam(createParam<VioMSwitch>(Vec(16, 65), module, BVCO::MODE1_PARAM));
	addParam(createParam<VioM2Switch>(Vec(106, 65), module, BVCO::LFOMODE1_PARAM));

	addParam(createParam<RedLargeKnob>(Vec(45, 50), module, BVCO::FREQ_PARAM));
	addParam(createParam<RedSmallKnob>(Vec(53, 115), module, BVCO::FINE_PARAM));
	addParam(createParam<BlueSmallKnob>(Vec(12, 167), module, BVCO::FMLIN_PARAM));
	addParam(createParam<BlueSmallKnob>(Vec(53, 167), module, BVCO::FMEXP_PARAM));
	addParam(createParam<BlueSmallKnob>(Vec(93, 167), module, BVCO::PW_PARAM));


	addInput(createInput<SilverSixPortA>(Vec(14, 213.5), module, BVCO::VOCT_INPUT));
	addInput(createInput<SilverSixPortD>(Vec(55, 213.5), module, BVCO::SYNC_INPUT));
	addInput(createInput<SilverSixPort>(Vec(95, 213.5), module, BVCO::PW_INPUT));
	addInput(createInput<SilverSixPortB>(Vec(14, 251.5), module, BVCO::LIN_INPUT));
	addInput(createInput<SilverSixPortC>(Vec(55, 251.5), module, BVCO::EXP_INPUT));
	addOutput(createOutput<SilverSixPortC>(Vec(95, 251.5), module, BVCO::RAMP_OUTPUT));

	addOutput(createOutput<SilverSixPortA>(Vec(14, 289.5), module, BVCO::SIN_OUTPUT));
	addOutput(createOutput<SilverSixPort>(Vec(55, 289.5), module, BVCO::TRI_OUTPUT));
	addOutput(createOutput<SilverSixPortD>(Vec(95, 289.5), module, BVCO::SAW_OUTPUT));
	addOutput(createOutput<SilverSixPortE>(Vec(14, 327.5), module, BVCO::SQR_OUTPUT));
	addOutput(createOutput<SilverSixPort>(Vec(55, 327.5), module, BVCO::HALF_OUTPUT));
	addOutput(createOutput<SilverSixPortD>(Vec(95, 327.5), module, BVCO::FULL_OUTPUT));

};

void BVCOWidget::step() {
#ifdef USING_CARDINAL_NOT_RACK
	pClassic->visible = !settings::preferDarkPanels;
	pNightMode->visible = settings::preferDarkPanels;
#else
	if (module) {
		BVCO *bvco = dynamic_cast<BVCO*>(module);
		assert(bvco);
		pClassic->visible = (bvco->Theme == 0);
		pNightMode->visible = (bvco->Theme == 1);
	}
#endif
	ModuleWidget::step();
}

Model *modelBVCO = createModel<BVCO, BVCOWidget>("Rogue");
