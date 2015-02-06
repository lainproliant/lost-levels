/*
 * timer_sdl2: SDL2 timer specialization.
 *
 * Author: Lain Supe (lainproliant)
 * Date: Friday, February 6th 2015
 *
 * Based on Timer code written in 2011 by Lee Supe.
 */
#pragma once
#include <SDL2/SDL.h>
#include "lost_levels/timer.h"

namespace lost_levels {
   namespace sdl2 {
      using namespace std;

      inline shared_ptr<Timer<uint32_t>> create_timer(uint32_t interval, bool accumulate = false) {
         return Timer<uint32_t>::create(SDL_GetTicks, interval, accumulate);
      }
   }
}
