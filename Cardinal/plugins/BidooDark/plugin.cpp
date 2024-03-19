#include "../Bidoo/src/plugin.hpp"

void InstantiateExpanderItem::onAction(const event::Action &e) {
	engine::Module* module = model->createModule();
	APP->engine->addModule(module);
	ModuleWidget* mw = model->createModuleWidget(module);
	if (mw) {
		APP->scene->rack->setModulePosNearest(mw, posit);
		APP->scene->rack->addModule(mw);
		history::ModuleAdd *h = new history::ModuleAdd;
		h->name = "create expander module";
		h->setModule(mw);
		APP->history->push(h);
	}
}

json_t* BidooModule::dataToJson() {
	return json_object();
}

void BidooModule::dataFromJson(json_t*) {
}

void BidooWidget::appendContextMenu(Menu*) {
}

void BidooWidget::writeThemeAndContrastAsDefault() {
}

void BidooWidget::readThemeAndContrastFromDefault() {
}

void BidooWidget::prepareThemes(const std::string& filename) {
	setPanel(APP->window->loadSvg(filename));
}

void BidooWidget::step() {
	ModuleWidget::step();
}
