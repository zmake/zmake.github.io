// Copyright (c) 2014 - 2016 Roger Hanna
// For conditions of distribution and use, see copyright notice in ~/src/zmake/_zmake.h

struct File
{
   enum Mode
   {
      kRead         = 1,
      kWrite        = 2,

      // only write the file if it's different from disc (occurs on close)
      kWriteChanged = 4|kWrite,
   };

   File(String path, Mode mode = kRead);
   ~File();

   // raw byte read/write
   void store(const void* src, int bytes);
   int load(void* dst, int bytes);

   int len();

   void invalidateWrite();
   bool willWrite();

   bool valid() const;

private:
   enum SeekType
   {
      kSeekSet,
      kSeekCurr,
      kSeekEnd,
   };
   void seek(int offset, SeekType st);
   int tell() const;

   void open();
   void validate() const;
   enum WriteState
   {
      kNewLine,
      kMidLine,
   };
   void writeChanged();


   FILE* mFd;
   uint8* mPreviousBuffer;
   int    mPreviousSize;
   Array<uint8> mWriteChangedOut;

   Mode mMode;
   const String mPath;
};
