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
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/extend_interfaces.h"
#include <type_traits>
#include <utility>

namespace Gaudi::Interface {
  template <typename IFace>
  struct InterfaceStub : IFace {
    long unsigned int addRef() override { return 0; }
    long unsigned int release() override { return 0; }
    StatusCode        queryInterface( const InterfaceID&, void** ) override { return StatusCode::FAILURE; }
  };

  template <typename IFace>
  struct AlgToolStub : InterfaceStub<IFace> {
    const std::string& name() const override {
      static std::string s{ "GoAway" };
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
  struct Stub
      : std::conditional_t<std::is_base_of_v<IAlgTool, IFace>, AlgToolStub<IFace>,
                           std::conditional_t<std::is_base_of_v<IInterface, IFace>, InterfaceStub<IFace>, IFace>> {};

  // see https://godbolt.org/z/KPMYd1sbr
  template <typename IFace>
  class BoxedInterface final {
    IFace const*               m_ptr = nullptr;
    std::aligned_storage_t<48> m_storage;
    void ( *m_destruct )( void* ) = nullptr;
    // static_assert( std::is_base_of_v<IAlgTool, IFace>  );
  public:
    // in case no binding is required...
    BoxedInterface( IFace const* ptr ) : m_ptr{ ptr } {}
    // bind the arguments...
    template <typename Ret, typename... Args, typename = std::enable_if_t<std::is_base_of_v<IFace, Ret>>>
    BoxedInterface( std::in_place_type_t<Ret>, Args&&... args )
        : m_ptr{ new ( &m_storage ) Ret{ std::forward<Args>( args )... } }
        , m_destruct{ []( void* ptr ) { static_cast<Ret*>( ptr )->~Ret(); } } {
      static_assert( sizeof( Ret ) <= sizeof( m_storage ) ); // TODO: add heap storage for large bindings...
    }
    ~BoxedInterface() {
      if ( m_destruct ) ( *m_destruct )( &m_storage );
    }
    BoxedInterface( const BoxedInterface& ) = delete;
    BoxedInterface& operator=( const BoxedInterface& ) = delete;
    BoxedInterface( BoxedInterface&& )                 = delete;
    BoxedInterface& operator=( BoxedInterface&& ) = delete;
                    operator const IFace&() const { return *m_ptr; }
    // operator IFace& () && = delete;
  };

  template <typename IFace>
  struct IBinder : extend_interfaces<IAlgTool> {
    DeclareInterfaceID( IBinder<IFace>, 1, 0 );
    virtual BoxedInterface<IFace> operator()( EventContext const& ) const = 0;
  };

} // namespace Gaudi::Interface
