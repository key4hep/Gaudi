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
// ============================================================================
#ifndef GAUDISVC_ALGCONTEXTSVC_H
#define GAUDISVC_ALGCONTEXTSVC_H 1
// ============================================================================
// Include files
// ============================================================================
#include <vector>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IAlgContextSvc.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/StatusCode.h"
#include <boost/thread.hpp>

// ============================================================================
// Forward declarations
// ============================================================================
class IIncidentSvc;
// ============================================================================
/** @class AlgContexSvc
 *  Simple implementation of interface IAlgContextSvc
 *  for Algorithm Context Service
 *  @author ATLAS Collaboration
 *  @author modified by Vanya BELYAEV ibelyaev@physics.sye.edu
 *  @author incident listening  removed by Benedikt Hegner
 *  @author S. Kama. Added multi-context incident based queueing to support
 *          Serial-MT cases
 *  @date 2007-03-07 (modified)
 */
class AlgContextSvc : public extends<Service, IAlgContextSvc, IIncidentListener> {
public:
  /// set the currently executing algorithm ("push_back") @see IAlgContextSvc
  StatusCode setCurrentAlg( IAlgorithm* a, const EventContext& context ) override;
  /// remove the algorithm ("pop_back") @see IAlgContextSvc
  StatusCode unSetCurrentAlg( IAlgorithm* a, const EventContext& context ) override;
  /// accessor to current algorithm: @see IAlgContextSvc
  IAlgorithm* currentAlg() const override;
  /// get the stack of executed algorithms @see IAlgContextSvc
  const IAlgContextSvc::Algorithms& algorithms() const override { return *m_algorithms; }

public:
  /// handle incident @see IIncidentListener
  void handle( const Incident& ) override;

public:
  /// standard initialization of the service @see IService
  StatusCode initialize() override;
  StatusCode start() override;
  /// standard finalization  of the service  @see IService
  StatusCode finalize() override;

public:
  using extends::extends;

private:
  // default/copy constructor & asignment are deleted
  AlgContextSvc()                       = delete;
  AlgContextSvc( const AlgContextSvc& ) = delete;
  AlgContextSvc& operator=( const AlgContextSvc& ) = delete;

private:
  // the stack of current algorithms
  boost::thread_specific_ptr<IAlgContextSvc::Algorithms> m_algorithms; ///< the stack of current algorithms
  // pointer to Incident Service
  SmartIF<IIncidentSvc> m_inc = nullptr; ///< pointer to Incident Service

  Gaudi::Property<bool> m_check{this, "Check", true, "Flag to perform more checks"};
  Gaudi::Property<bool> m_bypassInc{this, "BypassIncidents", false,
                                    "Flag to bypass begin/endevent incident requirement"};
  std::vector<int>      m_inEvtLoop;
};

// ============================================================================
// The END
// ============================================================================
#endif // GAUDISVC_ALGCONTEXTSVC_H
