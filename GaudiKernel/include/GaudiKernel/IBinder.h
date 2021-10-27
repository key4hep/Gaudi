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
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/bind.h"
#include <type_traits>
#include <utility>

namespace Gaudi::Interface::Bind {
  template <typename IFace>
  struct IBinder : extend_interfaces<IAlgTool> {
    DeclareInterfaceID( IBinder, 1, 0 );
    virtual Box i_bind( const EventContext& ctx ) const = 0;
  };

  /// Support binding of tools (FIXME: this could be moved into a ToolHandle (at which point some of the logic could be
  /// cached...)
  template <typename IFace>
  Box makeBox( IAlgTool const* tool, const EventContext& ctx ) {
    void* ptr = nullptr;
    if ( auto sc = const_cast<IAlgTool*>( tool )->queryInterface( IFace::interfaceID(), &ptr ); sc.isSuccess() ) {
      // TODO: what happens to the refCount?
      return Gaudi::Interface::Bind::Box( std::in_place_type<IFace>, static_cast<IFace const*>( ptr ) );
    } else if ( auto sc = const_cast<IAlgTool*>( tool )->queryInterface(
                    Gaudi::Interface::Bind::IBinder<IFace>::interfaceID(), &ptr );
                sc.isSuccess() ) {
      // TODO: what happens to the refCount?
      return static_cast<Gaudi::Interface::Bind::IBinder<IFace> const*>( ptr )->i_bind( ctx );
    } else {
      return Gaudi::Interface::Bind::Box{};
    }
  }

  template <typename IFace>
  struct AlgToolStub : IFace {

    using IFace::IFace;
    AlgToolStub( const AlgToolStub& ) = delete;
    AlgToolStub& operator=( const AlgToolStub& ) = delete;
    AlgToolStub( AlgToolStub&& )                 = delete;
    AlgToolStub& operator=( AlgToolStub&& ) = delete;

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
