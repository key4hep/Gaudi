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
# PartPropSvc subdirectory

# If HepPDT is not available, give up on this subdirectory:
if(NOT GAUDI_USE_HEPPDT)
    message(WARNING "Requirements for building PartPropSvc disabled. "
        "PartPropSvc will not be built. "
        "HepPDT is disabled.")
    return()
endif()

# Build the plugin
gaudi_add_module(PartPropSvc
                 SOURCES src/PartPropSvc.cpp
                 LINK GaudiKernel
                      Boost::headers
                      Boost::regex
                      HepPDT::heppdt)

# # Install shared files TODO: if it is not used, drop the whole directory
# install(FILES share/PartPropSvc.py
#         DESTINATION jobOptions/PartPropSvc)
# install(FILES share/PDGTABLE.MeV
#               share/PDGTABLE.2002.MeV
#               share/PDGTABLE.2014.MeV
#         DESTINATION share)
