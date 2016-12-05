// Copyright (c) 2014 - 2016 Roger Hanna
// For conditions of distribution and use, see copyright notice in ~/src/zmake/_zmake.h

#include "zmake/_zmake.h"

#ifdef WTON_PC
# include <direct.h>
#else
# define _getcwd getcwd
# include <unistd.h>
# include <dirent.h>
#endif

#include <sys/stat.h>

namespace FileMgr
{
   typedef void (*InternalOperateCB)(const char* file);
   typedef void (*OperateCB)(String file);
   Array<OperateCB> gCBs;
   Array<InternalOperateCB> gCB2s;
   Array<char*> gSzs;

   void operate(const char* pattern, bool dirs, OperateCB cb, InternalOperateCB cb2);
   void internalOperateCB(const char* file);
   void operateFile(String& fullname, OperateCB cb, InternalOperateCB cb2);
   void normalizePath(String& ipath);
};

Array<String> gDefaultRet;
void defaultCB(String file)
{
   gDefaultRet.push_back(file);
}

void FileMgr::normalizePath(String& ipath)
{
   ipath.replace("\\", "/");
}

void FileMgr::internalOperateCB(const char* file)
{
   WTON_ASSERT(gSzs.len());
   char* gSz = gSzs.back();
   OperateCB gCB = gCBs.back();
   InternalOperateCB gCB2 = gCB2s.back();
   String subpat(strCat(file, gSz + 1));
   operate(subpat.cStr(), false, gCB, gCB2);
}

void FileMgr::operateFile(String& fullname, OperateCB cb, InternalOperateCB cb2)
{
   normalizePath(fullname);

   // any files operate discover will be added to the list of files
   String hash(toString(fullname.cStr()));

   // hollaback!
   if (cb)
      cb(hash);
   else if (cb2)
      cb2(fullname.cStr());
}

void FileMgr::operate(const char* pattern, bool dirs, OperateCB cb, InternalOperateCB cb2)
{
   // exactly one of these should be set
   WTON_ASSERT(cb == NULL || cb2 == NULL);
   WTON_ASSERT(cb || cb2);

   String pat(pattern);
   if (pat.endsWith("\\"))
   {
      pat = pat.substr(0, pat.len() - 1);
   }
   String ipath;

   // we'll be gentle, promise!
   char* sz = const_cast<char*>(pat.cStr());
   bool wild = false;
   char* dir = NULL;
   while (*sz)
   {
      if (*sz == '/' || *sz == '\\')
      {
         if (wild)
         {
            *sz = 0;

            gSzs.push_back(sz);
            gCBs.push_back(cb);
            gCB2s.push_back(cb2);

            // recurse wildcard dirs to expand them
            operate(pat.cStr(), true, NULL, internalOperateCB);

            gSzs.pop_back();
            gCBs.pop_back();
            gCB2s.pop_back();

            *sz = '\\';
            return;
         }
         *sz = '\\';
         dir = sz;
      }
      else if (*sz == '*')
         wild = true;

      ++sz;
   }

#ifdef WTON_PC
   const char* qpat = pat.cStr();

   WIN32_FIND_DATA data;
   // FindFirstFileEx ?
   HANDLE hResult = FindFirstFile(qpat, &data);

   if (hResult == INVALID_HANDLE_VALUE)
      return;

   if (dir)
   {
      *dir = 0;
      ipath = pat;
      *dir = '\\';
   }

   do
   {
      // TODO: support .dotfiles
      if (startsWith(data.cFileName, "."))
         continue;

      if (dirs == ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0))
      {
         String fullname(dir ? strCat(ipath, "\\", data.cFileName) : data.cFileName);

         if (dirs)
         {
            fullname += "\\";
         }

         operateFile(fullname, cb, cb2);
      }
   }
   while (FindNextFile(hResult, &data));

   FindClose(hResult);

#else

   unused(dir);
   DIR* pdir;
   dirent* dent;
   pat.replace("\\", "/");
   int pos = pat.find_last_of('/');
   String patternn(pat.substr(pos + 1));
   int pos2 = patternn.find('*');
   String startPat, endPat;
   if (pos2 != String::npos)
   {
      startPat = patternn.substr(0, pos2);
      endPat = patternn.substr(pos2+1);
   }
   else
   {
      startPat = endPat = patternn;
   }

   pat = pat.substr(0, pos+1);

   WTON_ASSERT(pat[0] != '.' || pat[1] != '/', pat);
   pdir = opendir (strCat("./", pat.cStr()));

   if (pdir != NULL)
   {
      while ((dent = readdir (pdir)))
      {
         String name(dent->d_name);
         // TODO: support .dotfiles
         if (name.startsWith("."))
            continue;

         String fullpath(strCat(pat, name));

         bool entDir;
         {
            struct stat foo;
            int ret = stat(fullpath.cStr(), &foo);
            WTON_ASSERT(ret == 0);
            entDir = S_ISDIR(foo.st_mode);
         }

         if (dirs == entDir)
         {
            if (dirs)
            {
               fullpath += "/";
            }

            if (name.startsWith(startPat.cStr()) && name.endsWith(endPat.cStr()))
            {
               operateFile(fullpath, cb, cb2);
            }
         }
      }

      (void) closedir (pdir);
   }
   else
   {
      // this just means the fixed path doesn't exist
   }
#endif
}

Array<String> FileMgr::enumerate(const char* pattern)
{
   gDefaultRet.clear();

   operate(pattern, false, defaultCB, NULL);

   ArrayUtl::sort(gDefaultRet);

   return gDefaultRet;
}

Array<String> FileMgr::enumerateDirs(const char* pattern)
{
   gDefaultRet.clear();

   operate(pattern, true, defaultCB, NULL);
   ArrayUtl::sort(gDefaultRet);

   return gDefaultRet;
}

const char* FileMgr::path(const char* ipath)
{
   String s(ipath);
   int pos = s.find_last_of('/', s.len() - 2);
   if (pos == String::npos)
      return "";
   else
      return strCat(s.substr(0, pos+1).c_str());
}

const char* FileMgr::workDir()
{
   const int kBufLen = 1024;
   char buf[kBufLen];
   _getcwd(buf, kBufLen);
   strReplace(buf, '\\', '/');
   return strCat(buf, "/");
}

bool FileMgr::matchesPattern(const char* str, const char* ipattern)
{
   String pattern(ipattern);
   int pos2 = pattern.find('*');
   String startPat, endPat;
   if (pos2 != String::npos)
   {
      startPat = pattern.substr(0, pos2);
      endPat = pattern.substr(pos2+1);
   }
   else
   {
      startPat = endPat = pattern;
   }

   return startsWith(str, startPat.cStr()) && endsWith(str, endPat.cStr());
}

const char* FileMgr::name(const char* ipath)
{
   String s(ipath);
   int slen = s.len();
   int pos = s.find_last_of('/', slen - 2);
   if (s[slen-1] == '/')
   {
      return strCat(s.substr(pos + 1, slen - (pos + 2)));
   }
   else
   {
      return strCat(s.substr(pos + 1));
   }
}

const char* FileMgr::base(const char* ipath)
{
   String s(ipath);
   int pos = s.find_last_of('/');
   int pos2 = s.find_last_of('.');
   if (pos2 == String::npos || pos2 < pos)
   {
      return strCat(s.substr(pos + 1));
   }
   else
   {
      return strCat(s.substr(pos + 1, pos2 - pos - 1));
   }
}

const char* FileMgr::basepath(const char* ipath)
{
   String s(ipath);
   int pos = s.find_last_of('/');
   int pos2 = s.find_last_of('.');
   if (pos2 == String::npos || pos2 < pos)
   {
      return strCat(ipath);
   }
   else
   {
      return strCat(s.substr(0, pos2));
   }
}

const char* FileMgr::extension(const char* ipath)
{
   String s(ipath);
   int pos = s.find_last_of('/');
   int pos2 = s.find_last_of('.');
   if (pos2 == String::npos || pos2 < pos)
   {
      return "";
   }
   else
   {
      return strCat(s.substr(pos2));
   }
}

bool FileMgr::exists(String str)
{
   // folders - they can't have a trailing slash
   if (str.endsWith("/"))
   {
      str = str.substr(0, str.len() - 1);
   }

   struct stat foo;
   int ret = stat(str.cStr(), &foo);
   return ret == 0 ? true : false;
}

// this is probably going to take a few tries to get right, but bare with us!
// note: ipath MUST be fully qualified for this to work under all circumstances
const char* FileMgr::relativePath(const char* ipath, const char* newRoot)
{
   // "." = "../src" = "../run"
   if (streq(ipath, "") || streq(ipath, "."))
   {
      if (streq(newRoot, "../src/"))
         return "../run/";

      WTON_ASSERT(0);
   }

   // "../src/_app/App.cpp" - "" = "../src/_app/App.cpp"
   if (streq(newRoot, "") || streq(newRoot, "."))
      return strCat(ipath);

   // "../src/prims/prims.vcxproj" - "../src" = "prims/prims.vcxproj"
   // "../dev/dev.vcxproj" - "../src" = "../dev/dev.vcxproj"
   // "../src/rend_gl/" - "../src/rend" = "../rend_gl/"

   // find common root
   int i = 0;
   int slash = -1;
   while (true)
   {
      // TODO:
      WTON_ASSERT(ipath[i] != '\0');

      if (newRoot[i] != '\0')
      {
         if (ipath[i] != newRoot[i])
            break;
         else if (ipath[i] == '/')
            slash = i;
      }
      // we consumed the entire root
      else
      {
         if (ipath[i] == '/')
            return ipath + i + 1;
         else
         {
            WTON_ASSERT(slash != -1);
            break;
         }
      }

      ++i;
   }

   // for each path still in newRoot, we need a ../newPath
   int dirCount = 0;
   const char* s;
   for (s = newRoot + i; *s; ++s)
   {
      if (*s == '/')
         ++dirCount;
   }
   // if we don't end on a trailing slash we need to increment it one more
   if (s[-1] != '/')
      ++dirCount;

   return strCat(updirSpace(dirCount), ipath + slash + 1);
}
