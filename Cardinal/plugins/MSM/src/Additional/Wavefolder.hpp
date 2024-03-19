

class Wavefold {
	public:
		Wavefold() 
		{
			
		}
		
		void Shape(double in, double sc, double sccv, double sym, double range, double up, double down, bool folderactive) {
			_sc = sc + sccv;
			_sym = sym;
			_range = range;
			_up = up;
			_down = down;
			_in = in;
			_folderactive = folderactive;
		};

		void processA() {
				if(_folderactive) {
					double s = 1.0;
					double j = -1.0;
					double combine = 0, sig1 = 0, sig2 = 0, X = 0, Y = 0, Z = 0, W = 0, A1 = 0, A2 = 0, result = 0;
					
					combine = (_in + _sym) / (1.0f - _sc + 1.0f);
					
					sig1 = (-(combine - 2.0f) * _range);
					X = std::isgreater(combine, s); 
					Y =	sig1 * X;
					//X = combine > 1.0f;
					//Y = sig1 * X;
					A1 = fastSin(Y * _up) * 2.0f;
					
					sig2 = (-(combine + 2.0f) * _range);
					Z = std::isless(combine, j); 
					W =	sig2 * Z;
					//Z = combine < -1.0f;
					//W = sig2 * Z;
					A2 = fastSin(W * _down) * 2.0f;
					
					result = combine + A1 + A2;
					
					folderBuffer = (2.0f * fastSin(tanh_noclip(fastSin(result * M_PI / 2.0f)) / M_PI)) * 10.0f;
				}
		};
		
		void processB() {
				if(_folderactive) {
					double s = 1.0;
					double j = -1.0;
					double comb = 0, siga = 0, sigb = 0, d = 0, e = 0, f = 0, g = 0, resultb = 0, upS = 0, downS = 0;
												
					comb = (_in + _sym) / (1.0f - _sc + 1.0f);
					
					siga = (-(comb - 4.0f) * _range);
					d = std::isgreater(comb, s); 
					e =	siga * d;
					//d = comb > 1.0f;
					//e = siga * d;
					downS = (e * _down) * 1.5f;
					
					sigb = (-(comb + 4.0) * _range);
					f = std::isless(comb, j); 
					g =	sigb * f;
					//f = comb < -1.0f;
					//g = sigb * f;
					upS = (g * _up) * 1.5f;
					
					resultb = comb + downS + upS;
					
					folderBuffer = (4.0f * fastSin(atan(fastSin(resultb*M_PI/4.0f))/M_PI)) * 5.0f;
				}
		};

		inline double Output() {
			return folderBuffer;
		};
	
	private:
		
		double folderBuffer = 0.0f;
		
		double _sc = 0.0f;
		double _sym = 0.0f;
		double _range = 0.0f;
		double _up = 0.0f;
		double _down = 0.0f;
		double _in = 0.0f;
		
		bool _folderactive = false;
};