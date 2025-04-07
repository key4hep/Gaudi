/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/Auditor.h>
#include <GaudiKernel/IAlgorithm.h>

class IAlgContextSvc;

/** @class AlgContextAuditor
 *  Description:  Register/Unregister the AlgContext of each
 *  algorithm before entering the algorithm and after leaving it
 *  @author M. Shapiro, LBNL
 *  @author modified by Vanya BELYAEV ibelyaev@physics.syr.edu
 */
class AlgContextAuditor : public Auditor {
public:
  // IAuditor implementation
  using Auditor::after;
  using Auditor::before;
  void before( StandardEventType evt, INamedInterface* a ) override;
  void after( StandardEventType evt, INamedInterface* obj, const StatusCode& sc ) override;

public:
  using Auditor::Auditor;
  StatusCode initialize() override;
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
