// Copyright (c) 2014 - 2016 Roger Hanna
// For conditions of distribution and use, see copyright notice in _wton.h

#define _HAS_EXCEPTIONS 0

#include <string>
#include <vector>
#include <stdlib.h>

namespace WTON
{
   typedef std::string                    WtonName;
   typedef std::string                    WtonString;

   template <typename T>
   struct WtonArray : public std::vector<T>
   {
   };

   const int kMaxStrCatLen = 100000;
   // strCat internal working memory size in bytes
   const int kStrCatBufBytes = 12 * kMaxStrCatLen;
};

namespace WTON
{
   template< typename T >
   struct Trait
   {
   };

   template<>
   struct Trait< int >
   {
      inline static const char* str()
      {
         return "num";
      }
   };

   template<>
   struct Trait< WTON::WtonString >
   {
      inline static const char* str()
      {
         return "str";
      }
   };

   template<>
   struct Trait< bool >
   {
      inline static const char* str()
      {
         return "num";
      }
   };
}

#define WTON_ASSERT(test, ...)               (test ? 0 : WTON::debugOut(WTON::strCat("ASSERT(", #test, ");   ", WTON::strCat(__VA_ARGS__), "\n"), true))
#define WTON_WARN(...)                       WTON::debugOut(WTON::strCat(__VA_ARGS__, "\n"), false)
#define WTON_PRINT(...)                      WTON::debugOut(WTON::strCat(__VA_ARGS__, "\n"), false)
#define WTON_FAIL(...)                       WTON::debugOut(WTON::strCat(__VA_ARGS__, "\n"), true)

#define WTON_MALLOC                          malloc
#define WTON_FREE                            free

#define WTON_TYPE(iType)                     WTON::Trait<iType>::str()
#define WTON_OFFSET_POINTER(iType, iMember)  &(((iType*)0)->iMember)

#ifdef _WIN32
# define WTON_PC 1
# define WTON_SNPRINTF(buf, bufLen, ...)     sprintf_s(buf, bufLen, __VA_ARGS__)
#else
# define WTON_SNPRINTF(buf, bufLen, ...)     sprintf(buf, __VA_ARGS__)
#endif

#define WTON_LINE_BREAK "\n"
