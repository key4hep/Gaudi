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

#include <cstdio>
#include <memory_resource>

#include <cub/cub.cuh>
__global__ void makeGrid( const double* input, double* workspace ) {
  double at                                        = threadIdx.x * input[blockIdx.x];
  workspace[blockIdx.x * blockDim.x + threadIdx.x] = at;
}

StatusCode GPUCruncher::gpuExecute( const std::pmr::vector<double>& in, std::vector<double>& out ) const {
  // Create the stream
  Gaudi::CUDA::CUDAStream stream( dynamic_cast<const Gaudi::AsynchronousAlgorithm*>( this ) );

  // Allocate device memory
  double* d_input = stream.malloc<double>( in.size() );

  // Copy input
  CUDA_CHECK( cudaMemcpyAsync( d_input, in.data(), in.size() * sizeof( double ), cudaMemcpyHostToDevice, stream ) );

  // Run computation part 1 -- a math transform
  double* d_inter1 = stream.malloc<double>( 256 * in.size() ); // 256 doubles for each original double
  makeGrid<<<in.size(), 256, 0, stream>>>( d_input, d_inter1 );

  // Make histogram
  std::pmr::vector<int> hist( in.size() - 2, 0, pinned ); // histogram as ints
  int*                  d_hist   = stream.malloc<int>( in.size() - 2 );
  double*               d_bounds = stream.malloc<double>( 2 );
  struct bounds_t {
    double lower = 0.;
    double upper = 1.;
  };
  bounds_t* bounds = static_cast<bounds_t*>( pinned->allocate( sizeof( bounds_t ) ) );

  void*       d_wkspace        = nullptr;
  std::size_t wkspace_size     = 0;
  std::size_t wkspace_size_tmp = 0;
  // determine workspace size
  CUDA_CHECK( cub::DeviceReduce::Min( d_wkspace, wkspace_size, d_inter1, d_bounds, 256 * in.size(), stream ) );
  CUDA_CHECK( cub::DeviceReduce::Max( d_wkspace, wkspace_size_tmp, d_inter1, d_bounds + 1, 256 * in.size(), stream ) );
  if ( wkspace_size_tmp > wkspace_size ) wkspace_size = wkspace_size_tmp;
  wkspace_size_tmp = 0;
  CUDA_CHECK( cub::DeviceHistogram::HistogramEven( d_wkspace, wkspace_size_tmp, d_inter1, d_hist, hist.size() + 1,
                                                   bounds->lower, bounds->upper, 256 * in.size(), stream ) );
  if ( wkspace_size_tmp > wkspace_size ) wkspace_size = wkspace_size_tmp;

  // Allocate workspace
  d_wkspace = stream.malloc<void>( wkspace_size );

  // Get bounds
  std::size_t temp = wkspace_size;
  CUDA_CHECK( cub::DeviceReduce::Min( d_wkspace, temp, d_inter1, d_bounds, 256 * in.size(), stream ) );
  temp = wkspace_size;
  CUDA_CHECK( cub::DeviceReduce::Max( d_wkspace, temp, d_inter1, d_bounds + 1, 256 * in.size(), stream ) );
  CUDA_CHECK( cudaMemcpyAsync( bounds, d_bounds, 2 * sizeof( double ), cudaMemcpyDeviceToHost, stream ) );
  // Sync so we have the bounds on the host
  StatusCode sc = cuda_stream_await( stream );
  if ( sc.isFailure() ) return sc;
  // Make histogram
  temp = wkspace_size;
  for ( int i = 0; i < 10; ++i ) {
    CUDA_CHECK( cub::DeviceHistogram::HistogramEven( d_wkspace, temp, d_inter1, d_hist, hist.size() + 1, bounds->lower,
                                                     bounds->upper, 256 * in.size(), stream ) );
  }
  // Retrieve histogram
  CUDA_CHECK( cudaMemcpyAsync( hist.data(), d_hist, hist.size() * sizeof( int ), cudaMemcpyDeviceToHost, stream ) );

  // Convert histogram to doubles
  out = { bounds->lower, bounds->upper };
  pinned->deallocate( bounds, sizeof( bounds_t ) );
  out.reserve( hist.size() + 2 );
  for ( int x : hist ) { out.push_back( double( x ) ); }
  return StatusCode::SUCCESS;
}
