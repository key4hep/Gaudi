/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiPython/Algorithm.h>
#include <Python.h>

namespace GaudiPython {
  // ==========================================================================
  /** @class PyAlg
   *  general class to embed the existing algorithm/base class
   *  into the python
   *  @author Vanya BELYAEV  Ivan.Belyaev@lapp.in2p3.fr
   *  @date 2005-08-03
   */
  template <class ALGORITHM>
  class GAUDI_API PyAlg : public ALGORITHM {
    // ========================================================================
  public:
    // ========================================================================
    /** constructor from Python object and the name
     *  @param self python object
     *  @param name name of algorithm instance
     */
    PyAlg( PyObject* self, const std::string& name ) : ALGORITHM( name, Gaudi::svcLocator() ), m_self( self ) {
      // the printout of actual type for embedded algorithm has no sense
      this->setProperty( "TypePrint", false );
      // The owner of the Algorithm is Python (as creator) therefore
      // it should not be deleted by Gaudi (added an extra addRef()).
      this->addRef();
      this->addRef();
      this->setType( System::typeinfoName( typeid( PyAlg ) ) );
    }
    /// get the object
    PyObject* _obj() const { return m_self; } //     get the object
    // ========================================================================
  public:
    // ========================================================================
    StatusCode initialize() override { return GaudiPython::call_python_method( m_self, "initialize" ); }
    StatusCode start() override { return GaudiPython::call_python_method( m_self, "start" ); }
    StatusCode execute() override { return GaudiPython::call_python_method( m_self, "execute" ); }
    StatusCode stop() override { return GaudiPython::call_python_method( m_self, "stop" ); }
    StatusCode finalize() override { return GaudiPython::call_python_method( m_self, "finalize" ); }
    // ========================================================================
    virtual IAlgorithm* ialgorithm() { return this; }
    virtual IProperty*  iproperty() { return this; }
    // ========================================================================
    // preserve the existing methods
    virtual StatusCode initialize_() { return ALGORITHM::initialize(); }
    virtual StatusCode finalize_() { return ALGORITHM::finalize(); }
    // ========================================================================
  private:
    // ========================================================================
    /// the default constructor is disabled
    PyAlg() = delete;
    /// the copy constructor is disabled
    PyAlg( const PyAlg& ) = delete;
    /// the assignment operator is disabled
    PyAlg& operator=( const PyAlg& ) = delete;
    // ========================================================================
  private:
    // ========================================================================
    /// "shadow" python class
    PyObject* m_self; // "shadow" python class
    // ========================================================================
  };
  // ==========================================================================
} // namespace GaudiPython
