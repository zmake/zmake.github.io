// Copyright (c) 2014 - 2016 Roger Hanna
// For conditions of distribution and use, see copyright notice in _wton.h

#include "wton/_wton.h"

#define WTON_MIN(a, b) (a < b ? a : b)
#define WTON_ASSERT_PARSE(test, expected, found) if (!(test)) parseFail(toString(expected), toString(found))
namespace WTON
{
   void fopens(FILE** fd, const char* name, const char* perms)
   {
#ifndef WTON_PC
      *fd = fopen(name, perms);
#else
      fopen_s(fd, name, perms);
#endif
   }

   const char* kSpaceString = "                                ";
   int kSpaceStringLen = strlen(kSpaceString);
   const char* whiteSpace(int n)
   {
      return kSpaceString + kSpaceStringLen - n;
   }

   inline bool hasFlags(int flags, int test)
   {
      return (flags & test) == test;
   }

   void fromHexString(unsigned int& out, const char* s)
   {
      out = strtoul(s, NULL, 16);
   }

   const int TokenFile::kMaxTokenLen = 1024;
   TokenFile::TokenFile(WtonString ipath, Mode imode)
      : mFd(0)
      , mLine(1)
      , mIndent(0)
      , mWriteState(kNewLine)
      , mMode(imode)
      , mPath(ipath)
   {
      const char* _file = mPath.c_str();

      const char* mode = NULL;
      if (mMode == TokenFile::kRead)
         mode = "rb";
      else if (mMode == TokenFile::kWrite)
         mode = "wb";
      else
      {
         WTON_FAIL("unknown file mode ", int(mMode));
      }

      WTON_ASSERT(mode, mPath);
      fopens(&mFd, _file, mode);

      WTON_ASSERT(mFd != NULL);
   }

   TokenFile::~TokenFile()
   {
      if (mMode == TokenFile::kWrite)
      {
         WTON_PRINT("Wrote ", mPath);
      }
      fclose(mFd);
   }

   void TokenFile::writeToken(const char* s)
   {
      WTON_ASSERT(s, mPath);

      if (mWriteState != kNewLine)
         _store(" ", 1);
      else if (mIndent)
      {
         const int kIndentLen = 2;
         int totIndent = kIndentLen * mIndent;
         _store(whiteSpace(totIndent), totIndent);
      }

      int len = strlen(s);

      const char* space = strchr(s, ' ');
      const char* dquot = strchr(s, '\"');
      const char* quote = strchr(s, '\'');
      const char* slash = strchr(s, '\\');
      const char* liner = strchr(s, '\n');

      int startPos = ftell(mFd);

      // if it has no special chars, just blast it
      if (len > 0 && !space && !dquot && !quote && !slash && !liner)
      {
         _store(s, len);

         mWriteState = kMidLine;
      }
      else if (!dquot && !slash)
      {
         _store("\"", 1);
         _store(s, len);
         _store("\"", 1);
      }
      else if (!quote && !slash)
      {
         _store("\'", 1);
         _store(s, len);
         _store("\'", 1);
      }
      else
      {
         const char* start = s;

         const char* invalid = (const char*)0xffffffffu;
         if (!dquot)
            dquot = invalid;
         if (!slash)
            slash = invalid;

         _store("\"", 1);
         const char* minPos = WTON_MIN(dquot, slash);
         while (minPos != invalid)
         {
            _store(start, minPos - start);

            // escape the next character
            _store("\\", 1);
            start = minPos;
            if (start[0] == '"')
            {
               dquot = strchr(start+1, '\"');
               if (!dquot)
                  dquot = invalid;
            }
            else
            {
               slash = strchr(start+1, '\\');
               if (!slash)
                  slash = invalid;
            }
            minPos = WTON_MIN(dquot, slash);
         }
         _store(start, strlen(start));
         _store("\"", 1);
      }

      // ensure we're not writing out tokens we can't parse back in
      WTON_ASSERT(ftell(mFd) - startPos < kMaxTokenLen);
   }

   void TokenFile::readToken(WtonString& s)
   {
      do
      {
         _readToken(s);
      }
      while (s.compare(WTON_LINE_BREAK) == 0 && !eof());
   }

   void TokenFile::readAssert(const char* lit)
   {
      WtonString s;
      readToken(s);
      if (s.compare(lit) != 0)
      {
         parseFail(lit, s.c_str());
      }
   }

   void TokenFile::parseFail(const char* expected, const char* found)
   {
      // be sure to use absolute paths if you want this error clickable in Visual Studio output
      WTON_FAIL(mPath, "(", mLine, "): error: zmake text issue:  found '", found, "'  expected '", expected, "'");
   }

   void TokenFile::parseWarn(const char* found)
   {
      WTON_WARN(mPath, "(", mLine, "): warning: zmake text issue:  unexpected '", found, "'");
   }

   void TokenFile::_readToken(WtonString& s)
   {
      // for this impl pass, just read a huge amount that is > what we
      // care about, then seek back.
      char buf[kMaxTokenLen];

      int startTell = ftell(mFd);
      int kMaxTokenSize = kMaxTokenLen - 2;
      int count = _load(buf, kMaxTokenSize);
      if (count > 0)
      {
         // we wantonly look ahead 2 characters so our buf
         buf[count] = 0;
         buf[count+1] = 0;
      }
      else
      {
         // TODO: shouldn't this be an error to read past the end?
         s = WTON_LINE_BREAK;
         return;
      }

      int pos = 0;

      // skip lead whitespace
      while (pos < count && (buf[pos] == ' ' || buf[pos] == '\t'))
      {
         ++pos;
      }

      bool delim = false;
      bool delimDouble = false;
      bool delimSingle = false;
      bool delimComment = false;
      bool writeSentinel = false;
      if (buf[pos] == '"')
      {
         delim = true;
         delimDouble = true;
         ++pos;
      }
      else if (buf[pos] == '\'')
      {
         delim = true;
         delimSingle = true;
         ++pos;
      }
      else if (buf[pos] == '/' && buf[pos+1] == '/')
      {
         delim = true;
         delimComment = true;
         writeSentinel = true; // our output from a comment is just a sentinel token
      }

      int startPos = pos;
      int streamSlip = 0;
      while (pos + streamSlip < count)
      {
         char c = buf[pos];
         switch(c)
         {
         case 0:
            // this happens when a binary file is tried to be read as a text file
            parseFail("text", "binary");
         case ' ':
         case '\t':
            if (delim) goto loop;
            else goto done;
         case '\'':
            if (delimSingle) goto done;
            else if (delim) goto loop;
            else 
            {
               // "wasn't" is a single token
               // "find(' ')" is 3 tokens 
               char prev = buf[pos-1];
               if (('a' <= prev && prev <= 'z') ||
                  ('A' <= prev && prev <= 'Z'))
               {
                  goto loop;
               }
               else
               {
                  goto done;
               }
            }
         case '"':
            if (delimDouble) goto done;
            else if (delim) goto loop;
            // WTON_FAIL("What" is two tokens
            else goto done;
         case '\r':
            {
               int extraRs = 0;
               // how does this even happen?
               while (pos + extraRs + streamSlip < count && buf[pos + extraRs] == '\r')
               {
                  ++extraRs;
               }
               WTON_ASSERT(extraRs > 0);
               // newlines are allowed in constants because otherwise it's too unreadable
               if (delimComment)
               {
                  WTON_ASSERT(buf[pos + extraRs] == '\n' || buf[pos + extraRs] == 0, mPath);

                  ++mLine;
                  pos+=1+extraRs;

                  goto done;
               }
               else if (delim)
               {
                  if (extraRs)
                  {
                     // TODO: O(n) solution for n chars rather than O(n*m) for n chars and m special chars
                     // but low priority cuz it's dev-only
                     memmove(buf+pos, buf+pos+extraRs, count - pos);
                     streamSlip += extraRs;
                     mLine += extraRs - 1;
                  }

                  WTON_ASSERT(buf[pos] == '\n' || buf[pos] == 0, mPath);

                  // errors will show up on the last of multi-line tokens
                  ++mLine;
                  goto loop;
               }
               else
               {
                  WTON_ASSERT(buf[pos + extraRs] == '\n' || buf[pos + extraRs] == 0, mPath);
                  if (pos == startPos)
                  {
                     ++mLine;
                     pos+=1+extraRs;

                     writeSentinel = true;
                  }
                  goto done;
               }
            }
         case '\n':
            // newlines are allowed in constants because otherwise it's too unreadable
            if (delimComment)
            {
               ++mLine;
               ++pos;
               goto done;
            }
            else if (delim)
            {
               // errors will show up on the last of multi-line tokens
               ++mLine;
               goto loop;
            }
            else
            {
               if (pos == startPos)
               {
                  ++mLine;
                  ++pos;

                  writeSentinel = true;
               }
               goto done;
            }
         case '\\':
            int slipLen = 1;
            switch(buf[pos + 1])
            {
            case 'n':
               buf[pos + slipLen] = '\n';
            case '"':
            case '\'':
            case '\\':
               break;
            case 'x':
               slipLen = 3;

               char hexStr[3];
               hexStr[0] = buf[pos+2];
               hexStr[1] = buf[pos+3];
               hexStr[2] = 0;

               unsigned int out;
               fromHexString(out, hexStr);

               buf[pos + slipLen] = (char)out;
               // \x10 = (char)16
               break;

            default:
               {
                  WTON_WARN("unknown escape character ", mPath);
                  goto loop;
               }
            }

            // TODO: O(n) solution for n chars rather than O(n*m) for n chars and m special chars
            // but low priority cuz it's dev-only
            memmove(buf+pos, buf+pos+slipLen, count - pos);
            streamSlip += slipLen;

            goto loop;
         }
loop:
         ++pos;
      }
done:
      int endPos = pos;
      int tokenSize = endPos + streamSlip;

      // if you hit this, the constant _read size is too small.
      if (delimDouble || delimSingle)
         ++tokenSize;
      WTON_ASSERT_PARSE(tokenSize < kMaxTokenSize, strCat("token smaller than ", kMaxTokenSize), tokenSize);

      if (delimDouble)
      {
         // must have a terminal double quote if we started with one
         if (buf[pos] != '"')
         {
            parseFail("\"", buf + pos);
         }
      }
      else if (delimSingle)
      {
         // must have a terminal quote if we started with one
         if (buf[pos] != '\'')
         {
            parseFail("\'", buf + pos);
         }
      }

      fseek(mFd, long(startTell + tokenSize), SEEK_SET);

      buf[endPos] = 0;

      if (writeSentinel)
      {
         s = WtonString(WTON_LINE_BREAK);
      }
      else
      {
         s = WtonString(buf + startPos);
      }
      //PRINT("token: '", s, "'\t", tell(), "\t", mLine);
   }

   bool TokenFile::eof() const
   {
      return feof(mFd) != 0;
   }

   int TokenFile::_load(void* dst, int bytes)
   {
      WTON_ASSERT(mMode == TokenFile::kRead, mPath);
      WTON_ASSERT(mFd, "Couldn't load ", mPath);

      int rd = fread(dst, 1, bytes, mFd);
      return rd;
   }

   void TokenFile::_store(const void* src, int bytes)
   {
      WTON_ASSERT(hasFlags(mMode, TokenFile::kWrite), mPath);
      WTON_ASSERT(mFd);

      size_t ret = fwrite(src, 1, bytes, mFd);
      WTON_ASSERT((int)ret == bytes, mPath);
   }

   struct TextState
   {
      int mTell;
      int mLine;
      int mIndent;
   };

   void* TokenFile::allocState() const
   {
      TextState* ret = (TextState*)WTON_MALLOC(sizeof(TextState));
      ret->mTell   = ftell(mFd);
      ret->mLine   = mLine;
      ret->mIndent = mIndent;
      return ret;
   }

   void TokenFile::setState(void* state)
   {
      TextState* ret = (TextState*)state;
      fseek(mFd, long(ret->mTell), SEEK_SET);

      mLine   = ret->mLine;
      mIndent = ret->mIndent;
      WTON_FREE(ret);
   }

   void TokenFile::incIndent()
   {
      ++mIndent;
   }
   void TokenFile::decIndent()
   {
      --mIndent;
      WTON_ASSERT(mIndent >= 0);
   }
};
