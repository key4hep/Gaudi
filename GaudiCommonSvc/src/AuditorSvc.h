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

// Include Files
#include <GaudiKernel/IAuditorSvc.h>
#include <GaudiKernel/Service.h>
#include <list>
#include <vector>

//------------------------------------------------------------------
//
// ClassName:   AuditorSvc
//
// Description: This service manages Auditors.
//              Auditors can be inserted in the system to audit the Algorithms is
//              in their functions.
//------------------------------------------------------------------
class AuditorSvc : public extends<Service, IAuditorSvc> {
public:
  StatusCode initialize() override;
  StatusCode finalize() override;

  // IAuditorSvc interfaces overwrite
  //
  /// The following methods are meant to be implemented by the child class...

  void before( StandardEventType, INamedInterface* ) override;
  void before( StandardEventType, const std::string& ) override;

  void before( CustomEventTypeRef, INamedInterface* ) override;
  void before( CustomEventTypeRef, const std::string& ) override;

  void after( StandardEventType, INamedInterface*, const StatusCode& ) override;
  void after( StandardEventType, const std::string&, const StatusCode& ) override;

  void after( CustomEventTypeRef, INamedInterface*, const StatusCode& ) override;
  void after( CustomEventTypeRef, const std::string&, const StatusCode& ) override;

  bool isEnabled() const override;

  StatusCode sysInitialize() override;
  StatusCode sysFinalize() override;

  // management functionality: retrieve an Auditor
  IAuditor* getAuditor( const std::string& name ) override;

  using extends::extends;

private:
  // management helper
  SmartIF<IAuditor> newAuditor_( MsgStream&, std::string_view );
  SmartIF<IAuditor> findAuditor_( std::string_view );
  StatusCode        syncAuditors_();

  Gaudi::Property<std::vector<std::string>> m_audNameList{
      this, "Auditors", {}, "list of auditors names", "OrderedSet<std::string>" };
  Gaudi::Property<bool> m_isEnabled{ this, "Enable", true, "enable/disable alltogether the auditors" };

  // Manager list of Auditors
  std::vector<SmartIF<IAuditor>> m_pAudList;
};
