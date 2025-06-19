/***********************************************************************************\
* (c) Copyright 2025 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <GaudiKernel/IService.h>
#include <GaudiKernel/SmartIF.h>
#include <memory>
#include <string>
#include <utility>

#if __has_include( <catch2/catch.hpp> )
// Catch2 v2
#  include <catch2/catch.hpp>
#else
// Catch2 v3
#  include <catch2/catch_template_test_macros.hpp>
#  include <catch2/catch_test_macros.hpp>
#endif

namespace {
  struct BaseTestSvc : implements<IService> {
    static std::size_t s_instances;

    BaseTestSvc() { ++s_instances; }
    ~BaseTestSvc() { --s_instances; }

    // INamedInterface
    const std::string& name() const override {
      static const std::string name = "BaseTestSvc";
      return name;
    }

    // IStateful
    virtual StatusCode configure() override { return StatusCode::SUCCESS; }
    virtual StatusCode initialize() override { return StatusCode::SUCCESS; }
    virtual StatusCode start() override { return StatusCode::SUCCESS; }
    virtual StatusCode stop() override { return StatusCode::SUCCESS; }
    virtual StatusCode finalize() override { return StatusCode::SUCCESS; }
    virtual StatusCode terminate() override { return StatusCode::SUCCESS; }
    virtual StatusCode reinitialize() override { return StatusCode::SUCCESS; }
    virtual StatusCode restart() override { return StatusCode::SUCCESS; }

    virtual Gaudi::StateMachine::State FSMState() const override { return Gaudi::StateMachine::State::OFFLINE; }
    virtual Gaudi::StateMachine::State targetFSMState() const override { return Gaudi::StateMachine::State::OFFLINE; }

    // IService
    StatusCode sysInitialize() override { return StatusCode::SUCCESS; }
    StatusCode sysStart() override { return StatusCode::SUCCESS; }
    StatusCode sysStop() override { return StatusCode::SUCCESS; }
    StatusCode sysFinalize() override { return StatusCode::SUCCESS; }
    StatusCode sysReinitialize() override { return StatusCode::SUCCESS; }
    StatusCode sysRestart() override { return StatusCode::SUCCESS; }
    void       setServiceManager( ISvcManager* ) override {}
  };

  std::size_t BaseTestSvc::s_instances = 0;

  struct IAnotherInterface : virtual IInterface {
    DeclareInterfaceID( IAnotherInterface, 1, 0 );
    virtual ~IAnotherInterface() = default;
  };

  struct AnotherInterfaceImpl : virtual implements<IAnotherInterface> {};

  struct DelegateImpl : BaseTestSvc {
    const std::string& name() const override {
      static const std::string name = "DelegateImpl";
      return name;
    }

    StatusCode queryInterface( const InterfaceID& iid, void** ppvi ) override {
      if ( !ppvi ) { return StatusCode::FAILURE; }
      if ( iid == IAnotherInterface::interfaceID() ) {
        *ppvi = m_anotherInterface.get();
        if ( m_anotherInterface ) m_anotherInterface->addRef();
        return StatusCode::SUCCESS;
      }
      return BaseTestSvc::queryInterface( iid, ppvi );
    }

    void const* i_cast( const InterfaceID& iid ) const override {
      if ( auto output = BaseTestSvc::i_cast( iid ) ) { return output; }
      if ( iid == IAnotherInterface::interfaceID() ) { return m_anotherInterface.get(); }
      return nullptr;
    }

    SmartIF<IAnotherInterface> m_anotherInterface{ std::make_unique<AnotherInterfaceImpl>() };
  };
} // namespace

TEST_CASE( "SmartIF(nullptr)" ) {
  {
    IInterface*       p = nullptr;
    SmartIF<IService> svc( p );
    CHECK( svc == nullptr );
  }

  {
    SmartIF<IService> svc{};
    CHECK( svc == nullptr );
  }
}

TEST_CASE( "SmartIF test helper" ) {
  CHECK( BaseTestSvc::s_instances == 0 );
  {
    std::unique_ptr<IInterface> iface = std::make_unique<BaseTestSvc>();
    REQUIRE( iface );
    CHECK( iface->refCount() == 0 );
    CHECK( BaseTestSvc::s_instances == 1 );
  }
  CHECK( BaseTestSvc::s_instances == 0 );
}

TEST_CASE( "SmartIF trivial" ) {
  {
    IInterface* iface = new BaseTestSvc();
    REQUIRE( iface );
    {
      SmartIF<IInterface> svc{ iface };
      REQUIRE( svc );
      CHECK( svc.get() == iface );
      CHECK( svc->refCount() == 1 );
      CHECK( BaseTestSvc::s_instances == 1 );
    }
    // no instances left
    CHECK( BaseTestSvc::s_instances == 0 );
  }

  {
    IInterface* iface = new BaseTestSvc();
    REQUIRE( iface );
    {
      SmartIF<IService> svc{ iface };
      REQUIRE( svc );
      CHECK( svc.get() == iface );
      CHECK( svc->refCount() == 1 );
      CHECK( BaseTestSvc::s_instances == 1 );
    }
    // no instances left
    CHECK( BaseTestSvc::s_instances == 0 );
  }
}

TEST_CASE( "SmartIF conversions" ) {
  IInterface* iface = new BaseTestSvc();
  REQUIRE( iface );
  {
    SmartIF<IInterface> svc{ iface };
    REQUIRE( svc );
    CHECK( svc.get() == iface );
    CHECK( svc->refCount() == 1 );
    CHECK( BaseTestSvc::s_instances == 1 );

    SmartIF<IInterface> svc2{ svc };
    REQUIRE( svc2 );
    CHECK( svc2.get() == iface );
    CHECK( svc2->refCount() == 2 );
    CHECK( BaseTestSvc::s_instances == 1 );

    SmartIF<IService> svc3{ svc };
    REQUIRE( svc3 );
    CHECK( svc3.get() == iface );
    CHECK( svc3->refCount() == 3 );
    CHECK( BaseTestSvc::s_instances == 1 );

    auto svc4 = svc3.as<IInterface>();
    REQUIRE( svc4 );
    CHECK( svc4.get() == iface );
    CHECK( svc4->refCount() == 4 );
    CHECK( BaseTestSvc::s_instances == 1 );
  }
  // no instances left
  CHECK( BaseTestSvc::s_instances == 0 );
}

TEST_CASE( "SmartIF self copy" ) {
  IInterface* iface = new BaseTestSvc();
  REQUIRE( iface );
  {
    SmartIF<IInterface> svc{ iface };
    REQUIRE( svc );
    CHECK( svc.get() == iface );
    CHECK( svc->refCount() == 1 );
    CHECK( BaseTestSvc::s_instances == 1 );

    SmartIF<IService> svc2{ svc };
    REQUIRE( svc2 );
    CHECK( svc2.get() == iface );
    CHECK( svc2->refCount() == 2 );
    CHECK( BaseTestSvc::s_instances == 1 );

    svc2 = svc;
    REQUIRE( svc2 );
    CHECK( svc2.get() == iface );
    CHECK( svc2->refCount() == 2 );
    CHECK( BaseTestSvc::s_instances == 1 );
  }
  // no instances left
  CHECK( BaseTestSvc::s_instances == 0 );
}

TEST_CASE( "SmartIF invalid" ) {
  IInterface* iface = new BaseTestSvc();
  REQUIRE( iface );
  {
    SmartIF<IInterface> svc{ iface };
    REQUIRE( svc );
    CHECK( svc.get() == iface );
    CHECK( svc->refCount() == 1 );
    CHECK( BaseTestSvc::s_instances == 1 );
    {
      SmartIF<IAnotherInterface> svc2{ svc };
      CHECK( !svc2.isValid() );
      CHECK( svc->refCount() == 1 );
      CHECK( BaseTestSvc::s_instances == 1 );
    }
    {
      auto svc2 = svc.as<IAnotherInterface>();
      CHECK( !svc2.isValid() );
      CHECK( svc->refCount() == 1 );
      CHECK( BaseTestSvc::s_instances == 1 );
    }
  }
  // no instances left
  CHECK( BaseTestSvc::s_instances == 0 );
}

TEST_CASE( "SmartIF implicit conversion" ) {
  {
    IInterface* iface = new BaseTestSvc();
    REQUIRE( iface );
    SmartIF<IService> svc{ iface };
    REQUIRE( svc );
    CHECK( svc.get() == iface );
    CHECK( svc->refCount() == 1 );
    CHECK( BaseTestSvc::s_instances == 1 );

    IService* ptr = svc;
    REQUIRE( ptr );
    CHECK( ptr == iface );
    CHECK( ptr->refCount() == 1 );
    CHECK( BaseTestSvc::s_instances == 1 );

    SmartIF<IService> svc2( ptr );
    CHECK( ptr == iface );
    CHECK( ptr->refCount() == 2 );
    CHECK( BaseTestSvc::s_instances == 1 );
  }
  // no instances left
  CHECK( BaseTestSvc::s_instances == 0 );
}

TEST_CASE( "SmartIF interface delegation" ) {
  SmartIF<IInterface> iface{ new DelegateImpl() };
  REQUIRE( iface );
  {
    auto inamed = iface.as<INamedInterface>();
    REQUIRE( inamed );
    CHECK( inamed->name() == "DelegateImpl" );
  }

  SmartIF<DelegateImpl> impl{ iface };
  REQUIRE( impl );
  CHECK( impl->m_anotherInterface->refCount() == 1 );

  auto svc = iface.as<IService>();
  CHECK( impl->refCount() == 3 ); // iface, impl, svc
  CHECK( svc.get() == iface.get() );

  auto another = iface.as<IAnotherInterface>();
  REQUIRE( another );
  CHECK( impl->refCount() == 3 );    // iface, impl, svc
  CHECK( another->refCount() == 2 ); // indirect impl, direct another
  CHECK( another.get() == impl->m_anotherInterface.get() );
}

TEST_CASE( "SmartIF from unique_ptr" ) {
  {
    std::unique_ptr<IInterface> iface = std::make_unique<BaseTestSvc>();
    REQUIRE( iface );
    auto                old = iface.get();
    SmartIF<IInterface> svc{ std::move( iface ) };
    REQUIRE( svc );
    CHECK( svc.get() == old );
    CHECK( svc->refCount() == 1 );
    CHECK( BaseTestSvc::s_instances == 1 );
  }
  {
    std::unique_ptr<IInterface> iface = std::make_unique<BaseTestSvc>();
    REQUIRE( iface );
    auto              old = iface.get();
    SmartIF<IService> svc;
    svc = std::move( iface );
    REQUIRE( svc );
    CHECK( svc.get() == old );
    CHECK( svc->refCount() == 1 );
    CHECK( BaseTestSvc::s_instances == 1 );
  }
  // no instances left
  CHECK( BaseTestSvc::s_instances == 0 );
}

TEST_CASE( "SmartIF of const T" ) {
  {
    SmartIF<const IInterface> svc{ std::make_unique<BaseTestSvc>() };
    REQUIRE( svc );
    CHECK( svc->refCount() == 1 );
    CHECK( BaseTestSvc::s_instances == 1 );

    // Note:
    // ```cpp
    // auto another = svc.as<IService>();
    // ```
    // fails to compile

    auto another = svc.as<const IService>();
    REQUIRE( another );
    CHECK( another->refCount() == 2 );
    CHECK( another.get() == svc.get() );
  }

  { // conversion from non-const to const
    SmartIF<IInterface> svc{ std::make_unique<BaseTestSvc>() };
    REQUIRE( svc );
    CHECK( svc->refCount() == 1 );
    CHECK( BaseTestSvc::s_instances == 1 );

    auto another = svc.as<const IService>();
    REQUIRE( another );
    CHECK( another->refCount() == 2 );
    CHECK( another.get() == svc.get() );
  }
  // no instances left
  CHECK( BaseTestSvc::s_instances == 0 );
}
