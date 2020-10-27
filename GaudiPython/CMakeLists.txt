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
# GaudiPython subdirectory

# Build the library
gaudi_add_library(GaudiPythonLib
                  SOURCES src/Lib/AlgDecorators.cpp
                          src/Lib/CallbackStreamBuf.cpp
                          src/Lib/Algorithm.cpp
                          src/Lib/Helpers.cpp
                  LINK
                    PUBLIC GaudiAlgLib
                           Python::Python)
if(GAUDI_USE_AIDA)
   target_sources(GaudiPythonLib PRIVATE src/Lib/HistoDecorator.cpp)
   target_link_libraries(GaudiPythonLib PUBLIC AIDA::aida)
endif()
if(GAUDI_USE_CLHEP)
   target_sources(GaudiPythonLib PRIVATE src/Lib/TupleDecorator.cpp)
   target_link_libraries(GaudiPythonLib PUBLIC CLHEP::CLHEP)
endif()

# Build the plugin
gaudi_add_module(GaudiPython
                 SOURCES src/Services/PythonScriptingSvc.cpp
                 LINK GaudiPythonLib
                      GaudiUtilsLib)

# ROOT dictionaries
gaudi_add_dictionary(GaudiPythonDict
                     HEADERFILES dict/kernel.h
                     SELECTION dict/selection_kernel.xml
                     LINK GaudiPythonLib
                          GaudiUtilsLib)
if(GAUDI_USE_AIDA)
   target_compile_definitions(GaudiPythonDict PRIVATE AIDA_FOUND)
endif()
if(GAUDI_USE_CLHEP)
   target_compile_definitions(GaudiPythonDict PRIVATE CLHEP_FOUND)
endif()

# Install python modules
gaudi_install(PYTHON)

# Enable QMTest tests
gaudi_add_tests(QMTest)

# nosetests
gaudi_add_tests(nosetests)
