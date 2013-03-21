#include <iostream>
#include <string>
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/SmartIF.h"

// boost includes
#include "boost/program_options.hpp"

using namespace boost::program_options;

//-----------------------------------------------------------------------------
int main ( int argc, char** argv )
{
  // Declare the command line options and read them in
  options_description desc("Allowed options");
  desc.add_options()
        ("help", "produce help message")
        ("config", value<std::string>(), "the python configuration file")
  ;
  positional_options_description p;
  p.add("config", 1);
  variables_map vm;
  store(command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
  notify(vm);

  std::string fileName;
  // handle the retrieved options
  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }
  if (!vm.count("config")) {
    std::cout << "Please specify a config file" << std::endl;
    return 1;
  } else {
    fileName = vm["config"].as<std::string>();
  }
  // end of options handling

  IInterface* iface = Gaudi::createApplicationMgr();
  SmartIF<IProperty> propMgr ( iface );
  SmartIF<IAppMgrUI> appUI  ( iface );
  propMgr->setProperty("JobOptionsType","PYTHON");
  propMgr->setProperty("JobOptionsPath",fileName); 

  return appUI->run();
}
