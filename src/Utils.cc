// Copyright 2014 Marek Rogalski <marek@mrogalski.eu>
#include <GLES2/gl2.h>
#include <SDL.h>
#include <unordered_map>
#include "./Utils.h"

namespace constructor {
using std::unordered_map;

unordered_map<int, const char*> gl_errors {
  { GL_NO_ERROR, "GL_NO_ERROR: No error has been recorded." },
  { GL_INVALID_ENUM, "GL_INVALID_ENUM: An unacceptable value is specified for "
        "an enumerated argument." },
  { GL_INVALID_VALUE, "GL_INVALID_VALUE: A numeric argument is out of range." },
  { GL_INVALID_OPERATION, "GL_INVALID_OPERATION: The specified operation is "
        "not allowed in the current state." },
  { GL_INVALID_FRAMEBUFFER_OPERATION, "GL_INVALID_FRAMEBUFFER_OPERATION: The "
        "command is trying to render to or read from the framebuffer while "
        "the currently bound framebuffer is not framebuffer complete (i.e. the "
        "return value from glCheckFramebufferStatus is not "
        "GL_FRAMEBUFFER_COMPLETE)." },
  { GL_OUT_OF_MEMORY, "GL_OUT_OF_MEMORY: There is not enough memory left to "
        "execute the command." }
};

bool CheckGLError(const char* op, int line) {
#ifndef NDEBUG
  int error;
  if ((error = glGetError()) != GL_NO_ERROR) {
    Log("GL error (%s): %s\n", op, gl_errors[error]);
    if (line != -1)
      Log(" + line: %i\n", line);
    return true;
  }
#endif
  return false;
}

bool CheckSDLError(const char* op, const int line) {
#ifndef NDEBUG
  const char *error = SDL_GetError();
  if (*error != '\0') {
    Log("SDL error (%s): %s\n", op, error);
    if (line != -1)
      Log(" + line: %i\n", line);
    SDL_ClearError();
    return true;
  }
#endif
  return false;
}

void Quit(const char *msg, char return_code) {
  Log("Quitting: %s\n", msg);
  SDL_Quit();
  exit(return_code);
}
}
