/***********************************************************************************\
* (c) Copyright 1998-2021 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "GaudiKernel/SmartRefVector.h"
#include "Python.h"
#include "RVersion.h"

void SmartRefVectorImpl::SmartRefVectorPythonizer::__cppyy_pythonize__( PyObject* klass, const std::string& name ) {
  if ( name.substr( 0, 15 ) == "SmartRefVector<" ) {
    PyObject_DelAttrString( klass, "__getitem__" );
    PyObject_DelAttrString( klass, "__setitem__" );
  }
}
