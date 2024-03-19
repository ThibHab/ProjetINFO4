#include "widgets.hpp"

DaviesMaroonLargeSnap::DaviesMaroonLargeSnap() {
  snap = true;
  minAngle = -0.83 * M_PI;
  maxAngle = 0.83 * M_PI;
  setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/davies_maroon_large.svg")));
};

DaviesMaroonMedium::DaviesMaroonMedium() {
  minAngle = -0.83 * M_PI;
  maxAngle = 0.83 * M_PI;
  setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/davies_maroon_medium.svg")));
};

DaviesMaroonMediumSnap::DaviesMaroonMediumSnap() {
  snap = true;
};

DaviesGrayMedium::DaviesGrayMedium() {
  minAngle = -0.83 * M_PI;
  maxAngle = 0.83 * M_PI;
  setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/davies_gray_medium.svg")));
};

DaviesGrayMediumSnap::DaviesGrayMediumSnap() {
  snap = true;
};
