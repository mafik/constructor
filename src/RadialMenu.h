// Copyright 2014 Marek Rogalski <marek@mrogalski.eu>
#ifndef RADIALMENU_H_
#define RADIALMENU_H_
#include <SkColor.h>
#include <GrTexture.h>
#include <glm/mat4x4.hpp>
#include <vector>
#include <string>
#include "./Animators.h"
#include "./Object.h"
#include "./Touch.h"

namespace constructor {
  using std::vector;
  using std::string;
  using glm::mat4;

  class Entry {
 public:
    GrTexture* texture;
    string name;
    string description;
    string video_url;
    // float priority;
    // float direction;

    static Entry* NewFromPrototype(const Object& o);
    ~Entry();
  };

  class RadialMenu : public TouchHandler {
    static vector<RadialMenu*> radial_menus_;

    static constexpr double kOuterRadius = 0.02;
    static constexpr double kInnerRadius = 0.005;
    static constexpr double kMargin = 0.001;

    bool destroying_;
    Entry* active_entry_;
    vector<Entry*> entries_;
    Wobbler size_;
    dvec2 center_;
    dvec2 touch_;
    TouchHandler* next_handler_;
    SkColor color_;

   public:
    RadialMenu();
    virtual ~RadialMenu();
    virtual void Init(const Touch&, const SkColor color);
    virtual void AddEntry(Entry* entry);
    virtual void Scroll();
    virtual void CheckActivation();
    virtual Entry* FindActiveEntry(const dvec2& position);
    virtual Entry* EntryAtAlpha(const double alpha);
    virtual TouchHandler* Move(const Touch&) override;
    virtual TouchHandler* Finish(bool dragging);
    virtual void End(const Touch&) override;
    virtual void Draw();
    static void DrawMenus();
  };
}

#endif  // RADIALMENU_H_
