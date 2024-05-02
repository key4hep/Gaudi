/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <GaudiKernel/ConcurrencyFlags.h>
#include <GaudiKernel/IAlgContextSvc.h>
#include <GaudiKernel/IAlgorithm.h>
#include <GaudiKernel/IIncidentListener.h>
#include <GaudiKernel/IIncidentSvc.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/Service.h>
#include <GaudiKernel/StatusCode.h>
#include <boost/thread.hpp>
#include <vector>

// ============================================================================
/** @class AlgContextSvc
 *  Simple implementation of interface IAlgContextSvc
 *  for Algorithm Context Service
 *  @author ATLAS Collaboration
 *  @author modified by Vanya BELYAEV ibelyaev@physics.sye.edu
 *  @author incident listening  removed by Benedikt Hegner
 *  @author S. Kama. Added multi-context incident based queueing to support
 *          Serial-MT cases
 *  @date 2007-03-07 (modified)
 */
class AlgContextSvc : public extends<Service, IAlgContextSvc, IIncidentListener> {
public:
  /// set the currently executing algorithm ("push_back") @see IAlgContextSvc
  StatusCode setCurrentAlg( IAlgorithm* a, const EventContext& context ) override;
  /// remove the algorithm ("pop_back") @see IAlgContextSvc
  StatusCode unSetCurrentAlg( IAlgorithm* a, const EventContext& context ) override;
  /// accessor to current algorithm: @see IAlgContextSvc
  IAlgorithm* currentAlg() const override;
  /// get the stack of executed algorithms @see IAlgContextSvc
  const IAlgContextSvc::Algorithms& algorithms() const override {
    if ( !m_algorithms.get() ) {
      static IAlgContextSvc::Algorithms empty;
      return empty;
    }
    return *m_algorithms;
  }

public:
  void handle( const Incident& ) override;

public:
  StatusCode initialize() override;
  StatusCode start() override;
  StatusCode finalize() override;

public:
  using extends::extends;

private:
  /// the stack of current algorithms
  boost::thread_specific_ptr<IAlgContextSvc::Algorithms> m_algorithms;
  /// pointer to Incident Service
  SmartIF<IIncidentSvc> m_inc = nullptr;

  Gaudi::Property<bool> m_check{ this, "Check", true, "Flag to perform more checks" };
  Gaudi::Property<bool> m_bypassInc{ this, "BypassIncidents", false,
                                     "Flag to bypass begin/endevent incident requirement" };
  std::vector<int>      m_inEvtLoop;
};

DECLARE_COMPONENT( AlgContextSvc )

StatusCode AlgContextSvc::initialize() {
  // Initialize the base class
  StatusCode sc = Service::initialize();
  if ( sc.isFailure() ) { return sc; }
  // Incident Service
  if ( m_inc ) {
    m_inc->removeListener( this );
    m_inc.reset();
  }
  // perform more checks?
  auto numSlots = Gaudi::Concurrency::ConcurrencyFlags::numConcurrentEvents();
  numSlots      = ( 1 > numSlots ) ? 1 : numSlots;
  if ( numSlots > 1000 ) {
    warning() << "Num Slots are greater than 1000. Is this correct? numSlots=" << numSlots << endmsg;
    numSlots = 1000;
    warning() << "Setting numSlots to " << numSlots << endmsg;
  }
  m_inEvtLoop.resize( numSlots, 0 );

  if ( m_check ) {
    m_inc = Service::service( "IncidentSvc", true );
    if ( !m_inc ) {
      error() << "Could not locate 'IncidentSvc'" << endmsg;
      return StatusCode::FAILURE;
    }
    m_inc->addListener( this, IncidentType::BeginEvent );
    m_inc->addListener( this, IncidentType::EndEvent );
  }
  if ( m_algorithms.get() && !m_algorithms->empty() ) {
    warning() << "Non-empty stack of algorithms #" << m_algorithms->size() << endmsg;
  }
  return StatusCode::SUCCESS;
}

// implementation of start
// needs to be removed once we have a proper service
// for getting configuration information at initialization time
// S. Kama
StatusCode AlgContextSvc::start() {
  auto sc       = Service::start();
  auto numSlots = Gaudi::Concurrency::ConcurrencyFlags::numConcurrentEvents();
  numSlots      = ( 1 > numSlots ) ? 1 : numSlots;
  if ( numSlots > 1000 ) {
    warning() << "Num Slots are greater than 1000. Is this correct? numSlots=" << numSlots << endmsg;
    numSlots = 1000;
  }
  m_inEvtLoop.resize( numSlots, 0 );

  return sc;
}

StatusCode AlgContextSvc::finalize() {
  if ( m_algorithms.get() && !m_algorithms->empty() ) {
    warning() << "Non-empty stack of algorithms #" << m_algorithms->size() << endmsg;
  }
  // Incident Service
  if ( m_inc ) {
    m_inc->removeListener( this );
    m_inc.reset();
  }
  // finalize the base class
  return Service::finalize();
}

StatusCode AlgContextSvc::setCurrentAlg( IAlgorithm* a, const EventContext& context ) {
  if ( !a ) {
    warning() << "IAlgorithm* points to NULL" << endmsg;
    return StatusCode::RECOVERABLE;
  }
  if ( !m_bypassInc ) {
    if ( !m_inEvtLoop[context.valid() ? context.slot() : 0] ) return StatusCode::SUCCESS;
  }
  // check whether thread-local algorithm list already exists
  // if not, create it
  if ( !m_algorithms.get() ) { m_algorithms.reset( new IAlgContextSvc::Algorithms() ); }
  if ( a->type() != "IncidentProcAlg" ) m_algorithms->push_back( a );

  return StatusCode::SUCCESS;
}

StatusCode AlgContextSvc::unSetCurrentAlg( IAlgorithm* a, const EventContext& context ) {
  // check whether thread-local algorithm list already exists
  // if not, create it
  if ( !m_algorithms.get() ) { m_algorithms.reset( new IAlgContextSvc::Algorithms() ); }

  if ( !a ) {
    warning() << "IAlgorithm* points to NULL" << endmsg;
    return StatusCode::RECOVERABLE;
  }

  if ( !m_bypassInc ) {
    if ( !m_inEvtLoop[context.valid() ? context.slot() : 0] ) return StatusCode::SUCCESS;
  }

  if ( a->type() != "IncidentProcAlg" ) {
    // if ( m_algorithms->empty() || m_algorithms->back() != a ){
    //   error() << "Algorithm stack is invalid" << endmsg ;
    //   return StatusCode::FAILURE ;
    // }
    if ( !m_algorithms->empty() ) {
      if ( m_algorithms->back() == a ) { m_algorithms->pop_back(); }
    }
  }
  return StatusCode::SUCCESS;
}

IAlgorithm* AlgContextSvc::currentAlg() const {
  return ( m_algorithms.get() && !m_algorithms->empty() ) ? m_algorithms->back() : nullptr;
}

void AlgContextSvc::handle( const Incident& inc ) {
  // some false sharing is possible but it should be negligible
  auto currSlot = inc.context().slot();
  if ( currSlot == EventContext::INVALID_CONTEXT_ID ) { currSlot = 0; }
  if ( inc.type() == "BeginEvent" ) {
    m_inEvtLoop[currSlot] = 1;
  } else if ( inc.type() == "EndEvent" ) {
    m_inEvtLoop[currSlot] = 0;
  }

  // This check is invalidated with RTTI AlgContext object.
  // Whole service needs to be rewritten. Commenting the error until then
  // to prevent test failures.
  // if ( m_algorithms.get() && !m_algorithms->empty() ) {
  //   //skip incident processing algorithm endevent incident
  //   if((m_algorithms->size()!=1) ||
  //   (m_algorithms->back()->type()!="IncidentProcAlg")){
  //     error() << "Non-empty stack of algorithms #"
  // 	      << m_algorithms->size() << endmsg ;
  //   }
  // }
}

// From here on, we have unit tests.
#if defined( BUILD_UNIT_TESTS )
#  if __has_include( <catch2/catch.hpp>)
// Catch2 v2
#    include <catch2/catch.hpp>
#  else
// Catch2 v3
#    include <catch2/catch_test_macros.hpp>
#  endif

#  include <Gaudi/Algorithm.h>

namespace mock {
  struct ServiceLocator : implements<ISvcLocator> {
    std::list<IService*> m_services;
    SmartIF<IService>    m_null_svc;

    const std::list<IService*>& getServices() const override { return m_services; }
    bool                        existsService( std::string_view /* name */ ) const override { return false; }
    SmartIF<IService>&          service( const Gaudi::Utils::TypeNameString& /* typeName */,
                                         const bool /* createIf */ ) override {
      return m_null_svc;
    }
  };
  struct Algorithm : Gaudi::Algorithm {
    using Gaudi::Algorithm::Algorithm;
    StatusCode execute( const EventContext& ) const override { return StatusCode::SUCCESS; }
  };
} // namespace mock

TEST_CASE( "AlgContextSvc basic operations" ) {
  SmartIF<ISvcLocator> svcLoc{ new mock::ServiceLocator };
  AlgContextSvc        acs{ "AlgContextSvc", svcLoc };
  REQUIRE( acs.setProperty( "BypassIncidents", true ).isSuccess() ); // do not try to invoke incident svc

  // check that algorithms() never returns a nullptr
  // (see https://gitlab.cern.ch/gaudi/Gaudi/-/issues/304)
  auto empty_algorithms = &acs.algorithms();
  REQUIRE( empty_algorithms != nullptr );
  CHECK( empty_algorithms->empty() );

  mock::Algorithm alg{ "dummy", svcLoc };
  EventContext    ctx;

  // add an algorithm
  REQUIRE( acs.setCurrentAlg( &alg, ctx ).isSuccess() );
  {
    auto algorithms = &acs.algorithms();

    // what we get before adding the first algorithm is a dummy static instance
    // see https://gitlab.cern.ch/gaudi/Gaudi/-/issues/304#note_7930366
    CHECK( empty_algorithms != algorithms );

    REQUIRE( algorithms != nullptr );
    REQUIRE( algorithms->size() == 1 );
    CHECK( algorithms->at( 0 ) == &alg );
  }

  // removing the algorithm results in a an empty list
  REQUIRE( acs.unSetCurrentAlg( &alg, ctx ).isSuccess() );
  {
    auto algorithms = &acs.algorithms();
    REQUIRE( algorithms != nullptr );
    REQUIRE( algorithms->empty() );
  }
}
#endif
