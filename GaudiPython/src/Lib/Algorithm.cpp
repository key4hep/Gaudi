// $Id: Algorithm.cpp,v 1.8 2008/10/28 10:40:19 marcocle Exp $
// ============================================================================
// Incldue files
// ============================================================================
#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif
#include "Python.h"
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Bootstrap.h"
// ============================================================================
// GaudiPython
// ============================================================================
#include "GaudiPython/Algorithm.h"
// ============================================================================

namespace {
/// @FIXME: (MCl) work-around to fix the warning
///
///   warning: deprecated conversion from string constant to ‘char*’
///
/// that occurs when a string constant (e.g.: "abc", "") is passed to a function
/// expecting char *
  inline char *chr(const char*c){ return const_cast<char*>(c); }
}

// ============================================================================
/// call the python method
// ============================================================================
StatusCode GaudiPython::call_python_method
( PyObject* self, const char* method )
{
  StatusCode sc = StatusCode::FAILURE;
  // check arguments
  if ( 0 == self || 0 == method ) { return StatusCode::FAILURE ; }

  // call Python
  PyObject* r = PyObject_CallMethod(self, chr(method), chr(""));

  if ( 0 == r ) { PyErr_Print() ;              return sc ; } // RETURN

  if ( PyInt_Check ( r ) )
  { sc = PyInt_AS_LONG( r ) ; Py_DECREF( r ) ; return sc ; } // RETURN

  // look for the method getCode with the signature:
  //  ' int getCode() '
  PyObject* c = PyObject_CallMethod(r, chr("getCode"), chr(""));

  if      ( 0 == c           ) {      PyErr_Print()      ; }
  else if ( PyLong_Check( c )) { sc = PyLong_AsLong( c ); }
  else
  {
    std::string msg( " call_python_method unexpected type from '" );
    msg += method ;
    msg += "().getCode()' " ;
    PyErr_SetString( PyExc_TypeError , msg.c_str() ) ;
    PyErr_Print() ;
  }
  // release used objects
  Py_XDECREF( c ) ;
  Py_XDECREF( r ) ;
  //
  return sc;
} ;
// ============================================================================

// ============================================================================
/** constructor
 *  @param self python objects
 *  @param name name of algorithm instance
 */
// ============================================================================
GaudiPython::PyAlgorithm::PyAlgorithm
( PyObject*          self ,
  const std::string& name )
  : Algorithm ( name , Gaudi::svcLocator() )
  , m_self ( self )
{
  // The owner of the Algorithm is Python (as creator) therefore
  // it should not be deleted by Gaudi (added an extra addRef()).
  addRef() ;
  addRef() ;
};
// ============================================================================

// ============================================================================
StatusCode GaudiPython::PyAlgorithm::initialize ()
{ return GaudiPython::call_python_method ( m_self , "initialize" ) ; }
// ============================================================================
StatusCode GaudiPython::PyAlgorithm::start ()
{ return GaudiPython::call_python_method ( m_self , "start" ) ; }
// ============================================================================
StatusCode GaudiPython::PyAlgorithm::execute    ()
{ return GaudiPython::call_python_method ( m_self , "execute" ) ; }
// ============================================================================
StatusCode GaudiPython::PyAlgorithm::stop ()
{ return GaudiPython::call_python_method ( m_self , "stop" ) ; }
// ============================================================================
StatusCode GaudiPython::PyAlgorithm::finalize   ()
{ return GaudiPython::call_python_method ( m_self , "finalize" ) ; }

StatusCode GaudiPython::PyAlgorithm::beginRun() {
  return GaudiPython::call_python_method ( m_self , "beginRun" ) ; }
StatusCode GaudiPython::PyAlgorithm::endRun() {
  return GaudiPython::call_python_method ( m_self , "endRun" ) ; }

