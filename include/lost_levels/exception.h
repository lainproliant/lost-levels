/*
 * Exception Types
 *
 * Author: Lain Supe (lainproliant)
 * Date: Tuesday, Jan 6 2014
 */
#pragma once
#include "common.h"

namespace lost_levels {
   class DataFormatException : public Exception {
   public:
      DataFormatException(const string& message) :
         Exception(message) { }
   };
}
