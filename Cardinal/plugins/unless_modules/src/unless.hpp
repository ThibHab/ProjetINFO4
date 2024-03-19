#include "rack.hpp"
#include "theme.hpp"
#include "utils.hpp"
using namespace rack;

extern Plugin *pluginInstance;

#ifdef USING_CARDINAL_NOT_RACK
	static _less::Theme theme;
#else
	extern _less::Theme theme;
#endif 
	
extern Model *modelPiong;
extern Model *modelChainkov;
extern Model *modelAtoms;
extern Model *modelCantor;
extern Model *modelRoom;
extern Model *modelSnake;
extern Model *modelTowers;
extern Model *modelPianoid;
extern Model *modelPremuter;
extern Model *modelAvoider;