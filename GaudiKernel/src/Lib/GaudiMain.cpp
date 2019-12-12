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
//------------------------------------------------------------------------------
//
//  Description: Main Program for Gaudi applications
//
//------------------------------------------------------------------------------
#include <Gaudi/Application.h>
#include <gsl/span>
#include <iostream>
#include <string_view>

extern "C" GAUDI_API int GaudiMain( int argc, char** argv ) {
  Gaudi::Application::Options opts;

  std::string_view appType{"Gaudi::Application"};
  std::string_view optsFile;

  gsl::span args{argv, argc};

  auto usage = [name = args[0]]( std::ostream& out ) -> std::ostream& {
    return out << "usage: " << name << " [options] option_file\n";
  };

  auto arg = args.begin();
  ++arg; // ignore application name
  while ( arg != args.end() ) {
    std::string_view opt{*arg};
    if ( opt == "--application" )
      appType = *++arg;
    else if ( opt == "-h" || opt == "--help" ) {
      usage( std::cout );
      std::cout << R"(
Options:
  -h, --help            show this help message and exit
  --application APPLICATION
                        name of the application class to use [default: Gaudi::Application]
)";
      return EXIT_SUCCESS;
    } else if ( opt[0] == '-' ) {
      std::cerr << "error: unknown option " << opt << '\n';
      usage( std::cerr );
      return EXIT_FAILURE;
    } else {
      optsFile = *arg++;
      break; // we stop after the first positional argument
    }
    ++arg;
  }
  if ( arg != args.end() ) { std::cerr << "warning: ignoring extra positional arguments\n"; }
  if ( optsFile.empty() ) {
    std::cerr << "error: missing option file argument\n";
    usage( std::cerr );
    return EXIT_FAILURE;
  }

  if ( optsFile.size() > 3 && optsFile.substr( optsFile.size() - 3 ) == ".py" ) {
    opts["ApplicationMgr.EvtSel"]         = "NONE";
    opts["ApplicationMgr.JobOptionsType"] = "NONE";
    opts["ApplicationMgr.DLLs"]           = "['GaudiPython']";
    opts["ApplicationMgr.Runable"]        = "PythonScriptingSvc";
  } else {
    opts["ApplicationMgr.JobOptionsPath"] = optsFile;
  }

  auto app = Gaudi::Application::create( appType, std::move( opts ) );
  if ( !app ) {
    std::cerr << "error: failure creating " << appType << '\n';
    return EXIT_FAILURE;
  }

  return app->run();
}
