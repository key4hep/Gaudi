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
gaudi_subdir(RootHistCnv)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(AIDA)
find_package(ROOT COMPONENTS RIO Hist Matrix Tree Thread MathCore Net REQUIRED)
find_package(Boost REQUIRED)

# If AIDA is missing, don't build this module:
if( NOT AIDA_FOUND )
   return()
endif()

# Hide some ROOT, Boost compile time warnings
include_directories(SYSTEM ${ROOT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

#---Libraries---------------------------------------------------------------
gaudi_add_module(RootHistCnv src/*.cpp
                 LINK_LIBRARIES GaudiKernel ROOT
                 INCLUDE_DIRS Boost ROOT AIDA)
