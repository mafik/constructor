// Copyright 2014 Marek Rogalski <marek@mrogalski.eu>

#ifndef _TASK_H_
#define _TASK_H_

#include <memory>

namespace constructor {
  class Object;

  struct Task {
    Object* target;
    std::unique_ptr<Object> argument;
    Object* notify;
  };
}

#endif /* _TASK_H_ */
