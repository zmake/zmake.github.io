// Copyright (c) 2014 - 2016 Roger Hanna
// For conditions of distribution and use, see copyright notice in ~/src/zmake/_zmake.h

#include "zmake/_zmake.h"

#define ZM_SAFE_DELETE_ARRAY(mem) delete[] mem; mem = NULL

File::File(String ipath, Mode imode)
   : mFd(0)
   , mPreviousBuffer(NULL)
   , mMode(imode)
   , mPath(ipath)
{
   open();
}

File::~File()
{
   if (willWrite())
   {
      writeChanged();
   }

   ZM_SAFE_DELETE_ARRAY(mPreviousBuffer);
   if (mFd)
   {
      if (mMode == File::kWrite)
      {
         WTON_PRINT("Wrote ", mPath);
      }
      fclose(mFd);
   }
}

void File::invalidateWrite()
{
   if (valid())
   {
      WTON_ASSERT(!mFd, mPath);
      WTON_ASSERT(mMode == File::kWriteChanged, mPath);

      mMode = File::kRead;
      ZM_SAFE_DELETE_ARRAY(mPreviousBuffer);
      mWriteChangedOut.clear();
   }
}

bool File::willWrite()
{
   // if we didn't end on the same size, then it has changed
   return mMode == File::kWriteChanged && (mPreviousSize != (int)mWriteChangedOut.size() || memcmp(mPreviousBuffer, &mWriteChangedOut[0], mPreviousSize) != 0);
}

void File::open()
{
   const char* _file = mPath.c_str();

   const char* mode = NULL;
   if (mMode == File::kRead)
      mode = "rb";
   else if (mMode == File::kWriteChanged)
   {
      // load up the entire previous contents as a read-only file (for now)
      mode = "rb";
      WTON::fopens(&mFd, _file, mode);

      // conditional write request on a non-existant file will write if any bytes are written
      // conditional write with no output doesn't write!
      if (!mFd)
      {
         mPreviousSize = 0;
         mPreviousBuffer = new uint8[0];
      }
      else
      {
         int sz = len();
         WTON_ASSERT(sz >= 0, mPath);

         mPreviousSize = sz;
         mPreviousBuffer = new uint8[mPreviousSize];

         int rd = fread(mPreviousBuffer, 1, mPreviousSize, mFd);
         WTON_ASSERT(rd == mPreviousSize, mPath);

         fclose(mFd);
         mFd = NULL;
      }

      return;
   }
   else if (mMode == File::kWrite)
      mode = "wb";
   else
   {
      WTON_FAIL("unknown file mode ", mMode);
   }

   if (!mFd)
   {
      WTON_ASSERT(mode, mPath);
      WTON::fopens(&mFd, _file, mode);
   }
}

bool File::valid() const
{
   return mFd != NULL || mPreviousBuffer != NULL;
}

void File::validate() const
{
   WTON_ASSERT(valid());
}

int File::tell() const
{
   validate();
   return mPreviousBuffer ? (int)mWriteChangedOut.size() : ftell(mFd);
}

int File::len()
{
   validate();
   WTON_ASSERT(!mPreviousBuffer, mPath);
   int curr = tell();
   seek(0, File::kSeekEnd);
   int sz = tell();
   seek(curr, File::kSeekSet);
   return sz;
}

int File::load(void* dst, int bytes)
{
   validate();
   WTON_ASSERT(!mPreviousBuffer, mPath);
   WTON_ASSERT(mMode == File::kRead, mPath);
   WTON_ASSERT(mFd, "Couldn't load ", mPath);

   int rd = fread(dst, 1, bytes, mFd);
   return rd;
}

inline bool hasFlags(int flags, int test)
{
   return (flags & test) == test;
}

void File::store(const void* src, int bytes)
{
   validate();
   bool doWrite = true;

   // if conditionally writing (kWriteChanged), check the next segment of memory
   if (mPreviousBuffer)
   {
      int start = mWriteChangedOut.size();
      mWriteChangedOut.resize(mWriteChangedOut.size() + bytes);
      memcpy(&mWriteChangedOut[start], src, bytes);
      doWrite = false;
   }

   if (doWrite)
   {
      WTON_ASSERT(hasFlags(mMode, File::kWrite), mPath);
      WTON_ASSERT(mFd);

      size_t ret = fwrite(src, 1, bytes, mFd);
      WTON_ASSERT((int)ret == bytes, mPath);
   }
}

void File::seek(int offset, File::SeekType st)
{
   validate();
   WTON_ASSERT(!mPreviousBuffer, mPath);

   fseek(mFd, (long)offset, st);
}

void File::writeChanged()
{
   validate();
   mMode = File::kWrite;
   open();
   WTON_ASSERT(mFd, "Couldn't write to ", mPath);

   ZM_SAFE_DELETE_ARRAY(mPreviousBuffer);

   store(mWriteChangedOut.data(), mWriteChangedOut.size());
}
