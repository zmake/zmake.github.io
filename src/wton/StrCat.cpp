// Copyright (c) 2014 - 2016 Roger Hanna
// For conditions of distribution and use, see copyright notice in _wton.h

#include "wton/_wton.h"

#ifdef WTON_PC
# include <windows.h>
#else
# include <cxxabi.h>
#endif

// strCat failures can't rely on strCat
#define WTON_STATIC_ASSERT(test)          (test ? 0 : WTON::debugOut("ASSERT(" #test ");   \n", true))

#ifdef WTON_DEBUG
# ifdef WTON_PC
#  define WTON_DEBUG_BREAK __debugbreak()
# else
#  define WTON_DEBUG_BREAK __asm__ volatile("int $0x03")
# endif
#else
# define WTON_DEBUG_BREAK
#endif

namespace WTON
{
   int sStrBufPos = 0;
   int sStrBufMid = false;
   char sBufs[kStrCatBufBytes];

   char* _strCpy(char* dst, const char* src, const char* end)
   {
      WTON_STATIC_ASSERT(src < dst || src >= end);
      while (dst <= end && (*dst++ = *src++)){}
      return dst - 1;
   }

   char* strBuf()
   {
      WTON_STATIC_ASSERT(!sStrBufMid);
      sStrBufMid = true;
      return &sBufs[sStrBufPos];
   }

   void finishStrBuf(char* buf)
   {
      WTON_STATIC_ASSERT(sStrBufMid);
      sStrBufMid = false;

      WTON_STATIC_ASSERT(buf == &sBufs[sStrBufPos]);
      int len = strlen(buf) + 1;

      // if this gets hit, we may have overflowed
      WTON_STATIC_ASSERT(len < kMaxStrCatLen);

      // alignment?
      sStrBufPos += len;
      if (sStrBufPos > kStrCatBufBytes - kMaxStrCatLen)
      {
         sStrBufPos = 0;
      }
   }

   int debugOut(const char* msg, bool fail)
   {
      printf("%s", msg);
#ifdef WTON_PC
      OutputDebugString(msg);
#endif
      if (fail)
      {
         // we can't exit(1) because Visual Studio fails to emit the error message if we do
         // TODO: will makefiles stop making?
         WTON_DEBUG_BREAK;
         exit(0);
      }
      return 1;
   }

   const char* toString(const WtonString& s)
   {
      return s.c_str();
   }
   const char* toString(const char* s)
   {
      return s;
   }

   const char* toString(const int& i)
   {
      char* buf = strBuf();
      WTON_SNPRINTF(buf, kMaxStrCatLen, "%d", i);
      finishStrBuf(buf);

      return buf;
   }

   const char* strCat()
   {
      return "";
   }

   void strReplace(WtonString& s, const char* src, const char* dst)
   {
      size_t n1 = strlen(src);
      size_t n2 = strlen(dst);
      size_t pos = 0;
      while (true)
      {
         pos = s.find(src, pos);

         if (pos != WtonString::npos)
         {
            s.replace(pos, n1, dst, n2);
            pos += n2;
         }
         else
            break;
      }
   }

   WtonName demangleType(const char* type)
   {
#ifdef WTON_PC
      WtonString t(type);

      strReplace(t, "struct ", "");
      strReplace(t, "enum ", "");
      strReplace(t, "__cdecl*", "*");

      strReplace(t, "__int64", "int64");
#else
      int status;
      WtonString t(abi::__cxa_demangle(type, NULL, NULL, &status));

      strReplace(t, "long long", "int64");
#endif

      // const doesn't affect WTON's interpretation of type
      strReplace(t, " const", "");

      strReplace(t, "unsigned int",                 "uint");
      strReplace(t, "unsigned char",                "uint8");
      strReplace(t, "signed char",                  "int8");

      // kill any remaining spaces, e.g. MSVC: "Object *", gcc: "Map<A, B>", both: "void (*)()"
      strReplace(t, " ", "");

      return WtonName(t);
   }
};
