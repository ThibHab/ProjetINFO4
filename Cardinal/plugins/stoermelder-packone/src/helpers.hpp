#pragma once
#include "plugin.hpp"
#include "settings.hpp"

namespace StoermelderPackOne {
namespace Rack {

/** Move the view-port smoothly and center a Widget
 */
struct ViewportCenterSmooth {
	Vec source, target;
	float sourceZoom, targetZoom;
	int framecount = 0;
	int frame = 0;

	void trigger(Widget* w, float zoom, float framerate, float transitionTime = 1.f) {
		Vec target = w->getBox().getCenter();
		zoom = std::pow(2.f, zoom);
		trigger(target, zoom, framerate, transitionTime);
	}

	void trigger(Rect rect, float framerate, float transitionTime = 1.f) {
		float zx = APP->scene->rackScroll->box.size.x / rect.size.x * 0.9f;
		float zy = APP->scene->rackScroll->box.size.y / rect.size.y * 0.9f;
		float zoom = std::min(zx, zy);
		trigger(rect.getCenter(), zoom, framerate, transitionTime);
	}

	void trigger(Vec target, float zoom, float framerate, float transitionTime = 1.f) {
		// source is at top-left, translate to center of screen
		Vec source = APP->scene->rackScroll->offset / APP->scene->rackScroll->getZoom();
		Vec center = APP->scene->rackScroll->getSize() * (1.f / APP->scene->rackScroll->getZoom()) * 0.5f;

		this->source = source + center;
		this->target = target;
		this->sourceZoom = APP->scene->rackScroll->getZoom();
		this->targetZoom = zoom;
		this->framecount = int(transitionTime * framerate);
		this->frame = 0;
	}

	void reset() {
		frame = framecount = 0;
	}

	void process() {
		if (framecount == frame) return;

		float t = float(frame) / float(framecount - 1);
		// Sigmoid
		t = t * 8.f - 4.f;
		t = 1.f / (1.f + std::exp(-t));
		t = rescale(t, 0.0179f, 0.98201f, 0.f, 1.f);

		// Calculate interpolated view-point and zoom
		Vec p1 = source.mult(1.f - t);
		Vec p2 = target.mult(t);
		Vec p = p1.plus(p2);
		
		// Ignore tiny changes in zoom as they will cause graphical artifacts
		if (std::abs(sourceZoom - targetZoom) > 0.01f) {
			float z = sourceZoom * (1.f - t) + targetZoom * t;
			APP->scene->rackScroll->setZoom(z);
		}

		// Move the view
		Vec center = APP->scene->rackScroll->getSize() * (1.f / APP->scene->rackScroll->getZoom()) * 0.5f;
		APP->scene->rackScroll->setGridOffset((p - center - RACK_OFFSET) / RACK_GRID_SIZE);

		frame++;
	}
};

struct ViewportCenter {
	ViewportCenter(Widget* w, float zoomToWidget = -1.f, float zoom = -1.f) {
		float z;
		if (zoomToWidget > 0.f)
			z = APP->scene->rackScroll->getSize().y / w->getSize().y * zoomToWidget;
		else if (zoom > 0.f)
			z = std::pow(2.f, zoom);
		else
			z = 2.0f;
		Vec target = w->getBox().getCenter();
		Vec viewport = APP->scene->rackScroll->getSize() * (1.f / z);
		APP->scene->rackScroll->setZoom(z);
		APP->scene->rackScroll->setGridOffset((target - viewport * 0.5f - RACK_OFFSET) / RACK_GRID_SIZE);
	}

	ViewportCenter(Vec target) {
		float z = APP->scene->rackScroll->getZoom();
		Vec viewport = APP->scene->rackScroll->getSize() * (1.f / z);
		APP->scene->rackScroll->setZoom(z);
		APP->scene->rackScroll->setGridOffset((target - viewport * 0.5f - RACK_OFFSET) / RACK_GRID_SIZE);
	}

	ViewportCenter(Rect rect) {
		Vec target = rect.getCenter();
		float zx = APP->scene->rackScroll->getSize().x / rect.size.x * 0.9f;
		float zy = APP->scene->rackScroll->getSize().y / rect.size.y * 0.9f;
		float z = std::min(zx, zy);
		Vec viewport = APP->scene->rackScroll->getSize() * (1.f / z);
		APP->scene->rackScroll->setZoom(z);
		APP->scene->rackScroll->setGridOffset((target - viewport * 0.5f - RACK_OFFSET) / RACK_GRID_SIZE);
	}
};

struct ViewportTopLeft {
	ViewportTopLeft(Widget* w, float zoomToWidget = -1.f, float zoom = -1.f) {
		float z;
		if (zoomToWidget > 0.f)
			z = APP->scene->rackScroll->getSize().y / w->getSize().y * zoomToWidget;
		else if (zoom > 0.f)
			z = std::pow(2.f, zoom);
		else
			z = 2.0f;
		Vec target = w->getBox().getTopLeft();
		APP->scene->rackScroll->setZoom(z);
		APP->scene->rackScroll->setGridOffset((target - RACK_OFFSET) / RACK_GRID_SIZE);
	}
};


/** Creates a MenuItem that when hovered, opens a submenu with several MenuItems identified by a map.
Example:
	menu->addChild(createMapSubmenuItem<QUALITY>("Mode",
		{
			{ QUALITY::HIFI, "Hi-fi" },
			{ QUALITY::MIDFI, "Mid-fi" },
			{ QUALITY::LOFI, "Lo-fi" }
		},
		[=]() {
			return module->getMode();
		},
		[=](QUALITY mode) {
			module->setMode(mode);
		}
	));
*/
template <typename TEnum, class TMenuItem = ui::MenuItem>
ui::MenuItem* createMapSubmenuItem(std::string text, std::map<TEnum, std::string> labels, std::map<TEnum, std::string> labelsPlugin, std::function<TEnum()> getter, std::function<void(TEnum val)> setter, bool showRightText = true, bool disabled = false, bool alwaysConsume = false) {
	struct IndexItem : ui::MenuItem {
		std::function<TEnum()> getter;
		std::function<void(TEnum)> setter;
		TEnum index;
		bool alwaysConsume;

		void step() override {
			TEnum currIndex = getter();
			this->rightText = CHECKMARK(currIndex == index);
			MenuItem::step();
		}
		void onAction(const event::Action& e) override {
			setter(index);
			if (alwaysConsume)
				e.consume(this);
		}
	};

	struct Item : TMenuItem {
		std::function<TEnum()> getter;
		std::function<void(TEnum)> setter;
		std::map<TEnum, std::string> labels;
		TEnum currIndex;
		bool showRightText;
		bool alwaysConsume;

		void step() override {
			TEnum currIndex = getter();
			if (showRightText) {
				if (this->currIndex != currIndex) {
					std::string label = labels[currIndex];
					this->rightText = label + "  " + RIGHT_ARROW;
					this->currIndex = currIndex;
				}
			}
			else {
				this->rightText = RIGHT_ARROW;
			}
			TMenuItem::step();
		}
		ui::Menu* createChildMenu() override {
			ui::Menu* menu = new ui::Menu;
			for (const auto& i : labels) {
				IndexItem* item = createMenuItem<IndexItem>(i.second);
				item->getter = getter;
				item->setter = setter;
				item->index = i.first;
				item->alwaysConsume = alwaysConsume;
				menu->addChild(item);
			}
			return menu;
		}
	};

	Item* item = createMenuItem<Item>(text);
	item->getter = getter;
	item->setter = setter;
	item->labels = settings::isPlugin ? labelsPlugin : labels;
	item->showRightText = showRightText;
	item->disabled = disabled;
	item->alwaysConsume = alwaysConsume;
	return item;
}

template <typename TEnum, class TMenuItem = ui::MenuItem>
ui::MenuItem* createMapSubmenuItem(std::string text, std::map<TEnum, std::string> labels, std::function<TEnum()> getter, std::function<void(TEnum val)> setter, bool showRightText = true, bool disabled = false, bool alwaysConsume = false) {
	return createMapSubmenuItem(text, labels, labels, getter, setter, showRightText, disabled, alwaysConsume);
}


/** Easy wrapper for createMapPtrSubmenuItem() that controls a mapped label at a pointer address.
Example:
	menu->addChild(createMapPtrSubmenuItem("Mode",
		{
			{ QUALITY::HIFI, "Hi-fi" },
			{ QUALITY::MIDFI, "Mid-fi" },
			{ QUALITY::LOFI, "Lo-fi" }
		},
		&module->mode
	));
*/
template <typename TEnum>
ui::MenuItem* createMapPtrSubmenuItem(std::string text, std::map<TEnum, std::string> labels, TEnum* ptr, bool showRightText = true) {
	return createMapSubmenuItem<TEnum>(text, labels,
		[=]() { return *ptr; },
		[=](TEnum index) { *ptr = TEnum(index); },
		showRightText
	);
}

/** Easy wrapper for createMenuItem() to modify a property with a specific value.
Example:
	menu->addChild(createValuePtrMenuItem("Loop", &module->mode, MODE::LOOP));
*/
template <typename T>
ui::MenuItem* createValuePtrMenuItem(std::string text, T* ptr, T val) {
	return createMenuItem(text, CHECKMARK(*ptr == val), [=]() { *ptr = val; });
}


} // namespace Rack
} // namespace StoermelderPackOne