// Copyright (c) 2014 - 2016 Roger Hanna
// For conditions of distribution and use, see copyright notice in ~/src/zmake/_zmake.h

bool streq(const char* a, const char* b);
inline bool startsWith(const char* str, const char* prefix)
{
   return !strncmp(str, prefix, strlen(prefix));
}
inline bool endsWith(const char* str, const char* suffix)
{
   int slen = strlen(suffix);
   int len = strlen(str);
   return !strncmp(str + len - slen, suffix, slen);
}
const char* toHex(uint64 i,      int len = -1);
const char* zeroSpace(int n);
const char* updirSpace(int i);
uint64 hashString(const char* str, uint64 len);
void strReplace(char* str, char from, char to);

struct String : public std::string
{
   static const int npos;

   String()
      : std::string()
   {
   }
   String(const char* cstr)
      : std::string(cstr)
   {
   }
   String(const std::string& str)
      : std::string(str)
   {
   }

   bool operator==(const String& s) const
   {
      return streq(cStr(), s.cStr());
   }
   bool operator==(const char* s) const
   {
      return streq(cStr(), s);
   }
   bool operator!=(const String& s) const
   {
      return !streq(cStr(), s.cStr());
   }
   bool operator!=(const char* s) const
   {
      return !streq(cStr(), s);
   }
   bool operator<(const String& r) const
   {
      return strcmp(cStr(), r.cStr()) < 0;
   }
   inline bool startsWith(const char* prefix) const
   {
      return ::startsWith(cStr(), prefix);
   }

   inline bool endsWith(const char* suffix) const
   {
      return ::endsWith(cStr(), suffix);
   }


   int len() const { return (int)std::string::size(); }
   const char* cStr() const { return c_str(); }
   void toUpper();
   void toLower();
   void replace(const char* src, const char* dst);
   bool filled() const { return !empty(); }
   bool contains(const char* sub, int pos = 0) const { return (int)find(sub, pos) != npos; }
};
