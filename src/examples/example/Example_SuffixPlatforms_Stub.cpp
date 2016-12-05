// Copyright (c) 2014 - 2016 Roger Hanna
// For conditions of distribution and use, see copyright notice in ~/src/zmake/_zmake.h

#include "example/_example.h"

void suffixPlatformsExample()
{
   WTON_PRINT(__FILE__, "(", __LINE__, "):\t is compiled and linked on all platforms not in the set { Win32 } (Osx)");
}
