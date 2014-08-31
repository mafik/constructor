// Copyright 2014 Marek Rogalski <marek@mrogalski.eu>

#include "./Display.h"
#include "./Touch.h"

using constructor::Screen;
using constructor::Window;
using glm::dvec2;

// 1cm in inches divided by DPI (unit: m/px)
double Screen::dpi = 118/*px/in*/;
double Screen::pixels_to_meters = 0.01/*m/cm*/ * 2.54/*cm/in*/ / dpi/*px/in*/;
double Screen::meters_to_pixels = 1 / Screen::pixels_to_meters;

void Window::windowPixelsToWindowCoords(dvec2* position) {
  position->x = (position->x - Window::pixel_width / 2) *
      Screen::pixels_to_meters;
  position->y = (Window::pixel_height / 2 - position->y) *
      Screen::pixels_to_meters;
}

void Window::windowFactorToWindowCoords(dvec2* position) {
  position->x = (position->x - 0.5) * Window::width;
  position->y = (0.5 - position->y) * Window::height;
}

int Window::pixel_height;
int Window::pixel_width;

double Window::height;
double Window::width;
