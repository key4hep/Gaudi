/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Interfaces/IQueueingEventProcessor.h>
#include <GaudiKernel/AppReturnCode.h>
#include <GaudiKernel/MinimalEventLoopMgr.h>
#include <GaudiKernel/ThreadLocalContext.h> // for Gaudi::Hive::setCurrentContext
#include <tbb/concurrent_queue.h>
#include <thread>

#define ON_DEBUG if ( UNLIKELY( outputLevel() <= MSG::DEBUG ) )
#define ON_VERBOSE if ( UNLIKELY( outputLevel() <= MSG::VERBOSE ) )

#define DEBMSG ON_DEBUG debug()
#define VERMSG ON_VERBOSE verbose()

namespace Gaudi::Examples {
  class GAUDI_API QueueingEventLoopMgr
      : public extends<MinimalEventLoopMgr, Gaudi::Interfaces::IQueueingEventProcessor> {
  public:
    using extends::extends;

    StatusCode start() override;
    StatusCode stop() override;

    // backward compatible EventProcessor implementation.
    StatusCode executeEvent( EventContext&& ctx ) override {
      using namespace std::chrono_literals;
      push( std::move( ctx ) );
      std::optional<ResultType> result;
      while ( !( result = pop() ) ) std::this_thread::sleep_for( 1ms );
      return std::get<0>( std::move( *result ) );
    }

    void push( EventContext&& ctx ) override { m_incoming.push( std::move( ctx ) ); }

    /// Tell if the processor has events in the queues.
    bool empty() const override {
      // because of the way we count "in flight" (+1 while waiting that we get something from the queue)
      // and the way tbb:concurrent_bounded_queue reports the size while waiting for pop to return (-1)
      // this is a correct definition of "empty" (nothing pending, nothing being processed and no results
      // to be popped)
      return !( m_inFlight + m_done.size() + m_incoming.size() );
    }

    /// Get the next available result, if any.
    std::optional<ResultType> pop() override {
      ResultType out;
      if ( m_done.try_pop( out ) )
        return out;
      else
        return std::nullopt;
    }

  private:
    tbb::concurrent_bounded_queue<EventContext> m_incoming;
    tbb::concurrent_bounded_queue<ResultType>   m_done;
    std::atomic<std::size_t>                    m_inFlight{0};

    // our capacity is the input queue capacity + 1 (N events pending + 1 being processed)
    Gaudi::Property<std::size_t> m_queueCapacity{
        this, "Capacity", m_incoming.capacity() + 1,
        [this]( Gaudi::Details::PropertyBase& ) { m_incoming.set_capacity( m_queueCapacity - 1 ); }};

    std::tuple<StatusCode, EventContext> processEvent( EventContext&& context );

    std::thread m_evtLoopThread;
  };
} // namespace Gaudi::Examples

DECLARE_COMPONENT( Gaudi::Examples::QueueingEventLoopMgr )

using namespace Gaudi::Examples;

namespace {
  /// Helper class to set the application return code in case of early exit
  /// (e.g. exception). Copied from GaudiCoreSvc
  class RetCodeGuard {
  public:
    inline RetCodeGuard( SmartIF<IProperty> appmgr, int retcode )
        : m_appmgr( std::move( appmgr ) ), m_retcode( retcode ) {}
    inline void ignore() { m_retcode = Gaudi::ReturnCode::Success; }
    inline ~RetCodeGuard() {
      if ( UNLIKELY( Gaudi::ReturnCode::Success != m_retcode ) ) {
        Gaudi::setAppReturnCode( m_appmgr, m_retcode ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
      }
    }

  private:
    SmartIF<IProperty> m_appmgr;
    int                m_retcode;
  };
} // namespace

StatusCode QueueingEventLoopMgr::start() {
  auto ok = base_class::start();
  if ( !ok ) return ok;

  info() << m_queueCapacity << endmsg;

  m_evtLoopThread = std::thread( [this]() {
    while ( true ) {
      EventContext ctx;
      ++m_inFlight; // yes, this is not accurate.
      m_incoming.pop( ctx );
      if ( LIKELY( ctx.valid() ) ) {
        // the sleep is not strictly needed, but it should make the output more stable for the test
        std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
        m_done.push( processEvent( std::move( ctx ) ) );
        --m_inFlight; // yes, this is not accurate.
      } else {
        DEBMSG << "exiting event loop thread" << endmsg;
        m_done.emplace( StatusCode{StatusCode::SUCCESS, true}, std::move( ctx ) );
        --m_inFlight; // yes, this is not accurate.
        break;
      }
    }
  } );

  return ok;
}

std::tuple<StatusCode, EventContext> QueueingEventLoopMgr::processEvent( EventContext&& context ) {
  DEBMSG << "processing " << context << endmsg;

  bool eventfailed = false;

  m_aess->reset( context );
  Gaudi::Hive::setCurrentContext( context );

  // select the appropriate store
  if ( m_WB.isValid() ) m_WB->selectStore( context.slot() ).ignore();

  // Get the IProperty interface of the ApplicationMgr to pass it to RetCodeGuard
  const auto appmgr = serviceLocator()->as<IProperty>();
  // Call the execute() method of all top algorithms
  for ( auto& ita : m_topAlgList ) {
    StatusCode sc( StatusCode::FAILURE );
    try {
      if ( UNLIKELY( m_abortEventListener.abortEvent ) ) {
        DEBMSG << "AbortEvent incident fired by " << m_abortEventListener.abortEventSource << endmsg;
        m_abortEventListener.abortEvent = false;
        sc.ignore();
        break;
      }
      RetCodeGuard rcg( appmgr, Gaudi::ReturnCode::UnhandledException );
      sc = ita->sysExecute( context );
      rcg.ignore(); // disarm the guard
    } catch ( const GaudiException& Exception ) {
      fatal() << ".executeEvent(): Exception with tag=" << Exception.tag() << " thrown by " << ita->name() << endmsg;
      error() << Exception << endmsg;
    } catch ( const std::exception& Exception ) {
      fatal() << ".executeEvent(): Standard std::exception thrown by " << ita->name() << endmsg;
      error() << Exception.what() << endmsg;
    } catch ( ... ) { fatal() << ".executeEvent(): UNKNOWN Exception thrown by " << ita->name() << endmsg; }

    if ( UNLIKELY( !sc.isSuccess() ) ) {
      warning() << "Execution of algorithm " << ita->name() << " failed" << endmsg;
      eventfailed = true;
    }
  }

  m_aess->updateEventStatus( eventfailed, context );

  // ensure that the abortEvent flag is cleared before the next event
  if ( UNLIKELY( m_abortEventListener.abortEvent ) ) {
    DEBMSG << "AbortEvent incident fired by " << m_abortEventListener.abortEventSource << endmsg;
    m_abortEventListener.abortEvent = false;
  }

  // Call the execute() method of all output streams
  for ( auto& ito : m_outStreamList ) {
    AlgExecState& state = m_aess->algExecState( ito, context );
    state.setFilterPassed( true );
    StatusCode sc = ito->sysExecute( context );
    if ( UNLIKELY( !sc.isSuccess() ) ) {
      warning() << "Execution of output stream " << ito->name() << " failed" << endmsg;
      eventfailed = true;
    }
  }

  StatusCode outcome{StatusCode::SUCCESS, true};
  // Check if there was an error processing current event
  if ( UNLIKELY( eventfailed ) ) {
    error() << "Error processing event loop." << endmsg;
    std::ostringstream ost;
    m_aess->dump( ost, context );
    DEBMSG << "Dumping AlgExecStateSvc status:\n" << ost.str() << endmsg;
    outcome = StatusCode{StatusCode::FAILURE, true};
  }

  return {std::move( outcome ), std::move( context )};
}

StatusCode QueueingEventLoopMgr::stop() {
  // Send an invalid context to stop the processing thread
  push( EventContext{} );
  m_evtLoopThread.join();

  return base_class::stop();
}
