// Copyright 2014 Marek Rogalski <marek@mrogalski.eu>
#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <glm/vec2.hpp>

namespace constructor {
  using glm::dvec2;

  /*
    This class provides information about size of screen.
   */
  class Screen {
   public:
    static int pixel_width, pixel_height;  // given by the os
    static double diagonal_inches;  // given by the os (or guessed)

    /*
      User might adjust scaling factor to make objects appear smaller
      or larger - the width and height of the screen will increase to
      reflect that.

      If the screen has very low density (eg. 60 inch Full-HD
      display), the scaling factor will be adjusted such that 1
      virtual mm has at least 4 pixels (~101dpi).

      Initially computed from diagonal and pixel sceen size but might
      be adjustd by user;
     */
    static double dpi;
    static double pixels_to_meters;
    static double meters_to_pixels;  // inverse of the above
  };

  class Window {
   public:
    /*
      Hardware or window pixel coordinates will be scaled and expressed
      in meters so high-dpi and low-dpi displays will show content in
      the same size (but different quality).

      Try to keep all touchable elements above 8mm (0.008 m).
    */
    static int pixel_width, pixel_height;
    static double width, height;

    static void windowPixelsToWindowCoords(dvec2* position);
    static void windowFactorToWindowCoords(dvec2* position);

    /*
      Actually accesible area of the window might be smaller than the
      GL surface. These are the margins imposed by other windows in
      the OS.
     */
    static double touch_bottom_, touch_top_, touch_left_, touch_right_;
  };
}

#endif  // DISPLAY_H_
