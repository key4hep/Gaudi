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
// ============================================================================
// Include files
// ============================================================================
#include <Python.h>

// Python 3 compatibility
#if PY_MAJOR_VERSION >= 3

#  define PyInt_Check PyLong_Check

#  define PyInt_AS_LONG PyLong_AS_LONG

#endif

// ============================================================================
// GaudiKernel
// ============================================================================
#include <GaudiKernel/Bootstrap.h>
// ============================================================================
// GaudiPython
// ============================================================================
#include <GaudiPython/Algorithm.h>
// ============================================================================

namespace {
  /// @FIXME: (MCl) work-around to fix the warning
  ///
  ///   warning: deprecated conversion from string constant to ‘char*’
  ///
  /// that occurs when a string constant (e.g.: "abc", "") is passed to a function
  /// expecting char *
  inline char* chr( const char* c ) { return const_cast<char*>( c ); }
} // namespace

// ============================================================================
/// call the python method
// ============================================================================
StatusCode GaudiPython::call_python_method( PyObject* self, const char* method ) {
  StatusCode sc = StatusCode::FAILURE;
  // check arguments
  if ( !self || !method ) { return StatusCode::FAILURE; }

  // call Python
  PyObject* r = PyObject_CallMethod( self, chr( method ), nullptr );

  if ( !r ) {
    PyErr_Print();
    return sc;
  } // RETURN

  if ( PyInt_Check( r ) ) {
    sc = StatusCode( PyInt_AS_LONG( r ) );
    Py_DECREF( r );
    return sc;
  } // RETURN

  // look for the method getCode with the signature:
  //  ' int getCode() '
  PyObject* c = PyObject_CallMethod( r, chr( "getCode" ), nullptr );

  if ( !c ) {
    PyErr_Print();
  } else if ( PyLong_Check( c ) ) {
    sc = StatusCode( PyLong_AsLong( c ) );
  } else {
    std::string msg( " call_python_method unexpected type from '" );
    msg += method;
    msg += "().getCode()' ";
    PyErr_SetString( PyExc_TypeError, msg.c_str() );
    PyErr_Print();
  }
  // release used objects
  Py_XDECREF( c );
  Py_XDECREF( r );
  //
  return sc;
}
// ============================================================================

// ============================================================================
/** constructor
 *  @param self python objects
 *  @param name name of algorithm instance
 */
// ============================================================================
GaudiPython::PyAlgorithm::PyAlgorithm( PyObject* self, const std::string& name )
    : Algorithm( name, Gaudi::svcLocator() ), m_self( self ) {
  // The owner of the Algorithm is Python (as creator) therefore
  // it should not be deleted by Gaudi (added an extra addRef()).
  addRef();
  addRef();
}
// ============================================================================

// ============================================================================
StatusCode GaudiPython::PyAlgorithm::initialize() { return GaudiPython::call_python_method( m_self, "initialize" ); }
// ============================================================================
StatusCode GaudiPython::PyAlgorithm::start() { return GaudiPython::call_python_method( m_self, "start" ); }
// ============================================================================
StatusCode GaudiPython::PyAlgorithm::execute() { return GaudiPython::call_python_method( m_self, "execute" ); }
// ============================================================================
StatusCode GaudiPython::PyAlgorithm::stop() { return GaudiPython::call_python_method( m_self, "stop" ); }
// ============================================================================
StatusCode GaudiPython::PyAlgorithm::finalize() { return GaudiPython::call_python_method( m_self, "finalize" ); }
