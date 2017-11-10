#ifndef GAUDIKERNEL_SYSTEM_H
#define GAUDIKERNEL_SYSTEM_H

// Framework include files
#include "GaudiKernel/Kernel.h"
// STL include files
#include <cstring>
#include <string>
#include <typeinfo>
#include <vector>

#include "GaudiKernel/ModuleInfo.h"
#include "GaudiKernel/Timing.h"

#ifdef __linux
#include <pthread.h>
#ifndef __APPLE__
#include <execinfo.h>
#endif
#endif

/** Note: OS specific details as well as Gaudi details may not occur
    in this definition file, because this header is the used by both, the
    OS specific implementations and the gaudi specific implementation.
    Since e.g. IID is defined in both, this would lead automatically to
    complete comilation failures.....

    @author M.Frank
*/
namespace System
{
  /// Definition of an image handle
  typedef void* ImageHandle;
  /// Definition of the process handle
  typedef void* ProcessHandle;
  /// Definition of the "generic" DLL entry point function
  typedef unsigned long ( *EntryPoint )( const unsigned long iid, void** ppvObject );
  /// Definition of the "generic" DLL entry point function
  typedef void* ( *Creator )();
  /// Load dynamic link library
  GAUDI_API unsigned long loadDynamicLib( const std::string& name, ImageHandle* handle );
  /// unload dynamic link library
  GAUDI_API unsigned long unloadDynamicLib( ImageHandle handle );
  /// Get a specific function defined in the DLL
  GAUDI_API unsigned long getProcedureByName( ImageHandle handle, const std::string& name, EntryPoint* pFunction );
  /// Get a specific function defined in the DLL
  GAUDI_API unsigned long getProcedureByName( ImageHandle handle, const std::string& name, Creator* pFunction );
  /// Get last system known error
  GAUDI_API unsigned long getLastError();
  /// Get last system error as string
  GAUDI_API const std::string getLastErrorString();
  /// Retrieve error code as string for a given error
  GAUDI_API const std::string getErrorString( unsigned long error );
  /// Get platform independent information about the class type
  GAUDI_API const std::string typeinfoName( const std::type_info& );
  GAUDI_API const std::string typeinfoName( const char* );
  /// Host name
  GAUDI_API const std::string& hostName();
  /// OS name
  GAUDI_API const std::string& osName();
  /// OS version
  GAUDI_API const std::string& osVersion();
  /// Machine type
  GAUDI_API const std::string& machineType();
  /// Instruction Set "Level"
  //    0           = 80386 instruction set
  //    1  or above = SSE (XMM) supported by CPU (not testing for O.S. support)
  //    2  or above = SSE2
  //    3  or above = SSE3
  //    4  or above = Supplementary SSE3 (SSSE3)
  //    5  or above = SSE4.1
  //    6  or above = SSE4.2
  //    7  or above = AVX supported by CPU and operating system
  //    8  or above = AVX2
  //    9  or above = AVX512F
  GAUDI_API int instructionsetLevel();
  /// User login name
  GAUDI_API const std::string& accountName();
  /// Number of arguments passed to the commandline
  GAUDI_API long numCmdLineArgs();
  /// Number of arguments passed to the commandline (==numCmdLineArgs()); just to match argv call...
  GAUDI_API long argc();
  /// Command line arguments including executable name as arg[0] as vector of strings
  GAUDI_API const std::vector<std::string> cmdLineArgs();
  /// char** command line arguments including executable name as arg[0]; You may not modify them!
  GAUDI_API char** argv();
  /// get a particular environment variable (returning "UNKNOWN" if not set)
  GAUDI_API std::string getEnv( const char* var );
  /// get a particular environment variable, storing the value in the passed string if the
  /// variable is set. Returns true if the variable is set, false otherwise.
  GAUDI_API bool getEnv( const char* var, std::string& value );
  inline bool getEnv( const std::string& var, std::string& value ) { return getEnv( var.c_str(), value ); }
  /// get all environment variables
  GAUDI_API std::vector<std::string> getEnv();
  /// Set an environment variables.
  /// If value is empty, the variable is removed from the environment.
  /// When overwrite is 0, the variable is not set if already present.
  /// Returns 0 on success, -1 on failure.
  /// See man 3 setenv.
  GAUDI_API int setEnv( const std::string& name, const std::string& value, int overwrite = 1 );
  /// Check if an environment variable is set or not.
  GAUDI_API bool isEnvSet( const char* var );
#ifdef __linux
  /// A Thread handle
  typedef pthread_t ThreadHandle;
  /// thread handle "accessor"
  inline ThreadHandle threadSelf() { return pthread_self(); }
#else
  /// A Thread handle
  typedef void* ThreadHandle;
  /// thread handle "accessor"
  inline ThreadHandle threadSelf() { return (void*)0; }
#endif
  GAUDI_API int backTrace( void** addresses, const int depth );
  GAUDI_API bool backTrace( std::string& btrace, const int depth, const int offset = 0 );
  GAUDI_API bool getStackLevel( void* addresses, void*& addr, std::string& fnc, std::string& lib );

#if __GNUC__ >= 4
  /// Small helper function that performs the cast from void * to function pointer
  /// in a standards compliant way. For more information on this type of 'type punning, see
  /// https://github.com/CppCon/CppCon2017/blob/master/Presentations/Type%20Punning%20In%20C%2B%2B17%20-%20Avoiding%20Pun-defined%20Behavior/Type%20Punning%20In%20C%2B%2B17%20-%20Avoiding%20Pun-defined%20Behavior%20-%20Scott%20Schurr%20-%20CppCon%202017.pdf
  /// and https://www.youtube.com/watch?v=sCjZuvtJd-k
  template <typename DESTPTR, typename SRC>
  constexpr DESTPTR FuncPtrCast( SRC* const src_p ) noexcept
  {
    static_assert( std::is_pointer<DESTPTR>::value, "must be a pointer" );
    DESTPTR dst_p = nullptr; // must initialize to be a valid constexpr...
    std::memcpy( &dst_p, &src_p, sizeof( dst_p ) );
    return dst_p;
  }
#endif
}
#endif // SYSTEM_SYSTEM_H
