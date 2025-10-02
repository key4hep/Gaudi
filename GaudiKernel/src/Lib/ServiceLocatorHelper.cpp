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
#include <GaudiKernel/IService.h>
#include <GaudiKernel/ServiceLocatorHelper.h>

SmartIF<IService> ServiceLocatorHelper::service( std::string_view name, const bool quiet, const bool createIf ) const {
  SmartIF<IService> theSvc = serviceLocator()->service( name, createIf );

  if ( theSvc ) {
    if ( !quiet ) {
      if ( log().level() <= MSG::VERBOSE )
        log() << MSG::VERBOSE << "ServiceLocatorHelper::service: found service " << name << endmsg;
    }
  } else {
    // if not return an error
    if ( !quiet ) { log() << MSG::ERROR << "ServiceLocatorHelper::service: can not locate service " << name << endmsg; }
  }
  return theSvc;
}
