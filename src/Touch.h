// Copyright 2014 Marek Rogalski <marek@mrogalski.eu>
#ifndef TOUCH_H_
#define TOUCH_H_

#include <SDL.h>
#include <glm/vec2.hpp>
#include <unordered_map>

namespace constructor {
  struct TouchSource {
    bool is_mouse;
    union {
      struct {
        unsigned mouse_id;
      } mouse;
      struct {
        SDL_TouchID device_id;
        SDL_FingerID finger_id;
      } finger;
    };
    static TouchSource FromMouse(const SDL_MouseButtonEvent& evt);
    static TouchSource FromMouse(const SDL_MouseMotionEvent& evt);
    static TouchSource FromFinger(const SDL_TouchFingerEvent& evt);
    bool operator ==(const TouchSource& other) const;
  };
}

namespace std {
  using constructor::TouchSource;
  template <> struct hash<TouchSource> {
    size_t operator()(const TouchSource & x) const {
      if (x.is_mouse) {
        return (1 << 31) ^ x.mouse.mouse_id;
      } else {
        return x.finger.device_id ^ x.finger.finger_id;
      }
    }
  };
}

namespace constructor {
  using std::unordered_map;
  using glm::dvec2;

  struct Touch {
    TouchSource source;
    dvec2 last;
    dvec2 current;
  };

  class TouchHandler {
 public:
    // Called when the touch coords change.
    virtual TouchHandler* Move(const Touch&) = 0;
    virtual void End(const Touch&) = 0;
    // Called when the touch ends.
    virtual ~TouchHandler() {}
  };

  struct TouchSustainData {
    Touch touch;
    TouchHandler* handler;
  };

  class TouchManager {
    unordered_map<TouchSource, TouchSustainData> touches_;
 public:
    static TouchManager instance;

    void BeginTouch(const TouchSource& source, const dvec2& position);
    void MoveTouch(const TouchSource& source, const dvec2& position);
    void EndTouch(const TouchSource& source);

    void ReplaceHandler(TouchHandler* old, TouchHandler* replacement);

    void HandleMouseButtonDown(const SDL_MouseButtonEvent& event);
    void HandleMouseButtonUp(const SDL_MouseButtonEvent& event);
    void HandleMouseMotion(const SDL_MouseMotionEvent& event);
    void HandleFingerDown(const SDL_TouchFingerEvent& event);
    void HandleFingerUp(const SDL_TouchFingerEvent& event);
    void HandleFingerMotion(const SDL_TouchFingerEvent& event);
  };
}

#endif  // TOUCH_H_
