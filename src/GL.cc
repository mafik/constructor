// Copyright 2014 Marek Rogalski <marek@mrogalski.eu>
#include <SkImageDecoder.h>
#include <SkImageInfo.h>
#include "./GL.h"
#include "./Display.h"
#include "./SVG.h"

namespace constructor {
GL GL::instance;
void GL::Init() {
  context = GrContext::Create(kOpenGL_GrBackend, 0);
  GrBackendRenderTargetDesc targetDesc;
  targetDesc.fWidth = Window::pixel_width;
  targetDesc.fHeight = Window::pixel_height;
  targetDesc.fConfig = kRGBA_8888_GrPixelConfig;
  targetDesc.fRenderTargetHandle = 0;
  targetDesc.fStencilBits = 8;
  targetDesc.fSampleCnt = 0;
  SkAutoTUnref<GrRenderTarget> target(
      context->wrapBackendRenderTarget(targetDesc));
  device = SkGpuDevice::Create(target.get());
  canvas = new SkCanvas(device);
  canvas->translate(Window::pixel_width/2, Window::pixel_height/2);
  canvas->scale(Screen::meters_to_pixels, -Screen::meters_to_pixels);
  matrix = canvas->getTotalMatrix();
  icons.close = LoadSVG("close");
}
}
