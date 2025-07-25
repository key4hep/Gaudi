#####################################################################################
# (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# GaudiAud subdirectory

# Build the plugin
gaudi_add_module(GaudiAud
                 SOURCES src/AlgContextAuditor.cpp
                         src/AlgTimingAuditor.cpp
                         src/ChronoAuditor.cpp
                         src/MemoryAuditor.cpp
                         src/MemStatAuditor.cpp
                         src/NameAuditor.cpp
                         src/ProcStats.cpp
                 LINK GaudiKernel)

gaudi_add_pytest(tests/pytest)
