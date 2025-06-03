/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "COPYING".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Interfaces/IOptionsSvc.h>
#include <GaudiKernel/IAppMgrUI.h>
#include <GaudiKernel/IService.h>
#include <GaudiKernel/ISvcLocator.h>
#include <stdexcept>

Gaudi::Interfaces::IOptionsSvc& ISvcLocator::getOptsSvc() {
  using namespace Gaudi::Interfaces;
  auto p = dynamic_cast<IOptionsSvc*>( service( "JobOptionsSvc" ).get() );
  if ( !p ) throw std::runtime_error( "cannot dynamic_cast JobOptionsSvc to IOptionsSvc*" );
  return *p;
}

Gaudi::Monitoring::Hub& ISvcLocator::monitoringHub() { return as<IAppMgrUI>()->monitoringHub(); }
