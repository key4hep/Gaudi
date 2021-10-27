/*****************************************************************************\
* (c) Copyright 2021 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once

class EventContext;

namespace Gaudi::Interface::Bind {

  // see https://godbolt.org/z/KPMYd1sbr
  class Box final {
    unsigned long m_iid           = -1;
    const void*   m_ptr           = nullptr;
    void ( *m_destruct )( void* ) = nullptr;
    std::aligned_storage_t<64 - 2 * sizeof( void* ) - sizeof( long )> m_storage; // local storage for bound arguments...
  public:
    explicit Box() = default;
    // identity binding: no actual binding is required...
    template <typename IFace>
    Box( std::in_place_type_t<IFace>, IFace const* ptr ) : m_iid{ IFace::interfaceID().id() }, m_ptr{ ptr } {
      assert( m_ptr != nullptr );
    }
    // bind the arguments...
    template <typename IFace, typename Ret, typename... Args,
              typename = std::enable_if_t<std::is_base_of_v<IFace, Ret>>>
    Box( std::in_place_type_t<IFace>, std::in_place_type_t<Ret>, Args&&... args ) : m_iid{ IFace::interfaceID().id() } {
      if constexpr ( sizeof( Ret ) <= sizeof( m_storage ) ) {
        m_ptr      = new ( &m_storage ) Ret{ std::forward<Args>( args )... };
        m_destruct = []( void* ptr ) { static_cast<Ret*>( ptr )->~Ret(); };
      } else {
        m_ptr      = new Ret{ std::forward<Args>( args )... };
        m_destruct = []( void* ptr ) { delete static_cast<Ret*>( ptr ); };
      }
    }
    template <typename IFace, typename Bound, typename = std::enable_if_t<std::is_base_of_v<IFace, Bound>>>
    Box( std::in_place_type_t<IFace>, Bound&& bound ) {
      if constexpr ( sizeof( Bound ) <= sizeof( m_storage ) ) {
        m_ptr      = new ( &m_storage ) Bound{ std::forward<Bound>( bound ) };
        m_destruct = []( void* ptr ) { static_cast<Bound*>( ptr )->~Bound(); };
      } else {
        m_ptr      = new Bound{ std::forward<Bound>( bound ) };
        m_destruct = []( void* ptr ) { delete static_cast<Bound*>( ptr ); };
      }
    }

    ~Box() {
      if ( m_destruct ) ( *m_destruct )( &m_storage );
    }
    Box( const Box& ) = delete;
    Box& operator=( const Box& ) = delete;
    Box( Box&& )                 = delete;
    Box& operator=( Box&& ) = delete;
    template <typename IFace>
    const IFace* get() const {
      return IFace::interfaceID().id() == m_iid ? static_cast<const IFace*>( m_ptr ) : nullptr;
    }
    template <typename IFace>
    IFace* get() && = delete;
  };

} // namespace Gaudi::Interface::Bind
