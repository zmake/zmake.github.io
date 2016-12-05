// Copyright (c) 2014 - 2016 Roger Hanna
// For conditions of distribution and use, see copyright notice in ~/src/zmake/_zmake.h

namespace FileMgr
{
   bool exists(String path);
   const char* path(const char* path);
   const char* name(const char* path);
   const char* base(const char* path);
   const char* basepath(const char* ipath);
   const char* extension(const char* path);
   const char* workDir();
   Array<String> enumerate(const char* pattern);
   Array<String> enumerateDirs(const char* pattern);
   bool matchesPattern(const char* str, const char* pattern);
   const char* relativePath(const char* path, const char* newRoot);
};
