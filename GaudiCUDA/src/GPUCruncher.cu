/***********************************************************************************\
* (c) Copyright 2023-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

#include "GPUCruncher.h"

#include <Gaudi/CUDA/CUDAStream.h>

#include "CUDADeviceArray.h"

#include <cstdio>
#include <format>

#include <cub/cub.cuh>
#include <cub/util_device.cuh>
#define CUDA_CHECK( stmt )                                                                                              \
  if ( cudaError_t cudaErr = cudaGetLastError(); cudaErr != cudaSuccess ) {                                             \
    const char* errname = cudaGetErrorName( cudaErr );                                                                  \
    const char* errstr  = cudaGetErrorString( cudaErr );                                                                \
    std::string errmsg  = std::format( "Encountered CUDA error BEFORE {} [{}]: {} on {}:{} (dev {})", errname,          \
                                       int( cudaErr ), errstr, __FILE__, __LINE__, cub::CurrentDevice() );              \
    error() << errmsg << endmsg;                                                                                        \
    return StatusCode::FAILURE;                                                                                         \
  }                                                                                                                     \
  if ( cudaError_t cudaErr = ( stmt ); cudaErr != cudaSuccess ) {                                                       \
    const char* errname = cudaGetErrorName( cudaErr );                                                                  \
    const char* errstr  = cudaGetErrorString( cudaErr );                                                                \
    std::string errmsg  = std::format( "Encountered CUDA error {} [{}]: {} on {}:{} (dev {})", errname, int( cudaErr ), \
                                       errstr, __FILE__, __LINE__, cub::CurrentDevice() );                              \
    error() << errmsg << endmsg;                                                                                        \
    return StatusCode::FAILURE;                                                                                         \
  }

__global__ void makeGrid( const double* input, double* workspace ) {
  double at                                        = ( threadIdx.x + 1 ) * input[blockIdx.x];
  workspace[blockIdx.x * blockDim.x + threadIdx.x] = at;
}

struct bounds_t {
  double lower = 0.;
  double upper = 1.;
};

StatusCode GPUCruncher::gpuExecute( const std::vector<double>& in, std::vector<double>& out ) const {
  CUDA_CHECK( cudaSetDevice( 0 ) );
  // Create the stream
  Gaudi::CUDA::Stream stream( this );

  // Allocate device memory
  Gaudi::CUDA::DeviceArray<double> d_input( stream, in.size() );

  // Copy input
  d_input = in;

  // Run computation part 1 -- a math transform
  Gaudi::CUDA::DeviceArray<double> d_inter1( stream, 256 * in.size() ); // 256 doubles for each original double
  makeGrid<<<in.size(), 256, 0, stream>>>( &d_input, &d_inter1 );

  // Make histogram
  std::vector<int>                   hist( 10, 0 ); // histogram as ints
  Gaudi::CUDA::DeviceArray<int>      d_hist( stream, 10 );
  bounds_t                           bounds{};
  Gaudi::CUDA::DeviceArray<bounds_t> d_bounds( stream, 1 );

  std::size_t wkspace_size     = 0;
  std::size_t wkspace_size_tmp = 0;
  // determine workspace size
  CUDA_CHECK( cub::DeviceReduce::Min( nullptr, wkspace_size, &d_inter1, &d_bounds->lower, 256 * in.size(), stream ) );
  CUDA_CHECK(
      cub::DeviceReduce::Max( nullptr, wkspace_size_tmp, &d_inter1, &d_bounds->upper, 256 * in.size(), stream ) );
  if ( wkspace_size_tmp > wkspace_size ) wkspace_size = wkspace_size_tmp;
  wkspace_size_tmp = 0;
  CUDA_CHECK( cub::DeviceHistogram::HistogramEven( nullptr, wkspace_size_tmp, &d_inter1, &d_hist, hist.size() + 1,
                                                   bounds.lower, std::nextafter( bounds.upper, bounds.upper + 1 ),
                                                   256 * in.size(), stream ) );
  if ( wkspace_size_tmp > wkspace_size ) wkspace_size = wkspace_size_tmp;

  // Allocate workspace
  Gaudi::CUDA::DeviceArray<std::byte> d_wkspace( stream, wkspace_size );

  // Get bounds
  std::size_t temp = wkspace_size;
  CUDA_CHECK( cub::DeviceReduce::Min( &d_wkspace, temp, &d_inter1, &d_bounds->lower, 256 * in.size(), stream ) );
  temp = wkspace_size;
  CUDA_CHECK( cub::DeviceReduce::Max( &d_wkspace, temp, &d_inter1, &d_bounds->upper, 256 * in.size(), stream ) );
  // Copy bounds back to host. Implicit await while this happens
  d_bounds.toHost( bounds );

  // Make histogram
  temp = wkspace_size;
  for ( int i = 0; i < 10; ++i ) {
    CUDA_CHECK( cub::DeviceHistogram::HistogramEven( &d_wkspace, temp, &d_inter1, &d_hist, hist.size() + 1,
                                                     bounds.lower, std::nextafter( bounds.upper, bounds.upper + 1 ),
                                                     256 * in.size(), stream ) );
  }
  // Retrieve histogram. Implicit await while this happens.
  d_hist.toHost( hist );

  out = { bounds.lower, bounds.upper };
  out.reserve( hist.size() + 2 );
  for ( int x : hist ) { out.push_back( double( x ) ); }

  // Stream associated DeviceArrays get deallocated, then an implicit Stream await when the Stream is destroyed
  return StatusCode::SUCCESS;
}
