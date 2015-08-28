#include "Python.h"   //included the python API
// ============================================================================
#include "GaudiKernel/Bootstrap.h"
// ============================================================================
#include <algorithm>
// ============================================================================
#include "GaudiPython/GaudiPython.h"
// ============================================================================
#include "GaudiPython/CallbackStreamBuf.h"
// ============================================================================

using namespace std;

// ============================================================================
/// call a python method with a string as an argument
// ============================================================================
int GaudiPython::call_python_method
( PyObject* self, const char* method, char* buf )
{
  if ( !self || !method ) { return 1 ; }
  PyObject* r = PyObject_CallMethod(self, const_cast<char*>(method),
                                          const_cast<char*>("s"), buf);
  if ( !r ) {
    string err("Unsuccessful call to bound Python method");
    err += method;
    PyErr_SetString( PyExc_TypeError , err.c_str() ) ;
    PyErr_Print() ;
    return 1 ;
  }
  Py_DECREF( r );
  return 0 ;
}

// ============================================================================
/// CallbackStreamBuf constructor
// ============================================================================
GaudiPython::CallbackStreamBuf::CallbackStreamBuf(PyObject* self): 
    m_self(self),
    m_callbackBuff{ new char[512] } //used for passing the flushed chars in the python callback
{
  Py_INCREF(m_self);
}

// ============================================================================
/// CallbackStreamBuf destructor
// ============================================================================
GaudiPython::CallbackStreamBuf::~CallbackStreamBuf()
{
  Py_DECREF( m_self );
}

// ============================================================================
/// reimplementation of stringbuf::sync()
// ============================================================================
int GaudiPython::CallbackStreamBuf::sync ()
{
  int length;
  char *x;
  for ( length = 0, x = pbase(); x < epptr(); x++ , length++ ) ;
  //getting in a null terminated buffer the characters
  memcpy( m_callbackBuff.get(), pbase(), std::min(length, 512) );
  m_callbackBuff[ std::min(length, 512) ] = '\0';
  //calling the python method
  GaudiPython::call_python_method(m_self, "_sync", m_callbackBuff.get());
  //reseting in/out buffer pointers
  setp( pbase() , pbase() );
  setg( eback(), eback(), eback() );
  return 0;
}
