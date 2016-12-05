// Copyright (c) 2014 - 2016 Roger Hanna
// For conditions of distribution and use, see copyright notice in ~/src/zmake/_zmake.h

// GlobalConfig theConfig defines global parameters in zmake (loaded from zmake-config.ztxt)

struct GlobalConfig
{
   GlobalConfig(const char* cfgPath);

   // which file types to ignore
   Array<String> IgnorePatterns;

   // which paths to include
   Array<String> IncludePatterns;

   // how do we identify external library projects?
   String LibPattern;

   // what is the set of platforms and configurations we're trying to generate
   Array<String> AllPlatforms;
   Array<String> AllConfigs;

   Array<String> MsvcPlatforms;
   Array<String> MakePlatforms;

   String MsSlnFormatVersion;

   String ZmakeToRoot;
   String RootToProps;
   String RootToSln;
   String SlnToRoot;

   String PchPattern;
   String PchPrefix;

   Map<String, String>         SolutionPlatform;
   Map<String, String>         ExtensionItem;  // { { cpp ClCompile } { lib Library } etc }
   Array<String>               CompilationUnitExtensions;
   Map<String, Array<String> > SuffixPlatforms; // Foo_PC.cpp -> { Win32 x64 } etc
   String StubSuffix;

   // this propagates to all Projects
   Array<String> AdditionalFileProps;
};

extern GlobalConfig* theConfig;
