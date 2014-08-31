// Copyright 2014 Marek Rogalski <marek@mrogalski.eu>

#ifndef CORE_H_
#define CORE_H_

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <deque>
#include <functional>

#include "./Connection.h"
#include "./Object.h"
#include "./Touch.h"
#include "./Task.h"
#include "./Action.h"
#include "./RadialMenu.h"

using std::function;

namespace constructor {

  class Core {
    std::string dataDirectory_;
    std::vector<Object*> prototypes_;

    std::unordered_set<Object*> objects_;
    std::unordered_set<Connection*> connections_;

    std::deque<Task> tasks_;
    std::deque<Action> pastActions_;
    std::deque<Action> futureActions_;

    Core(const Core&);
    Core() = default;
    void operator=(Core const&) = delete;
    ~Core();

 public:
    static Core instance;

    double offset_x, offset_y;
    double scale;

    void Init();

    void QueryRange(double x, double y, double range,
                    function<void(Object*)> callback);

    RadialMenu* MakeGlobalMenu(const Touch& t);

    void GLInit();
    void GLDraw();
  };

  extern Core core;
}

#endif  // CORE_H_
