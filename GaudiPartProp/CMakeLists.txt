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
gaudi_subdir(GaudiPartProp)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(Boost REQUIRED)

# Hide some Boost compile time warnings
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

gaudi_add_module(GaudiPartProp src/ParticlePropertySvc.cpp
                 LINK_LIBRARIES GaudiKernel)
