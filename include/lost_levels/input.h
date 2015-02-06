/*
 * input: Interfaces for translating input from various
 *    various sources into events for the EventBus.
 *
 * Author: Lain Supe (lainproliant)
 * Date: Monday, Jan 26 2015
 */
#pragma once
#include "lost_levels/event.h"

namespace lost_levels {
   using namespace std;

   /**
    * InputProvider <<abstract class>>
    *
    * An interface for fetching input from a particular source
    * and feeding it into an EventBus.
    */
   class InputProvider {
   public:
      InputProvider() { }
      virtual ~InputProvider() { }

      virtual void channel(EventBus& bus) = 0;
   };
}

