/*
 * Timer: A timer and relative timer wrapper for SDL_GetTime.
 *
 * Author: Lain Supe (lainproliant)
 * Date: Sunday Jan 25 2014
 *
 * Based on Timer code written in 2011 by Lee Supe.
 */
#pragma once
#include "common.h"

namespace lost_levels {
   using namespace std;

   template<class T>
   class RelativeTimer;

   template <class T>
   class Timer : public enable_shared_from_this<Timer<T>> {
   public:
      typedef function<T(void)> TimeFunction;
      static shared_ptr<Timer<T>> create(TimeFunction getTime, Uint32 interval,
            bool accumulate = false) {
         return shared_ptr<Timer<T>>(new Timer<T>(getTime, interval, accumulate));
      }

      virtual ~Timer() { }

      void start() {
         T tnow = getTime();
         T tdiff = t1 - t0;

         t0 = tnow;
         t1 = tnow + tdiff;
         t2 = t0 + interval;
         paused = false;
      }

      void pause() {
         paused = true;
      }

      bool update(T* pterr = nullptr) {
         if (paused || interval == 0) {
            return false;
         }

         T tnow = getTime();
         if (tnow < t0) {
            // The timer function has wrapped.
            T tdiff = t1 - t0;
            t0 = tnow;
            t1 = tnow + tdiff;
            t2 = t0 + interval;
            return update();
         }

         t1 = tnow;
         if (t1 >= t2) {
            T terr = t1 - t2;

            t0 = tnow;
            t1 = t0;
            frames ++;

            if (accumulate) {
               // Subtract a frame from the accumulator.
               if (tacc > interval) {
                  tacc -= interval;
               } else {
                  tacc = 0;
               }

               tacc += terr;

               if (tacc > interval) {
                  t2 = t1;

               } else {
                  t2 = t0 + (interval - tacc);
               }

            } else {
               t2 = t0 + interval;
            }

            if (pterr != nullptr) {
               *pterr = terr;
            }

            return true;

         } else {
            return false;
         }
      }

      void reset() {
         t0 = getTime();
         t1 = t0;
         t2 = t0 + interval;
         frames = 0;
      }

      T get_frames() const {
         return frames;
      }

      T get_time() const {
         return t1;
      }

      T get_wait_time() const {
         T tnow = getTime();

         if (tnow < t0 || tnow >= t0 + interval) {
            return 0;

         } else {
            return t0 + interval - tnow;
         }
      }

      Timer<T> relative_timer(T frameInterval) const {
         return RelativeTimer<T>(this->shared_from_this());
      }

   protected:
      Timer(TimeFunction getTime, T interval, bool accumulate = false) :
         getTime(getTime), interval(interval), accumulate(accumulate) {
         t0 = 0;
         t1 = 0;
         tacc = 0;
         frames = 0;
         paused = true;
      }

   private:
      bool paused, accumulate;
      T t0, t1, t2, tacc, interval, frames;
      TimeFunction getTime;
   };

   template<class T>
   class RelativeTimer : public Timer<T> {
   private:
      RelativeTimer(shared_ptr<Timer<T>> referenceTimer, T interval) :
         Timer<T>([referenceTimer]() -> T {
                     return referenceTimer->get_frames();
                  }, interval) { }
      virtual ~RelativeTimer() { }
   };

   inline shared_ptr<Timer<Uint32>> create_sdl_timer(Uint32 interval, bool accumulate = false) {
      return Timer<Uint32>::create(SDL_GetTicks, interval, accumulate);
   }
}
