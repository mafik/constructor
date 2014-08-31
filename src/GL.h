// Copyright 2014 Marek Rogalski <marek@mrogalski.eu>
#ifndef GL_H_
#define GL_H_
#include <SkMatrix.h>
#include <unordered_map>
#include <utility>
#include <string>

class GrContext;
class SkGpuDevice;
class SkCanvas;
class SkPicture;

namespace constructor {
  using std::string;
  using std::unordered_map;
  using std::pair;

  class GL {
   public:
    static GL instance;

    GrContext* context;
    SkGpuDevice* device;
    SkCanvas* canvas;
    SkMatrix matrix;

    struct {
      SkPicture* close;
    } icons;

    void Init();
  };
}

#endif  // GL_H_
