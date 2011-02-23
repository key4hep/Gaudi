// Include files
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/System.h"
#include <iostream>

//--- Example main program
extern "C" 
#ifdef WIN32
__declspec(dllexport)
#endif
int ExampleMain () {
  int argc    = System::argc();
  char** argv = &(System::argv())[1];
  // Create an instance of an application manager
  IInterface* iface = Gaudi::createApplicationMgr();
  SmartIF<IAppMgrUI> appMgr(iface);
  SmartIF<IProperty> propMgr(iface);

  if( !appMgr.isValid() || !propMgr.isValid() ) {
    std::cout << "Fatal error while creating the ApplicationMgr " << std::endl;
    return 1;
  }

  // Get the input configuration file from arguments
  std::string opts = (argc>1) ? argv[1] : "jobOptions.txt";

  propMgr->setProperty( "JobOptionsPath", opts );
  if( opts.substr( opts.length() - 3, 3 ) == ".py" ) {
    propMgr->setProperty( "EvtSel",         "NONE" );
    propMgr->setProperty( "JobOptionsType", "NONE" );
    propMgr->setProperty( "DLLs",           "['GaudiPython']" );
    propMgr->setProperty( "Runable",        "PythonScriptingSvc" );
  }

  // Run the application manager and process events
  appMgr->run();

  // All done - exit
  iface->release();
  return 0;
}
