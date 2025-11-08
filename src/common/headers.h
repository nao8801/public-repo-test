// Platform-independent headers
// This file provides common headers for cross-platform builds
//
// For Windows builds, use src/win32/headers.h instead

#pragma once

// Standard C++ headers
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <stdint.h>

// STL headers
#include <map>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>

// Platform-specific includes
#ifdef _WIN32
    // Windows platform
    #include "../win32/headers.h"
#else
    // Unix-like platforms (Linux, macOS, etc.)
    #include "types.h"

    using namespace std;

    // Define min/max if not defined
    #ifndef max
    #define max(a,b) (((a) > (b)) ? (a) : (b))
    #endif
    #ifndef min
    #define min(a,b) (((a) < (b)) ? (a) : (b))
    #endif
#endif
