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
#ifndef GAUDIPYTHON_ALGORITHM_H
#define GAUDIPYTHON_ALGORITHM_H
// ============================================================================
// Include Files
// ============================================================================
// Python
// ============================================================================
#include <Python.h>
// ============================================================================
// GaudiKernel
// ============================================================================
#include <GaudiKernel/Algorithm.h>
#include <GaudiKernel/Bootstrap.h>
// ============================================================================
// GaudiPython
// ============================================================================
#include <GaudiPython/GaudiPython.h>
// ============================================================================

namespace GaudiPython {
  /// call the python method
  GAUDI_API StatusCode call_python_method( PyObject* self, const char* method );
} // namespace GaudiPython

namespace GaudiPython {

  /** @class PyAlgorithm
   *  Python Algorithm base class
   */
  class GAUDI_API PyAlgorithm : public Algorithm {
  public:
    /** constructor
     *  @param self python objects
     *  @param name name of algorithm instance
     */
    PyAlgorithm( PyObject* self, const std::string& name );

  public:
    StatusCode  initialize() override;
    StatusCode  start() override;
    StatusCode  execute() override;
    StatusCode  stop() override;
    StatusCode  finalize() override;
    IAlgorithm* myself() { return this; }

  private:
    PyObject* m_self;
  };
  // ==========================================================================
} //                                               end of namespace GaudiPython
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIPYTHON_ALGORITHM_H
