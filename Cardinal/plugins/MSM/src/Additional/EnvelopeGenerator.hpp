
struct Generator {
	float _current = 0.0;

	Generator() {}
	virtual ~Generator() {}

	float current() {
		return _current;
	}

	float next() {
		return _current = _next();
	}

	virtual float _next() = 0;
};

struct EnvelopeGenerator : Generator {
	float _sampleRate;
	float _sampleTime;

	EnvelopeGenerator(float sampleRate = 1000.0f)
	: _sampleRate(sampleRate > 1.0 ? sampleRate : 1.0)
	, _sampleTime(1.0f / _sampleRate)
	{
	}

	void setSampleRate(float sampleRate);
	virtual void _sampleRateChanged() {}
};

struct EnvelopeGen : EnvelopeGenerator {
	enum Stage {
		STOPPED_STAGE,
		ATTACK_STAGE,
		DECAY_STAGE,
		SUSTAIN_STAGE,
		RELEASE_STAGE
	};

	Stage _stage = STOPPED_STAGE;
	bool _gated = false;
	float _attack = 0.0f;
	float _decay = 0.0f;
	float _sustain = 1.0f;
	float _release = 0.0f;
	float _attackShape;
	float _decayShape;
	float _releaseShape;
	float _stageProgress = 0.0f;
	float _releaseLevel = 0.0f;
	float _envelope = 0.0f;

	EnvelopeGen(char linear = 0, float sampleRate = 1000.0f) : EnvelopeGenerator(sampleRate) {
		setLinearShape(linear);
	}
		
	void reset();
	void setGate(bool high, bool retrig);
	void setAttack(float seconds);
	void setDecay(float seconds);
	void setSustain(float level, float cv);
	void setRelease(float seconds);
	void setLinearShape(char linear);
	void setShapes(float attackShape, float decayShape, float releaseShape);
	bool isStage(Stage stage) { return _stage == stage; }

	float _next() override;
};

void EnvelopeGenerator::setSampleRate(float sampleRate) {
	if (_sampleRate != sampleRate && sampleRate >= 1.0) {
		_sampleRate = sampleRate;
		_sampleTime = 1.0f / sampleRate;
		_sampleRateChanged();
	}
}

void EnvelopeGen::reset() {
	_stage = STOPPED_STAGE;
	_gated = false;
	_envelope = 0.0f;
}

void EnvelopeGen::setGate(bool high, bool retrig) {
	_gated = high;
	if(retrig) {
		_stage = DECAY_STAGE;
	}
}

void EnvelopeGen::setAttack(float seconds) {
	assert(_attack >= 0.0f);
	_attack = std::max(seconds, 0.01f);
}

void EnvelopeGen::setDecay(float seconds) {
	assert(_decay >= 0.0f);
	_decay = std::max(seconds, 0.01f);
}

void EnvelopeGen::setSustain(float level, float cv) {
	assert(_sustain >= 0.0f);
	assert(_sustain <= 1.0f);
	_sustain = clamp(level + cv / 10.0f, 0.0f, 1.0f);
}

void EnvelopeGen::setRelease(float seconds) {
	assert(_release >= 0.0f);
	_release = std::max(seconds, 0.01f);
}

void EnvelopeGen::setLinearShape(char linear) {
	if(linear == 0){
		setShapes(1.5f, 0.5f, 0.5f);
	}
	else if(linear == 1) {
		setShapes(0.5f, 0.5f, 0.5f);
	}
	
	else if (linear == 2) {
		setShapes(1.0f, 1.0f, 1.0f);
	}
}

void EnvelopeGen::setShapes(float attackShape, float decayShape, float releaseShape) {
	assert(attackShape >= 0.1f && attackShape <= 10.0f);
	assert(decayShape >= 0.1f && decayShape <= 10.0f);
	assert(releaseShape >= 0.1f && releaseShape <= 10.0f);
	_attackShape = attackShape;
	_decayShape = decayShape;
	_releaseShape = releaseShape;
}

float EnvelopeGen::_next() {
	if (_gated) {
		switch (_stage) {
			case STOPPED_STAGE: {
				_stage = ATTACK_STAGE;
				_stageProgress = 0.0f;
				break;
			}
			case ATTACK_STAGE: {
				if (_envelope >= 1.0) {
					_stage = DECAY_STAGE;
					_stageProgress = 0.0f;
				}
				break;
			}
			case DECAY_STAGE: {
				if (_stageProgress >= _decay) {
					_stage = SUSTAIN_STAGE;
					_stageProgress = 0.0f;
				}
				break;
			}
			case SUSTAIN_STAGE: {
				break;
			}
			case RELEASE_STAGE: {
				_stage = ATTACK_STAGE;
				_stageProgress = _attack * powf(_envelope, 1.0f / _releaseShape);
				break;
			}
		}
	}
	else {
		switch (_stage) {
			case STOPPED_STAGE: {
				break;
			}
			case ATTACK_STAGE:
			case DECAY_STAGE:

			case SUSTAIN_STAGE: {
				_stage = RELEASE_STAGE;
				_stageProgress = 0.0f;
				_releaseLevel = _envelope;
				break;
			}
			case RELEASE_STAGE: {
				if (_stageProgress >= _release) {
					_stage = STOPPED_STAGE;
				}
				break;
			}
		}
	}

	switch (_stage) {
		case STOPPED_STAGE: {
			_envelope = 0.0f;
			break;
		}
		case ATTACK_STAGE: {
			_stageProgress += _sampleTime;
			_envelope = _stageProgress / _attack;
			_envelope = powf(_envelope, _attackShape);
			break;
		}
		case DECAY_STAGE: {
			_stageProgress += _sampleTime;
			_envelope = _stageProgress / _decay;
			_envelope = powf(_envelope, _decayShape);
			_envelope *= 1.0f - _sustain;
			_envelope = 1.0f - _envelope;
			break;
		}
		case SUSTAIN_STAGE: {
			break;
		}
		case RELEASE_STAGE: {
			_stageProgress += _sampleTime;
			_envelope = _stageProgress / _release;
			_envelope = powf(_envelope, _releaseShape);
			_envelope *= _releaseLevel;
			_envelope = _releaseLevel - _envelope;
			break;
		}
	}

	return _envelope;
}
