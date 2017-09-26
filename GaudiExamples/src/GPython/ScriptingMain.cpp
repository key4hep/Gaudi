// Include files
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/SmartIF.h"

#include <iostream>

//--- Example main program
int main( int argc, char** argv )
{
  StatusCode status = StatusCode::SUCCESS;
  // Create an instance of an application manager
  IInterface* iface = Gaudi::createApplicationMgr();
  SmartIF<IAppMgrUI> appMgr( iface );
  auto propMgr = appMgr.as<IProperty>();

  if ( !appMgr || !propMgr ) {
    std::cout << "Fatal error while creating the ApplicationMgr " << std::endl;
    return 1;
  }

  // Get the input configuration file from arguments
  std::string opts = ( argc > 1 ) ? argv[1] : "jobOptions.txt";

  propMgr->setProperty( "JobOptionsPath", opts );
  if ( opts.compare( opts.length() - 3, 3, ".py" ) == 0 ) {
    propMgr->setProperty( "JobOptionsType", "NONE" );
    propMgr->setProperty( "DLLs", "['SIPython']" );
    propMgr->setProperty( "Runable", "PythonScriptingSvc" );
  }

  // Run the application manager and process events
  appMgr->run();

  // All done - exit
  return 0;
}
