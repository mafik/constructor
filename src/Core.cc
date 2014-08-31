// Copyright 2014 Marek Rogalski <marek@mrogalski.eu>
#include <SkCanvas.h>
#include <SkGpuDevice.h>
#include <GLES2/gl2.h>
#include <utility>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "./Core.h"
#include "./Utils.h"
#include "./Storage.h"
#include "./Display.h"

namespace constructor {
  using glm::mat4;
  using glm::vec3;

  Core Core::instance;

  void Core::Init() {
    Log("Getting storage directory\n");
    dataDirectory_ = getStorageDirectory();
    Log("Data directory: %s\n", dataDirectory_.c_str());
    // Deserialize nodes
  }

  Core::~Core() {
    // Serialize nodes
  }

  void Core::GLInit() {
    GL::instance.Init();
  }

  void Core::GLDraw() {
    GL::instance.canvas->setMatrix(GL::instance.matrix);
    GL::instance.device->clear(SK_ColorGRAY);
    SkRect r;
    r.set(0, 0, Window::width, Window::height);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorGREEN);
    GL::instance.canvas->drawOval(r, paint);
    paint.setColor(SK_ColorBLUE);
    paint.setStrokeWidth(Window::width/15);
    paint.setStyle(SkPaint::kStroke_Style);
    GL::instance.canvas->drawLine(0, 0, r.fRight, r.fBottom, paint);
    GL::instance.canvas->drawLine(0, r.fBottom, r.fRight, 0, paint);
    RadialMenu::DrawMenus();
    GL::instance.context->flush();
  }

  RadialMenu* Core::MakeGlobalMenu(const Touch& touch) {
    auto menu = new RadialMenu();
    menu->Init(touch, SkColorSetRGB(0xff, 0, 0));
    for (auto&& proto : prototypes_) {
      menu->AddEntry(Entry::NewFromPrototype(*proto));
    }
    return menu;
  }
}
