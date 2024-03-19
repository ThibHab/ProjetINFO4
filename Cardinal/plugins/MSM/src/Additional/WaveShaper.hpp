

template <int OVERSAMPLE, int QUALITY>
class WaveShape {
	public:
		WaveShape() 
		{
			
		}
		
		void Shape1(double in, double sc, double sccv, bool folderactive) {
			_sc1 = sc + sccv;
			_in1 = in;
			_folderactive1 = folderactive;
		};

		void Shape2(double in, double sc, double sccv, bool folderactive) {
			_sc2 = sc + sccv;
			_in2 = in;
			_folderactive2 = folderactive;
		};
		
		void Shape3(double in, double sc, double sccv, bool folderactive) {
			_sc3 = sc + sccv;
			_in3 = in;
			_folderactive3 = folderactive;
		};
		
		void process() {
				if(_folderactive1) {
					double s = 1.0;
					double j = -1.0;
					double k = 0, combine = 0, sig1 = 0, sig2 = 0, X = 0, Y = 0, Z = 0, W = 0, result1 = 0;
					
					k = _sc1 * (_sc1 - 4.0f);
					
					combine = (_in1) / (1.0f - k + 1.0f);
					
					sig1 = -(combine - 1.0f) * 2.0f;
						X = std::isgreater(combine, s); 
						Y =	sig1 * X;
					
					sig2 = -(combine + 1.0f) * 2.0f;
						Z = std::isless(combine, j);
						W = sig2 * Z;
					
					result1 = combine + Y + W;
					
					folderBuffer1 = (2.0f * fastSin(tanh_noclip(fastSin(result1 * M_PI / 2.0f)) / M_PI)) * 10.0f;
				}

				if(_folderactive2) {
					double comba = 0, comb1 = 0, resultb = 0;
					
					comba = (_in2) / (1.0f - _sc2 + 1.0f);
					comb1 = _in2 + 1.0f;
														
					resultb = comba * (_sc2 * comb1 + 2.0f);
																				
					folderBuffer2 = (2.0f * fastSin(fastSin(fastSin(resultb * M_PI / 4.0f)) / M_PI)) * 10.0f;
				}

				if(_folderactive3) {
					double t = 1.0;
					double o = -1.0;
					double combB = 0, siga = 0, sigb = 0, d = 0, e = 0, f = 0, g = 0, downS = 0, upS = 0, resultc = 0;
												
					combB = (_in3) / (1.0f - _sc3 + 1.0f);
					
					siga = (-(combB - 1.0f) * 2.0f);
					d = std::isgreater(combB, t); 
					e =	siga * d;
					downS = fastSin(e * 1.0f) * 1.0f;
					
					sigb = (-(combB + 1.0) * 2.0f);
					f = std::isless(combB, o);
					g = sigb * f;
					upS = fastSin(g * 1.0f) * 1.0f;
					
					resultc = (combB + downS + upS) * 1.0f;
					
					folderBuffer3 = (2.0f * fastSin(tanh_noclip(fastSin(resultc*M_PI/2.0f))/M_PI)) * 10.0f;
				}
		};
		
		inline double MasterOutput() {
			return folderBuffer1 + folderBuffer2 + folderBuffer3;
		};

	private:

		double folderBuffer1 = 0.0f;
		double folderBuffer2 = 0.0f;
		double folderBuffer3 = 0.0f;
		
		double _sc1 = 0.0f, _sc2 = 0.0f, _sc3 = 0.0f;
		double _in1 = 0.0f, _in2 = 0.0f, _in3 = 0.0f;
				
		bool _folderactive1 = false, _folderactive2 = false, _folderactive3 = false;
};