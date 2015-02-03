/*
 * Common includes which are used by most source files.
 *
 * TODO: Eliminate this file.  This exists for convenience only,
 *    including all of these headers in every compilation unit
 *    will increase build times significantly.  Figure out which
 *    headers each compilation unit actually needs, easy enough.
 *
 * Author: Lain Supe (lainproliant)
 * Date: Thursday, January 1st 2015
 */
#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <queue>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>
#include <cfloat>
#include <cmath>

#include <SDL2/SDL.h>

#include "lain/exception.h"
#include "lain/string.h"
#include "lain/getopt.h"
#include "tinyformat/tinyformat.h"
#include "picojson/picojson.h"

