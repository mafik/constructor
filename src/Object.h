// Copyright 2014 Marek Rogalski <marek@mrogalski.eu>

#ifndef OBJECT_H_
#define OBJECT_H_

#include <SDL.h>
#include <unordered_set>
#include <string>

#include "./GL.h"
#include "./Touch.h"
#include "./Connection.h"

using std::string;
using std::unordered_set;

namespace constructor {
  
  class Visible {
 public:
    
    virtual double GetRadius() const = 0;
    
    virtual void Init(GL* shared_GL_state) = 0;
    virtual void Draw(GL* shared_GL_state) = 0;
    virtual void Finish(GL* shared_GL_state) = 0;

    virtual TouchHandler* BeginTouch(const Touch&) = 0;
    
    virtual ~Visible();
  };

  enum DisplayState {
    VISIBLE,
    ICON,
    DOT
  };

  class Object {
 public:
    double x_;
    double y_;
    DisplayState display_state_;
    
    // connections
    unordered_set<Connection*> incoming_connections_;
    unordered_set<Connection*> outgoing_connections_;

    // rendering
    virtual Visible* MakeVisible();

    // message handling
    virtual void HandleMessage(const Object& message) = 0;

    // description
    virtual void DrawIcon(SDL_Surface* surface) const = 0;
    virtual string GetName() const = 0;
    virtual string GetDescription() const = 0;
    virtual string GetVideoURL() const = 0;

    // serialization
    explicit Object(const string& data);
    virtual ~Object();
    virtual void Save(string *data) const = 0;
    virtual Object* Clone() const = 0;
    virtual const char* GetTypeIdentifier() const = 0;
    
  };

}

#endif  // OBJECT_H_
