/***********************************************************************************\
* (c) Copyright 2024-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

// Gaudi
#include <Gaudi/CUDA/CUDAStream.h>
#include <GaudiKernel/StatusCode.h>

// Standard Library
#include <ranges>
#include <span>
#include <type_traits>

namespace Gaudi::CUDA {
  /// Ensure type is trivially copyable (conceptization of std::is_trivially_copyable)
  template <class T>
  concept TriviallyCopyable = std::is_trivially_copyable<T>::value;

  /// Constrain ranges that can represent host memory compatible with this array
  template <class R, class T>
  concept HostRange = std::ranges::contiguous_range<R> && std::ranges::sized_range<R> &&
                      std::is_same_v<T, std::ranges::range_value_t<R>>;

  struct DeviceArrayGlobalTag_t {
    const bool hasAsyncParent;
    union {
      Gaudi::AsynchronousAlgorithm* const asyncPtr;
      Gaudi::Algorithm* const             syncPtr;
    };

    DeviceArrayGlobalTag_t( Gaudi::AsynchronousAlgorithm* parent ) : hasAsyncParent( true ), asyncPtr( parent ) {}
    DeviceArrayGlobalTag_t( Gaudi::Algorithm* parent ) : hasAsyncParent( false ), syncPtr( parent ) {}
  };

  template <TriviallyCopyable T>
  class DeviceArray {
  public:
    /// Construct DeviceArray local to a Stream (i.e. Algorithm). May not be stored in whiteboard.
    DeviceArray( Stream& stream, std::size_t len );
    /// Construct global DeviceArray (may be stored in whiteboard)
    DeviceArray( DeviceArrayGlobalTag_t globalTag, std::size_t len );
    /// Destructor
    ~DeviceArray();

    /// Copy from a HostRange
    template <HostRange<T> R>
    DeviceArray& operator=( const R& src );
    /// Copy from a host value (for size 1)
    DeviceArray& operator=( const T& src ) { return this->operator=( std::span<T, 1>( &src, 1 ) ); }

    /// Copy from another DeviceArray
    /// You can copy between arrays in the same stream, or between a local and a global DeviceArray.
    /// You cannot copy between arrays in different streams.
    DeviceArray& operator=( const DeviceArray& rhs );
    /// Copy to a HostRange
    template <HostRange<T> R>
    void toHost( R& dest );
    /// Copy to a host value (for size 1)
    void toHost( T& dest );
    /// Return raw device pointer
    T* devPtr() { return m_ptr; };
    /// Alias for devPtr
    T* operator&() { return m_ptr; }
    /// Allow access to struct members
    T* operator->() { return m_ptr; }

  private:
    T*                m_ptr;
    const std::size_t m_len;
    const std::size_t m_size;
    Stream* const     m_stream;
  };

  namespace Detail {
    void* allocateWithStream( std::size_t size, Stream& stream );
    void* allocateNoStream( std::size_t size, Gaudi::AsynchronousAlgorithm* parent );

    void freeWithStream( void* ptr, Stream& stream );
    void freeNoStream( void* ptr );

    void copyHostToDeviceWithStream( void* devPtr, const void* hstPtr, std::size_t size, Stream& stream );
    void copyHostToDeviceNoStream( void* devPtr, const void* hstPtr, std::size_t size );

    void copyDeviceToHostWithStream( void* hstPtr, const void* devPtr, std::size_t size, Stream& stream );
    void copyDeviceToHostNoStream( void* hstPtr, const void* devPtr, std::size_t size );

    void copyDeviceToDeviceWithStream( void* destDevPtr, const void* srcDevPtr, std::size_t size, Stream& stream );
    void copyDeviceToDeviceNoStream( void* destDevPtr, const void* srcDevPtr, std::size_t size );
  } // namespace Detail

  template <TriviallyCopyable T>
  DeviceArray<T>::DeviceArray( Stream& stream, std::size_t len )
      : m_ptr( static_cast<T*>( Detail::allocateWithStream( len * sizeof( T ), stream ) ) )
      , m_len( len )
      , m_size( len * sizeof( T ) )
      , m_stream( &stream ) {
    stream.registerDependency();
  }

  template <TriviallyCopyable T>
  DeviceArray<T>::DeviceArray( DeviceArrayGlobalTag_t globalTag, std::size_t len )
      : m_ptr( static_cast<T*>(
            Detail::allocateNoStream( len * sizeof( T ), globalTag.hasAsyncParent ? globalTag.asyncPtr : nullptr ) ) )
      , m_len( len )
      , m_size( len * sizeof( T ) )
      , m_stream( nullptr ) {}

  template <TriviallyCopyable T>
  DeviceArray<T>::~DeviceArray() {
    if ( m_stream == nullptr ) {
      Detail::freeNoStream( m_ptr );
    } else {
      Detail::freeWithStream( m_ptr, *m_stream );
      m_stream->removeDependency();
    }
  }

  template <TriviallyCopyable T>
  template <HostRange<T> R>
  DeviceArray<T>& DeviceArray<T>::operator=( const R& src ) {
    // Guard against mismatched sizes
    if ( std::ranges::size( src ) != m_len ) {
      if ( m_stream != nullptr ) {
        m_stream->parent()->error() << "Host to device copy with mismatched sizes: " << std::ranges::size( src )
                                    << " on host and " << m_len << " on device" << endmsg;
        throw GaudiException( "Host to device copy with mismatched sizes", "CUDADeviceArrayException",
                              StatusCode::FAILURE );
      } // We want the error message so we throw an exception
      throw GaudiException( "Host to device copy with mismatched sizes", "CUDADeviceArrayException",
                            StatusCode::FAILURE );
    }
    if ( m_stream == nullptr ) {
      Detail::copyHostToDeviceNoStream( m_ptr, std::ranges::data( src ), m_size );
      return *this;
    }
    Detail::copyHostToDeviceWithStream( m_ptr, std::ranges::data( src ), m_size, *m_stream );
    return *this;
  }

  template <TriviallyCopyable T>
  DeviceArray<T>& DeviceArray<T>::operator=( const DeviceArray<T>& rhs ) {
    if ( this == &rhs ) { return *this; }
    if ( m_stream != nullptr && rhs.m_stream != nullptr && m_stream != rhs.m_stream ) {
      m_stream->parent()->error()
          << "Device to device copies between DeviceArrays on different streams are not allowed!" << endmsg;
      throw GaudiException( "Device to device copy with mismatched streams", "CUDADeviceArrayException",
                            StatusCode::FAILURE );
    }
    if ( m_len != rhs.m_len ) {
      if ( m_stream != nullptr ) {
        m_stream->parent()->error() << "Device to device copy with mismatched sizes: " << rhs.m_len << " and " << m_len
                                    << endmsg;
        throw GaudiException( "Device to device copy with mismatched sizes", "CUDADeviceArrayException",
                              StatusCode::FAILURE );
      }
      if ( rhs.m_stream != nullptr ) {
        rhs.m_stream->parent()->error() << "Device to device copy with mismatched sizes: " << rhs.m_len << " and "
                                        << m_len << endmsg;
        throw GaudiException( "Device to device copy with mismatched sizes", "CUDADeviceArrayException",
                              StatusCode::FAILURE );
      }
      // We want the error message so we throw an exception
      throw GaudiException( "Device to device copy with mismatched sizes", "CUDADeviceArrayException",
                            StatusCode::FAILURE );
    }

    // Do the copy
    if ( m_stream != nullptr ) {
      Detail::copyDeviceToDeviceWithStream( m_ptr, rhs.m_ptr, m_size, *m_stream );
    } else if ( rhs.m_stream != nullptr ) {
      Detail::copyDeviceToDeviceWithStream( m_ptr, rhs.m_ptr, m_size, *rhs.m_stream );
    } else {
      Detail::copyDeviceToDeviceNoStream( m_ptr, rhs.m_ptr, m_size );
    }
    return *this;
  }

  template <TriviallyCopyable T>
  template <HostRange<T> R>
  void DeviceArray<T>::toHost( R& dest ) {
    // Guard against mismatched sizes
    if ( std::ranges::size( dest ) != m_len ) {
      if ( m_stream != nullptr ) {
        m_stream->parent()->error() << "Device to host copy with mismatched sizes: " << m_len << " on device and "
                                    << std::ranges::size( dest ) << " on host" << endmsg;
        throw GaudiException( "Device to host copy with mismatched sizes", "CUDADeviceArrayException",
                              StatusCode::FAILURE );
      } // We want the error message so we throw an exception
      throw GaudiException( "Device to host copy with mismatched sizes", "CUDADeviceArrayException",
                            StatusCode::FAILURE );
    }
    if ( m_stream == nullptr ) {
      Detail::copyDeviceToHostNoStream( std::ranges::data( dest ), m_ptr, m_size );
      return;
    }
    Detail::copyDeviceToHostWithStream( std::ranges::data( dest ), m_ptr, m_size, *m_stream );
  }

  template <TriviallyCopyable T>
  void DeviceArray<T>::toHost( T& dest ) {
    std::span<T, 1> dest_span( &dest, 1 );
    this->toHost( dest_span );
  }

} // namespace Gaudi::CUDA
