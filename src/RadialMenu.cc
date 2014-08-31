// Copyright 2014 Marek Rogalski <marek@mrogalski.eu>
#include "./RadialMenu.h"
#include <GLES2/gl2.h>
#include <glm/mat4x4.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <utility>
#include <algorithm>
#include "./Utils.h"
#include "./Display.h"
#include "./Touch.h"
#include "./Core.h"

namespace constructor {
using glm::mat4;
using glm::vec3;

Entry* Entry::NewFromPrototype(const Object& o) {
  auto e = new Entry();
  //e->texture.LoadIcon(o);
  e->name = o.GetName();
  e->description = o.GetDescription();
  e->video_url = o.GetVideoURL();
  return e;
}

void RadialMenu::Init(const Touch& t, const SkColor color) {
  center_ = t.current;
  color_ = color;
  touch_ = t.current;
  active_entry_ = nullptr;
  next_handler_ = this;
  destroying_ = false;
  radial_menus_.push_back(this);
  size_ = { 0, 1, 0.3, 0, 0.7 };
}

RadialMenu::RadialMenu() {}

RadialMenu::~RadialMenu() {
  TouchManager::instance.ReplaceHandler(this, nullptr);
  auto it = std::find(radial_menus_.begin(), radial_menus_.end(), this);
  radial_menus_.erase(it);
}

void RadialMenu::AddEntry(Entry* entry) {
  entries_.push_back(entry);
}

TouchHandler* RadialMenu::Move(const Touch& t) {
  if (destroying_) {
    return next_handler_;  // not this
  }
  touch_ = t.current;
  CheckActivation();
  return next_handler_;
}

void RadialMenu::End(const Touch&) {
  Finish(false);
}

void RadialMenu::Scroll() {
  if (destroying_) return;
  const double kScrollMargin = kOuterRadius / 2;
  const double kScrollSpeed = 0.05f;
  dvec2 v;
  if (touch_.x + Window::width / 2 < kScrollMargin) {
    v.x -= (kScrollMargin - touch_.x - Window::width / 2) * kScrollSpeed;
  }
  if (touch_.x > Window::width / 2 - kScrollMargin) {
    v.x += (touch_.x - Window::width / 2 + kScrollMargin) * kScrollSpeed;
  }
  if (touch_.y + Window::height / 2 < kScrollMargin) {
    v.y -= (kScrollMargin - touch_.y - Window::height / 2) * kScrollSpeed;
  }
  if (touch_.y > Window::height / 2 - kScrollMargin) {
    v.y += (touch_.y - Window::height / 2 + kScrollMargin) * kScrollSpeed;
  }

  if (nullptr == active_entry_) {
    const dvec2 delta = touch_ - center_;
    const double r = glm::length(delta);
    if (r < kInnerRadius) {
      v += (- glm::sin(r / kInnerRadius * glm::pi<double>()) * delta / 40.);
    }
  }
  center_ -= v;
}

void draw_pie(float x, float y, float a, float b, float near, float far,
              float margin, const SkPaint& paint) {
  static SkPath path;
  float deltaNear = glm::atan(margin / 2, near) / M_PI * 180,
        deltaFar = glm::atan(margin / 2, far) / M_PI * 180;
  path.rewind();
  path.arcTo({x - near, y - near, x + near, y + near},
             a + deltaNear, b - a - deltaNear * 2, true);
  path.arcTo({x - far, y - far, x + far, y + far},
             b - deltaFar, a - b + deltaFar * 2, false);
  path.close();
  GL::instance.canvas->drawPath(path, paint);
}

void RadialMenu::Draw() {
  Scroll();
  CheckActivation();

  const double last_size = size_.value;
  size_.step();
  if (size_.value <= 0) {
    if (destroying_)
      delete this;
    return;
  }

  /*
  for(const Entry current : entries) {
    Entry prev = entries.lower(current);
    if(prev == null) prev = entries.last();
    Entry next = entries.higher(current);
    if(next == null) next = entries.first();

    double leftAngle = angleBetween(prev.getDirection(), current.getDirection());
    double rightAngle = angleBetween(current.getDirection(), next.getDirection());
    if(rightAngle < leftAngle) {
      rightAngle += Math.PI * 2;
    }

    const int entryColor = current.isEnabled() ?
        (current == activeEntry ? current.getActiveColor(activeColor) :
         current.getInactiveColor(inactiveColor)) : disabledColor;

    pie.draw(tempMatrix, leftAngle, rightAngle, inner_radius, outer_radius, PIE_MARGIN,
             abs(speedOfRadius) * 2, entryColor);
    current.draw(tempMatrix, this, leftAngle, rightAngle, inner_radius, currentRadius);

  }
  */

  SkPaint paint;
  paint.setAntiAlias(true);
  paint.setColor(color_);
  draw_pie(center_.x, center_.y, 0, 90, size_.value * (kInnerRadius + kMargin),
           size_.value * kOuterRadius, kMargin, paint);

  // const int cancelColor = (null == activeEntry) ? activeColor : inactiveColor;
  GL::instance.canvas->drawCircle(center_.x, center_.y, size_.value * kInnerRadius, paint);

  GL::instance.canvas->save();
  GL::instance.canvas->translate(center_.x, center_.y);
  GL::instance.icons.close->draw(GL::instance.canvas);
  GL::instance.canvas->restore();
  //GL::instance.textures.close->Draw(centered);

  // cancelSprite.draw(tempMatrix);
}

void RadialMenu::CheckActivation() {
  if (destroying_) return;
  const dvec2 delta = touch_ - center_;
  Entry* new_active_entry = FindActiveEntry(delta);
  if (new_active_entry != active_entry_) {
    if (new_active_entry != nullptr) {
      // entryAnimators.get(new_active_entry).activate();
    }
    if (active_entry_ != nullptr) {
      // entryAnimators.get(activeEntry).deactivate();
    }
    active_entry_ = new_active_entry;
    // Sound.RADIAL_MENU_OPTION_TICK.play(mX);
  }

  const double r = glm::length(delta);
  if (r > kOuterRadius) {
    Log("Activating! x=%lf, y=%lf\n", delta.x, delta.y);
    next_handler_ = Finish(true);
  }
}

TouchHandler* RadialMenu::Finish(bool dragging) {
  if (destroying_) return next_handler_;
  destroying_ = true;
  size_.target = 0;
  return nullptr;
  /*
  if (active_entry_ == nullptr) {
    Sound.MENU_CLOSE.play(mX); // TODO: use screen coordinate
    return nullptr;
  } else if (dragging) {
    entryAnimators.get(activeEntry).deactivate();
    Entry active = active_entry_;
    if (active.isEnabled()) {
      active.getSound().play(mX);
      active.activated(radialLayer.masterView.guiToScreenX(lastX), radialLayer.masterView.guiToScreenY(lastY), true);
      return active;
    } else {
      return null;
    }
  } else {
    entryAnimators.get(activeEntry).deactivate();
    Sound.MENU_BAD_CLOSE.play(mX);
    return nullptr;
  }
  */
}


Entry* RadialMenu::EntryAtAlpha(const double alpha) {
  if (entries_.size() == 0) return nullptr;
  Entry* closestEntry = entries_.front();
  double closestDist = 1000;
  for (size_t i = 0; i < entries_.size(); i++) {
    auto e = entries_[i];
    const double dir = 2 * glm::pi<double>() * i / entries_.size();
    const double dist1 = glm::abs(dir - alpha);
    const double dist2 = glm::abs(dir - alpha - 2 * glm::pi<double>());
    const double dist3 = glm::abs(dir - alpha + 2 * glm::pi<double>());
    const double dist = glm::min(glm::min(dist1, dist2), dist3);
    if (dist < closestDist) {
      closestDist = dist;
      closestEntry = e;
    }
  }
  return closestEntry;
}


Entry* RadialMenu::FindActiveEntry(const dvec2& position) {
  const double r = glm::length(position);
  if (r < kInnerRadius) return nullptr;
  const double a = glm::atan(position.y, position.x);
  return EntryAtAlpha(a);
}

vector<RadialMenu*> RadialMenu::radial_menus_;

void RadialMenu::DrawMenus() {
  for (auto it : radial_menus_) {
    it->Draw();
  }
}
}
