# ####################################################################################
# (c) Copyright 2024-2026 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
# ####################################################################################
# GaudiCUDA subdirectory
# Build libraries

find_package(CUDAToolkit REQUIRED)

# The library should not have a public dependency on CUDA, to avoid clients
# who don't want to build/use CUDA to have to have it in their build


gaudi_add_library(GaudiCUDALib
                  SOURCES src/CUDAStream.cpp
                  LINK
                    PUBLIC  GaudiKernel
                            Boost::headers
                            Boost::fiber
                    PRIVATE CUDA::cudart
)

gaudi_add_module(GaudiCUDA
                  SOURCES src/GPUCruncher.cpp
                          src/GPUCruncher.cu
                          src/CUDADeviceArray.cpp
                  LINK GaudiKernel
                       GaudiCUDALib
                       Boost::headers
                       Boost::fiber
                       CUDA::cudart
)
