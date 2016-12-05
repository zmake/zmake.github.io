// Copyright (c) 2014 - 2016 Roger Hanna
// For conditions of distribution and use, see copyright notice in ~/src/zmake/_zmake.h

#include "zmake/_zmake.h"

const int String::npos = (int)std::string::npos;
bool streq(const char* a, const char* b)
{
   return strcmp(a, b) == 0;
}

bool fromString(   String& out, const char* s, bool )
{
   out = s;
   return true;
}

const char* kZeroString = "00000000000000000000000000000000";
int kZeroStringLen = strlen(kZeroString);
const char* zeroSpace(int n)
{
   return kZeroString + kZeroStringLen - n;
}
const char* kUpDirString = "../../../../../../../../../../../";
int kUpDirStringLen = strlen(kUpDirString);
const char* updirSpace(int i)
{
   // always comes in triplets
   int n = 3 * i;
   return kUpDirString + kUpDirStringLen - n;
}

const char* toHex(uint32 i, int len)
{
   char* buf = WTON::strBuf();
   if (len != -1)
   {
      WTON_SNPRINTF(buf, WTON::kMaxStrCatLen, "%0*x", len, i);
   }
   else
   {
      WTON_SNPRINTF(buf, WTON::kMaxStrCatLen, "%x", i);
   }
   WTON::finishStrBuf(buf);
   return buf;
}

const char* toHex(uint64 i, int len)
{
   uint32 lo = (uint32)(i & 0xFFFFFFFF);
   uint32 hi = (uint32)((i >> 32LL) & 0xFFFFFFFF);

   if (!hi)
   {
      return toHex(lo, len);
   }

   char* buf = WTON::strBuf();
   if (len > 8)
   {
      WTON_SNPRINTF(buf, WTON::kMaxStrCatLen, "%0*x%08x", len - 8, hi, lo);
   }
   else
   {
      WTON_SNPRINTF(buf, WTON::kMaxStrCatLen, "%x%08x", hi, lo);
   }

   WTON::finishStrBuf(buf);
   return buf;
}

const char* WTON::toString(char c)
{
   char* buf = strBuf();
   buf[0] = c;
   buf[1] = 0;
   finishStrBuf(buf);
   return buf;
}

const uint64 gPrime = 65599;
uint64 hashString(const char* str, uint64 len)
{
   uint64 c = str[len];
   if (len == 0)
   {
      return c;
   }
   else
   {
      return hashString(str, len - 1) * gPrime + c;
   }
}
void String::replace(const char* src, const char* dst)
{
   int n1 = strlen(src);
   int n2 = strlen(dst);
   int pos = 0;
   while (true)
   {
      pos = find(src, pos);

      if (pos != npos)
      {
         std::string::replace(pos, n1, dst, n2);
         pos += n2;
      }
      else
         break;
   }
}

void String::toUpper()
{
   char* sz = const_cast<char*>(cStr());

   int delt = 'A' - 'a';
   while (*sz)
   {
      if (*sz >= 'a' && *sz <= 'z')
      {
         *sz += delt;
      }

      ++sz;
   }
}

void String::toLower()
{
   char* sz = const_cast<char*>(cStr());

   int delt = 'A' - 'a';
   while (*sz)
   {
      if (*sz >= 'A' && *sz <= 'Z')
      {
         *sz -= delt;
      }

      ++sz;
   }
}

void strReplace(char* str, char from, char to)
{
   WTON_ASSERT(str);
   for (char* s = str; *s; ++s)
   {
      if (*s == from)
      {
         *s = to;
      }
   }
}

bool fromString(bool& i, const char* s, bool fail)
{
   if (strcmp(s, "1") == 0)
   {
      i = true;
   }
   else if (strcmp(s, "0") == 0)
   {
      i = false;
   }
   else
   {
      WTON_ASSERT(!fail);
      return false;
   }
   return true;
}

bool fromString(int& i, const char* s, bool)
{
   i = strtoul(s, NULL, 10);
   return true;
}
