// Copyright 2014 Marek Rogalski <marek@mrogalski.eu>

#include <SDL.h>
#include "./Touch.h"
#include "./Display.h"
#include "./Utils.h"
#include "./Core.h"

namespace constructor {
TouchSource TouchSource::FromMouse(const SDL_MouseButtonEvent& evt) {
  TouchSource ts;
  ts.is_mouse = true;
  ts.mouse.mouse_id = evt.which;
  return ts;
}

TouchSource TouchSource::FromMouse(const SDL_MouseMotionEvent& evt) {
  TouchSource ts;
  ts.is_mouse = true;
  ts.mouse.mouse_id = evt.which;
  return ts;
}

TouchSource TouchSource::FromFinger(const SDL_TouchFingerEvent& evt) {
  TouchSource ts;
  ts.is_mouse = false;
  ts.finger.device_id = evt.touchId;
  ts.finger.finger_id = evt.fingerId;
  return ts;
}


bool TouchSource::operator ==(const TouchSource& other) const {
  if (is_mouse != other.is_mouse) return false;
  if (is_mouse) {
    return mouse.mouse_id == other.mouse.mouse_id;
  } else {
    return finger.device_id == other.finger.device_id &&
        finger.finger_id == other.finger.finger_id;
  }
}

template<class T> dvec2 ExtractPosition(T&& event) {
  dvec2 position = { static_cast<double>(event.x),
                     static_cast<double>(event.y) };
  Window::windowPixelsToWindowCoords(&position);
  return position;
}

template<> dvec2 ExtractPosition(const SDL_TouchFingerEvent& event) {
  dvec2 position = { static_cast<double>(event.x),
                     static_cast<double>(event.y) };
  Window::windowFactorToWindowCoords(&position);
  return position;
}

TouchManager TouchManager::instance;

void TouchManager::HandleMouseButtonDown(const SDL_MouseButtonEvent& event) {
  if (event.which == SDL_TOUCH_MOUSEID) return;
  if (event.button != SDL_BUTTON_LEFT) return;
  //Log("Mouse down!");
  BeginTouch(TouchSource::FromMouse(event), ExtractPosition(event));
}

void TouchManager::HandleMouseButtonUp(const SDL_MouseButtonEvent& event) {
  if (event.which == SDL_TOUCH_MOUSEID) return;
  if (event.button != SDL_BUTTON_LEFT) return;
  //Log("Mouse up!");
  EndTouch(TouchSource::FromMouse(event));
}

void TouchManager::HandleMouseMotion(const SDL_MouseMotionEvent& event) {
  if (event.which == SDL_TOUCH_MOUSEID) return;
  if ((event.state & SDL_BUTTON(SDL_BUTTON_LEFT)) == 0) return;
  //Log("Mouse is moving %d!\n", event.state);
  auto touch_source = TouchSource::FromMouse(event);
  auto position = ExtractPosition(event);
  MoveTouch(touch_source, position);
}

void TouchManager::HandleFingerDown(const SDL_TouchFingerEvent& event) {
  //Log("Finger down!");
  BeginTouch(TouchSource::FromFinger(event), ExtractPosition(event));
}

void TouchManager::HandleFingerUp(const SDL_TouchFingerEvent& event) {
  //Log("Finger up!");
  EndTouch(TouchSource::FromFinger(event));
}

void TouchManager::HandleFingerMotion(const SDL_TouchFingerEvent& event) {
  //Log("Finger is moving!");
  MoveTouch(TouchSource::FromFinger(event), ExtractPosition(event));
}

void TouchManager::ReplaceHandler(TouchHandler* old,
                                  TouchHandler* replacement) {
  for (auto&& entry : touches_) {
    if (entry.second.handler == old) {
      entry.second.handler = replacement;
    }
  }
}

void TouchManager::BeginTouch(const TouchSource& source,
                              const dvec2& position) {
  //Log("New touch at %.2lf mm, %.2lf mm\n",
  //    position.x * 1000, position.y * 1000);
  TouchSustainData& sustain_data = touches_[source] =
      {{source, position, position}, nullptr};
  sustain_data.handler = Core::instance.MakeGlobalMenu(
      sustain_data.touch);
}

void TouchManager::MoveTouch(const TouchSource& source,
                             const dvec2& position) {
  auto&& sustain_data = touches_[source];
  Touch& touch = sustain_data.touch;
  touch.last = touch.current;
  touch.current = position;
  if (sustain_data.handler != nullptr) {
    sustain_data.handler = sustain_data.handler->Move(touch);
  }
}

void TouchManager::EndTouch(const TouchSource& source) {
  auto&& sustain_data = touches_[source];
  if (sustain_data.handler != nullptr) {
    sustain_data.handler->End(sustain_data.touch);
  }
  touches_.erase(source);
}
}
