// Include files
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/SmartIF.h"
#include "boost/algorithm/string/predicate.hpp"
#include <iostream>

//--- Example main program
int main( int argc, char** argv )
{
  // Create an instance of an application manager
  auto appMgr  = SmartIF<IAppMgrUI>( Gaudi::createApplicationMgr() );
  auto propMgr = appMgr.as<IProperty>();

  if ( !appMgr || !propMgr ) {
    std::cout << "Fatal error while creating the ApplicationMgr " << std::endl;
    return 1;
  }

  // Get the input configuration file from arguments
  std::string opts = ( argc > 1 ? argv[1] : "jobOptions.txt" );

  propMgr->setProperty( "JobOptionsPath", opts );

  if ( boost::algorithm::ends_with( opts, ".py" ) ) {
    propMgr->setProperty( "EvtSel", "NONE" );
    propMgr->setProperty( "JobOptionsType", "NONE" );
    propMgr->setProperty( "DLLs", "['GaudiPython']" );
    propMgr->setProperty( "Runable", "PythonScriptingSvc" );
  }

  // Run the application manager and process events
  appMgr->run().ignore();

  // All done - exit
  propMgr.reset();
  appMgr.reset();
  return 0;
}
