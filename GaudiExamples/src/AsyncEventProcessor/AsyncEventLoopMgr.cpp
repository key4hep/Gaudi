#include <Gaudi/Interfaces/IAsyncEventProcessor.h>
#include <GaudiKernel/AppReturnCode.h>
#include <GaudiKernel/MinimalEventLoopMgr.h>
#include <GaudiKernel/ThreadLocalContext.h> // for Gaudi::Hive::setCurrentContext
#include <mutex>
#include <queue>
#include <thread>

#define ON_DEBUG if ( UNLIKELY( outputLevel() <= MSG::DEBUG ) )
#define ON_VERBOSE if ( UNLIKELY( outputLevel() <= MSG::VERBOSE ) )

#define DEBMSG ON_DEBUG debug()
#define VERMSG ON_VERBOSE verbose()

namespace Gaudi::Examples {
  class GAUDI_API AsyncEventLoopMgr : public extends<MinimalEventLoopMgr, Gaudi::Interfaces::IAsyncEventProcessor> {
  public:
    using extends::extends;

    StatusCode start() override;
    StatusCode stop() override;

    StatusCode executeEvent( EventContext&& ctx ) override {
      return std::get<0>( asyncExecuteEvent( std::move( ctx ) ).get() );
    }

    std::future<std::tuple<StatusCode, EventContext>> asyncExecuteEvent( EventContext&& ctx ) override;

  private:
    using ActionHandle = std::tuple<EventContext, std::promise<std::tuple<StatusCode, EventContext>>>;

    void processEvent( ActionHandle&& action );

    std::queue<ActionHandle> m_eventsQueue;

    std::thread             m_evtLoopThread;
    std::mutex              m_eventsQueueMtx;
    std::condition_variable m_eventsInQueue;
  };
} // namespace Gaudi::Examples

DECLARE_COMPONENT( Gaudi::Examples::AsyncEventLoopMgr )

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
      if ( UNLIKELY( Gaudi::ReturnCode::Success != m_retcode ) ) { Gaudi::setAppReturnCode( m_appmgr, m_retcode ); }
    }

  private:
    SmartIF<IProperty> m_appmgr;
    int                m_retcode;
  };
} // namespace

StatusCode AsyncEventLoopMgr::start() {
  auto ok = base_class::start();
  if ( !ok ) return ok;

  m_evtLoopThread = std::thread( [this]() {
    ActionHandle action;
    while ( true ) {
      {
        std::unique_lock lk{m_eventsQueueMtx};
        m_eventsInQueue.wait( lk, [this]() { return !m_eventsQueue.empty(); } );
        action = std::move( m_eventsQueue.front() );
        m_eventsQueue.pop();
      }
      const auto& ctx = std::get<0>( action );
      if ( LIKELY( ctx.valid() ) ) {
        processEvent( std::move( action ) );
      } else {
        DEBMSG << "exiting event loop thread" << endmsg;
        std::get<1>( action ).set_value( {StatusCode{StatusCode::SUCCESS, true}, std::move( std::get<0>( action ) )} );
        break;
      }
    }
  } );

  return ok;
}

std::future<std::tuple<StatusCode, EventContext>> AsyncEventLoopMgr::asyncExecuteEvent( EventContext&& ctx ) {
  ON_DEBUG {
    if ( ctx.valid() )
      debug() << "scheduling " << ctx << endmsg;
    else
      debug() << "scheduling STOP of event loop" << endmsg;
  }
  std::promise<std::tuple<StatusCode, EventContext>> promise;
  auto                                               future = promise.get_future();
  {
    std::lock_guard lk{m_eventsQueueMtx};
    m_eventsQueue.emplace( ActionHandle{std::move( ctx ), std::move( promise )} );
  }
  m_eventsInQueue.notify_one();
  return future;
}

void AsyncEventLoopMgr::processEvent( ActionHandle&& action ) {
  auto [context, promise] = std::move( action );
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
    debug() << "Dumping AlgExecStateSvc status:\n" << ost.str() << endmsg;
    outcome = StatusCode{StatusCode::FAILURE, true};
  }

  promise.set_value( {std::move( outcome ), std::move( context )} );
}

StatusCode AsyncEventLoopMgr::stop() {
  // Send an invalid context to stop the processing thread
  asyncExecuteEvent( EventContext{} ).get();
  m_evtLoopThread.join();

  return base_class::stop();
}
