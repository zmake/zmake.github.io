// Copyright (c) 2014 - 2016 Roger Hanna
// For conditions of distribution and use, see copyright notice in _wton.h

void fopens(FILE** fd, const char* name, const char* perms);

struct TokenFile
{
   static const int kMaxTokenLen;

   enum Mode
   {
      kRead         = 1,
      kWrite        = 2,
   };

   TokenFile(WtonString path, Mode mode = kRead);
   ~TokenFile();

   void writeToken(const char* s);
   void readToken(WtonString& s);

   void readAssert(const char* str);
   void parseFail(const char* expected, const char* found);
   void parseWarn(const char* found);

   bool eof() const;

   void* allocState() const;
   void setState(void* state);

   void incIndent();
   void decIndent();

private:
   // raw byte read/write
   void _store(const void* src, int bytes);
   int  _load(void* dst, int bytes);

   // will return either a line return or a string
   void _readToken(WtonString& s);

   enum WriteState
   {
      kNewLine,
      kMidLine,
   };

   FILE*             mFd;
   int               mLine;
   int               mIndent;
   WriteState        mWriteState;

   const Mode        mMode;
   const WtonString  mPath;
};
