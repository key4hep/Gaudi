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
#include "GaudiKernel/ServiceLocatorHelper.h"

#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"

StatusCode ServiceLocatorHelper::locateService( const std::string& name, const InterfaceID& iid, void** ppSvc,
                                                bool quiet ) const {
  auto theSvc = service( name, quiet, false );
  if ( !theSvc ) return StatusCode::FAILURE;
  StatusCode sc = theSvc->queryInterface( iid, ppSvc );
  if ( !sc.isSuccess() ) {
    *ppSvc = nullptr;
    if ( !quiet )
      log() << MSG::ERROR << "ServiceLocatorHelper::locateService: wrong interface id " << iid << " for service "
            << name << endmsg;
  }
  return sc;
}

StatusCode ServiceLocatorHelper::createService( const std::string& name, const InterfaceID& iid, void** ppSvc ) const {
  auto theSvc = service( name, false, true );
  if ( !theSvc ) return StatusCode::FAILURE;
  StatusCode sc = theSvc->queryInterface( iid, ppSvc );
  if ( !sc.isSuccess() ) {
    *ppSvc = nullptr;
    log() << MSG::ERROR << "ServiceLocatorHelper::createService: wrong interface id " << iid << " for service " << name
          << endmsg;
  }
  return sc;
}

StatusCode ServiceLocatorHelper::createService( std::string_view type, std::string_view name, const InterfaceID& iid,
                                                void** ppSvc ) const {
  return createService( std::string{type}.append( "/" ).append( name ), iid, ppSvc );
}

SmartIF<IService> ServiceLocatorHelper::service( const std::string& name, const bool quiet,
                                                 const bool createIf ) const {
  SmartIF<IService> theSvc = serviceLocator()->service( name, createIf );

  if ( theSvc ) {
    if ( !quiet ) {
      if ( UNLIKELY( log().level() <= MSG::VERBOSE ) )
        log() << MSG::VERBOSE << "ServiceLocatorHelper::service: found service " << name << endmsg;
    }
  } else {
    // if not return an error
    if ( !quiet ) { log() << MSG::ERROR << "ServiceLocatorHelper::service: can not locate service " << name << endmsg; }
  }
  return theSvc;
}
