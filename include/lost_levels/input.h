/*
 * input: Interfaces for translating input from various
 *    various sources into events for the EventBus.
 *
 * Author: Lain Supe (lainproliant)
 * Date: Monday, Jan 26 2015
 */
#pragma once
#include "common.h"
#include "lost_levels/event.h"

namespace lost_levels {
   using namespace std;
   using namespace lain;

   /**
    * InputSource <<abstract class>>
    *
    * An interface for fetching input from a particular source
    * and feeding it into an EventBus.
    */
   class InputSource {
   public:
      InputSource() { }
      virtual ~InputSource() { }

      virtual void feed(EventBus& bus) = 0;
   };

   class NullInputSource : public InputSource {
      void feed(EventBus&) override { }
   };
}

