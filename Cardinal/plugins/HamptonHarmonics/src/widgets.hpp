#pragma once

#include "rack.hpp"

using namespace rack;

extern Plugin *pluginInstance;

struct DaviesMaroonLargeSnap : app::SvgKnob {
  DaviesMaroonLargeSnap();
};

struct DaviesMaroonMedium : app::SvgKnob {
  DaviesMaroonMedium();
};

struct DaviesMaroonMediumSnap : DaviesMaroonMedium {
  DaviesMaroonMediumSnap();
};

struct DaviesGrayMedium : app::SvgKnob {
  DaviesGrayMedium();
};

struct DaviesGrayMediumSnap : DaviesGrayMedium {
  DaviesGrayMediumSnap();
};
