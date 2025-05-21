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

// Include files
#include <GaudiKernel/INamedInterface.h>
#include <GaudiKernel/IStateful.h>
#include <string>

/** @class IService IService.h GaudiKernel/IService.h

    General service interface definition

    @author Pere Mato
*/
class ISvcManager;
class ServiceManager;

class GAUDI_API IService : virtual public extend_interfaces<INamedInterface, IStateful> {
  friend class ServiceManager;

public:
  /// InterfaceID
  DeclareInterfaceID( IService, 4, 0 );

  /// Initialize Service
  virtual StatusCode sysInitialize() = 0;
  /// Start Service
  virtual StatusCode sysStart() = 0;
  /// Stop Service
  virtual StatusCode sysStop() = 0;
  /// Finalize Service
  virtual StatusCode sysFinalize() = 0;
  /// Re-initialize the Service
  virtual StatusCode sysReinitialize() = 0;
  /// Re-start the Service
  virtual StatusCode sysRestart() = 0;

protected:
  virtual void setServiceManager( ISvcManager* ) = 0;
};
