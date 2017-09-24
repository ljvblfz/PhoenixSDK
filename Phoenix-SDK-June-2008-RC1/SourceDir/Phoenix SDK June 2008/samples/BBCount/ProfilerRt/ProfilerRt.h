#define PROFILERRT extern "C" __declspec(dllexport)

// The default name of profile log file.

#define DEFAULT_LOGFILE _T("bbcount.log")

// The environment variable name that indicates the name of log file.

#define BBC_LOGFILE _T("BBC_LOGFILE")

// The default name of the count file (# basic blocks).

#define DEFAULT_CNTFILE _T("bbcount.cnt")

// The environment variable name that indicates the name of count file.

#define BBC_CNTFILE _T("BBC_CNTFILE")

// The API that will be invoked by the instrumented code.

PROFILERRT void BBCount(unsigned int Id);

// Helper functions

void Initialize();
void DumpReport();
