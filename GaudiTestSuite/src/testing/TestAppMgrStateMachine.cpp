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
// Executable to check some state transitions of the ApplicationMgr

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/StatusCode.h"
#include <iostream>

#define CHECK( SC )                                                                                                    \
  if ( SC.isFailure() ) return 1;

int main() {
  SmartIF<IAppMgrUI> appMgr( Gaudi::createApplicationMgr() );
  auto               propMgr = appMgr.as<IProperty>();

  if ( !appMgr || !propMgr ) {
    std::cout << "Fatal error while creating the ApplicationMgr " << std::endl;
    return 1;
  }

  propMgr->setProperty( "JobOptionsType", "NONE" ).ignore();
  propMgr->setProperty( "EvtMax", 1 ).ignore();

  CHECK( appMgr->configure() );
  CHECK( appMgr->initialize() );
  CHECK( appMgr->reinitialize() );
  CHECK( appMgr->start() );
  CHECK( appMgr->stop() );
  CHECK( appMgr->start() );
  CHECK( appMgr->run() );
}
