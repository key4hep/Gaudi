//====================================================================
//	Debugger.cpp
//--------------------------------------------------------------------
//
//	Package    : System (The LHCb System service)
//
//  Description: Invoke interactively the debugger from a
//               running application
//
//	Author     : M.Frank
//  Created    : 13/1/99
//	Changes    :
//====================================================================
#define GAUDIKERNEL_DEBUGGER_CPP

#ifdef _WIN32
namespace Win {
// Avoid conflicts between Windows' headers and MSG.
#ifndef NOMSG
#  define NOMSG
#  ifndef NOGDI
#    define NOGDI
#  endif
#endif
#include "windows.h"
#include "process.h"
};
#else
  #include <unistd.h>
#endif

// Framework include files
#include "GaudiKernel/Debugger.h"

/// Break the execution of the application and invoke the debugger
long System::breakExecution()   {
#ifdef _WIN32
  _asm int 3
  return 1;
#else
  // I have no clu how to do this in linux
  return 0;
#endif
}

/// Break the execution of the application and invoke the debugger on a possibly remote process
long System::breakExecution(long pid)   {
#ifdef _WIN32
  long result = 0;
  if ( pid == Win::_getpid() )    {
    _asm int 3
    return 1;
  }
  else    {
    Win::LPTHREAD_START_ROUTINE fun;
    Win::HANDLE th, ph;
    Win::HINSTANCE mh;
    Win::DWORD id;
    mh = Win::LoadLibrary( "Kernel32" );
    if ( 0 != mh )    {
      fun = (Win::LPTHREAD_START_ROUTINE)Win::GetProcAddress(mh, "DebugBreak");
      if ( 0 != fun )   {
        ph = Win::OpenProcess (PROCESS_ALL_ACCESS, TRUE, pid);
        if ( 0 != ph )      {
          th = Win::CreateRemoteThread(ph,NULL,0,fun,0,0,&id);
          if ( 0 != th )   {
            Win::CloseHandle(th);
            result = 1;
          }
          Win::CloseHandle(ph);
        }
      }
      Win::FreeLibrary(mh);
    }
  }
  if ( result != 1 ) result = Win::GetLastError();
  return result;
#else
  // I have no clu how to do this in linux
  return pid;
#endif
}










