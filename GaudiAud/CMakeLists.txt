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
# GaudiAud subdirectory

# Build the plugin
gaudi_add_module(GaudiAud
                 SOURCES src/AlgContextAuditor.cpp
                         src/ChronoAuditor.cpp
                         src/CommonAuditor.cpp
                         src/MemoryAuditor.cpp
                         src/MemStatAuditor.cpp
                         src/NameAuditor.cpp
                         src/ProcStats.cpp
                 LINK GaudiKernel)

# QMTest
gaudi_add_tests(QMTest)
