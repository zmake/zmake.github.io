// Copyright (c) 2014 - 2016 Roger Hanna
// For conditions of distribution and use, see copyright notice in _wton.h

#include "wton/_wton.h"

namespace WTON
{
   template <typename T>
   void fastRemoveIdx(WtonArray<T>& arr, int idx)
   {
      int last = arr.size() - 1;
      arr[idx] = arr[last];
      arr.pop_back();
   }

   bool loadVerboseHeader(TokenFile& s, WtonName& type, WtonName& name)
   {
      WtonName prefix;

      void* startState = s.allocState();
      s.readToken(prefix);

      // its either end of file or end of object, either way we're done
      if (prefix.compare("{") != 0)
      {
         if (prefix.compare("}") != 0 && !s.eof())
         {
            s.parseFail("{ or }", prefix.c_str());
         }
         s.setState(startState);
         return false;
      }
      WTON_FREE(startState);

      s.readToken(type);
      s.readToken(name);
      s.readAssert("=");

      return true;
   }

   void seekNextPost(TokenFile& s)
   {
      WtonName postfix;
      int postCount = 1;
      while (postCount)
      {
         s.readToken(postfix);
         if (postfix.compare("}") == 0)
            --postCount;
         else if (postfix.compare("{") == 0)
            ++postCount;
      }
   }

   int findMatching(const WtonArray<MemberNotation*>& members, WtonName iname)
   {
      for (int i = members.size() - 1; i >= 0; --i)
      {
         MemberNotation* m = members[i];
         if (m->name().compare(iname) == 0)
         {
            return i;
         }
      }
      return -1;
   }

   void ClassNotation::load(TokenFile& s, void* data)
   {
      WtonArray<MemberNotation*> members = mMembers;

      WtonName type, iname;
      while (loadVerboseHeader(s, type, iname))
      {
         int idx = findMatching(members, iname);
         if (idx != -1)
         {
            const MemberNotation* m = members[idx];

            if (type.compare(m->type()) != 0)
            {
               bool remapOk = false;

#define WTON_TYPE_REMAP(oldT, newT) \
   if (type.compare(#oldT) == 0 && m->type().compare(#newT) == 0) \
      remapOk = true \

               // encourage signed datatypes unless there's a strict performance reason necessitating this datatype is unsigned
               WTON_TYPE_REMAP(uint, int);
               WTON_TYPE_REMAP(Array<uint>, Array<int>);

               if (!remapOk)
               {
                  s.parseFail(m->type().c_str(), type.c_str());
               }
            }

            m->load(s, data);

            fastRemoveIdx(members, idx);

            // close member
            s.readAssert("}");
         }
         else
         {
            s.parseWarn(iname.c_str());
            seekNextPost(s);
         }
      }

      // TODO: assign values to prototype value if they aren't loaded?
   }

   MemberNotation::MemberNotation(WtonName iName, WtonName iType, int iOffset)
      : mName(iName)
      , mType(iType)
      , mOffset(iOffset)
   {
   }
};
