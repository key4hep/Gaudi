//====================================================================
//	System.cpp
//--------------------------------------------------------------------
//
//	Package    : System (The LHCb System service)
//
//  Description: Implementation of Systems internals
//
//	Author     : M.Frank
//  Created    : 13/1/99
//	Changes    :
//====================================================================
#define SYSTEM_SYSTEM_CPP
#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <typeinfo>

#include "GaudiKernel/System.h"

// Platform specific include(s):
#ifdef __linux__
#include "Platform/SystemLinux.h"
#elif defined( __APPLE__ )
#include "Platform/SystemMacOS.h"
#elif defined( _WIN32 )
#include "Platform/SystemWin32.h"
#endif

#define VCL_NAMESPACE Gaudi
#include "instrset_detect.cpp"
#undef VCL_NAMESPACE

#ifdef _WIN32
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define getpid _getpid
#define NOMSG
#define NOGDI
#include "process.h"
#include "windows.h"
#undef NOMSG
#undef NOGDI
static const std::array<const char*, 1> SHLIB_SUFFIXES = {".dll"};
#else // UNIX...: first the EGCS stuff, then the OS dependent includes
#include "libgen.h"
#include "sys/times.h"
#include "unistd.h"
#include <cstdio>
#include <cxxabi.h>
#include <errno.h>
#include <string.h>
#if defined( __linux ) || defined( __APPLE__ )
#include "dlfcn.h"
#include <sys/utsname.h>
#include <unistd.h>
#elif __hpux
#include "dl.h"
struct HMODULE {
  shl_descriptor dsc;
  long numSym;
  shl_symbol* sym;
};
#endif // HPUX or not...

#ifdef __APPLE__
static const std::array<const char*, 2> SHLIB_SUFFIXES = {".dylib", ".so"};
#else
static const std::array<const char*, 1> SHLIB_SUFFIXES = {".so"};
#endif // __APPLE__

#endif // Windows or Unix...

// Note: __attribute__ is a GCC keyword available since GCC 3.4
#ifdef __GNUC__
#if __GNUC__ < 3 || ( __GNUC__ == 3 && ( __GNUC_MINOR__ < 4 ) )
// GCC < 3.4
#define __attribute__( x )
#endif
#else
// non-GCC
#define __attribute__( x )
#endif

static unsigned long doLoad( const std::string& name, System::ImageHandle* handle )
{
#ifdef _WIN32
  void* mh = ::LoadLibrary( name.length() == 0 ? System::exeName().c_str() : name.c_str() );
  *handle  = mh;
#else
  const char* path = name.c_str();
#if defined( __linux ) || defined( __APPLE__ )
  void* mh         = ::dlopen( name.length() == 0 ? nullptr : path, RTLD_LAZY | RTLD_GLOBAL );
  *handle          = mh;
#elif __hpux
  shl_t mh     = ::shl_load( name.length() == 0 ? 0 : path, BIND_IMMEDIATE | BIND_VERBOSE, 0 );
  HMODULE* mod = new HMODULE;
  if ( 0 != mh ) {
    if ( 0 != ::shl_gethandle_r( mh, &mod->dsc ) ) {
      std::cout << "System::loadDynamicLib>" << ::strerror( getLastError() ) << std::endl;
    } else {
      typedef void* ( *___all )();
      ___all _alloc = (___all)malloc;
      mod->numSym   = ::shl_getsymbols( mod->dsc.handle, TYPE_PROCEDURE, EXPORT_SYMBOLS, malloc, &mod->sym );
      *handle       = mod;
    }
  }
#endif
#endif
  if ( !*handle ) {
    return System::getLastError();
  }
  return 1;
}

static unsigned long loadWithoutEnvironment( const std::string& name, System::ImageHandle* handle )
{

  // If the name is empty, don't do anything complicated.
  if ( name.length() == 0 ) {
    return doLoad( name, handle );
  }

  // Check if the specified name has a shared library suffix already. If it
  // does, don't bother the name any more.
  std::string dllName = name;
  bool hasShlibSuffix = false;
  for ( const char* suffix : SHLIB_SUFFIXES ) {
    const size_t len = strlen( suffix );
    if ( dllName.compare( dllName.length() - len, len, suffix ) == 0 ) {
      hasShlibSuffix = true;
      break;
    }
  }

  // If it doesn't have a shared library suffix on it, add the "default" shared
  // library suffix to the name.
  if ( !hasShlibSuffix ) {
    dllName += SHLIB_SUFFIXES[0];
  }

  // Load the library.
  return doLoad( dllName, handle );
}

/// Load dynamic link library
unsigned long System::loadDynamicLib( const std::string& name, ImageHandle* handle )
{
  unsigned long res = 0;
  // if name is empty, just load it
  if ( name.length() == 0 ) {
    res = loadWithoutEnvironment( name, handle );
  } else {
    // If the name is a logical name (environment variable), the try
    // to load the corresponding library from there.
    std::string imgName;
    if ( getEnv( name, imgName ) ) {
      res = loadWithoutEnvironment( imgName, handle );
    } else {
      // build the dll name
      std::string dllName = name;
// Add a possible "lib" prefix to the name on unix platforms. But only if
// it's not an absolute path name.
#if defined( __linux ) || defined( __APPLE__ )
      if ( ( dllName.find( '/' ) == std::string::npos ) && ( dllName.compare( 0, 3, "lib" ) != 0 ) ) {
        dllName = "lib" + dllName;
      }
#endif // unix
      // Now try loading the library with all possible suffixes supported by the
      // platform.
      for ( const char* suffix : SHLIB_SUFFIXES ) {
        // Add the suffix if necessary.
        std::string libName = dllName;
        const size_t len    = strlen( suffix );
        if ( dllName.compare( dllName.length() - len, len, suffix ) != 0 ) {
          libName += suffix;
        }
        // Try to load the library.
        res = loadWithoutEnvironment( libName, handle );
        // If the load succeeded, stop here.
        if ( res == 1 ) {
          break;
        }
      }
    }
    if ( res != 1 ) {
#if defined( __linux ) || defined( __APPLE__ )
      errno = 0xAFFEDEAD;
#endif
      // std::cout << "System::loadDynamicLib>" << getLastErrorString() << std::endl;
    }
  }
  return res;
}

/// unload dynamic link library
unsigned long System::unloadDynamicLib( ImageHandle handle )
{
#ifdef _WIN32
  if ( !::FreeLibrary( (HINSTANCE)handle ) ) {
#elif defined( __linux ) || defined( __APPLE__ )
  ::dlclose( handle );
  if ( 0 ) {
#elif __hpux
  // On HP we have to run finalization ourselves.....
  Creator pFinalize = 0;
  if ( getProcedureByName( handle, "_fini", &pFinalize ) ) {
    pFinalize();
  }
  HMODULE* mod = (HMODULE*)handle;
  if ( 0 == ::shl_unload( mod->dsc.handle ) ) {
    delete mod;
  } else {
#else
  if ( false ) {
#endif
    return getLastError();
  }
  return 1;
}

/// Get a specific function defined in the DLL
unsigned long System::getProcedureByName( ImageHandle handle, const std::string& name, EntryPoint* pFunction )
{
#ifdef _WIN32
  *pFunction = ( EntryPoint )::GetProcAddress( (HINSTANCE)handle, name.data() );
  if ( 0 == *pFunction ) {
    return System::getLastError();
  }
  return 1;
#elif defined( __linux )
  *pFunction = reinterpret_cast<EntryPoint>(::dlsym( handle, name.c_str() ) );
  if ( !*pFunction ) {
    errno = 0xAFFEDEAD;
    // std::cout << "System::getProcedureByName>" << getLastErrorString() << std::endl;
    return 0;
  }
  return 1;
#elif defined( __APPLE__ )
  *pFunction = ( EntryPoint )::dlsym( handle, name.c_str() );
  if ( !( *pFunction ) ) {
    // Try with an underscore :
    std::string sname = "_" + name;
    *pFunction        = ( EntryPoint )::dlsym( handle, sname.c_str() );
  }
  if ( 0 == *pFunction ) {
    errno = 0xAFFEDEAD;
    std::cout << "System::getProcedureByName>" << getLastErrorString() << std::endl;
    // std::cout << "System::getProcedureByName> failure" << std::endl;
    return 0;
  }
  return 1;
#elif __hpux
  HMODULE* mod = (HMODULE*)handle;
  if ( 0 != mod ) {
    long ll1 = name.length();
    for ( int i = 0; i < mod->numSym; i++ ) {
      long ll2 = strlen( mod->sym[i].name );
      if ( 0 != ::strncmp( mod->sym[i].name, name.c_str(), ( ll1 > ll2 ) ? ll1 : ll2 ) == 0 ) {
        *pFunction = (EntryPoint)mod->sym[i].value;
        return 1;
      }
    }
  }
  return 0;
#endif
}

/// Get a specific function defined in the DLL
unsigned long System::getProcedureByName( ImageHandle handle, const std::string& name, Creator* pFunction )
{
  return getProcedureByName( handle, name, (EntryPoint*)pFunction );
}

/// Retrieve last error code
unsigned long System::getLastError()
{
#ifdef _WIN32
  return ::GetLastError();
#else
  // convert errno (int) to unsigned long
  return static_cast<unsigned long>( static_cast<unsigned int>( errno ) );
#endif
}

/// Retrieve last error code as string
const std::string System::getLastErrorString()
{
  const std::string errString = getErrorString( getLastError() );
  return errString;
}

/// Retrieve error code as string for a given error
const std::string System::getErrorString( unsigned long error )
{
  std::string errString = "";
#ifdef _WIN32
  LPVOID lpMessageBuffer;
  ::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error,
                   MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), // The user default language
                   (LPTSTR)&lpMessageBuffer, 0, NULL );
  errString = (const char*)lpMessageBuffer;
  // Free the buffer allocated by the system
  ::LocalFree( lpMessageBuffer );
#else
  char* cerrString( nullptr );
  // Remember: for linux dl* routines must be handled differently!
  if ( error == 0xAFFEDEAD ) {
    cerrString                    = (char*)::dlerror();
    if ( !cerrString ) cerrString = ::strerror( error );
    if ( !cerrString ) {
      cerrString = (char*)"Unknown error. No information found in strerror()!";
    } else {
      errString = std::string( cerrString );
    }
    errno = 0;
  } else {
    cerrString = ::strerror( error );
    errString  = std::string( cerrString );
  }
#endif
  return errString;
}

const std::string System::typeinfoName( const std::type_info& tinfo ) { return typeinfoName( tinfo.name() ); }

const std::string System::typeinfoName( const char* class_name ) { return Platform::typeinfoName( class_name ); }

/// Host name
const std::string& System::hostName()
{
  static const std::string host = Platform::hostName();
  return host;
}

/// OS name
const std::string& System::osName()
{
  static const std::string osname = Platform::osName();
  return osname;
}

/// OS version
const std::string& System::osVersion()
{
  static const std::string osver = Platform::osVersion();
  return osver;
}

/// Machine type
const std::string& System::machineType()
{
  static const std::string mach = Platform::machineType();
  return mach;
}

int System::instructionsetLevel()
{
  using namespace Gaudi;
  return instrset_detect();
}

/// User login name
const std::string& System::accountName()
{
  static const std::string account = Platform::accountName();
  return account;
}

/// Number of arguments passed to the commandline
long System::numCmdLineArgs() { return cmdLineArgs().size(); }

/// Number of arguments passed to the commandline
long System::argc() { return cmdLineArgs().size(); }

/// Const char** command line arguments including executable name as arg[0]
const std::vector<std::string> System::cmdLineArgs()
{
  static const std::vector<std::string> args = Platform::cmdLineArgs();
  return args;
}

/// Const char** command line arguments including executable name as arg[0]
char** System::argv()
{
  auto helperFunc = []( const std::vector<std::string>& args ) -> std::vector<const char*> {
    std::vector<const char*> result;
    std::transform( args.begin(), args.end(), std::back_inserter( result ),
                    []( const std::string& s ) { return s.c_str(); } );
    return result;
  };
  static const std::vector<const char*> args = helperFunc( cmdLineArgs() );
  // We rely here on the fact that a vector's allocation table is contiguous
  return (char**)&( args[0] );
}

#ifdef WIN32
// disable warning
//   C4996: 'getenv': This function or variable may be unsafe.
#pragma warning( disable : 4996 )
#endif

/// get a particular env var, return "UNKNOWN" if not defined
std::string System::getEnv( const char* var )
{
  char* env;
  if ( ( env = getenv( var ) ) != nullptr ) {
    return env;
  } else {
    return "UNKNOWN";
  }
}

/// get a particular env var, storing the value in the passed string (if set)
bool System::getEnv( const char* var, std::string& value )
{
  char* env;
  if ( ( env = getenv( var ) ) != nullptr ) {
    value = env;
    return true;
  } else {
    return false;
  }
}

bool System::isEnvSet( const char* var ) { return getenv( var ) != nullptr; }

/// get all defined environment vars
#if defined( __APPLE__ )
// Needed for _NSGetEnviron(void)
#include "crt_externs.h"
#endif
std::vector<std::string> System::getEnv()
{
#if defined( _WIN32 )
#define environ _environ
#elif defined( __APPLE__ )
  static char** environ = *_NSGetEnviron();
#endif
  std::vector<std::string> vars;
  for ( int i = 0; environ[i] != nullptr; ++i ) {
    vars.push_back( environ[i] );
  }
  return vars;
}

// -----------------------------------------------------------------------------
// backtrace utilities
// -----------------------------------------------------------------------------
#ifdef __linux
#include <execinfo.h>
#endif

int System::backTrace( void** addresses __attribute__( ( unused ) ), const int depth __attribute__( ( unused ) ) )
{

#ifdef __linux

  int count = backtrace( addresses, depth );
  return count > 0 ? count : 0;

#else // windows and osx parts not implemented
  return 0;
#endif
}

bool System::backTrace( std::string& btrace, const int depth, const int offset )
{
  try {
    // Always hide the first two levels of the stack trace (that's us)
    const int totalOffset = offset + 2;
    const int totalDepth  = depth + totalOffset;

    std::string fnc, lib;

    std::vector<void*> addresses( totalDepth, nullptr );
    int count = System::backTrace( addresses.data(), totalDepth );
    for ( int i = totalOffset; i < count; ++i ) {
      void* addr = nullptr;

      if ( System::getStackLevel( addresses[i], addr, fnc, lib ) ) {
        std::ostringstream ost;
        ost << "#" << std::setw( 3 ) << std::setiosflags( std::ios::left ) << i - totalOffset + 1;
        ost << std::hex << addr << std::dec << " " << fnc << "  [" << lib << "]" << std::endl;
        btrace += ost.str();
      }
    }
    return true;
  } catch ( const std::bad_alloc& e ) {
    return false;
  }
}

bool System::getStackLevel( void* addresses __attribute__( ( unused ) ), void*& addr __attribute__( ( unused ) ),
                            std::string& fnc __attribute__( ( unused ) ), std::string& lib __attribute__( ( unused ) ) )
{

#ifdef __linux

  Dl_info info;

  if ( dladdr( addresses, &info ) && info.dli_fname && info.dli_fname[0] != '\0' ) {
    const char* symbol = info.dli_sname && info.dli_sname[0] != '\0' ? info.dli_sname : nullptr;

    lib  = info.dli_fname;
    addr = info.dli_saddr;

    if ( symbol ) {
      int stat = -1;
      auto dmg =
          std::unique_ptr<char, decltype( free )*>( abi::__cxa_demangle( symbol, nullptr, nullptr, &stat ), std::free );
      fnc = ( stat == 0 ) ? dmg.get() : symbol;
    } else {
      fnc = "local";
    }
    return true;
  } else {
    return false;
  }

#else // not implemented for windows and osx
  return false;
#endif
}

/// set an environment variables. @return 0 if successful, -1 if not
int System::setEnv( const std::string& name, const std::string& value, int overwrite )
{
#ifndef WIN32
  // UNIX version
  return value.empty() ?
                       // remove if set to nothing (and return success)
         ::unsetenv( name.c_str() ),
         0 :
                       // set the value
         ::setenv( name.c_str(), value.c_str(), overwrite );
#else
  // Windows version
  if ( value.empty() ) {
    // equivalent to unsetenv
    return ::_putenv( ( name + "=" ).c_str() );
  } else {
    if ( !getenv( name.c_str() ) || overwrite ) {
      // set if not yet present or overwrite is set (force)
      return ::_putenv( ( name + "=" + value ).c_str() );
    }
  }
  return 0; // if we get here, we are trying to set a variable already set, but
            // not to overwrite.
            // It is considered a success on Linux (man P setenv)
#endif
}
