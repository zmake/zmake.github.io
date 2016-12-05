// Copyright (c) 2014 - 2016 Roger Hanna
// For conditions of distribution and use, see copyright notice in ~/src/zmake/_zmake.h

#include "zmake/_zmake.h"

#ifndef WTON_PC
#define _stricmp strcasecmp
#define _chdir chdir
#define _mkdir(x) mkdir(x, 0777)
#include <sys/stat.h>
#include <unistd.h>
#else
#include <direct.h>
#endif
#include <sys/stat.h>

bool fileExists(const char* path)
{
   struct stat foo;
   int ret = stat(path, &foo);
   return ret == 0 ? true : false;
}

// possibly related to StrCmpLogicalW?
const char* kVisualStudioDisplayOrder = "'-!\\\"#$%()*,./?@[]^_`{}~+=>";
int kVisualStudioDisplayOrderLen = strlen(kVisualStudioDisplayOrder);
// TODO: this doesn't handle numbers "12" "1" correctly, but hey VS and explorer differ on "02" and "1"
int strCmpLogical(const char* a, const char* b)
{
   char ai,bi;
   while(*a)
   {
      ai = tolower(*a);
      bi = tolower(*b);

      if (ai != bi)
         break;

      a++;
      b++;
   }
   const char* posA = strchr(kVisualStudioDisplayOrder, ai);
   const char* posB = strchr(kVisualStudioDisplayOrder, bi);
   if (posA == NULL && posB == NULL)
   {
      return (unsigned char)ai - (unsigned char)bi;
   }
   else
   {
      if (posA == NULL)
      {
         posA = kVisualStudioDisplayOrder + kVisualStudioDisplayOrderLen;
      }
      else if (posB == NULL)
      {
         posB = kVisualStudioDisplayOrder + kVisualStudioDisplayOrderLen;
      }

      return (ptrdiff_t)posA - (ptrdiff_t)posB;
   }
}

bool projCmp(ProjectData* const & a, ProjectData* const & b)
{
   return strCmpLogical(a->mDisplayName.c_str(), b->mDisplayName.c_str()) < 0;
}

bool writeSln(String name, const Array<ProjectData*>& projData, const Array<String>& configs, const Array<String>& plats, bool forceSln)
{
   File fs(strCat(theConfig->RootToSln, name), forceSln ? File::kWrite : File::kWriteChanged);
   if (!fs.valid())
      return false;

   // header
   writeRaw(fs, 
      strCat("Microsoft Visual Studio Solution File, Format Version ", theConfig->MsSlnFormatVersion, "\r\n"
      "# generated zmake solution\r\n"));

   // projects
   for (int i = 0; i < projData.len(); ++i)
   {
      const ProjectData* pd = projData[i];

      String projPath(toString(pd->mPath));

      writeRaw(fs, "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"");
      writeRaw(fs, toString(pd->mDisplayName));
      writeRaw(fs, "\", \"");

      String relPath(strCat(".zproj\\", pd->vcxName()));
      relPath.replace("/", "\\");
      writeRaw(fs, relPath.c_str());
      writeRaw(fs, "\", \"{");
      writeRaw(fs, ProjectData::toGuid(pd->mDisplayName));
      writeRaw(fs, "}\"\r\n");
      if (!pd->Prebuild)
      {
         Array<const ProjectData*> deps;
         for (int j = 0; j < projData.len(); ++j)
         {
            const ProjectData* dpd = projData[j];
            if (dpd->Prebuild)
            {
               // all static libraries depend on Prebuild
               bool dependOnPrebuild = true;

               // exes with static libraries do not depend on Prebuild
               if (pd->Exe)
               {
                  for (int k = 0; k < projData.len(); ++k)
                  {
                     const ProjectData* kpd = projData[k];
                     if (kpd->Prebuild || kpd->Exe)
                        continue;
                     if (!pd->Dev && kpd->Dev)
                        continue;
                     dependOnPrebuild = false;
                     break;
                  }
               }

               if (dependOnPrebuild)
               {
                  deps.push_back(dpd);
               }
            }
            else if (!pd->Dev && dpd->Dev)
            {
               // never a dependency = non-conditional
            }
            // conditional static libraries need to be explicit dependencies
            else if (pd->Exe && !dpd->Exe && (dpd->Platforms.filled() && dpd->Configs.filled()))
            {
               Array<String> relevantPlatforms = pd->Platforms;
               ArrayUtl::intersectFast(relevantPlatforms, theConfig->MsvcPlatforms);
               Array<String> relevantConfigs = pd->Configs;

               bool allCondition = true;
               for (int qq = 0; qq < relevantPlatforms.len(); ++qq)
               {
                  if (!dpd->Platforms.contains(relevantPlatforms[qq]))
                  {
                     allCondition = false;
                     break;
                  }
               }
               for (int qq = 0; qq < relevantConfigs.len(); ++qq)
               {
                  if (!dpd->Configs.contains(relevantConfigs[qq]))
                  {
                     allCondition = false;
                     break;
                  }
               }

               if (!allCondition)
               {
                  deps.push_back(dpd);
               }
            }
         }
         if (deps.filled())
         {
            writeRaw(fs, "	ProjectSection(ProjectDependencies) = postProject\r\n");

            for (int j = 0; j < deps.len(); ++j)
            {
               writeRaw(fs, "		{");
               writeRaw(fs, ProjectData::toGuid(deps[j]->mDisplayName));
               writeRaw(fs, "} = {");
               writeRaw(fs, ProjectData::toGuid(deps[j]->mDisplayName));
               writeRaw(fs, "}\r\n");
            }

            writeRaw(fs, "	EndProjectSection\r\n");
         }
      }
      writeRaw(fs, "EndProject\r\n");
   }

   writeRaw(fs, 
      "Global\r\n"
      "	GlobalSection(SolutionConfigurationPlatforms) = preSolution\r\n");

   for (int k = 0; k < configs.len(); ++k)
   {
      String config = configs[k];
      for (int l = 0; l < plats.len(); ++l)
      {
         String plat = theConfig->SolutionPlatform[plats[l]];
         writeRaw(fs, strCat("		", config, "|", plat, " = ", config, "|", plat, "\r\n"));
      }
   }
   writeRaw(fs, 
      "	EndGlobalSection\r\n"
      "	GlobalSection(ProjectConfigurationPlatforms) = postSolution\r\n");

   for (int i = 0; i < projData.len(); ++i)
   {
      const ProjectData* pd = projData[i];

      String projPath(pd->mPath);
      String projName(FileMgr::base(toString(projPath)));
      String guid(ProjectData::toGuid(pd->mDisplayName));

      for (int j = 0; j < configs.len(); ++j)
      {
         String config = configs[j];
         bool validConfig = pd->Configs.contains(config);

         for (int q = 0; q < plats.len(); ++q)
         {
            String plat = plats[q];
            bool validPlat = pd->Platforms.contains(plat);

            bool valid = validConfig && validPlat;

            String rConfig = valid ? config : "None";
            String rPlat   = valid ? plat   : "None";
            String sPlat = theConfig->SolutionPlatform[plat];

            writeRaw(fs, strCat("		{", guid, "}.", config, "|", sPlat, ".ActiveCfg = ", rConfig, "|", rPlat, "\r\n"));
            if (valid)
            {
               writeRaw(fs, strCat("		{", guid, "}.", config, "|", sPlat, ".Build.0 = ",   rConfig, "|", rPlat, "\r\n"));
               if (pd->Exe)
               {
                  writeRaw(fs, strCat("		{", guid, "}.", config, "|", sPlat, ".Deploy.0 = ",  rConfig, "|", rPlat, "\r\n"));
               }
            }
         }
      }
   }

   writeRaw(fs, 
      "	EndGlobalSection\r\n"
      "	GlobalSection(SolutionProperties) = preSolution\r\n"
      "		HideSolutionNode = FALSE\r\n"
      "	EndGlobalSection\r\n"
      "EndGlobal\r\n");

   return fs.willWrite();
}

void enumerateProjects(Array<ProjectData*>& projData, Array<ProjectData*>& makeData)
{
   {
      Map<String, bool> projectNames;
      Array<String> projects;

      for (int i = theConfig->IncludePatterns.len() - 1; i >= 0; --i)
      {
         projects += FileMgr::enumerateDirs(theConfig->IncludePatterns[i].cStr());
      }

      // parse individual src/<project> and src/lib/<project> directories
      for (int i = 0; i < projects.len(); ++i)
      {
         String projPath(projects[i]);

         ProjectData* pd = new ProjectData(projPath);

         // skip ignored
         if (pd->Ignore)
         {
            delete pd;
            continue;
         }

         projData.push_back(pd);
         makeData.push_back(pd);
         bool& duplicateProject = projectNames[pd->mDisplayName];
         WTON_ASSERT(!duplicateProject, pd->mDisplayName);
         duplicateProject = true;
      }
   }

   ArrayUtl::sort(projData, projCmp);
}

void writeSlnProj(Array<ProjectData*>& projData, bool forceSln)
{
   // write the solution file
   {
      String appName(FileMgr::name(FileMgr::path(FileMgr::workDir())));
      writeSln(strCat(appName, ".sln"), projData, theConfig->AllConfigs, theConfig->MsvcPlatforms, forceSln);
   }

   // write them!
   for (int i = projData.len() - 1; i >= 0; --i)
   {
      ProjectData* pd = projData[i];

      File* vcx    = NULL;
      File* filter = NULL;
      pd->writeVcxproj(vcx, filter, projData);

      bool wasTest = !forceSln && (vcx->willWrite() || (filter && filter->willWrite()));

      if (wasTest)
      {
         vcx->invalidateWrite();
         if (filter)
            filter->invalidateWrite();
      }

      delete vcx;
      delete filter;

      if (wasTest)
      {
         // do the whole thing and force sln
         writeSlnProj(projData, true);
         return;
      }
   }
}


void mkDirRecurse(const char* ipath)
{
   if (streq(ipath, ".."))
      return;
   else if (streq(ipath, ""))
      return;

   String p(ipath);
   int pos = p.find_last_of('/');

   if (pos != String::npos)
   {
      String sub(p.substr(0, pos));
      mkDirRecurse(sub.cStr());
   }

   _mkdir(ipath);
}

void exampleMain();

int zmakeMain(int argc, char** argv)
{
   if (argc == 2 && streq(argv[1], "--version"))
   {
      WTON_PRINT(COPYRIGHT_NOTICE);
      return 0;
   }
   const char* globalConfigPath = argc > 1 ? argv[1] : "zmake-config.ztxt";
   if (!fileExists(globalConfigPath))
   {
      if (argc > 1)
      {
         WTON_PRINT("config file: '", globalConfigPath, "' not found");
      }
      else
      {
         WTON_PRINT("usage:  \"zmake --version\" OR \"zmake path-to/zmake-config.ztxt\"");
      }
      return 0;
   }

   // load the config data
   new GlobalConfig(globalConfigPath);

   // now change to the config dir, before we switch to code root
   if (argc > 1)
   {
      char* nwd = argv[1];
      // change to exe directory
      String str = nwd;
      str.replace("\\", "/");
      str = FileMgr::path(str.c_str());
      // cwd is the project dir (zmake/.zproj/...)
      // need to change into the zmake-config.ztxt directory,
      // which ideally is not the same as the zmake.exe directory
      _chdir(str.cStr());
   }

   // change to the code root
   WTON_ASSERT(_chdir(toString(theConfig->ZmakeToRoot)) == 0);

   // make the output dirs
   String outDir = strCat(theConfig->RootToSln, ".zproj/");
   outDir.replace("\\", "/");
   mkDirRecurse(outDir.cStr());

   Array<ProjectData*> projData;
   Array<ProjectData*> makeData;

   enumerateProjects(projData, makeData);

   bool doMake = true;
   bool doSln = true;
   if (argc > 2)
   {
      if (streq(argv[2], "--make"))
      {
         doSln = false;
      }
      else if (streq(argv[2], "--sln"))
      {
         doMake = false;
      }
   }

   if (doSln)
   {
      writeSlnProj(projData, false);
   }

   if (doMake)
   {
      for (int i = projData.len() - 1; i >= 0; --i)
      {
         ProjectData* pd = projData[i];
         pd->writeMake(makeData);
      }
   }

#ifdef ZM_DEBUG
   exampleMain();
#endif

   // never return an error value so that compilation continue
   return 0;
}
