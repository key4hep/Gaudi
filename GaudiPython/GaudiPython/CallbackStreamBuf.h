#ifndef GAUDIPYTHON_CALLBACKSTREAMBUF_H
#define GAUDIPYTHON_CALLBACKSTREAMBUF_H
// ============================================================================
#include "Python.h"   //included for the python API
// ============================================================================
#include <sstream>    //included for stringbuf
#include <array>
// ============================================================================
#include "GaudiPython/GaudiPython.h"
#include "GaudiKernel/Kernel.h"
// ============================================================================

namespace GaudiPython
{
  int GAUDI_API call_python_method(PyObject* self, const char* method, char* buf);

  class GAUDI_API CallbackStreamBuf: public std::stringbuf
  {
    private:
      class PyObject_t {
          PyObject *m_obj;
      public:
          PyObject_t( PyObject *obj = nullptr) : m_obj( obj ) { if (m_obj) Py_INCREF(m_obj); }
          ~PyObject_t() { if (m_obj) Py_DECREF(m_obj); }
          PyObject *get() { return m_obj; }
      };
      PyObject_t m_self;
      std::array<char,512> m_callbackBuff;//used for passing the flushed chars in the python callback

    public:
      CallbackStreamBuf(PyObject* self);
      int sync() override;
  };
}

#endif
