
extern float grainTableA[2048];
extern float grainTableB[2048];
extern float grainTableC[2048];
extern float grainTableD[2048];
extern float grainTableE[2048];
extern float grainTableF[2048];
extern float grainTableG[2048];
extern float grainTableH[2048];
extern float grainTableI[2048];
extern float grainClarinett[2048];
extern float grainCello[2048];
extern float grainViolin[2048];
extern float grainpluck[2048];
extern float grainTableJ[2048];

template <int OVERSAMPLE, int QUALITY>
class MSVCO {
	public:
	MSVCO()
	{
		modder1 = 0.0f;
		modder2 = 0.0f;
		modder3 = 0.0f;
		lastSyncValue = 0.0f;
		phase = 0.0f;
		pitch = 0.0f;
		freq = 0.0f;
		deltaPhase = 0.0f;
		modder1 = 0.0f;
		_wdw = 0.0f;
		_type = 0;
		interp.reset_hist();

	}

	void setPitch(float _pitchK, float _pitchCV, char _lfomode) {
		pitch = _pitchK;
		pitch = roundf(pitch);
		pitch += _pitchCV;

		switch(_lfomode) {
			case 0:
				freq = (261.626f * std::pow(2.0f, pitch / 12.0f)) / 100.0f;
			break;
			case 1:
				freq = 261.626f * std::pow(2.0f, pitch / 12.0f);
			break;
		}
	}

	void Window(float _maxWindow) {
		_wdw = _maxWindow;
	}

	void Mods(float _mod1, float _mod2, float _mod3) {
		modder1 = _mod1;
		modder2 = _mod2;
		modder3 = _mod3;
	}

	void TYPE(float type) {
		_type = type;
	}

	void outEnabled(bool _enable) {
		_outEnabled = _enable;
	}

	void process(float deltaTime, float syncValue, bool sync) {
		syncEnabled = sync;
		// Advance phase
		deltaPhase = clamp(freq * deltaTime, 1e-6, 0.5f) * (1.0f / OVERSAMPLE);
		// Detect sync
		int syncIndex = -1.0f;
		float syncCrossing = 0.0f;
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

		for (int i = 0; i < OVERSAMPLE; i++) {
			if (syncIndex == i) {
				phase = 0.0f;
			}

			if(_outEnabled) {
				for(int y = 0; y < modder1; y++) {
					switch(_type) {
						case 0:
							a[i] =  interp.process4x((cosf(yamt * M_PI * phase / modder1) + modder2) * clamp(fastSin((yamt * M_PI) * fastSin(x * interp.process4xP(grainTableA, phase * _wdw, oo))) / modder3, rl, rh), out);
						break;
						case 1:
							a[i] = interp.process4x((fastSin(yamt * M_PI * phase / modder1) + modder2) * clamp(cosf((yamt * M_PI) * cosf(x * interp.process4xP(grainTableB, phase * _wdw, oo))) / modder3, rl, rh), out);
						break;
						case 2:
							a[i] = interp.process8x(clamp(fastSin(fastSin(((yamt / modder3) * M_PI / modder2) * (x * interpolateLinear(grainTableC, phase * _wdw)) / modder1)), rl, rh), out);
						break;
						case 3:
							a[i] = interp.process8x(clamp(fastSin((yamt * M_PI / modder2) * (x * interpolateLinear(grainTableD, phase * _wdw)) / modder1), rl, rh) / modder3, out);
						break;
						case 4:
							a[i] = interp.process8x(clamp(fastSin((yamt * (M_PI / modder3) / modder2) * (sqramt * interpolateLinear(grainTableE, phase * _wdw)) / modder1), rl, rh), out);
						break;
						case 5:
							a[i] = interp.process8x(clamp(fastSin((yamt * M_PI / modder2) * (x * interpolateLinear(grainTableF, phase * _wdw)) / modder1), rl, rh) / modder3, out);
						break;
						case 6:
							a[i] = interp.process8x(clamp(fastSin((yamt * M_PI / modder2) * (x * interpolateLinear(grainTableG, phase * _wdw)) / modder1), rl, rh) / modder3, out);
						break;
						case 7:
							a[i] = interp.process8x(clamp(fastSin((z * M_PI / modder2) * (x * interpolateLinear(grainTableH, phase * _wdw)) / modder1), rl, rh) / modder3, out);
						break;
						case 8:
							a[i] = interp.process8x(clamp(fastSin(((yamt / modder3) * M_PI / modder2) * (x * interpolateLinear(grainTableI, phase * _wdw)) / modder1), rl, rh), out);
						break;
						case 9:
							a[i] = interp.process8x(clamp(fastSin((yamt * M_PI / modder2) * (x * interpolateLinear(grainClarinett, phase * _wdw)) / modder1), rl, rh) / modder3, out);
						break;
						case 10:
							a[i] = interp.process8x(clamp(fastSin((yamt * M_PI / modder2) * (x * interpolateLinear(grainCello, phase * _wdw)) / modder1), rl, rh) / modder3, out);
						break;
						case 11:
							a[i] = interp.process8x(clamp(fastSin((z * M_PI / modder2) * (x * interpolateLinear(grainViolin, phase * _wdw)) / modder1), rl, rh) / modder3, out);
						break;
						case 12:
							a[i] = interp.process8x(clamp(fastSin((yamt * M_PI / modder2) * (x * interpolateLinear(grainpluck, phase * _wdw)) / modder1), rl, rh) / modder3, out);
						break;
						case 13:
							a[i] = interp.process8x(clamp(fastSin((yamt * M_PI / modder2) * (x * interpolateLinear(grainTableJ, phase * _wdw)) / modder1), rl, rh) / modder3, out);
						break;
					}
				}
			}
		}


			// Advance phase
			phase += deltaPhase;
			while (phase > 1.0f) {
                phase -= 1.0f;
            }
            while (phase < 0) {
                phase += 1.0f;
            }
	}

	inline float getOutput() {
		return  OutDecimator.process(out);
	}

	private:

		float a[OVERSAMPLE] = {};
		float out[OVERSAMPLE] = {};
		float oo[OVERSAMPLE] = {};
		interpolator_linear interp;

		dsp::Decimator<OVERSAMPLE, QUALITY> OutDecimator;

		float lastSyncValue;
		float phase;
		float freq;
		float pitch;
		float deltaPhase;

		char _type;
		float _wdw;
		float modder1;
		float modder2;
		float modder3;

		bool syncEnabled = false;
		bool syncDirection = false;

		bool _outEnabled = false;

		const float x = 1.25f;
		const float yamt = 2.0f;
		const float z = 1.0f;
		const float rl = -6.0f;
		const float rh = 6.0f;
		const float sqramt = 2.5f;
};
