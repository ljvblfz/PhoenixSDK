// ProfilerRt.cpp : Defines the entry point for the DLL application.

#include <stdio.h>
#include <stdlib.h>
#include "stdafx.h"
#include "ProfilerRt.h"

using namespace std;

#include <vector>

static vector<unsigned int> bbcount;

BOOL APIENTRY
DllMain
(
   HANDLE hModule,
   DWORD  ul_reason_for_call,
   LPVOID lpReserved
)
{
   switch (ul_reason_for_call)
   {
      case DLL_PROCESS_ATTACH:

         // HERE: Initialize profiling data structure here.

         Initialize();
         break;

      case DLL_THREAD_ATTACH:

         break;

      case DLL_THREAD_DETACH:

         break;

      case DLL_PROCESS_DETACH:

         // HERE: dump out the profiling data.

         DumpReport();
         break;
   }
   return TRUE;
}

void
Initialize()
{
}

// Increment the counter of one specified basic block.

// id: the unique id of a basic block that is assigned during
// instrumentation.

PROFILERRT void
BBCount
(
   unsigned int id
)
{
   if (bbcount.size() <= id)
   {
      bbcount.resize(id + 1);
   }

   ::InterlockedIncrement((long *) &bbcount[id]); // syncronized access
}

void
DumpReport()
{
   // Check whether the output file name is specified by an environment
   // variable. If not, use the default one: "ProfileRt.out"

   TCHAR logFileName[_MAX_PATH] = DEFAULT_LOGFILE;

   ::GetEnvironmentVariable(BBC_LOGFILE, logFileName, sizeof(logFileName)
      / sizeof(logFileName[0]));

   // Create the output file for dumping profiling data.

   FILE *log = 0;
   ::fopen_s(&log, logFileName, "w");
   if (log)
   {
      for (size_t i = 0; i < bbcount.size(); i++)
      {
         ::fprintf(log, "%d\t%d\n", i, bbcount[i]);
      }
      ::fclose(log);
   }
   else
   {
      char const *ARGV0 = "ProfilerRt";

      ::fprintf(stderr, "%s: Unable to open log %s!\n", ARGV0, logFileName);
      ::perror(ARGV0);
      ::exit(-1);
   }
}
