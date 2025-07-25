#####################################################################################
# (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# GaudiPartProp subdirectory

if(NOT GAUDI_ENABLE_GAUDIPARTPROP)
   return()
endif()

gaudi_add_library(GaudiPartProp
    SOURCES
        src/CC.cpp
        src/Decay.cpp
        src/IParticlePropertySvc.cpp
        src/NodePIDs.cpp
        src/Nodes.cpp
        src/ParticleID.cpp
        src/ParticleProperty.cpp
        src/Symbols.cpp
        src/TrackDefaultParticles.cpp
        src/iNode.cpp
    LINK
        PUBLIC
            Gaudi::GaudiKernel
)

gaudi_add_module(GaudiPartPropModule
    SOURCES
        src/Module/ParticlePropertySvc.cpp
    LINK
        GaudiPartProp
)

gaudi_add_dictionary(GaudiPartPropDict
    HEADERFILES dict/PartPropDict.h
    SELECTION dict/PartProp.xml
    LINK
        GaudiPartProp
)

gaudi_install(PYTHON)

gaudi_install(SCRIPTS)

gaudi_add_pytest(tests/pytest)
