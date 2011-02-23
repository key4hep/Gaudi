// $Id: GaudiMain.cpp,v 1.4 2006/09/13 15:25:15 hmd Exp $
//------------------------------------------------------------------------------
//
//  Description: Main Program for Gaudi applications
//
//------------------------------------------------------------------------------
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/StatusCode.h"
#include <iostream>

extern "C" GAUDI_API int GaudiMain(int argc,char **argv) {
  IInterface* iface = Gaudi::createApplicationMgr();
  SmartIF<IProperty>     propMgr ( iface );
  SmartIF<IAppMgrUI>     appMgr  ( iface );

  if( !appMgr.isValid() || !propMgr.isValid() ) {
    std::cout << "Fatal error while creating the ApplicationMgr " << std::endl;
    return 1;
  }

  // Get the input configuration file from arguments
  std:: string opts = (argc>1) ? argv[1] : "jobOptions.txt";

  propMgr->setProperty( "JobOptionsPath", opts ).ignore();

  if( opts.substr( opts.length() - 3, 3 ) == ".py" ) {
    propMgr->setProperty( "EvtSel",         "NONE" ).ignore();
    propMgr->setProperty( "JobOptionsType", "NONE" ).ignore();
    propMgr->setProperty( "DLLs",           "['GaudiPython']" ).ignore();
    propMgr->setProperty( "Runable",        "PythonScriptingSvc" ).ignore();
  }

  // Run the application manager and process events
  StatusCode sc = appMgr->run();
  IntegerProperty returnCode("ReturnCode", 0);
  propMgr->getProperty(&returnCode).ignore();
  // Release Application Manager
  iface->release();
  // All done - exit
  if (sc.isFailure() && returnCode.value() == 0) {
    // propagate a valid error code in case of failure
    returnCode.setValue(1);
  }
  return returnCode.value();
}
