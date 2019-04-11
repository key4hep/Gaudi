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
  propMgr->setProperty( "EvtMax", "1" ).ignore();

  CHECK( appMgr->configure() );
  CHECK( appMgr->initialize() );
  CHECK( appMgr->reinitialize() );
  CHECK( appMgr->start() );
  CHECK( appMgr->stop() );
  CHECK( appMgr->start() );
  CHECK( appMgr->run() );
}
