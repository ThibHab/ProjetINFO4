#include "unless.hpp"

Plugin *pluginInstance;

_less::Theme theme = _less::Theme();

void init(rack::Plugin *p) {
  pluginInstance = p;
  
  _less::ThemeFile t = _less::ThemeFile();
  t.check(&theme);

  p->addModel(modelPianoid);
  p->addModel(modelAvoider);
  p->addModel(modelCantor);
  p->addModel(modelTowers);
  p->addModel(modelRoom);
  p->addModel(modelPremuter);
  p->addModel(modelSnake);
  p->addModel(modelPiong);
  p->addModel(modelAtoms);
  p->addModel(modelChainkov);
}
