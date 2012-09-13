#include "Python.h"   //included for the python API
// ============================================================================
#include "GaudiKernel/Bootstrap.h"
// ============================================================================
#include <sstream>    //included for stringbuf
// ============================================================================
#include "GaudiPython/GaudiPython.h"
// ============================================================================
#include "GaudiPython/CallbackStreamBuf.h" //include header file
// ============================================================================

#define min(a,b) (((a)<(b))?(a):(b))

using namespace std;

// ============================================================================
/// call a python method with a string as an argument
// ============================================================================
int GaudiPython::call_python_method
( PyObject* self, const char* method, char* buf )
{
  if ( 0 == self || 0 == method ) { return 1 ; }
  PyObject* r = PyObject_CallMethod(self, const_cast<char*>(method),
                                          const_cast<char*>("s"), buf);
  if ( 0 == r ) {
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
GaudiPython::CallbackStreamBuf::CallbackStreamBuf
(PyObject* self): stringbuf(), m_self(self)
{
  Py_INCREF(this->m_self);
  this->m_callbackBuff = new char[512]; //used for passing the flushed chars in the python callback
}

// ============================================================================
/// CallbackStreamBuf destructor
// ============================================================================
GaudiPython::CallbackStreamBuf::~CallbackStreamBuf()
{
  Py_DECREF( this->m_self );
  delete this->m_callbackBuff;
}

// ============================================================================
/// reimplementation of stringbuf::sync()
// ============================================================================
int GaudiPython::CallbackStreamBuf::sync ()
{
  int length;
  char *x;
  for ( length = 0, x = this->pbase(); x < this->epptr(); x++ , length++ ) ;
  //getting in a null terminated buffer the characters
  memcpy( this->m_callbackBuff, this->pbase(), min(length, 512) );
  this->m_callbackBuff[ min(length, 512) ] = '\0';
  //calling the python method
  GaudiPython::call_python_method(this->m_self, "_sync", this->m_callbackBuff);
  //reseting in/out buffer pointers
  setp( this->pbase() , this->pbase() );
  setg( this->eback(), this->eback(), this->eback() );
  return 0;
}
