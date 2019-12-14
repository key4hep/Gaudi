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
gaudi_subdir(GaudiMP)

gaudi_depends_on_subdirs(GaudiKernel GaudiAlg)

find_package(ROOT COMPONENTS Net RIO Thread REQUIRED)
find_package(PythonLibs REQUIRED)
find_package(AIDA)

# The list of sources to build into the module:
set( module_sources src/component/IoComponentMgr.cpp )
if( AIDA_FOUND )
   list( APPEND module_sources src/component/RecordOutputStream.cpp
      src/component/ReplayOutputStream.cpp )
endif()

# Hide some Boost/ROOT compile time warnings
find_package(Boost)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS})

#---Libraries---------------------------------------------------------------
gaudi_add_library(GaudiMPLib src/Lib/*.cpp
                  LINK_LIBRARIES GaudiKernel PythonLibs ROOT
                  INCLUDE_DIRS ROOT PythonLibs
                  PUBLIC_HEADERS GaudiMP)
gaudi_add_module(GaudiMP ${module_sources}
                 LINK_LIBRARIES GaudiMPLib PythonLibs ROOT GaudiAlgLib)

#---Dictionaries------------------------------------------------------------
gaudi_add_dictionary(GaudiMP dict/gaudimp_dict.h dict/selection.xml LINK_LIBRARIES GaudiMPLib)

#---Installation------------------------------------------------------------
gaudi_install_python_modules()
gaudi_install_scripts()
