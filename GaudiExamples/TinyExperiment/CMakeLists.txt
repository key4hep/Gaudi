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

# Build the module for TinyExperiment example
gaudi_add_module(GaudiExampleTinyExperiment
                  SOURCES src/GeneratorAlg.cpp
                          src/SimulationAlg.cpp
                          src/DigitizationAlg.cpp
                          src/TrackingAlg.cpp
                          src/CheckerAlg.cpp
                          src/RandomGenSvc.cpp
                  LINK PUBLIC GaudiKernel
                              Gaudi::Functional)

gaudi_install(PYTHON)
gaudi_add_pytest(tests/pytest)
