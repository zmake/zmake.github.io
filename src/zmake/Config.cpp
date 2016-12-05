// Copyright (c) 2014 - 2016 Roger Hanna
// For conditions of distribution and use, see copyright notice in ~/src/zmake/_zmake.h

#include "zmake/_zmake.h"

GlobalConfig* theConfig = NULL;

GlobalConfig::GlobalConfig(const char* cfgPath)
{
   WTON_ASSERT(theConfig == NULL);

   WTON::ClassNotation* classy = NULL;
   {
      classy = new WTON::ClassNotation();
      typedef GlobalConfig ActiveClass;

      // list members for wton serialization
      WTON_MEMBER(ZmakeToRoot);
      WTON_MEMBER(RootToProps);
      WTON_MEMBER(RootToSln);
      WTON_MEMBER(SlnToRoot);
      WTON_MEMBER(IncludePatterns);
      WTON_MEMBER(IgnorePatterns);
      WTON_MEMBER(AllPlatforms);
      WTON_MEMBER(AllConfigs);
      WTON_MEMBER(MsvcPlatforms);
      WTON_MEMBER(MakePlatforms);
      WTON_MEMBER(MsSlnFormatVersion);
      WTON_MEMBER(PchPattern);
      WTON_MEMBER(PchPrefix);
      WTON_MEMBER(LibPattern);
      WTON_MEMBER(SolutionPlatform);
      WTON_MEMBER(ExtensionItem);
      WTON_MEMBER(CompilationUnitExtensions);
      WTON_MEMBER(SuffixPlatforms);
      WTON_MEMBER(StubSuffix);
      WTON_MEMBER(AdditionalFileProps);
   }

   // default values
   MsSlnFormatVersion = "11.00";
   StubSuffix = "_Stub";

   WTON::TokenFile cfgFile(cfgPath);
   classy->load(cfgFile, this);

#ifdef WTON_PC
   RootToProps.replace("/", "\\");
   SlnToRoot.replace("/", "\\");
   RootToSln.replace("/", "\\");
#endif

   for (int i = 0; i < AllPlatforms.len(); ++i)
   {
      const String& p = AllPlatforms[i];
      String& p2 = SolutionPlatform[p];

      // unspecified Platforms have the same Solution name as Project name
      if (p2.empty())
      {
         p2 = p;
      }
   }

   theConfig = this;
}
