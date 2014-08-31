// Copyright 2014 Marek Rogalski <marek@mrogalski.eu>
#ifndef UTILS_H_
#define UTILS_H_

#ifdef __ANDROID__
#include <android/log.h>
#define Log(...) __android_log_print(ANDROID_LOG_DEBUG, "Constructor", __VA_ARGS__)
#else // not ANDROID
#define Log(...) printf(__VA_ARGS__)
#endif

namespace constructor {
  bool CheckGLError(const char* operation, int line = -1);
  bool CheckSDLError(const char* operation, int line = -1);
  void Quit(const char *msg, char return_code = 1);
}

#endif  // UTILS_H_
