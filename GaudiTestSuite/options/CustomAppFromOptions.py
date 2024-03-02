#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# Example of an option file that can be used to override the application class
#
# The custom application class is implemented in C++ via ROOT interpreter

# Workaround for ROOT-10769
import warnings

with warnings.catch_warnings():
    warnings.simplefilter("ignore")
    import cppyy

# - we have to load GaudiKernel get the base class
cppyy.gbl.gSystem.Load("libGaudiKernel.so")

# - pass to ROOT interpreter our code
cppyy.gbl.gInterpreter.Declare(
    r"""
#include <Gaudi/Application.h>
#include <iostream>

namespace Test {
  class CustomApp : public Gaudi::Application {
    using Application::Application;
    int run() override {
      std::cout << "=== Custom Application ===\n";
      return EXIT_SUCCESS;
    }
  };
}
DECLARE_COMPONENT(Test::CustomApp)
"""
)

# IMPORTANT: gaudirun.py must be invoked passing "--application=Test::CustomApp"
