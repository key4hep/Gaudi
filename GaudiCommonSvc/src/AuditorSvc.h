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

#include <GaudiKernel/IAuditorSvc.h>
#include <GaudiKernel/Service.h>
#include <memory>
#include <vector>

/**
 * This service manages Auditors.
 * Auditors can be inserted in the system to audit the Algorithms is in their functions.
 */
class AuditorSvc : public extends<Service, IAuditorSvc> {
public:
  StatusCode finalize() override;

  void before( std::string const&, std::string const&, EventContext const& ) override;
  void after( std::string const&, std::string const&, EventContext const&, StatusCode const& ) override;

  bool isEnabled() const override { return m_isEnabled; };

  StatusCode sysInitialize() override { return Service::sysInitialize(); }
  StatusCode sysFinalize() override { return Service::sysFinalize(); }

  /// management functionality: retrieve an Auditor
  Gaudi::IAuditor* getAuditor( std::string const& name ) const override;
  /// management functionality: check if an Auditor exists
  bool hasAuditor( std::string const& name ) const override { return getAuditor( name ) != nullptr; };
  /// adds a new Auditor
  StatusCode addAuditor( std::string const& name ) override;
  /// removes an Auditor. Returns whether the Auditor was present (and thus removed)
  std::optional<StatusCode> removesAuditor( std::string const& name ) override;

  using extends::extends;

private:
  std::unique_ptr<Gaudi::IAuditor> newAuditor( MsgStream&, std::string_view );
  /// internal mathod to update auditors when m_audNameList is changed
  StatusCode syncAuditors();

  Gaudi::Property<std::vector<std::string>> m_audNameList{ this,
                                                           "Auditors",
                                                           {},
                                                           [this]( auto& ) {
                                                             auto sc = syncAuditors();
                                                             if ( sc.isFailure() ) {
                                                               error() << "Unable to update Auditors : " << endmsg;
                                                             };
                                                           },
                                                           "list of auditors names",
                                                           "OrderedSet<std::string>" };
  Gaudi::Property<bool> m_isEnabled{ this, "Enable", true, "enable/disable alltogether the auditors" };

  // Manager list of Auditors
  std::vector<std::unique_ptr<Gaudi::IAuditor>> m_pAudList;
};
