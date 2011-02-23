//$Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiExamples/src/GPython/ScriptingMain.cpp,v 1.4 2001/06/29 16:30:21 mato Exp $	//

// Include files
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/IProperty.h"

#include <iostream>

//--- Example main program
int main ( int argc, char** argv ) {
  StatusCode status = StatusCode::SUCCESS;
  // Create an instance of an application manager
  IInterface* iface = Gaudi::createApplicationMgr();
  SmartIF<IProperty>     propMgr ( iface );
  SmartIF<IAppMgrUI>     appMgr  ( iface );

  if( !appMgr.isValid() || !propMgr.isValid() ) {
    std::cout << "Fatal error while creating the ApplicationMgr " << std::endl;
    return 1;
  }

  // Get the input configuration file from arguments
  std:: string opts = (argc>1) ? argv[1] : "jobOptions.txt";

  propMgr->setProperty( "JobOptionsPath", opts );
  if( opts.substr( opts.length() - 3, 3 ) == ".py" ) {
    propMgr->setProperty( "JobOptionsType", "NONE" );
    propMgr->setProperty( "DLLs",           "['SIPython']" );
    propMgr->setProperty( "Runable",        "PythonScriptingSvc" );
  }

  // Run the application manager and process events
  appMgr->run();

  // All done - exit
  return 0;
}
