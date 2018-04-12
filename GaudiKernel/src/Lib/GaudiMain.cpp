//------------------------------------------------------------------------------
//
//  Description: Main Program for Gaudi applications
//
//------------------------------------------------------------------------------
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/StatusCode.h"
#include <iostream>

extern "C" GAUDI_API int GaudiMain( int argc, char** argv )
{
  IInterface*        iface = Gaudi::createApplicationMgr();
  SmartIF<IAppMgrUI> appMgr( iface );
  auto               propMgr = appMgr.as<IProperty>();

  if ( !appMgr || !propMgr ) {
    std::cout << "Fatal error while creating the ApplicationMgr " << std::endl;
    return 1;
  }

  // Get the input configuration file from arguments
  std::string opts = ( argc > 1 ) ? argv[1] : "jobOptions.txt";

  propMgr->setProperty( "JobOptionsPath", opts ).ignore();

  if ( opts.compare( opts.length() - 3, 3, ".py" ) == 0 ) {
    propMgr->setProperty( "EvtSel", "NONE" ).ignore();
    propMgr->setProperty( "JobOptionsType", "NONE" ).ignore();
    propMgr->setProperty( "DLLs", "['GaudiPython']" ).ignore();
    propMgr->setProperty( "Runable", "PythonScriptingSvc" ).ignore();
  }

  // Run the application manager and process events
  StatusCode           sc = appMgr->run();
  Gaudi::Property<int> returnCode( "ReturnCode", 0 );
  propMgr->getProperty( &returnCode ).ignore();
  // Release Application Manager
  propMgr.reset();
  appMgr.reset();
  // All done - exit
  if ( sc.isFailure() && returnCode == 0 ) {
    // propagate a valid error code in case of failure
    returnCode.setValue( 1 );
  }
  return returnCode.value();
}
