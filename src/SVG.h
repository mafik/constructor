// Copyright 2014 Marek Rogalski <marek@mrogalski.eu>
#ifndef SVG_H_
#define SVG_H_
#include <string>

class SkPicture;

namespace constructor {
SkPicture* LoadSVG(const std::string& resource_name);
}

#endif  // SVG_H_
