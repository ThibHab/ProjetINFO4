//  Author: Andrew Simper of Vellocet
//          andy@vellocet.com
//
// Purpose: c++ class to generate white, pink and brown noise
//
// Sources:
// This is a c++ implementation of put together from the
// code provided by the following people mainly from
// the music-dsp mailing list:  Allan Herriman, James McCartney,
// Phil Burk and Paul Kellet and the web page by Robin Whittle:
// http://www.firstpr.com.au/dsp/pink-noise/
//

class VRand
{
public:
  VRand()
  {
    //m_white = 0;
    m_brown = 0.0f;
	//m_seed = 0.0f;
  };
   /*
	void seed(unsigned int seed=0) {
		std::uniform_int_distribution<int> randomNumber(-1, 1);
		seed = randomNumber(randomGenerator);
		m_seed = seed;
	};

  // returns psuedo random white noise number
  // in the range -scale to scale
  //
	*/
	inline float white() {
		//m_seed   = (m_seed * 196314165) + 907633515;
		//m_white  = m_seed >> 9;
		//m_white |= 0x40000000;
		return 2.0f * random::normal();
	};

  // returns brown noise random number in the range -0.5 to 0.5
  //
	inline float brown(void) {
		while (true) {
		  float  r = white();
		  m_brown += r;
		  if (m_brown<-8.0f || m_brown>8.0f) m_brown -= r;
		  else break;
		}
		return m_brown*0.0625f;
	}

private:
	//unsigned int  m_seed;
	//unsigned int  m_white;
	//unsigned int randomNumber;

	//std::random_device randomGenerator;
	float          m_brown;
};
