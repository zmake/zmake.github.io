// Copyright (c) 2014 - 2016 Roger Hanna
// For conditions of distribution and use, see copyright notice in ~/src/zmake/_zmake.h

#include "zmake/_zmake.h"

// this is the byte order mark http://en.wikipedia.org/wiki/Byte_order_mark
#define kBomStr "\xEF\xBB\xBF"

const char* kMakeConfig = "zmake.ztxt";

// doesn't really matter as long as its between '_' and '~' so that it outputs app proj before lib
const char* kNoPrefix = "\x7c";

const char* ProjectData::toGuid(String guidHash)
{
   // TODO: a better collision resistant one-way hash?

   String hex(toHex(hashString(guidHash.cStr(), guidHash.len())));
   hex.toUpper();
   int len = hex.len();
   hex = String(strCat(zeroSpace(16 - len), hex));

   String hexA(hex.substr(0, 4));
   String hexB(hex.substr(4, 4));
   String hexC(hex.substr(8, 4));
   String hexD(hex.substr(12, 4));
   return strCat(strCat(hexA, hexB, "-", hexC, "-", hexD), "-", hexA, "-", hexB, hexC, hexD);
}

void stripOmitted(Array<String>& other, const Array<String>& projIgnore)
{
   for (int qq = other.len() - 1; qq >= 0; --qq)
   {
      const char* path = other[qq].cStr();
      String sName = FileMgr::name(path);
      const char* name = sName.cStr();

      for (int i = 1; i >= 0; --i)
      {
         const Array<String>& ignore = i ? projIgnore : theConfig->IgnorePatterns;
         for (int k = ignore.len() - 1; k >= 0; --k)
         {
            const char* ignoreK = ignore[k].cStr();
            if (FileMgr::matchesPattern(name, ignoreK) || strstr(path, ignoreK))
            {
               other.fastRemoveIdx(qq);

               // done!
               k = -1;
               i = -1;
            }
         }
      }
   }
}

// returns true if it has stuff
bool recurseSubdirs(const char* idir, Array<String>& other, ProjectData* pd)
{
   Array<String> subs = FileMgr::enumerateDirs(idir);

   bool hasStuff = false;

   for (int qq = 0; qq < subs.len(); ++qq)
   {
      String dir = subs[qq];

      String cfgHash = strCat(dir, kMakeConfig);
      if (FileMgr::exists(cfgHash))
      {
         ProjectData* pdc = new ProjectData(dir);

         if (pdc->Ignore)
         {
            delete pdc;
         }
         else
         {
            pd->mChildProjects.push_back(pdc);
         }
      }
      else
      {
         Array<String> files = FileMgr::enumerate(strCat(dir, "*"));
         stripOmitted(files, pd->IgnorePatterns);
         other += files;

         bool subHasStuff = recurseSubdirs(strCat(dir, "*"), other, pd) || !files.empty();

         if (subHasStuff)
         {
            pd->mSubdirs += dir;
            hasStuff = true;
         }
      }
   }

   return hasStuff;
}

ProjectData::ProjectData(String projPath)
{
   WTON_ASSERT(projPath.filled());

   Dev = false;
   Platforms = theConfig->AllPlatforms;
   mLib = false;
   Exe = false;
   Prebuild = false;
   Dll = false;
   mHasCode = false;
   Configs = theConfig->AllConfigs;
   Prefix = kNoPrefix;
   mPath = projPath;

   String cfg = strCat(FileMgr::workDir(), projPath, kMakeConfig);
   if (FileMgr::exists(cfg))
   {
      Ignore = false;

      WTON::TokenFile tfs(cfg);
      TKN()->load(tfs, this);

      // Dll is now a specialization of Exe since both are linked
      if (Dll)
         Exe = true;

      if (Ignore)
         return;
   }
   else
   {
      Ignore = true;
   }

   AdditionalFileProps += theConfig->AdditionalFileProps;

   if (projPath.contains(theConfig->LibPattern.cStr()))
   {
      mLib = true;
      if (Prefix == kNoPrefix)
      {
         Prefix = "~";
      }
   }
   else if (Exe)
   {
      if (Prefix == kNoPrefix)
      {
         Prefix = "#";
      }
   }
   else
   {
      if (Prefix == kNoPrefix)
      {
         Prefix = ".";
      }
   }

   mRootName = FileMgr::name(toString(projPath));

   // get all files
   mOther = FileMgr::enumerate(strCat(projPath, "*"));
   stripOmitted(mOther, IgnorePatterns);

   recurseSubdirs(strCat(mPath, "*"), mOther, this);

   // no files? no project
   if (mOther.empty())
   {
      Ignore = true;
      return;
   }

   // dole them out to their respective type (mCode / mOther)
   Map<String, String> namePaths;
   for (int qq = mOther.len() - 1; qq >= 0; --qq)
   {
      const String& s = mOther[qq];

      String ext(FileMgr::extension(toString(s)));
      if (theConfig->CompilationUnitExtensions.contains(ext))
      {
         String uname(FileMgr::base(s.c_str()));
         String& duplicateUnitName = namePaths[uname];
         WTON_ASSERT(duplicateUnitName.empty(), duplicateUnitName, " and ", s, " cannot both exist");
         duplicateUnitName = s;

         mCode += s;
         mOther.fastRemoveIdx(qq);
      }
   }

   ArrayUtl::sort(mCode);
   ArrayUtl::sort(mOther);

   mHasCode = !mCode.empty();
   if (!mHasCode)
   {
      for (int i = 0; i < mChildProjects.len(); ++i)
      {
         if (mChildProjects[i]->mHasCode)
         {
            mHasCode = true;
            break;
         }
      }
   }

   // no explicit config
   if (Ignore)
   {
      // no code? implies no build configurations
      if (!mHasCode)
      {
         Platforms.clear();
         Configs.clear();

         // has to have the default prefix from above
         WTON_ASSERT(Prefix == ".");
         Prefix = "";
      }
      Ignore = false;
   }

   if (Prefix != kNoPrefix && Prefix.filled())
   {
      mDisplayName = strCat(Prefix, " ", mRootName);
   }
   else
   {
      mDisplayName = mRootName;
   }

   // TODO: warn about Prefix containing "&;"
   // TODO: warn about Prefix or dirname containing "\/:*?\"<>|" (can't write lastbuildstate)
}

void fileExtPlats(Array<String>& plats, const String& base)
{
   Map<String, Array<String> >::const_iterator itc;
   for (itc = theConfig->SuffixPlatforms.begin(); itc != theConfig->SuffixPlatforms.end(); ++itc)
   {
      if (base.endsWith(toString(itc->first)))
      {
         plats = itc->second;
         break;
      }
   }
}

void filePlatOverride(Array<String>& plats, const String& base, const String& fullPath, const Array<String>& files)
{
   const Array<String> startPlats = plats;

   if (base.endsWith(toString(theConfig->StubSuffix)))
   {
      // find all other extensions that match this file
      String path = FileMgr::path(toString(fullPath));
      path += base.substr(0, base.len() - theConfig->StubSuffix.len());
      Array<String> otherPlats;
      Array<String> temp;
      for (int i = 0; i < files.len(); ++i)
      {
         const String& tpath = files[i];
         if (tpath.startsWith(toString(path)))
         {
            if (tpath == fullPath)
               continue;

            String tbase = FileMgr::base(toString(tpath));
            temp.clear();
            fileExtPlats(temp, tbase);
            otherPlats += temp;
         }
      }

      plats.clear();
      // a stub that's not replaced on some platform isn't a stub
      if (otherPlats.filled())
      {
         for (int i = 0; i < theConfig->AllPlatforms.len(); ++i)
         {
            const String& pp = theConfig->AllPlatforms[i];
            if (!otherPlats.contains(pp))
            {
               plats.push_back(pp);
            }
         }
      }

      ArrayUtl::intersectFast(plats, startPlats);
      return;
   }

   fileExtPlats(plats, base);
   ArrayUtl::intersectFast(plats, startPlats);
}

void writeGroup(File& fs, ProjectData* root, ProjectData* active, const Array<String>& files, const char* filter)
{
   Array<String> basePlats = active->Platforms;
   ArrayUtl::intersectFast(basePlats, theConfig->MsvcPlatforms);
   const Array<String>& baseConfigs = active->Configs;

   bool hasCpp = false;
   bool hasC = false;
   // TODO: this doesn't cover the case of a child project mismatching its parent cpp/c type
   for (int j = 0; j < files.len(); ++j)
   {
      if (files[j].endsWith(".c"))
      {
         hasC = true;
      }
      if (files[j].endsWith(".cpp"))
      {
         hasCpp = true;
      }
   }
   bool mixedCode = hasC && hasCpp;

   if (files.filled())
   {
      writeRaw(fs, "  <ItemGroup>\r\n");
      for (int j = 0; j < files.len(); ++j)
      {
         String filePath(toString(files[j]));
         const char* rtype;
         // TODO: zmake-config.ztxt parameter that maps extensions -> vcxproj type, that way other types (cxx, lib, etc) are handled on a per-sln basis
         String ext(FileMgr::extension(toString(filePath)));
         const String& extItem = theConfig->ExtensionItem[ext];
         if (extItem.filled())
            rtype = extItem.c_str();
         else
            rtype = "None";

         const bool code = streq(rtype, "ClCompile") || streq(rtype, "Library") || streq(rtype, "ResourceCompile");

         String base(FileMgr::base(filePath.cStr()));
         String relPath(FileMgr::relativePath(toString(files[j]), toString(root->mPath)));
         String parentPath(FileMgr::path(relPath.cStr()));
         relPath.replace("/", "\\");

         String modifiers;
         if (code && !filter)
         {
            Array<String> plats = basePlats;
            if (FileMgr::matchesPattern(base.cStr(), toString(theConfig->PchPattern)))
            {
               modifiers += "      <PrecompiledHeader>Create</PrecompiledHeader>\r\n";
            }

            // which platforms do we need to explicitly block it on
            filePlatOverride(plats, base, filePath, files);

            Array<String> excludedPlats;
            int vcxProjPlats = 0;
            for (int qq = 0; qq < theConfig->MsvcPlatforms.len(); ++qq)
            {
               // if the root project doesn't support it, we don't need to explicitly exclude it
               if (!root->Platforms.contains(theConfig->MsvcPlatforms[qq]))
               {
                  continue;
               }
               ++vcxProjPlats;
               if (!plats.contains(theConfig->MsvcPlatforms[qq]))
               {
                  excludedPlats.push_back(theConfig->MsvcPlatforms[qq]);
               }
            }

            Array<String> excludedConfigs;
            for (int qq = 0; qq < theConfig->AllConfigs.len(); ++qq)
            {
               // if the root project doesn't support it, we don't need to explicitly exclude it
               if (!root->Configs.contains(theConfig->AllConfigs[qq]))
               {
                  continue;
               }
               if (!baseConfigs.contains(theConfig->AllConfigs[qq]))
               {
                  excludedConfigs.push_back(theConfig->AllConfigs[qq]);
               }
            }

            if (excludedPlats.len() == vcxProjPlats)
            {
               if (vcxProjPlats != 0)
               {
                  modifiers += "      <ExcludedFromBuild>true</ExcludedFromBuild>\r\n";
               }
            }
            else
            {
               if (excludedPlats.filled())
               {
                  if (plats.len() == 1)
                  {
                     modifiers += strCat("      <ExcludedFromBuild Condition=\"'$(Platform)'!='", plats[0], "'\">true</ExcludedFromBuild>\r\n");
                  }
                  else
                  {
                     // fall back to exhaustive excluded list
                     for (int q = 0; q < excludedPlats.len(); ++q)
                     {
                        modifiers += strCat("      <ExcludedFromBuild Condition=\"'$(Platform)'=='", excludedPlats[q], "'\">true</ExcludedFromBuild>\r\n");
                     }
                  }
               }
               if (excludedConfigs.filled())
               {
                  // fall back to exhaustive excluded list
                  for (int q = 0; q < excludedConfigs.len(); ++q)
                  {
                     modifiers += strCat("      <ExcludedFromBuild Condition=\"'$(Configuration)'=='", excludedConfigs[q], "'\">true</ExcludedFromBuild>\r\n");
                  }
               }
            }
            if (mixedCode && filePath.endsWith(".c"))
            {
               modifiers += "      <PrecompiledHeader>NotUsing</PrecompiledHeader>\r\n";
            }

            if (root->AdditionalFileProps.filled())
            {
               // pairs people!
               WTON_ASSERT(root->AdditionalFileProps.len() % 2 == 0);
               String name(FileMgr::name(filePath.c_str()));
               for (int i = 0; i < root->AdditionalFileProps.len(); i += 2)
               {
                  if (name == root->AdditionalFileProps[i])
                  {
                     modifiers += root->AdditionalFileProps[i+1];
                     modifiers += "\r\n";
                  }
               }
            }
         }

         if (filter)
         {
            const char* origFilter = filter;
            while (parentPath.startsWith("../"))
            {
               parentPath = parentPath.substr(3);
            }
            if (parentPath.filled() && streq(filter, " "))
            {
               parentPath.replace("/", "\\");
               if (parentPath.endsWith("\\"))
               {
                  parentPath = parentPath.substr(0, parentPath.len() - 1);
               }
               filter = parentPath.cStr();
            }

            if (!streq(filter, " "))
            {
               modifiers += strCat("      <Filter>", filter, "</Filter>\r\n");
            }
            filter = origFilter;
         }

         if (modifiers.empty())
         {
            writeRaw(fs, strCat("    <", rtype, " Include=\"$(ZmakeProjRoot)\\", relPath, "\" />\r\n"));
         }
         else
         {
            writeRaw(fs, strCat("    <", rtype, " Include=\"$(ZmakeProjRoot)\\", relPath, "\">\r\n"));
            writeRaw(fs, modifiers.cStr());
            writeRaw(fs, strCat("    </", rtype, ">\r\n"));
         }
      }
      writeRaw(fs, "  </ItemGroup>\r\n");
   }
}

const char* simplifyPath(const char* iPath)
{
   char *buf, *dst, *end;
   dst = buf = WTON::strBuf();
   end = buf + WTON::kMaxStrCatLen - 1;
   dst = WTON::_strCpy(dst, iPath, end);
   *end = 0;

   // convert all / to \\ for MSVC
   dst = buf;
   while (*dst)
   {
      if (*dst == '/')
      {
         *dst = '\\';
      }
      ++dst;
   }

   // eliminate as many "../"s and "./"'s as possible
   dst = buf;

   Array<char*> validDirs;
   char* cur = dst;
   while (*cur)
   {
      if (*cur == '\\')
      {
         // is it an updir?
         if (cur[1] == '.' && cur[2] == '.' && (cur[3] == '\\' || cur[3] == 0))
         {
            if (validDirs.filled())
            {
               dst = validDirs.back();
               validDirs.pop_back();
               cur += 3;

               // we have to continue because the next dir segment may be another updir/nopdir
               continue;
            }
         }
         // is it a noop dir?
         else if (cur[1] == '.' && (cur[2] == '\\' || cur[2] == 0))
         {
            cur += 2;

            // we have to continue because the next dir segment may be another updir/nopdir
            continue;
         }
         // if not, it's a valid dir
         else
         {
            validDirs.push_back(cur);
         }
      }

      // memmove those characters!
      *dst = *cur;

      ++dst;
      ++cur;
   }

   *dst = 0;

   WTON::finishStrBuf(buf);
   return buf;
}

void ProjectData::writeVcxproj(File*& outVcx, File*& outFilter, Array<ProjectData*>& projData)
{
   const Array<String>& code  = mCode;

   bool trivialProj = code.empty() && !Exe;

   //PRINT("Generating ", mDisplayName, " at ", mPath);

   // write the project's vcxproj property sheet
   outVcx = new File(strCat(theConfig->RootToSln, ".zproj/", vcxName()), File::kWriteChanged);
   if (outVcx->valid())
   {
      File& fs = *outVcx;

      // vcxproj header
      writeRaw(fs, 
         kBomStr
         "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
         "<Project DefaultTargets=\"Build\" ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\r\n"
         "  <ItemGroup Label=\"ProjectConfigurations\">\r\n");

      // config|plat combinatoric
      bool noVcxPlat = true;
      for (int q = 0; q < Platforms.len(); ++q)
      {
         String plat = Platforms[q];
         if (!theConfig->MsvcPlatforms.contains(plat))
            continue;

         noVcxPlat = false;
         for (int j = 0; j < Configs.len(); ++j)
         {
            String config = Configs[j];

            writeRaw(fs, strCat("    <ProjectConfiguration Include=\"", config, "|", plat, "\">\r\n"));
            writeRaw(fs, strCat("      <Configuration>", config, "</Configuration>\r\n"));
            writeRaw(fs, strCat("      <Platform>", plat, "</Platform>\r\n"));
            writeRaw(fs, "    </ProjectConfiguration>\r\n");
         }
      }
      if (noVcxPlat)
      {
         trivialProj = true;

         String plat = "Win32";
         String config = "Debug";
         writeRaw(fs, strCat("    <ProjectConfiguration Include=\"", config, "|", plat, "\">\r\n"));
         writeRaw(fs, strCat("      <Configuration>", config, "</Configuration>\r\n"));
         writeRaw(fs, strCat("      <Platform>", plat, "</Platform>\r\n"));
         writeRaw(fs, "    </ProjectConfiguration>\r\n");
      }
      writeRaw(fs, "  </ItemGroup>\r\n");

      // project identifiers
      writeRaw(fs, "  <PropertyGroup Label=\"Globals\">\r\n");

      writeRaw(fs, strCat("    <ProjectGuid>{", toGuid(mDisplayName), "}</ProjectGuid>\r\n"));
      writeRaw(fs, strCat("    <RootNamespace>", mDisplayName, "</RootNamespace>\r\n"));
      writeRaw(fs, strCat("    <ZmakeRootName>", mRootName, "</ZmakeRootName>\r\n"));
      writeRaw(fs, strCat("    <ZmakeProjRoot>..\\", simplifyPath(strCat(theConfig->SlnToRoot, mPath.substr(0, mPath.len() - 1))), "</ZmakeProjRoot>\r\n"));

      writeRaw(fs, "  </PropertyGroup>\r\n");

      // property sheets
      const char* projProp;
      if (Dll)
      {
         projProp = "zmake_dll";
      }
      else if (Exe)
      {
         projProp = "zmake_exe";
      }
      else
      {
         projProp = "zmake_lib";
      }

      writeRaw(fs, strCat("  <Import Project=\"$(SolutionDir)", simplifyPath(strCat(theConfig->SlnToRoot, theConfig->RootToProps)), projProp, ".props\" />\r\n"));

      writeRaw(fs, "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />\r\n");
      writeRaw(fs, "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />\r\n");

      writeRaw(fs, strCat("  <Import Project=\"$(SolutionDir)", simplifyPath(strCat(theConfig->SlnToRoot, theConfig->RootToProps)), "zmake_base.props\" />\r\n"));

      if (PrecompiledHeader.filled())
      {
         writeRaw(fs, "  <ItemDefinitionGroup>\r\n");
         writeRaw(fs, "    <ClCompile>\r\n");
         writeRaw(fs, strCat("      <PrecompiledHeaderFile>", PrecompiledHeader, "</PrecompiledHeaderFile>\r\n"));
         writeRaw(fs, "    </ClCompile>\r\n");
         writeRaw(fs, "  </ItemDefinitionGroup>\r\n");
      }

      if (AdditionalProps.filled())
      {
         writeRaw(fs, strCat(AdditionalProps.cStr(), "\r\n"));
      }

      writeGroup(fs, this, this, mCode, NULL);
      for (int q = 0; q < mChildProjects.len(); ++q)
      {
         ProjectData* pdd = mChildProjects[q];
         writeGroup(fs, this, pdd, pdd->mCode, NULL);
      }

      writeGroup(fs, this, this, mOther, NULL);
      for (int q = 0; q < mChildProjects.len(); ++q)
      {
         ProjectData* pdd = mChildProjects[q];
         writeGroup(fs, this, pdd, pdd->mOther, NULL);
      }

      if (Exe)
      {
         Array<String> relevantPlatforms = Platforms;
         ArrayUtl::intersectFast(relevantPlatforms, theConfig->MsvcPlatforms);
         Array<String> relevantConfigs = Configs;

         String prevGroupCondition;
         // write app project references
         for (int q = 0; q < projData.len(); ++q)
         {
            ProjectData* dpd = projData[q];

            if (dpd->Exe || dpd->Prebuild)
               continue;

            if (dpd->Dev && !Dev)
               continue;

            bool allPlat   = true;
            bool allConfig = true;
            // is it in all relevant platforms and configurations?
            for (int qq = 0; qq < relevantPlatforms.len(); ++qq)
            {
               if (!dpd->Platforms.contains(relevantPlatforms[qq]))
               {
                  allPlat = false;
                  break;
               }
            }

            for (int qq = 0; qq < relevantConfigs.len(); ++qq)
            {
               if (!dpd->Configs.contains(relevantConfigs[qq]))
               {
                  allConfig = false;
                  break;
               }
            }

            String groupCondition;
            if (allPlat && allConfig)
            {
               groupCondition = "  <ItemGroup>\r\n";
            }
            else
            {
               Array<String> plats = dpd->Platforms;
               ArrayUtl::intersectFast(plats, relevantPlatforms);
               Array<String> configs = dpd->Configs;
               ArrayUtl::intersectFast(configs, relevantConfigs);

               String s;
               if (allPlat)
               {
                  for (int i = 0; i < configs.len(); ++i)
                  {
                     if (s.filled())
                     {
                        s += " Or ";
                     }
                     s += strCat("'$(Configuration)'=='", configs[i], "'");
                  }
               }
               else if (allConfig)
               {
                  for (int i = 0; i < plats.len(); ++i)
                  {
                     if (s.filled())
                     {
                        s += " Or ";
                     }
                     s += strCat("'$(Platform)'=='", plats[i], "'");
                  }
               }
               else
               {
                  for (int i = 0; i < plats.len(); ++i)
                  {
                     for (int j = 0; j < configs.len(); ++j)
                     {
                        if (s.filled())
                        {
                           s += " Or ";
                        }
                        s += strCat("'$(Platform)|$(Configuration)'=='", plats[i], "|", configs[j], "'");
                     }
                  }
               }
               if (s.empty())
                  continue;

               groupCondition = strCat("  <ItemGroup Condition=\"", s, "\">\r\n");
            }

            if (groupCondition != prevGroupCondition)
            {
               if (prevGroupCondition.filled())
               {
                  writeRaw(fs, "  </ItemGroup>\r\n");
               }
               prevGroupCondition = groupCondition;
               writeRaw(fs, groupCondition.c_str());
            }
            writeRaw(fs, strCat("    <ProjectReference Include=\"", dpd->vcxName(), "\">\r\n"));
            writeRaw(fs, strCat("      <Project>{", toGuid(dpd->mDisplayName), "}</Project>\r\n"));
            writeRaw(fs, "      <ReferenceOutputAssembly>false</ReferenceOutputAssembly>\r\n");
            writeRaw(fs, "    </ProjectReference>\r\n");
         }

         if (prevGroupCondition.filled())
         {
            writeRaw(fs, "  </ItemGroup>\r\n");
         }
      }

      // write footer
      writeRaw(fs, "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />\r\n");
      writeRaw(fs, "  <ImportGroup Label=\"ExtensionTargets\">\r\n");
      writeRaw(fs, "  </ImportGroup>\r\n");
      writeRaw(fs, "</Project>");
   }

   // write the filters file (if it needs it)
   bool multipleFilters = mChildProjects.filled() || mSubdirs.filled();
   if (multipleFilters)
   {
      outFilter = new File(strCat(theConfig->RootToSln, ".zproj/", vcxName(), ".filters"), File::kWriteChanged);
      if (outFilter->valid())
      {
         File& fs = *outFilter;

         // filter header
         writeRaw(fs, 
            kBomStr
            "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
            "<Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\r\n");

         {
            writeRaw(fs, "  <ItemGroup>\r\n");
            for (int q = 0; q < mChildProjects.len(); ++q)
            {
               ProjectData* pdd = mChildProjects[q];
               writeRaw(fs, strCat(
                  "    <Filter Include=\"", pdd->mRootName, "\">\r\n"
                  "    </Filter>\r\n"));
            }
            for (int q = 0; q < mSubdirs.len(); ++q)
            {
               String relPath(FileMgr::relativePath(toString(mSubdirs[q]), toString(mPath)));
               relPath.replace("/", "\\");
               if (relPath.endsWith("\\"))
               {
                  relPath = relPath.substr(0, relPath.len() - 1);
               }
               writeRaw(fs, strCat(
                  "    <Filter Include=\"", relPath, "\">\r\n"
                  "    </Filter>\r\n"));
            }
            writeRaw(fs, "  </ItemGroup>\r\n");
         }

         writeGroup(fs, this, this, mCode, " ");
         for (int q = 0; q < mChildProjects.len(); ++q)
         {
            ProjectData* pdd = mChildProjects[q];
            String filter(pdd->mRootName);
            writeGroup(fs, this, pdd, pdd->mCode, filter.cStr());
         }
         writeGroup(fs, this, this, mOther, " ");
         for (int q = 0; q < mChildProjects.len(); ++q)
         {
            ProjectData* pdd = mChildProjects[q];
            String filter(pdd->mRootName);
            writeGroup(fs, this, pdd, pdd->mOther, filter.cStr());
         }

         writeRaw(fs, "</Project>");
      }
   }
}

WTON::ClassNotation* ProjectData::TKN()
{
   static WTON::ClassNotation* classy = NULL;

   if (!classy)
   {
      classy = new WTON::ClassNotation();
      typedef ProjectData ActiveClass;

      WTON_MEMBER(Prefix);
      WTON_MEMBER(Exe);
      WTON_MEMBER(Dev);
      WTON_MEMBER(Prebuild);
      WTON_MEMBER(Dll);
      WTON_MEMBER(Ignore);
      WTON_MEMBER(Platforms);
      WTON_MEMBER(Configs);
      WTON_MEMBER(AdditionalProps);
      WTON_MEMBER(AdditionalMake);
      WTON_MEMBER(AdditionalFileProps);
      WTON_MEMBER(PrecompiledHeader);
      WTON_MEMBER(IgnorePatterns);
   }

   return classy;
}

struct FileData
{
   FileData(const String& iBase, const Array<String>& iPlats, const Array<String>& iConfigs)
      : base(iBase)
      , plats(iPlats)
      , configs(iConfigs)
   {
   }
   String base;
   Array<String> plats;
   Array<String> configs;
};

bool condSort(FileData const & a, FileData const & b)
{
   {
      int aCount = a.plats.len();
      int bCount = b.plats.len();
      if (aCount != bCount)
         return aCount < bCount;

      for (int i = 0; i < aCount; ++i)
      {
         int iCmp = strcmp(a.plats[i].c_str(), b.plats[i].c_str());
         if (iCmp != 0)
            return iCmp < 0;
      }
   }

   {
      int aConfigCount = a.configs.len();
      int bConfigCount = b.configs.len();
      if (aConfigCount != bConfigCount)
         return aConfigCount < bConfigCount;
      for (int i = 0; i < aConfigCount; ++i)
      {
         int iCmp = strcmp(a.configs[i].c_str(), b.configs[i].c_str());
         if (iCmp != 0)
            return iCmp < 0;
      }
   }

   return b.base < a.base;
}

void writeMakeGroup(File& fs, ProjectData* root, ProjectData* active, const Array<String>& files)
{
   Array<String> basePlats = active->Platforms;
   ArrayUtl::intersectFast(basePlats, theConfig->MakePlatforms);

   Array<FileData> condFiles;

   String lastConditional = "nan";
   for (int j = files.len() - 1; j >= 0; --j)
   {
      String filePath(toString(files[j]));
      String basePath(FileMgr::basepath(FileMgr::relativePath(filePath.c_str(), toString(root->mPath))));
      String base(FileMgr::base(basePath.c_str()));
      if (FileMgr::matchesPattern(base.cStr(), toString(theConfig->PchPattern)))
      {
         // no pch cpp for gcc/clang (they do it directly on the header)
         continue;
      }

      String ext(FileMgr::extension(toString(files[j])));
      if (!theConfig->CompilationUnitExtensions.contains(ext))
         continue;

      Array<String> plats = basePlats;
      // which platforms do we need to explicitly block it on
      filePlatOverride(plats, base, filePath, files);

      if (plats.empty())
         continue;

      condFiles.push_back(FileData(basePath, plats, active->Configs));
   }

   // count our make plats
   int makePlats = 0;
   for (int qq = 0; qq < theConfig->MakePlatforms.len(); ++qq)
   {
      // if the root project doesn't support it, we don't need to explicitly exclude it
      if (!root->Platforms.contains(theConfig->MakePlatforms[qq]))
      {
         continue;
      }
      ++makePlats;
   }

   // sort our files
   ArrayUtl::sort(condFiles, condSort);

   for (int j = condFiles.len() - 1; j >= 0; --j)
   {
      const Array<String>& plats   = condFiles[j].plats;
      const Array<String>& configs = condFiles[j].configs;

      const char* conditional = "";
      if (plats.len() != makePlats)
      {
         if (plats.len() == 1)
         {
            String low = plats[0];
            low.toLower();
            conditional = strCat("ifeq ($(PLATFORM),", low, ")\n");
         }
         else
         {
            //TODO("make multi-platform conditional");
         }
      }

      if (configs.len() != theConfig->AllConfigs.len())
      {
         if (configs.len() == 1)
         {
            String low = configs[0];
            low.toLower();
            conditional = strCat("ifeq ($(CONFIG),", low, ")\n");
         }
         else
         {
            conditional = "OR_COND = false\n";
            for (int i = 0; i < configs.len(); ++i)
            {
               String low = configs[i];
               low.toLower();
               conditional = strCat(conditional, "ifeq ($(CONFIG),", low, ")\nOR_COND = true\nendif\n");

               //TODO("make config+plat cases)
            }
            conditional = strCat(conditional, "ifeq ($(OR_COND),true)\n");
         }
      }

      bool sameCond = lastConditional == conditional;
      if (!sameCond)
      {
         writeRaw(fs, "\n");
         if (lastConditional.filled() && lastConditional != "nan")
         {
            writeRaw(fs, "endif\n");
         }
         writeRaw(fs, conditional);
         writeRaw(fs, "SRC += \\\n");
         lastConditional = conditional;
      }

      writeRaw(fs, "   ");
      writeRaw(fs, condFiles[j].base.cStr());
      writeRaw(fs, " \\\n");
   }

   if (lastConditional.filled() && lastConditional != "nan")
   {
      writeRaw(fs, "\n");
      writeRaw(fs, "endif\n");
   }
}

void ProjectData::writeMake(const Array<ProjectData*>& makeData)
{
   if (!mHasCode)
      return;

   bool needMake = false;
   for (int q = 0; q < theConfig->MakePlatforms.len(); ++q)
   {
      if (Platforms.contains(theConfig->MakePlatforms[q]))
      {
         needMake = true;
         break;
      }
   }

   // TODO platform specific .mk files
   if (!needMake)
      return;

   String projPath(toString(mPath));

   File fs(strCat(projPath, "Makefile"), File::kWriteChanged);

   writeRaw(fs, strCat("# Generated by ", ZMAKE_VERSION, "\n\n"));

   String relPath(FileMgr::relativePath("zbuild", toString(mPath)));
   writeRaw(fs, strCat("ZBUILD = ", relPath, "\n"));
   writeRaw(fs, strCat("include $(ZBUILD)/zmake_top.mk\n\n"));

   writeRaw(fs, strCat("PCH = ", theConfig->PchPrefix, mRootName, ".h\n\n"));

   writeMakeGroup(fs, this, this, mCode);
   for (int q = 0; q < mChildProjects.len(); ++q)
   {
      ProjectData* pdd = mChildProjects[q];
      writeMakeGroup(fs, this, pdd, pdd->mCode);
   }
   writeRaw(fs, "\n");

   if (Exe)
   {
      Array<String> baseConfigs = Configs;
      ArrayUtl::intersectFast(baseConfigs, theConfig->AllConfigs);
      String lastConditional = "nan";

      for (int j = 0; j < makeData.len(); ++j)
      {
         ProjectData* ppd = makeData[j];
         if (ppd->Exe || !ppd->mHasCode)
         {
            continue;
         }
         if (ppd->Dev && !Dev)
         {
            continue;
         }

         Array<String> ppdConfigs = ppd->Configs;
         ArrayUtl::intersectFast(ppdConfigs, baseConfigs);

         const char* conditional = "";
         if (ppdConfigs.len() != baseConfigs.len())
         {
            conditional = "OR_COND = false\n";
            for (int i = 0; i < ppdConfigs.len(); ++i)
            {
               String low = ppdConfigs[i];
               low.toLower();
               conditional = strCat(conditional, "ifeq ($(CONFIG),", low, ")\nOR_COND = true\nendif\n");
            }
            conditional = strCat(conditional, "ifeq ($(OR_COND),true)\n");
         }

         bool sameCond = lastConditional == conditional;
         if (!sameCond)
         {
            writeRaw(fs, "\n");
            if (lastConditional.filled() && lastConditional != "nan")
            {
               writeRaw(fs, "endif\n");
            }
            writeRaw(fs, conditional);
            writeRaw(fs, "PROJS += \\\n");
            lastConditional = conditional;
         }

         writeRaw(fs, strCat("   ", ppd->mPath.substr(0, ppd->mPath.len() - 1), " \\\n"));
      }
      writeRaw(fs, "\n");
   }

   if (AdditionalMake.filled())
   {
      writeRaw(fs, strCat(AdditionalMake.cStr(), "\n\n"));
   }

   // TODO: this needs to be relative to the path
   writeRaw(fs, strCat("include $(ZBUILD)/zmake_base.mk\n"));
}
