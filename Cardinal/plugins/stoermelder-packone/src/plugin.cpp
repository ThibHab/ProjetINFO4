#include "plugin.hpp"

Plugin* pluginInstance;

void init(rack::Plugin* p) {
	pluginInstance = p;

	p->addModel(modelCVMap);
	p->addModel(modelCVMapCtx);
	p->addModel(modelCVMapMicro);
	p->addModel(modelCVPam);
	p->addModel(modelRotorA);
	p->addModel(modelReMoveLite);
	p->addModel(modelBolt);
	p->addModel(modelInfix);
	p->addModel(modelInfixMicro);
	p->addModel(modelStrip);
	p->addModel(modelStripBay4);
	p->addModel(modelStripPp);
	p->addModel(modelEightFace);
	p->addModel(modelEightFaceX2);
	p->addModel(modelMidiCat);
	p->addModel(modelMidiCatMem);
	p->addModel(modelMidiCatCtx);
	p->addModel(modelSipo);
	p->addModel(modelFourRounds);
	p->addModel(modelArena);
	p->addModel(modelMaze);
	p->addModel(modelHive);
	p->addModel(modelIntermix);
	p->addModel(modelIntermixGate);
	p->addModel(modelIntermixEnv);
	p->addModel(modelIntermixFade);
	p->addModel(modelSail);
	p->addModel(modelPile);
	p->addModel(modelPilePoly);
	p->addModel(modelMidiStep);
	p->addModel(modelMirror);
	p->addModel(modelAffix);
	p->addModel(modelAffixMicro);
	p->addModel(modelGrip);
	p->addModel(modelGlue);
	p->addModel(modelGoto);
	p->addModel(modelStroke);
	p->addModel(modelSpin);
	p->addModel(modelTransit);
	p->addModel(modelTransitEx);
	p->addModel(modelX4);
	p->addModel(modelMacro);
	p->addModel(modelRaw);
	p->addModel(modelMidiMon);
	p->addModel(modelOrbit);
	p->addModel(modelEightFaceMk2);
	p->addModel(modelEightFaceMk2Ex);
	p->addModel(modelMidiPlug);
	p->addModel(modelDirt);
	p->addModel(modelMidiKey);
	p->addModel(modelAudioInterface64);
	p->addModel(modelMb);
	p->addModel(modelMe);

	pluginSettings.readFromJson();
}


namespace StoermelderPackOne {

std::map<std::string, Widget*> singletons;

bool registerSingleton(std::string name, Widget* mw) {
	auto it = singletons.find(name);
	if (it == singletons.end()) {
		singletons[name] = mw;
		return true;
	}
	return false;
}

bool unregisterSingleton(std::string name, Widget* mw) {
	auto it = singletons.find(name);
	if (it != singletons.end() && it->second == mw) {
		singletons.erase(it);
		return true;
	}
	return false;
}

Widget* getSingleton(std::string name) {
	auto it = singletons.find(name);
	return it != singletons.end() ? it->second : NULL;
}

} // namespace StoermelderPackOne