/*
based on Fundamental LFO by Andrew Belt
*/

class LowFrequencyOscillator {
	public:
		LowFrequencyOscillator()
		{

		}

		void Settings(bool offset, bool invert) {
			_offset = offset;
			_invert = invert;
		}

		void setPitch(float pitch) {
			pitch = fminf(pitch, 10.0f);
			freq = powf(2.0f, pitch);
		}
		void setPulseWidth(float pw_) {
			const float pwMin = 0.01f;
			pw = clamp(pw_, pwMin, 1.0f - pwMin);
		}
		void setReset(float reset) {
			if (resetTrigger.process(reset / 0.01f)) {
				phase = 0.0f;
			}
		}
		void step(float dt) {
			float deltaPhase = fminf(freq * dt, 0.5f);
			phase += deltaPhase;
			if (phase >= 1.0f)
				phase -= 1.0f;
		}
		inline float sin() {
			if (_offset)
				return 1.0f - std::cos(2.0f*M_PI * phase) * (_invert ? -1.0f : 1.0f);
			else
				return std::sin(2.0f*M_PI * phase) * (_invert ? -1.0f : 1.0f);
		}
		inline float tri(float x) {
			return 4.0f * fabsf(x - std::roundf(x));
		}
		inline float tri() {
			if (_offset)
				return tri(_invert ? phase - 0.5f : phase);
			else
				return -1.0f + tri(_invert ? phase - 0.25f : phase - 0.75f);
		}
		inline float saw(float x) {
			return 2.0f * (x - std::roundf(x));
		}
		inline float saw() {
			if (_offset)
				return _invert ? 2.0f * (1.0f - phase) : 2.0f * phase;
			else
				return saw(phase) * (_invert ? -1.0f : 1.0f);
		}
		inline float sqr() {
			float sqr = ((phase < pw) ^ _invert) ? 1.0f : -1.0f;
			return _offset ? sqr + 1.0f : sqr;
		}

	private:
		float phase = 0.0f;
		float pw = 0.5f;
		float freq = 1.0f;
		bool _offset = false;
		bool _invert = false;
		dsp::SchmittTrigger resetTrigger;
};
