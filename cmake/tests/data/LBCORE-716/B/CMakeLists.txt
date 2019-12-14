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
cmake_minimum_required(VERSION 3.6)
find_package(GaudiProject)
# prevent irrelevant problems on SLC6
set(GAUDI_CXX_STANDARD "c++98"
    CACHE STRING "Version of the C++ standard to be used.")
gaudi_project(B HEAD USE A HEAD)
