/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIAUD_ALGCONTEXTAUDITOR_H
#define GAUDIAUD_ALGCONTEXTAUDITOR_H
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Auditor.h"
#include "GaudiKernel/IAlgorithm.h"
// ============================================================================
// Forward declarations
// ============================================================================
class IAlgContextSvc;
// ============================================================================
/** @class AlgContextAuditor
 *  Description:  Register/Unregister the AlgContext of each
 *  algorithm before entering the algorithm and after leaving it
 *  @author M. Shapiro, LBNL
 *  @author modified by Vanya BELYAEV ibelyaev@physics.syr.edu
 */
class AlgContextAuditor : public Auditor {
public:
  // IAuditor implementation
  void beforeInitialize( INamedInterface* a ) override;
  void afterInitialize( INamedInterface* a ) override;
  //
  void beforeExecute( INamedInterface* a ) override;
  void afterExecute( INamedInterface* a, const StatusCode& s ) override;
  //
  void beforeFinalize( INamedInterface* a ) override;
  void afterFinalize( INamedInterface* a ) override;

public:
  /// standard constructor/destructor @see Auditor
  AlgContextAuditor( const std::string& name, ISvcLocator* pSvc );
  /// standard initialization, @see IAuditor
  StatusCode initialize() override;
  /// standard finalization, @see IAuditor
  StatusCode finalize() override;

private:
  /// delete the default/copy constructor and assignment
  AlgContextAuditor()                                      = delete;
  AlgContextAuditor( const AlgContextAuditor& )            = delete;
  AlgContextAuditor& operator=( const AlgContextAuditor& ) = delete;

private:
  /// the pointer to Algorithm Context Service
  SmartIF<IAlgContextSvc> m_svc;
};

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIAUD_ALGCONTEXTAUDITOR_H
