/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
//====================================================================
//	ModuleInfo.cpp
//--------------------------------------------------------------------
//
//	Package    : System (The LHCb System service)
//
//  Description: Implementation of Systems internals
//
//	Author     : M.Frank
//      Created    : 13/1/99
//	Changes    :
//====================================================================
#define SYSTEM_MODULEINFO_CPP

// #include <ctime>
#include <cstdlib>
#include <cstring>
// #include <iostream>
// #include <typeinfo>

#include <GaudiKernel/ModuleInfo.h>
#include <GaudiKernel/System.h>

#ifdef _WIN32
#  define NOMSG
#  define NOGDI
#  define strcasecmp _stricmp
#  define strncasecmp _strnicmp
#  include "Win32PsApi.h"
#  include <process.h>
#  include <windows.h>
static PsApiFunctions _psApi;
#  define getpid _getpid
#  undef NOMSG
#  undef NOGDI
#  ifndef PATH_MAX
#    define PATH_MAX 1024
#  endif
#else // UNIX...: first the EGCS stuff, then the OS dependent includes
#  include <cstdio>
#  include <dlfcn.h>
#  include <errno.h>
#  include <libgen.h>
#  include <string.h>
#  include <sys/param.h>
#  include <sys/times.h>
#  include <unistd.h>
#endif

static System::ImageHandle      ModuleHandle = nullptr;
static std::vector<std::string> s_linkedModules;

/// Retrieve base name of module
const std::string& System::moduleName() {
  static std::string module( "" );
  if ( module == "" ) {
    if ( processHandle() && moduleHandle() ) {
#ifdef _WIN32
      char moduleName[256] = { "Unknown.module" };
      moduleName[0]        = 0;
      if ( _psApi ) {
        _psApi.GetModuleBaseNameA( processHandle(), (HINSTANCE)moduleHandle(), moduleName, sizeof( moduleName ) );
      }
      std::string mod = moduleName;
#elif defined( __linux ) || defined( __APPLE__ )
      std::string mod  = ::basename( (char*)( (Dl_info*)moduleHandle() )->dli_fname );
#elif __hpux
      std::string mod = ::basename( ( (HMODULE*)moduleHandle() )->dsc.filename );
#endif
      module = mod.substr( 0, mod.rfind( '.' ) );
    }
  }
  return module;
}

/// Retrieve full name of module
const std::string& System::moduleNameFull() {
  static std::string module( "" );
  if ( module == "" ) {
    if ( processHandle() && moduleHandle() ) {
      char name[PATH_MAX] = { "Unknown.module" };
      name[0]             = 0;
#ifdef _WIN32
      if ( _psApi ) {
        _psApi.GetModuleFileNameExA( processHandle(), (HINSTANCE)moduleHandle(), name, sizeof( name ) );
        module = name;
      }
#else

#  if defined( __linux ) || defined( __APPLE__ )
      const char* path = ( (Dl_info*)moduleHandle() )->dli_fname;
#  elif __hpux
      const char* path = ( (HMODULE*)moduleHandle() )->dsc.filename;
#  endif
      if ( ::realpath( path, name ) ) module = name;
#endif
    }
  }
  return module;
}

/// Get type of the module
System::ModuleType System::moduleType() {
  static ModuleType type = UNKNOWN;
  if ( type == UNKNOWN ) {
    const std::string& module = moduleNameFull();
    size_t             loc    = module.rfind( '.' ) + 1;
    if ( loc == 0 )
      type = EXECUTABLE;
    else if ( module[loc] == 'e' || module[loc] == 'E' )
      type = EXECUTABLE;
#ifdef _WIN32
    else if ( module[loc] == 'd' || module[loc] == 'D' )
#else
    else if ( module[loc] == 's' && module[loc + 1] == 'o' )
#endif
      type = SHAREDLIB;
    else
      type = UNKNOWN;
  }
  return type;
}

/// Retrieve processhandle
void* System::processHandle() {
  static long pid = ::getpid();
#ifdef _WIN32
  static HANDLE hP = ::OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid );
#else
  static void* hP = (void*)pid;
#endif
  return hP;
}

void System::setModuleHandle( System::ImageHandle handle ) { ModuleHandle = handle; }

System::ImageHandle System::moduleHandle() {
  if ( !ModuleHandle ) {
    if ( processHandle() ) {
#ifdef _WIN32
      static HINSTANCE handle = 0;
      DWORD            cbNeeded;
      if ( 0 == handle && _psApi ) {
        if ( _psApi.EnumProcessModules( processHandle(), &handle, sizeof( ModuleHandle ), &cbNeeded ) ) {}
      }
      return handle;
#elif defined( __linux ) || defined( __APPLE__ )
      static Dl_info info;
      if ( ::dladdr( reinterpret_cast<void*>( System::moduleHandle ), &info ) ) return &info;
#elif __hpux
      return 0; // Don't know how to solve this .....
#endif
    }
  }
  return ModuleHandle;
}

System::ImageHandle System::exeHandle() {
#ifdef _WIN32
  if ( processHandle() ) {
    static HINSTANCE handle = 0;
    DWORD            cbNeeded;
    if ( 0 == handle && _psApi ) {
      if ( _psApi.EnumProcessModules( processHandle(), &handle, sizeof( ModuleHandle ), &cbNeeded ) ) {}
    }
    return handle;
  }
  return 0;
#elif defined( __linux ) || defined( __APPLE__ )
  // This does NOT work!
  static Dl_info infoBuf, *info = &infoBuf;
  if ( !info ) {
    void* handle = ::dlopen( nullptr, RTLD_LAZY );
    // printf("Exe handle:%X\n", handle);
    if ( handle ) {
      void* func = ::dlsym( handle, "main" );
      // printf("Exe:Func handle:%X\n", func);
      if ( func ) {
        if ( 0 != ::dladdr( func, &infoBuf ) ) {
          // std::cout << "All OK" << std::endl;
          info = &infoBuf;
        }
      }
    }
  }
  return info;
#elif __hpux
  // Don't know how to solve this .....
  return 0;
#endif
  return 0;
}

const std::string& System::exeName() {
  static std::string module( "" );
  if ( module.length() == 0 ) {
    char name[PATH_MAX] = { "Unknown.module" };
    name[0]             = 0;
#ifdef _WIN32
    if ( _psApi && processHandle() ) {
      _psApi.GetModuleFileNameExA( processHandle(), (HINSTANCE)exeHandle(), name, sizeof( name ) );
      module = name;
    }
#elif defined( __linux ) || defined( __APPLE__ )
    char cmd[512];
    ::sprintf( cmd, "/proc/%d/exe", ::getpid() );
    module = "Unknown";
    if ( ::readlink( cmd, name, sizeof( name ) ) >= 0 ) module = name;
#elif __hpux
    if ( ::realpath( ( (HMODULE*)exeHandle() )->dsc.filename, name ) ) module = name;
#endif
  }
  return module;
}

const std::vector<std::string> System::linkedModules() {
  if ( s_linkedModules.size() == 0 ) {
#ifdef _WIN32
    char      name[255]; // Maximum file name length on NT 4.0
    DWORD     cbNeeded;
    HINSTANCE handle[1024];
    if ( _psApi ) {
      if ( _psApi.EnumProcessModules( processHandle(), handle, sizeof( handle ), &cbNeeded ) ) {
        for ( size_t i = 0; i < cbNeeded / sizeof( HANDLE ); i++ ) {
          if ( 0 < _psApi.GetModuleFileNameExA( processHandle(), handle[i], name, sizeof( name ) ) ) {
            s_linkedModules.push_back( name );
          }
        }
      }
    }
#elif defined( __linux ) || defined( __APPLE__ )
    char ff[512], cmd[1024], fname[1024], buf1[64], buf2[64], buf3[64], buf4[64];
    ::sprintf( ff, "/proc/%d/maps", ::getpid() );
    FILE* maps = ::fopen( ff, "r" );
    while ( ::fgets( cmd, sizeof( cmd ), maps ) ) {
      int len;
      sscanf( cmd, "%s %s %s %s %d %s", buf1, buf2, buf3, buf4, &len, fname );
      if ( len > 0 && strncmp( buf2, "r-xp", strlen( "r-xp" ) ) == 0 ) { s_linkedModules.push_back( fname ); }
    }
    ::fclose( maps );
#endif
  }
  return s_linkedModules;
}
