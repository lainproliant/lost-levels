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
    * FrameCalculator <concrete class>
    *
    * Calculate and report FPS metrics.
    *
    * USAGE:
    * - Construct the FrameCalculator with a timer that fires every second
    *   (the monitor_timer) and a timer to be monitored (the monitoring_timer).
    * - Call update() regularly to update the FPS metric.
    * - Call get_fps() to get the FPS metric.  If the FPS metric has not yet
    *   been calculated, the number of frames passed on the monitoring_timer
    *   will be returned.
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

