//====================================================================
//	Win32PsApi.h
//--------------------------------------------------------------------
//
//	Package    : System (The LHCb System service)
//
//  Description: Definition of Systems internals for WIN32
//               This file is NOT meant to be used outside this
//               sub-package!
//
//	Author     : M.Frank
//  Created    : 13/11/00
//	Changes    : 
//====================================================================
#ifndef GAUDI_SYSTEM_WIN32PSAPI_H
#define GAUDI_SYSTEM_WIN32PSAPI_H
#ifdef _WIN32
    typedef struct _MODULEINFO {
      LPVOID lpBaseOfDll;
      DWORD SizeOfImage;
      LPVOID EntryPoint;
    } MODULEINFO, *LPMODULEINFO;
    typedef struct _PSAPI_WS_WATCH_INFORMATION {
      LPVOID FaultingPc;
      LPVOID FaultingVa;
    } PSAPI_WS_WATCH_INFORMATION, *PPSAPI_WS_WATCH_INFORMATION;
    typedef struct _PROCESS_MEMORY_COUNTERS {
        DWORD cb;
        DWORD PageFaultCount;
        DWORD PeakWorkingSetSize;
        DWORD WorkingSetSize;
        DWORD QuotaPeakPagedPoolUsage;
        DWORD QuotaPagedPoolUsage;
        DWORD QuotaPeakNonPagedPoolUsage;
        DWORD QuotaNonPagedPoolUsage;
        DWORD PagefileUsage;
        DWORD PeakPagefileUsage;
    } PROCESS_MEMORY_COUNTERS;
    typedef PROCESS_MEMORY_COUNTERS *PPROCESS_MEMORY_COUNTERS;

  class PsApiFunctions    {
  public:
    bool m_isValid;
    BOOL  (WINAPI *EnumProcesses)           (DWORD* lpidProcess, DWORD   cb, DWORD * cbNeeded);
    BOOL  (WINAPI *EnumProcessModules)      (HANDLE hProcess, HINSTANCE *lphModule, DWORD cb, LPDWORD lpcbNeeded);
    DWORD (WINAPI *GetModuleBaseNameA)      (HANDLE hProcess, HINSTANCE hModule, LPSTR lpBaseName, DWORD nSize );
    DWORD (WINAPI *GetModuleFileNameExA)    (HANDLE hProcess, HINSTANCE hModule, LPSTR lpFilename, DWORD nSize);
    BOOL  (WINAPI *GetModuleInformation)    (HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb );
    BOOL  (WINAPI *EmptyWorkingSet)         (HANDLE hProcess);
    BOOL  (WINAPI *QueryWorkingSet)         (HANDLE hProcess, PVOID pv, DWORD cb );
    BOOL  (WINAPI *InitializeProcessForWsWatch)(HANDLE hProcess);
    BOOL  (WINAPI *GetWsChanges)            (HANDLE hProcess, PPSAPI_WS_WATCH_INFORMATION lpWatchInfo, DWORD cb);
    DWORD (WINAPI *GetMappedFileNameA)      (HANDLE hProcess, LPVOID lpv, LPSTR lpFilename, DWORD nSize );
    BOOL  (WINAPI *EnumDeviceDrivers)       (LPVOID *lpImageBase, DWORD cb, LPDWORD lpcbNeeded );
    DWORD (WINAPI *GetDeviceDriverBaseNameA)(LPVOID ImageBase, LPSTR lpBaseName, DWORD nSize );
    DWORD (WINAPI *GetDeviceDriverFileNameA)(LPVOID ImageBase, LPSTR lpFilename, DWORD nSize );
    BOOL  (WINAPI *GetProcessMemoryInfo)    (HANDLE Process,   PPROCESS_MEMORY_COUNTERS ppsmemCounters, DWORD cb );
    void load(HINSTANCE handle, const char* name, void** pFunction)    {
      *pFunction = (void*)::GetProcAddress(handle, name);
      if ( 0 == *pFunction ) m_isValid = false;
    }
    bool isValid()    const   {
      return m_isValid;
    }
    PsApiFunctions()    {
      m_isValid = true;
      HINSTANCE mh = ::LoadLibrary("PSAPI.DLL");
      if ( 0 == mh )  {
        m_isValid = false;
      }
      else  {
        load( mh, "EnumProcesses",                (void**)&EnumProcesses );
        load( mh, "EnumProcessModules",           (void**)&EnumProcessModules );
        load( mh, "GetModuleBaseNameA",           (void**)&GetModuleBaseNameA );
        load( mh, "GetModuleFileNameExA",         (void**)&GetModuleFileNameExA );
        load( mh, "GetModuleInformation",         (void**)&GetModuleInformation );
        load( mh, "EmptyWorkingSet",              (void**)&EmptyWorkingSet );
        load( mh, "QueryWorkingSet",              (void**)&QueryWorkingSet );
        load( mh, "InitializeProcessForWsWatch",  (void**)&InitializeProcessForWsWatch );
        load( mh, "GetWsChanges",                 (void**)&GetWsChanges );
        load( mh, "GetMappedFileNameA",           (void**)&GetMappedFileNameA );
        load( mh, "EnumDeviceDrivers",            (void**)&EnumDeviceDrivers );
        load( mh, "GetDeviceDriverBaseNameA",     (void**)&GetDeviceDriverBaseNameA );
        load( mh, "GetDeviceDriverFileNameA",     (void**)&GetDeviceDriverFileNameA );
        load( mh, "GetProcessMemoryInfo",         (void**)&GetProcessMemoryInfo );
      }
    }
  };
#endif

#endif    // GAUDI_SYSTEM_WIN32PSAPI_H
