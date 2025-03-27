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

#include <Gaudi/IAuditor.h>
#include <GaudiKernel/IService.h>
#include <GaudiKernel/System.h>

#include <optional>
#include <string>

/**
 * The interface implemented by the IAuditorSvc base class.
 * @author Pere Mato
 */
class GAUDI_API IAuditorSvc : virtual public extend_interfaces<IService, Gaudi::IAuditor> {
public:
  /// InterfaceID
  DeclareInterfaceID( IAuditorSvc, 4, 0 );

  /// management functionality: retrieve an Auditor
  virtual Gaudi::IAuditor* getAuditor( std::string const& name ) const = 0;
  /// management functionality: check if an Auditor exists
  virtual bool hasAuditor( std::string const& name ) const = 0;

  /// adds a new Auditor
  virtual StatusCode addAuditor( std::string const& name ) = 0;

  /// removes an Auditor. Returns whether the removal was successful if the Auditor was present
  virtual std::optional<StatusCode> removesAuditor( std::string const& name ) = 0;
};
