/*
 * diag: Diagnostic utilities.
 *
 * Author: Lain Supe (lainproliant)
 * Date: Thursday, May 14 2015
 */
#pragma once

#include "timer.h"

namespace lost_levels {
   using namespace std;

   /**
    * Calculate and report FPS metrics.
    */
   template<class T>
   class FrameCalculator {
   public:
      FrameCalculator(shared_ptr<Timer<T>> monitor_timer,
                      shared_ptr<const Timer<T>> monitoring_timer) :
         monitor_timer(monitor_timer), monitoring_timer(monitoring_timer) {
         monitor_timer->start();
      }

      void update() {
         if (monitor_timer->update()) {
            T frames = monitoring_timer->get_frames();
            fps = frames - prev_frames;
            prev_frames = frames;
         }
      }

      T get_fps() const {
         if (fps == 0) {
            return monitoring_timer->get_frames();

         } else {
            return fps;
         }
      }

   private:
      shared_ptr<Timer<T>> monitor_timer;
      shared_ptr<const Timer<T>> monitoring_timer;

      T fps = 0;
      T prev_frames = 0;
   };
}

