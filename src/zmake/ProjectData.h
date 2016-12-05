// Copyright (c) 2014 - 2016 Roger Hanna
// For conditions of distribution and use, see copyright notice in ~/src/zmake/_zmake.h

struct ProjectData
{
   ProjectData(String projPath);

   // saved data
   Array<String> Platforms;
   Array<String> Configs;
   String AdditionalProps;
   String AdditionalMake;
   Array<String> AdditionalFileProps;
   bool Dev;
   bool Exe;
   bool Dll;
   bool Ignore;
   bool Prebuild;
   String Prefix;
   String PrecompiledHeader;
   Array<String> IgnorePatterns;

   // transient
   bool mLib;
   bool mHasCode;
   String mPath;
   String mDisplayName;
   String mRootName;
   Array<ProjectData*> mChildProjects;
   Array<String> mSubdirs;
   Array<String> mCode;
   Array<String> mOther;

   const char* vcxName() const
   {
      return strCat(mDisplayName, ".vcxproj"); 
   }

   void writeVcxproj(File*& outVcx, File*& outFilter, Array<ProjectData*>& projData);
   void writeMake(const Array<ProjectData*>& makeData);

   static WTON::ClassNotation* TKN();

   static const char* toGuid(String guidHash);
};

