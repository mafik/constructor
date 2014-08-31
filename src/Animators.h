// Copyright 2014 Marek Rogalski <marek@mrogalski.eu>
#ifndef ANIMATORS_H_
#define ANIMATORS_H_

namespace constructor {
struct Wobbler {
  double value;
  double target;
  double attraction;
  // force = (target - value) * attraction
  double last_speed;
  double damping;
  // last_speed *= damping;
  // value += last_speed;
  void step() {
    const double force = (target - value) * attraction;
    last_speed = last_speed * damping + force;
    value += last_speed;
  }
};

struct Approacher {
  double value;
  double target;
  double speed;  // from 0 to 1
  void step() {
    value += (target - value) * speed;
  }
};
}

#endif  // ANIMATORS_H_
