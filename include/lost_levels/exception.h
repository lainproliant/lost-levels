/*
 * Exception Types
 *
 * Author: Lain Supe (lainproliant)
 * Date: Tuesday, Jan 6 2014
 */
#pragma once
#include "lain/exception.h"

namespace lost_levels {
   class DataFormatException : public lain::Exception {
   public:
      DataFormatException(const std::string& message) :
         Exception(message) { }
   };
}
