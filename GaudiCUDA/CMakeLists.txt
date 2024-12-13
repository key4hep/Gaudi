# ####################################################################################
# (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      #
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
gaudi_add_library(GaudiCUDALib
                  SOURCES src/CUDAStream.cpp
                  LINK
                    PUBLIC GaudiKernel
                           Boost::headers
                           Boost::fiber
                           CUDA::cudart
)

gaudi_add_module(GaudiCUDA
                  SOURCES src/GPUCruncher.cpp
                          src/GPUCruncher.cu
                          src/CUDADeviceArray.cpp
                  LINK
                    PUBLIC GaudiKernel
                           GaudiCUDALib
                           CUDA::cudart
)
