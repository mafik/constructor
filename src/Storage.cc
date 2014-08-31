// Copyright 2014 Marek Rogalski <marek@mrogalski.eu>

#include <SDL.h>
#include <string>
#include "./Storage.h"
#include "./Utils.h"

namespace constructor {
  using std::string;
  string getStorageDirectory() {
#ifdef __ANDROID__
    const char* buffer = SDL_AndroidGetInternalStoragePath();
    auto s = string(buffer);
    return s;
#else
    char* buffer = SDL_GetPrefPath("Constructor", "Storage");
    string str(buffer);
    SDL_free(buffer);
    return str;
#endif
  }
}
