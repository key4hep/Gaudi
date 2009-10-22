///////////////////////// -*- C++ -*- /////////////////////////////
/// @file DsoUtils.h
/// @brief A mixed bag of "portable" utils for DSO (Dynamic Shared Objects)
/// @author S.Binet <binet@cern.ch>
/// @author Markus Frank <Markus.Frank@cern.ch> (Win32 code)
//  note: This, I believe, should be part of Reflex::SharedLibrary

#ifndef GAUDIKERNEL_DSOUTILS_H
#define GAUDIKERNEL_DSOUTILS_H

// STL includes
#include <string>

#include "GaudiKernel/System.h"

// Reflex includes
#include "Reflex/Reflex.h"

namespace DsoUtils {

inline std::string libNativeName( const std::string& libName )
{
#if defined(_WIN32)
  return libName+".dll";
#elif defined(__linux) || defined(__APPLE__)
  return "lib"+libName+".so";
#else
  // variant of the GIGO design pattern
  return libName;
#endif
}

#ifdef _GNU_SOURCE
#include <dlfcn.h>
static std::string dsoName( const ROOT::Reflex::Member& mem )
{
  Dl_info info;
  if (dladdr (
#if __GNUC__ < 4
      (void*)mem.Stubfunction()
#else
      System::FuncPtrCast<void*>(mem.Stubfunction())
#endif
      , &info) == 0)
    return "";

  const char* pos = strrchr (info.dli_fname, '/');
  if (pos)
    ++pos;
  else
    pos = info.dli_fname;
  return pos;
}
#elif defined(_WIN32)
#include <windows.h>

static std::string dsoName( const ROOT::Reflex::Member& mem )
{
  void* addr = (void*)(mem.Stubfunction());
  if (addr) {
    MEMORY_BASIC_INFORMATION mbi;
    if ( VirtualQuery(addr, &mbi, sizeof(mbi)) )    {
      HMODULE h_module = (HMODULE)mbi.AllocationBase;
      char mod[1024];
      if( GetModuleFileName(h_module, mod, sizeof(mod)) ) {
        const char* pos = strrchr (mod, '\\');
        if (pos)
          ++pos;
        else
          pos = mod;
        return pos;
      }
    }
  }
  return "";
}

#else // dummy implementation for unknown platforms
static std::string dsoName( const ROOT::Reflex::Member& )
{
  return "";
}

#endif

static bool inDso( const ROOT::Reflex::Member& mem,
                    const std::string& dsoname )
{
#ifdef _WIN32
  char sep = '\\';
#else
  char sep = '/';
#endif

  std::string srcname = dsoName(mem);
  if (srcname.empty()) {
    // we do not know the name of the library, let's guess it's OK
    return true;
  }

  std::string::size_type pos = dsoname.find_last_of(sep);
  std::string curname;
  if (std::string::npos == pos) {
    curname = dsoname;
  } else {
    curname = dsoname.substr(pos+1);
  }

  return srcname == curname;
}

} // end namespace DsoUtils

#endif // not GAUDIKERNEL_DSOUTILS_H

