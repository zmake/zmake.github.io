// Copyright (c) 2014 - 2016 Roger Hanna
// For conditions of distribution and use, see copyright notice in ~/src/zmake/_zmake.h

// prim specifies the primitive types zmake uses

// normal uses of WTON shouldn't use its internals like this, but
// zmake is standalone, so it's convenient to not have to redefine the
// same string concatenation and error handling tools
namespace WTON
{
   const char* toString(char c);
};
#include "wton/_wton.h"
using WTON::strCat;
using WTON::toString;

#ifdef WTON_PC
# include <windows.h>
#endif

#include <map>
#include <algorithm>

typedef signed char              int8;
typedef signed int               int32;
typedef signed long long int     int64;
typedef unsigned char            uint8;
typedef unsigned int             uint32;
typedef unsigned long long int   uint64;

template<typename T>             void unused(const T&) {}

#include "Array.h"
#include "String.h"
#include "File.h"
#include "FileMgr.h"

namespace ArrayUtl
{
   template <typename T>
   void sort(Array<T>& arr, bool (*sortFunc)(const T& a, const T& b))
   {
      std::sort(arr.begin(), arr.end(), sortFunc);
   }

   // use default sort order
   template <typename T>
   void sort(Array<T>& arr)
   {
      std::sort(arr.begin(), arr.end());
   }

   template <typename T, typename U>
   void intersectFast(Array<T>& arr, const Array<U>& arr2)
   {
      for (int i = arr.len() - 1; i >= 0; i--)
      {
         const T& t = arr[i];

         // if the sub doesn't contain something, it gets stripped out
         if (!arr2.contains(t))
         {
            arr.fastRemoveIdx(i);
         }
      }
   }
};

bool fromString(     bool& out, const char* s, bool fail = true);
bool fromString(   String& out, const char* s, bool fail = true);
bool fromString(      int& out, const char* s, bool fail = true);

template <typename K, typename V>
struct Map : public std::map<K, V>
{

};

namespace WTON
{
   inline TokenFile& operator<<(TokenFile& f, const std::string& i)
   {
      f.writeToken(i.c_str());
      return f;
   }

   inline TokenFile& operator<<(TokenFile& f, const bool& i)
   {
      f.writeToken(i ? "t" : "f");
      return f;
   }

   inline TokenFile& operator>>(TokenFile& f, bool& i)
   {
      std::string s;
      f.readToken(s);
      fromString(i, s.c_str());
      return f;
   }

   inline TokenFile& operator<<(TokenFile& f, const int& i)
   {
      f.writeToken(toString(i));
      return f;
   }

   inline TokenFile& operator>>(TokenFile& f, int& i)
   {
      std::string s;
      f.readToken(s);
      fromString(i, s.c_str());
      return f;
   }

   inline TokenFile& operator<<(TokenFile& f, const char& i)
   {
      f.writeToken(strCat('\'', i, '\''));
      return f;
   }

   inline TokenFile& operator>>(TokenFile& f, char& i)
   {
      std::string s;
      f.readToken(s);
      if (s.size() != 1)
      {
         f.parseFail("<char>", s.c_str());
      }
      i = s[0];
      return f;
   }

   inline TokenFile& operator<<(TokenFile& f, const char* s)
   {
      f.writeToken(s);
      return f;
   }

   inline TokenFile& operator>>(TokenFile& f, std::string& s)
   {
      f.readToken(s);
      return f;
   }

   template <typename T>
   TokenFile& operator>>(TokenFile& in, Array<T>& a)
   {
      in.readAssert("{");

      a.resize(0);

      String token;
      while (true)
      {
         void* startState = in.allocState();
         in >> token;
         if (token == "}")
         {
            WTON_FREE(startState);
            // continue to read tokens until we reach the terminal token
            break;
         }
         else
         {
            in.setState(startState);
            T t;
            in >> t;
            a += t;
         }
      };
      return in;
   }

   template <typename T>
   TokenFile& operator<<(TokenFile& out, const Array<T>& a)
   {
      out << "{";
      out.incIndent();

      int sz = a.len();
      for (int i = 0; i < sz; ++i)
      {
         out << WTON_LINE_BREAK;
         out << a[i];
      }

      out << WTON_LINE_BREAK;
      out.decIndent();
      out << "}";

      return out;
   }

   template <typename K, typename V>
   TokenFile& operator>>(TokenFile& src, Map<K, V>& dst)
   {
      src.readAssert("{");

      dst.clear();

      String token;
      while (true)
      {
         void* startState = src.allocState();
         src >> token;
         if (token == "}")
         {
            WTON_FREE(startState);
            // continue to read tokens until we reach the terminal token
            break;
         }
         else
         {
            src.setState(startState);
            K k;
            V v;
            src >> k;
            src >> v;

            dst[k] = v;
         }
      };
      return src;
   }

   template <typename K, typename V>
   TokenFile& operator<<(TokenFile& dst, const Map<K, V>& src)
   {
      dst << "{";
      dst.incIndent();

      typename Map<K, V>::const_iterator itc;
      for (itc = src.begin(); itc != src.end(); ++itc)
      {
         dst << WTON_LINE_BREAK;
         dst << itc->first;
         dst << itc->second;
      }

      dst << WTON_LINE_BREAK;
      dst.decIndent();
      dst << "}";

      return dst;
   }
}

namespace WTON
{
   template<>
   struct Trait< String >
   {
      inline static const char* str()
      {
         return "str";
      }
   };

   template<>
   struct Trait< Array<String> >
   {
      inline static const char* str()
      {
         return "str[]";
      }
   };

   template<>
   struct Trait< Map<String, String> >
   {
      inline static const char* str()
      {
         return "map[str,str]";
      }
   };

   template<>
   struct Trait< Map<String, Array<String> > >
   {
      inline static const char* str()
      {
         return "map[str,str[]]";
      }
   };
}

// raw string write (no special character correction)
inline void writeRaw(File& f, const char* text)
{
   f.store(text, strlen(text));
};
