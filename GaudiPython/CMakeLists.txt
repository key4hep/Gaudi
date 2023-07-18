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
                  SOURCES src/Lib/CallbackStreamBuf.cpp
                          src/Lib/Algorithm.cpp
                          src/Lib/Helpers.cpp
                  LINK
                    PUBLIC Gaudi::GaudiKernel
                           Python::Python)

# Build the plugin
gaudi_add_module(GaudiPython
                 SOURCES src/Services/PythonScriptingSvc.cpp
                 LINK GaudiPythonLib
                      GaudiUtilsLib)

# ROOT dictionaries
gaudi_add_dictionary(GaudiPythonDict
                     HEADERFILES dict/kernel.h
                     SELECTION ${CMAKE_CURRENT_BINARY_DIR}/selection.xml
                     LINK GaudiPythonLib
                          GaudiUtilsLib)

set(selection_fragments main)
if(GAUDI_USE_AIDA)
   target_compile_definitions(GaudiPythonDict PRIVATE AIDA_FOUND)
endif()
if(GAUDI_USE_CLHEP)
   target_compile_definitions(GaudiPythonDict PRIVATE CLHEP_FOUND)
endif()
list(TRANSFORM selection_fragments PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/dict/selection_fragment_")
list(TRANSFORM selection_fragments APPEND ".xml")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/gen_selection.cmake
"
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/selection.xml \"<lcgdict>\")
foreach(fragment IN ITEMS ${selection_fragments})
   file(READ \${fragment} fragment_data)
   file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/selection.xml \"\${fragment_data}\")
endforeach()
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/selection.xml \"</lcgdict>\")
"
)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/selection.xml
   COMMENT "Generate GaudiPythonDict selection.xml"
   COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/gen_selection.cmake
   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/gen_selection.cmake ${selection_fragments}
)

# Install python modules
gaudi_install(PYTHON)

# Enable QMTest tests
gaudi_add_tests(QMTest)

gaudi_add_pytest(tests/nose)
