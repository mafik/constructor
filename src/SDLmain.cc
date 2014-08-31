// Copyright 2014 Marek Rogalski <marek@mrogalski.eu>
#include <SDL.h>
#include <GLES2/gl2.h>
#include <unordered_map>
#include <cstdlib>
#include "./Utils.h"
#include "./Storage.h"
#include "./Core.h"
#include "./Touch.h"
#include "./Display.h"

using std::unordered_map;
using constructor::Window;
using constructor::Core;
using constructor::TouchManager;
using constructor::CheckSDLError;
using constructor::Quit;

unordered_map<int, const char*> event_types {
  {SDL_FIRSTEVENT, "SDL_FIRSTEVENT: do not remove (unused)"},
  {SDL_QUIT, "SDL_QUIT: user-requested quit; see Remarks for details"},
  {SDL_APP_TERMINATING, "SDL_APP_TERMINATING: OS is terminating the "
        "application"},
  {SDL_APP_LOWMEMORY, "SDL_APP_LOWMEMORY: OS is low on memory; free some"},
  {SDL_APP_WILLENTERBACKGROUND, "SDL_APP_WILLENTERBACKGROUND: application "
        "is entering background"},
  {SDL_APP_DIDENTERBACKGROUND, "SDL_APP_DIDENTERBACKGROUND: application "
        "entered background"},
  {SDL_APP_WILLENTERFOREGROUND, "SDL_APP_WILLENTERFOREGROUND: application "
        "is entering foreground"},
  {SDL_APP_DIDENTERFOREGROUND, "SDL_APP_DIDENTERFOREGROUND: application "
        "entered foreground"},
  {SDL_WINDOWEVENT, "SDL_WINDOWEVENT: window state change"},
  {SDL_SYSWMEVENT, "SDL_SYSWMEVENT: system specific event"},
  {SDL_KEYDOWN, "SDL_KEYDOWN: key pressed"},
  {SDL_KEYUP, "SDL_KEYUP: key released"},
  {SDL_TEXTEDITING, "SDL_TEXTEDITING: keyboard text editing (composition)"},
  {SDL_TEXTINPUT, "SDL_TEXTINPUT: keyboard text input"},
  {SDL_MOUSEMOTION, "SDL_MOUSEMOTION: mouse moved"},
  {SDL_MOUSEBUTTONDOWN, "SDL_MOUSEBUTTONDOWN: mouse button pressed"},
  {SDL_MOUSEBUTTONUP, "SDL_MOUSEBUTTONUP: mouse button released"},
  {SDL_MOUSEWHEEL, "SDL_MOUSEWHEEL: mouse wheel motion"},
  {SDL_JOYAXISMOTION, "SDL_JOYAXISMOTION: joystick axis motion"},
  {SDL_JOYBALLMOTION, "SDL_JOYBALLMOTION: joystick trackball motion"},
  {SDL_JOYHATMOTION, "SDL_JOYHATMOTION: joystick hat position change"},
  {SDL_JOYBUTTONDOWN, "SDL_JOYBUTTONDOWN: joystick button pressed"},
  {SDL_JOYBUTTONUP, "SDL_JOYBUTTONUP: joystick button released"},
  {SDL_JOYDEVICEADDED, "SDL_JOYDEVICEADDED: joystick connected"},
  {SDL_JOYDEVICEREMOVED, "SDL_JOYDEVICEREMOVED: joystick disconnected"},
  {SDL_CONTROLLERAXISMOTION, "SDL_CONTROLLERAXISMOTION: controller axis "
        "motion"},
  {SDL_CONTROLLERBUTTONDOWN, "SDL_CONTROLLERBUTTONDOWN: controller button "
        "pressed"},
  {SDL_CONTROLLERBUTTONUP, "SDL_CONTROLLERBUTTONUP: controller button "
        "released"},
  {SDL_CONTROLLERDEVICEADDED, "SDL_CONTROLLERDEVICEADDED: controller "
        "connected"},
  {SDL_CONTROLLERDEVICEREMOVED, "SDL_CONTROLLERDEVICEREMOVED: controller "
        "disconnected"},
  {SDL_CONTROLLERDEVICEREMAPPED, "SDL_CONTROLLERDEVICEREMAPPED: controller "
        "mapping updated"},
  {SDL_FINGERDOWN, "SDL_FINGERDOWN: user has touched input device"},
  {SDL_FINGERUP, "SDL_FINGERUP: user stopped touching input device"},
  {SDL_FINGERMOTION, "SDL_FINGERMOTION: user is dragging finger on input "
        "device"},
  {SDL_DOLLARGESTURE, "SDL_DOLLARGESTURE: ?"},
  {SDL_DOLLARRECORD, "SDL_DOLLARRECORD: ?"},
  {SDL_MULTIGESTURE, "SDL_MULTIGESTURE: ?"},
  {SDL_CLIPBOARDUPDATE, "SDL_CLIPBOARDUPDATE: the clipboard changed"},
  {SDL_DROPFILE, "SDL_DROPFILE: the system requests a file open"},
  {SDL_RENDER_TARGETS_RESET, "SDL_RENDER_TARGETS_RESET: the render targets "
        "have been reset and their contents need to be updated (>= SDL 2.0.2)"},
      // {SDL_RENDER_DEVICE_RESET, "SDL_RENDER_DEVICE_RESET: the device has "
      // "been reset and all textures need to be recreated (>= SDL 2.0.4)"},
  {SDL_USEREVENT, "SDL_USEREVENT: a user-specified event"},
  {SDL_LASTEVENT, "SDL_LASTEVENT: only for bounding internal arrays"}
};

SDL_Window* createWindow();

bool handleEvent(const SDL_Event& event) {
  switch (event.type) {
    case SDL_MOUSEBUTTONUP:
      TouchManager::instance.HandleMouseButtonUp(event.button);
      break;
    case SDL_MOUSEBUTTONDOWN:
      TouchManager::instance.HandleMouseButtonDown(event.button);
      break;
    case SDL_MOUSEMOTION:
      TouchManager::instance.HandleMouseMotion(event.motion);
      break;
    case SDL_FINGERMOTION:
      TouchManager::instance.HandleFingerMotion(event.tfinger);
      break;
    case SDL_FINGERDOWN:
      TouchManager::instance.HandleFingerDown(event.tfinger);
      break;
    case SDL_FINGERUP:
      TouchManager::instance.HandleFingerUp(event.tfinger);
      break;
    case SDL_QUIT:
      Log("Received SDL_QUIT event: finishing!\n");
      return false;
    default:
      Log("%s\n", event_types[event.type]);
      break;
  }
  return true;
}

int EventFilter(void*, SDL_Event* event) {
  switch (event->type) {
  case SDL_MULTIGESTURE:
    return 0;
  }
  return 1;
}

struct A {
  int a;
  int b;
};

int main(int argc, char *argv[]) {
  SDL_Window *window = createWindow();
  SDL_GetWindowSize(window, &Window::pixel_width, &Window::pixel_height);
  Window::width = Window::pixel_width * Screen::pixels_to_meters;
  Window::height = Window::pixel_height * Screen::pixels_to_meters;
  SDL_GLContext ctx = SDL_GL_CreateContext(window);
  SDL_GL_SetSwapInterval(1);
  SDL_SetEventFilter(EventFilter, nullptr);
  Log("Window with size %d x %d\n",
      Window::pixel_width, Window::pixel_height);
  Core::instance.Init();
  Core::instance.GLInit();
  bool keepRunning = true;
  while (keepRunning) {
    SDL_Event event;
    while (keepRunning && SDL_PollEvent(&event)) {
      keepRunning = handleEvent(event);
    }
    constructor::Core::instance.GLDraw();
    SDL_GL_SwapWindow(window);
    CheckSDLError("Frame ended", __LINE__);
  }
  Log("Event Loop done\n");
  SDL_GL_DeleteContext(ctx);
  SDL_DestroyWindow(window);
  Quit("Bye bye!", 0);
  return 0;
}


SDL_Window* createWindow() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    Quit("Unable to initialize SDL");

  /*
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  */
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

  auto w = SDL_CreateWindow("Constructor",
      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      512, 512, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

  if (!w) /* Die if creation failed */
    Quit("Unable to create window");

  CheckSDLError("Creating window", __LINE__);

  return w;
}
