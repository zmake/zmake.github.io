/* wton.h - (WTON) Weakly Typed Object Notation is a lightweight, typed data format

  wton version 0.2.0, Aug 31th 2016

  The MIT License (MIT)

  Copyright (c) 2014 - 2016 Roger Hanna <wton@glugglug.net>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#ifndef WTON_H
#define WTON_H 1

#include "Types.h"

#define WTON_MEMBER(member) \
   classy->mMembers.push_back(WTON::newMember(WTON_OFFSET_POINTER(ActiveClass, member), #member))

#define WTON_MEMBER_TYPE(type, member) \
   classy->mMembers.push_back(WTON::newMember(WTON_OFFSET_POINTER(ActiveClass, member), #member, type))

namespace WTON
{
   #include "TokenFile.h"

   WtonName demangleType(const char* type);

   struct MemberNotation
   {
      MemberNotation(WtonName iName, WtonName iType, int iOffset);

      int offset() const { return mOffset; }

      const WtonName& name() const { return mName; }
      const WtonName& type() const { return mType; }

      virtual void save(TokenFile& dst, const void* src) const = 0;
      virtual void load(TokenFile& src, void* dst) const = 0;

   private:
      const WtonName mName;
      const WtonName mType;
      const int      mOffset;
   };

   template <class T>
   struct MemberNotationType : public MemberNotation
   {
      MemberNotationType(WtonName iName, int iOffset)
         : MemberNotation(iName, WTON_TYPE(T), iOffset)
      {
      }
      MemberNotationType(WtonName iName, WtonName iType, int iOffset)
         : MemberNotation(iName, iType, iOffset)
      {
      }
   protected:
      size_t addr(const void* o) const
      {
         return size_t(o) + offset();
      }
      const T& cmember(const void* o) const
      {
         const T* t = (const T*)addr(o);
         return *t;
      }
      T& member(void* o) const
      {
         T* t = (T*)addr(o);
         return *t;
      }
      virtual void save(TokenFile& dst, const void* src) const
      {
         dst << cmember(src);
      }
      virtual void load(TokenFile& src, void* dst) const
      {
         src >> member(dst);
      }
   };

   template<typename T>
   MemberNotation* newMember(T* offsetMember, WtonName iname)
   {
      return new MemberNotationType<T>(iname, (int)(size_t)offsetMember);
   }

   template<typename T>
   MemberNotation* newMember(T* offsetMember, WtonName iname, WtonName itype)
   {
      return new MemberNotationType<T>(iname, itype, (int)(size_t)offsetMember);
   }

   struct ClassNotation
   {
      WtonArray<MemberNotation*> mMembers;

      void load(TokenFile& src, void* dst);
   };
};

#endif

namespace WTON
{
   #include "StrCat.h"

   int debugOut(const char* msg, bool fail);
};
