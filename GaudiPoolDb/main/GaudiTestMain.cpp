#include <string>
#include <iostream>

#ifdef WIN32
  extern "C" __declspec(dllimport) void* __stdcall LoadLibraryA(const char*);
  extern "C" __declspec(dllimport) void* __stdcall GetProcAddress(void*, const char*);
  #define LOAD_LIB(x)  ::LoadLibraryA( x )
  #define GETPROC(h,x) ::GetProcAddress ( h, #x )

#endif

#ifndef WIN32

  #include <dlfcn.h>
  #include <unistd.h>
  void* LOAD_LIB(const char* x)  {
    std::string l = "lib"; l+=x; l+=".so";
    return ::dlopen( l.c_str() , RTLD_NOW|RTLD_GLOBAL);
    //return ::dlopen( l.c_str() , RTLD_NOW);
  }
  #define GETPROC(h,x) ::dlsym ( h, #x )
#endif
#include <vector>
#include <algorithm>

int main (int argc, char** argv)  {
  if ( argc < 2 )  {
    std::cout << "Usage: main.exe <example library> arg [arg [arg]]" << std::endl;
  }
  else  {
    typedef long (*func)(int, char**);
    // LOAD_LIB("GaudiPoolDb.dll");
    void* handle = LOAD_LIB( argv[1] );
    if ( 0 != handle )  {
      func fun = (func)GETPROC(handle, ExampleMain );
      if ( fun ) {
        return (*fun)(argc, argv);
      }
      std::cout << "Failed to access test procedure ExampleMain" << std::endl;
      return 0;
    }
    std::cout << "Failed to load test library:" << argv[1] << std::endl;
#ifdef WIN32
#else
    std::cout << "Error:" << ::dlerror() << std::endl;
#endif
  }
  return 0;
}
