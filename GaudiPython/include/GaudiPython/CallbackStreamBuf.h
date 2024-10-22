/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIPYTHON_CALLBACKSTREAMBUF_H
#define GAUDIPYTHON_CALLBACKSTREAMBUF_H
// ============================================================================
#include <Python.h> //included for the python API
// ============================================================================
#include <array>
#include <sstream> //included for stringbuf
// ============================================================================
#include <GaudiKernel/Kernel.h>
#include <GaudiPython/GaudiPython.h>
// ============================================================================

namespace GaudiPython {
  int GAUDI_API call_python_method( PyObject* self, const char* method, char* buf );

  class GAUDI_API CallbackStreamBuf : public std::stringbuf {
  private:
    class PyObject_t {
      PyObject* m_obj;

    public:
      PyObject_t( PyObject* obj = nullptr ) : m_obj( obj ) {
        if ( m_obj ) Py_INCREF( m_obj );
      }
      ~PyObject_t() {
        if ( m_obj ) Py_DECREF( m_obj );
      }
      PyObject* get() { return m_obj; }
    };
    PyObject_t            m_self;
    std::array<char, 512> m_callbackBuff; // used for passing the flushed chars in the python callback

  public:
    CallbackStreamBuf( PyObject* self );
    int sync() override;
  };
} // namespace GaudiPython

#endif
