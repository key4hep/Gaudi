/***********************************************************************************\
* (c) Copyright 2024-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "COPYING".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/IInterface.h>
#include <memory>
#include <string>

class TFile;

namespace Gaudi {
  namespace Interfaces {
    /** Interface for a component that manages file access within Gaudi applications.
     *
     * This service allows algorithms to access ROOT files via a centralized service,
     * ensuring that files are shared appropriately and not unintentionally overwritten
     * or duplicated.
     */
    struct IFileSvc : virtual IInterface {
      // InterfaceID declaration.
      DeclareInterfaceID( IFileSvc, 1, 1 );

      /// Get a shared pointer to a TFile based on an identifier.
      virtual std::shared_ptr<TFile> getFile( const std::string& identifier ) = 0;

      /// Check if a given identifier is known to the service.
      virtual bool hasIdentifier( const std::string& identifier ) const = 0;
    };
  } // namespace Interfaces
} // namespace Gaudi
