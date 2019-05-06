# Example of an option file that can be used to override the application class
#
# The custom application class is implemented in C++ via ROOT interpreter

import cppyy

# - we have to load GaudiKernel get the base class
cppyy.gbl.gSystem.Load("libGaudiKernel.so")

# - pass to ROOT interpreter our code
cppyy.gbl.gInterpreter.Declare(r'''
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
''')

# IMPORTANT: gaudirun.py must be invoked passing "--application=Test::CustomApp"
