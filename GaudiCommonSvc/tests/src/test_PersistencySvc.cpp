/***********************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <GaudiKernel/ConversionSvc.h>
#include <GaudiKernel/IMessageSvc.h>
#include <GaudiKernel/IPersistencySvc.h>
#include <GaudiKernel/IService.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/Message.h>
#include <GaudiKernel/Service.h>
#include <chrono>
#include <mutex>
#include <thread>

#if __has_include( <catch2/catch.hpp>)
// Catch2 v2
#  include <catch2/catch.hpp>
using Catch::Matchers::Contains;
#  define ContainsSubstring Contains
#else
// Catch2 v3
#  include <catch2/catch_test_macros.hpp>
#  include <catch2/matchers/catch_matchers_string.hpp>
using Catch::Matchers::ContainsSubstring;
#endif

namespace {
  /// Minimal stand-alone ISvcLocator implementation valid for this test
  struct MiniSvcMgr : implements<ISvcLocator> {
    SmartIF<IService>& service( const Gaudi::Utils::TypeNameString& typeName, const bool createIf = true ) override {
      if ( auto it = serviceMap.find( typeName.name() ); it != serviceMap.end() ) { return it->second; }
      if ( createIf ) {
        auto svc = Service::Factory::create( typeName.type(), typeName.name(), this );
        if ( svc ) {
          serviceMap[typeName.name()] = SmartIF<IService>{ svc.release() };
          return serviceMap[typeName.name()];
        }
      }
      static SmartIF<IService> not_found;
      return not_found;
    }

    const std::list<IService*>& getServices() const override {
      static std::list<IService*> servicesList;
      return servicesList;
    }

    bool existsService( std::string_view name ) const override {
      return serviceMap.find( std::string{ name } ) != serviceMap.end();
    }

    StatusCode queryInterface( const InterfaceID& iid, void** pp ) override {
      if ( iid == IMessageSvc::interfaceID() ) {
        auto m = ISvcLocator::service<IMessageSvc>( "MessageSvc" ).get();
        m->addRef();
        *pp = static_cast<IMessageSvc*>( m );
        return ( *pp ) ? StatusCode::SUCCESS : StatusCode::FAILURE;
      }
      return implements::queryInterface( iid, pp );
    }

    std::map<std::string, SmartIF<IService>> serviceMap;
  };

  /// Minimal IMessageSvc that collect all reported messages.
  struct CapturingMsgSvc : extends<Service, IMessageSvc> {
    using extends::extends;
    void reportMessage( const Message& message ) override {
      std::lock_guard _{ messages_mutex };
      all_messages.emplace_back( message );
    }
    void reportMessage( const Message& msg, int outputLevel ) override {
      Message message = msg;
      message.setType( outputLevel );
      reportMessage( message );
    }
    void reportMessage( const StatusCode&, std::string_view ) override { throw std::logic_error( "not implemented" ); }
    void reportMessage( std::string, int, std::string ) override { throw std::logic_error( "not implemented" ); }

    void insertMessage( const StatusCode&, Message ) override { throw std::logic_error( "not implemented" ); }
    void eraseMessage() override { throw std::logic_error( "not implemented" ); }
    void eraseMessage( const StatusCode& ) override { throw std::logic_error( "not implemented" ); }
    void eraseMessage( const StatusCode&, const Message& ) override { throw std::logic_error( "not implemented" ); }

    void insertStream( int, std::string, std::ostream* ) override { throw std::logic_error( "not implemented" ); }
    void eraseStream() override { throw std::logic_error( "not implemented" ); }
    void eraseStream( int ) override { throw std::logic_error( "not implemented" ); }
    void eraseStream( int, std::ostream* ) override { throw std::logic_error( "not implemented" ); }
    void eraseStream( std::ostream* ) override { throw std::logic_error( "not implemented" ); }

    std::ostream* defaultStream() const override { return nullptr; }
    void          setDefaultStream( std::ostream* ) override { throw std::logic_error( "not implemented" ); }

    int outputLevel() const override { return MSG::VERBOSE; }
    int outputLevel( std::string_view ) const override { return MSG::VERBOSE; }

    void setOutputLevel( int ) override {}
    void setOutputLevel( std::string_view, int ) override {}

    bool useColor() const override { return false; }

    std::string getLogColor( int ) const override { return {}; }
    int         messageCount( MSG::Level ) const override { return 0; }

    std::mutex         messages_mutex;
    std::list<Message> all_messages;
  };
  DECLARE_COMPONENT_WITH_ID( CapturingMsgSvc, "CapturingMsgSvc" )

  struct DummyCnvSvc : ConversionSvc {
    DummyCnvSvc( const std::string& name, ISvcLocator* svc ) : ConversionSvc( name, svc, 0x20 ) {}

    // this method is used inside PersistencySvc::addCnvService and is a perfect place
    // to synchronize two threads just ahead of the call to map::emplace
    long repSvcType() const override {
      auto out = ConversionSvc::repSvcType();
      using namespace std::chrono_literals;
      auto t1 = checkpoint + 200ms;
      auto t2 = t1 + 0.002ms;
      bool b  = true;
      std::this_thread::sleep_until( first.compare_exchange_strong( b, false ) ? t1 : t2 );
      return out;
    }
    // avoid a segfault that may arise from the race condition we want to check
    std::size_t release() override { return 1; }
    // I use this time_point to synchronize the threads and expose race conditions.
    static std::chrono::steady_clock::time_point checkpoint;
    // flag to be able to introduce a tiny delay between the two threads
    static std::atomic<bool> first;
  };
  DECLARE_COMPONENT_WITH_ID( DummyCnvSvc, "DummyCnvSvc" )

  std::atomic<bool>                     DummyCnvSvc::first{ true };
  std::chrono::steady_clock::time_point DummyCnvSvc::checkpoint;

  // flag to break the loop over the attempts to trigger the race condition
  bool race_condition_exposed = false;
} // namespace

TEST_CASE( "PersistencySvc thread safety" ) {
  // From https://gitlab.cern.ch/gaudi/Gaudi/-/issues/314
  //
  // When PersistencySvc::getService is invoked by multiple threads at the same time,
  // we have a race condition where both threads try to add the service to the service
  // map, in which case one of the two fails because the other completed the emplace
  // just before.
  // This test tries to expose the race condition by synchronizing two threads.

  // The race condition does not happens all the times, so we run the test multiple times
  [[maybe_unused]] auto _ = GENERATE( repeat( 10, value( 0 ) ) );

  // once we managed to expose the race condition there is no need to try again
  if ( race_condition_exposed ) return;

  SmartIF<ISvcLocator> svcMgr{ new MiniSvcMgr{} };

  // ensure we use the custom MsgSvc
  REQUIRE( svcMgr->service<IMessageSvc>( "CapturingMsgSvc/MessageSvc" ) );

  auto ps = svcMgr->service<IPersistencySvc>( "PersistencySvc" );
  REQUIRE( ps );
  auto prop = ps.as<IProperty>();
  REQUIRE( prop );
  REQUIRE( prop->setPropertyRepr( "CnvServices", R"(["DummyCnvSvc"])" ) );

  StatusCode thread_1_result, thread_2_result;
  DummyCnvSvc::checkpoint = std::chrono::steady_clock::now();
  std::thread t1{ [ps, &thread_1_result]() {
    IConversionSvc* pCnvSvc{};
    thread_1_result = ps->getService( 0x20, pCnvSvc );
    if ( pCnvSvc ) pCnvSvc->addRef();
  } };
  std::thread t2{ [ps, &thread_2_result]() {
    IConversionSvc* pCnvSvc{};
    thread_2_result = ps->getService( 0x20, pCnvSvc );
    if ( pCnvSvc ) pCnvSvc->addRef();
  } };
  t1.join();
  t2.join();
  CHECK( thread_1_result );
  CHECK( thread_2_result );
  race_condition_exposed = !( thread_1_result && thread_2_result );

  // make sure no error message was reported
  auto c = dynamic_cast<CapturingMsgSvc*>( svcMgr.as<IMessageSvc>().get() );
  for ( auto m : c->all_messages ) {
    std::ostringstream msg;
    msg << m;
    CHECK_THAT( msg.str(), !ContainsSubstring( "ERROR" ) );
  }
}
