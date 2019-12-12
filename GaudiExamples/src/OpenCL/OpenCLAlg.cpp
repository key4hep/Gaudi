/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "GaudiAlg/GaudiAlgorithm.h"

#ifdef __APPLE__
#  include <OpenCL/opencl.h>
#else
#  include <CL/cl.h>
#endif

// Used to prefer a device type over another one
#define DEVICE_CPU 0
#define DEVICE_GPU 1
#define DEVICE_ACCELERATOR 2
#define DEVICE_PREFERENCE DEVICE_GPU

#define clCheck( stmt )                                                                                                \
  {                                                                                                                    \
    cl_int status = stmt;                                                                                              \
    if ( status != CL_SUCCESS ) {                                                                                      \
      std::cerr << "Error in function " << #stmt << std::endl;                                                         \
      std::cerr << "Error status: " << status << std::endl;                                                            \
      exit( -1 );                                                                                                      \
    }                                                                                                                  \
  }

namespace Gaudi {
  namespace Examples {

    class OpenCLAlg : public GaudiAlgorithm {
    public:
      using GaudiAlgorithm::GaudiAlgorithm;
      StatusCode initialize() override {
        StatusCode sc = GaudiAlgorithm::initialize();

        // Choose platform according to the macro DEVICE_PREFERENCE
        cl_device_id*  devices;
        cl_platform_id platform = NULL;
        clChoosePlatform( devices, platform );

        // Your OpenCL code here

        return sc;
      }
      StatusCode execute() override {
        info() << "Executing " << name() << endmsg;
        return StatusCode::SUCCESS;
      }

    private:
      void clChoosePlatform( cl_device_id*& devices, cl_platform_id& platform ) {
        // Choose the first available platform
        cl_uint numPlatforms;
        clCheck( clGetPlatformIDs( 0, NULL, &numPlatforms ) );
        if ( numPlatforms > 0 ) {
          cl_platform_id* platforms = (cl_platform_id*)malloc( numPlatforms * sizeof( cl_platform_id ) );
          clCheck( clGetPlatformIDs( numPlatforms, platforms, NULL ) );
          platform = platforms[0];
          free( platforms );
        }

        // Choose a device from the platform according to DEVICE_PREFERENCE
        cl_uint numCpus         = 0;
        cl_uint numGpus         = 0;
        cl_uint numAccelerators = 0;
        clGetDeviceIDs( platform, CL_DEVICE_TYPE_CPU, 0, NULL, &numCpus );
        clGetDeviceIDs( platform, CL_DEVICE_TYPE_GPU, 0, NULL, &numGpus );
        clGetDeviceIDs( platform, CL_DEVICE_TYPE_ACCELERATOR, 0, NULL, &numAccelerators );
        devices = (cl_device_id*)malloc( numAccelerators * sizeof( cl_device_id ) );

        debug() << "Devices available: " << endmsg;
        debug() << "CPU: " << numCpus << endmsg;
        debug() << "GPU: " << numGpus << endmsg;
        debug() << "Accelerators: " << numAccelerators << endmsg;

        if ( DEVICE_PREFERENCE == DEVICE_CPU && numCpus > 0 ) {
          debug() << "Choosing CPU" << endmsg;
          clCheck( clGetDeviceIDs( platform, CL_DEVICE_TYPE_CPU, numCpus, devices, NULL ) );
        } else if ( DEVICE_PREFERENCE == DEVICE_GPU && numGpus > 0 ) {
          debug() << "Choosing GPU" << endmsg;
          clCheck( clGetDeviceIDs( platform, CL_DEVICE_TYPE_GPU, numGpus, devices, NULL ) );
        } else if ( DEVICE_PREFERENCE == DEVICE_ACCELERATOR && numAccelerators > 0 ) {
          debug() << "Choosing accelerator" << endmsg;
          clCheck( clGetDeviceIDs( platform, CL_DEVICE_TYPE_ACCELERATOR, numAccelerators, devices, NULL ) );
        } else {
          // We couldn't match the preference.
          // Let's try the first device that appears available.
          cl_uint numDevices = 0;
          clCheck( clGetDeviceIDs( platform, CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices ) );
          if ( numDevices > 0 ) {
            debug() << "Preference device couldn't be met" << std::endl
                    << "Choosing an available OpenCL capable device" << endmsg;
            clCheck( clGetDeviceIDs( platform, CL_DEVICE_TYPE_ALL, numDevices, devices, NULL ) );
          } else {
            debug() << "No OpenCL capable device detected" << std::endl
                    << "Check the drivers, OpenCL runtime or ICDs are available" << endmsg;
            exit( -1 );
          }
        }
      }
    };
  } // namespace Examples
} // namespace Gaudi

DECLARE_COMPONENT( Gaudi::Examples::OpenCLAlg )
