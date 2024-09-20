/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Property.h>
#include <GaudiKernel/Bootstrap.h>
#include <GaudiKernel/IAppMgrUI.h>
#include <GaudiKernel/IProperty.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/SmartIF.h>
#include <iostream>
#include <string>

// boost includes
#include <boost/program_options.hpp>

using namespace boost::program_options;

//-----------------------------------------------------------------------------
int main( int argc, char** argv ) {
  // Declare the command line options and read them in
  options_description desc( "Allowed options" );
  desc.add_options()( "help,h", "produce help message" )( "config", value<std::string>(),
                                                          "comma separated list of python configuration files" )(
      "post-option", value<std::string>(), "python command to be run after ConfigurableUser has been applied" )(
      "param", value<std::vector<std::string>>(), "parameters to the configuration file, multiple can be given" );
  positional_options_description p;
  p.add( "config", 1 );
  variables_map vm;
  try {
    store( command_line_parser( argc, argv ).options( desc ).positional( p ).run(), vm );
  } catch ( boost::exception_detail::clone_impl<
            boost::exception_detail::error_info_injector<boost::program_options::unknown_option>>& ) {
    std::cerr << "Unknown option(s) detected." << std::endl << "Usage:" << std::endl << desc << std::endl;
    return ( 1 );
  }
  notify( vm );

  std::string fileName;
  // handle the retrieved options
  if ( vm.count( "help" ) or vm.count( "h" ) ) {
    std::cout << desc << std::endl;
    return 1;
  }
  if ( !vm.count( "config" ) ) {
    std::cout << "Please specify a config file" << std::endl;
    return 1;
  } else {
    fileName = vm["config"].as<std::string>();
  }

  std::stringstream params;
  if ( 0 != vm.count( "param" ) ) {
    std::vector<std::string> vParams = vm["param"].as<std::vector<std::string>>();
    std::cout << "Configuration parameters are:" << std::endl;
    for ( std::string s : vParams ) {
      params << s << "\n";
      std::cout << "\t" << s << std::endl;
    }
  }

  std::string postAction;
  if ( 0 != vm.count( "post-option" ) ) { postAction = vm["post-option"].as<std::string>(); }
  // end of options handling

  IInterface*        iface = Gaudi::createApplicationMgr();
  SmartIF<IProperty> propMgr( iface );
  SmartIF<IAppMgrUI> appUI( iface );
  propMgr->setProperty( "JobOptionsType", "PYTHON" ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
  propMgr->setProperty( "JobOptionsPath", fileName ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
  propMgr->setProperty( "JobOptionsPreAction", params.str() ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
  propMgr->setProperty( "JobOptionsPostAction", postAction )
      .ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ ); // TODO: grep this from command line
  return appUI->run().getCode();
}
