// Copyright 2014 Marek Rogalski <marek@mrogalski.eu>

#ifndef CONNECTION_H_
#define CONNECTION_H_

namespace constructor {
  class Object;

  enum {
    CONNCTION_FLAG_A_ARROW = 0,
    CONNCTION_FLAG_B_ARROW = 1,
  };

  class Connection {
   public:
    Object* a;
    Object* b;
    int type;
    int flags;
  };
}

#endif  // CONNECTION_H_
