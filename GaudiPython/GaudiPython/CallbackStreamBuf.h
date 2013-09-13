#ifndef GAUDIPYTHON_CALLBACKSTREAMBUF_H
#define GAUDIPYTHON_CALLBACKSTREAMBUF_H
// ============================================================================
#ifdef linux

#include <stdio.h>

#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif

#ifdef _FILE_OFFSET_BITS
#undef _FILE_OFFSET_BITS
#endif

#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif

#endif // linux

#include "Python.h"   //included for the python API
// ============================================================================
#include <sstream>    //included for stringbuf
// ============================================================================
#include "GaudiPython/GaudiPython.h"
// ============================================================================

namespace GaudiPython
{
  int GAUDI_API call_python_method(PyObject* self, const char* method, char* buf);

  class GAUDI_API CallbackStreamBuf: public std::stringbuf
  {
    private:
      PyObject* m_self;
      char* m_callbackBuff;

    public:
      CallbackStreamBuf(PyObject* self);
      ~CallbackStreamBuf();
      virtual int sync() ;
  }; //CallbackStreamBuf

} //namespace GaudiPython

#endif
