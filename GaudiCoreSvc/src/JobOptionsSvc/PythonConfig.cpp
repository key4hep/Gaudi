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
#include "PythonConfig.h"

#include <iostream>

// boost includes
#include <boost/python.hpp>
using namespace boost::python;

//-----------------------------------------------------------------------------
StatusCode PythonConfig::evaluateConfig( const std::string& filename, const std::string& preAction,
                                         const std::string& postAction ) {
  try {
    // Prepare a Python environment
    Py_Initialize();
    object main_module( ( handle<>( borrowed( PyImport_AddModule( "__main__" ) ) ) ) );
    object main_namespace = main_module.attr( "__dict__" );

    // make the translator class known to Python
    main_namespace["PythonAdaptor"] =
        class_<PythonAdaptor>( "PythonAdaptor", boost::python::init<Gaudi::Interfaces::IOptionsSvc*>() )
            .def( "addPropertyToJobOptions", &PythonAdaptor::addPropertyToJobOptions );

    // create an instance of it and pass it to Python
    PythonAdaptor adaptor( m_optsSvc );
    main_namespace["adaptor"] = ptr( &adaptor );

    // some python helper
    std::string command( preAction );
    command += "\nfor name in '";
    command += filename + "'.split(','):\n";
    command += "    with open(name) as f:\n";
    command += "        code = compile(f.read(), name, 'exec')\n";
    command += "        exec(code)\n";
    command += "from GaudiKernel.Configurable import expandvars\nfrom GaudiKernel.Proxy.Configurable import "
               "applyConfigurableUsers\napplyConfigurableUsers()\n";
    command += postAction;
    command += "\nfor n, c in Configurable.allConfigurables.items():\n  for p, v in  c.getValuedProperties().items() "
               ":\n    v = expandvars(v)\n    if   type(v) == str : v = repr(v)  # need double quotes\n    elif "
               "type(v) == long: v = '%d' % v  # prevent pending 'L'\n    adaptor.addPropertyToJobOptions(n,p,str(v))";

    // Now fire off the translation
    handle<> ignored( ( PyRun_String( command.c_str(), Py_file_input, main_namespace.ptr(), main_namespace.ptr() ) ) );
  } catch ( const error_already_set& ) {
    std::cout << "Error in python script:" << std::endl;
    PyErr_Print();
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}
