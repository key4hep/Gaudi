/*****************************************************************************\
* (c) Copyright 2021-2024 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once
#include <GaudiKernel/IAlgTool.h>
#include <GaudiKernel/IInterface.h>
#include <type_traits>
#include <utility>

class EventContext;

namespace Gaudi::Interface::Bind {

  // see https://godbolt.org/z/KPMYd1sbr
  template <typename IFace>
  class Box final {
    const IFace* m_ptr            = nullptr;
    void ( *m_destruct )( void* ) = nullptr;
    std::aligned_storage_t<64 - 2 * sizeof( void* )> m_storage; // local storage for bound arguments... fit into a
                                                                // cacheline
  public:
    // identity binding: no actual binding is required...
    Box( IFace const* ptr ) : m_ptr{ ptr } { assert( m_ptr != nullptr ); }
    // bind the arguments...
    template <typename Ret, typename... Args, typename = std::enable_if_t<std::is_base_of_v<IFace, Ret>>>
    Box( std::in_place_type_t<Ret>, Args&&... args ) {
      if constexpr ( sizeof( Ret ) <= sizeof( m_storage ) ) {
        m_ptr      = new ( &m_storage ) Ret{ std::forward<Args>( args )... };
        m_destruct = []( void* ptr ) { static_cast<Ret*>( ptr )->~Ret(); };
      } else {
        m_ptr      = new Ret{ std::forward<Args>( args )... };
        m_destruct = []( void* ptr ) { delete static_cast<Ret*>( ptr ); };
      }
    }

    ~Box() {
      if ( m_destruct ) ( *m_destruct )( const_cast<IFace*>( m_ptr ) );
    }
    Box( const Box& )            = delete;
    Box& operator=( const Box& ) = delete;
    Box( Box&& rhs )             = delete;
    Box& operator=( Box&& )      = delete;

    operator IFace const&() const { return *m_ptr; }
    // operator IFace&() && = delete;
  };

  template <typename IFace>
  struct IBinder : extend_interfaces<IAlgTool> {
    DeclareInterfaceID( IBinder, 1, 0 );
    virtual Box<IFace> bind( const EventContext& ctx ) const = 0;
  };

  template <typename IFace>
  struct AlgToolStub : IFace {

    using IFace::IFace;
    AlgToolStub( const AlgToolStub& )            = delete;
    AlgToolStub& operator=( const AlgToolStub& ) = delete;
    AlgToolStub( AlgToolStub&& )                 = delete;
    AlgToolStub& operator=( AlgToolStub&& )      = delete;

    const std::string& name() const override {
      static std::string s{ "<STUB>" };
      return s;
    }
    const std::string& type() const override { return name(); }
    const IInterface*  parent() const override { return nullptr; }

    StatusCode                 configure() override { return StatusCode::FAILURE; }
    StatusCode                 initialize() override { return StatusCode::FAILURE; }
    StatusCode                 start() override { return StatusCode::FAILURE; }
    StatusCode                 stop() override { return StatusCode::FAILURE; }
    StatusCode                 finalize() override { return StatusCode::FAILURE; }
    StatusCode                 terminate() override { return StatusCode::FAILURE; }
    StatusCode                 reinitialize() override { return StatusCode::FAILURE; }
    StatusCode                 restart() override { return StatusCode::FAILURE; }
    Gaudi::StateMachine::State FSMState() const override { return Gaudi::StateMachine::RUNNING; }
    StatusCode                 sysInitialize() override { return StatusCode::FAILURE; }
    StatusCode                 sysStart() override { return StatusCode::FAILURE; }
    StatusCode                 sysStop() override { return StatusCode::FAILURE; }
    StatusCode                 sysFinalize() override { return StatusCode::FAILURE; }
    StatusCode                 sysReinitialize() override { return StatusCode::FAILURE; }
    StatusCode                 sysRestart() override { return StatusCode::FAILURE; }
  };

  template <typename IFace>
  struct Stub : implements<AlgToolStub<IFace>> {};

} // namespace Gaudi::Interface::Bind
