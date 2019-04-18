/** @file Example of implementation of a Gaudi::Application using IAsyncEventProcessor.

In this example we use the Gaudi::Interfaces::IAsyncEventProcessor implementation AsyncEventLoopMgr,
which simply enqueues events to be processed by a dedicated thread, independent from the main thread.

After creating, configuring, initializing and starting the ApplicationMgr instance,the AsyncEventLoopMgr
is ready to process events.

For each event we want to process we have to create a new EventContext via the createEventContext() method,
then we enqueue it with asyncExecuteEvent(EventContext&&). The returned std::future instances can be used to
check which events completed.

In this example we produce events to be processed in batches of `evts_in_batch` events, we keep maximum
`evts_in_flight` events in the processing queue of AsyncEventLoopMgr, and we stop after `n_of_batches`
complete batches of events.

See the comments in the code for the details.
*/

#include <Gaudi/Application.h>
#include <Gaudi/Interfaces/IAsyncEventProcessor.h>
#include <GaudiKernel/AppReturnCode.h>
#include <GaudiKernel/IAppMgrUI.h>
#include <GaudiKernel/IJobOptionsSvc.h>
#include <GaudiKernel/IMessageSvc.h>
#include <GaudiKernel/IProperty.h>
#include <GaudiKernel/IStateful.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/Property.h>
#include <GaudiKernel/SmartIF.h>
#include <queue>
#include <thread>

namespace Gaudi::Examples {
  class AsyncApplication : public Gaudi::Application {
    // - nothing special to add to the base constructor (which implements the default configuration)
    using Application::Application;

    // this method is used to implement the main application logic (prepare to run, loop over events, terminate)
    int run() override {
      // - get ready to process events
      app->initialize().ignore();
      app->start().ignore(); // this starts the AsyncEventLoopMgr processing thread

      // - this MsgStream is useful to have uniform printout
      MsgStream log( app.as<IMessageSvc>(), "<main>" );

      // - main processing loop
      {
        // - get the IAsyncEventProcessor interface of the application
        SmartIF<Gaudi::Interfaces::IAsyncEventProcessor> aep{app};

        // - parameters for the job
        const std::size_t n_of_batches   = 2;
        const std::size_t evts_in_batch  = 5;
        const std::size_t evts_in_flight = 3;
        log << MSG::INFO << "parameters:" << endmsg;
        log << MSG::INFO << "  n_of_batches :" << n_of_batches << endmsg;
        log << MSG::INFO << "  evts_in_batch: " << evts_in_batch << endmsg;
        log << MSG::INFO << "  evts_in_flight: " << evts_in_flight << endmsg;

        // - processing state informations
        //   - number of batches already produced
        std::size_t batches_count = 0;
        //   - events ready to be processed
        std::queue<EventContext> ready;
        //   - events enqueued
        std::list<std::future<std::tuple<StatusCode, EventContext>>> running;

        // - keep looping as long as there's something in the system
        while ( batches_count < n_of_batches || !ready.empty() || !running.empty() ) {
          log << MSG::INFO << "main loop:" << endmsg;
          log << MSG::INFO << "  prepared batches: " << batches_count << '/' << n_of_batches << endmsg;
          log << MSG::INFO << "  events in queue: " << ready.size() << endmsg;
          log << MSG::INFO << "  events in flight: " << running.size() << '/' << evts_in_flight << endmsg;

          // - if we do not have events ready to be enqueued, and we still have batches to process
          //   prepare the events of the next batch
          if ( ready.empty() && batches_count < n_of_batches ) {
            log << MSG::INFO << "prepare another batch of events" << endmsg;
            if ( batches_count ) {
              log << MSG::INFO << "  (pretend we need time so that the processing thread drains the queue)" << endmsg;
              std::this_thread::sleep_for( std::chrono::milliseconds( 5000 ) );
              log << MSG::INFO << "  (all events in queue should have been processed by now)" << endmsg;
            }
            for ( std::size_t i = 0; i < evts_in_batch; ++i ) {
              // - create a new EventContext for each event in the batch
              auto ctx = aep->createEventContext();
              // ... here you can do something with the context ... like setting I/O related stuff
              // - declare the event as ready to be enqueued
              ready.push( std::move( ctx ) );
            }
            ++batches_count;
          }

          // - if we still have pending events and less events in flight than wished, we enqueue a few more
          if ( !ready.empty() && running.size() < evts_in_flight ) {
            log << MSG::INFO << "scheduling " << std::min( evts_in_flight - running.size(), ready.size() ) << " events"
                << endmsg;
            while ( !ready.empty() && running.size() < evts_in_flight ) {
              running.push_back( aep->asyncExecuteEvent( std::move( ready.front() ) ) );
              ready.pop();
            }
            // this is an absolutely artificial, pointless waiting, but it makes the stdout more reproducible
            std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
          }

          // - look in the list of running events if some have been completed
          log << MSG::INFO << "checking for completed events" << endmsg;
          {
            bool completed_events = false;
            auto entry            = begin( running );
            while ( entry != end( running ) ) {
              if ( entry->wait_for( std::chrono::milliseconds( 1 ) ) == std::future_status::ready ) {
                // - if an event completed, we can do something (e.g. I/O) and remove it from the list
                auto&& [sc, ctx] = entry->get();
                log << MSG::INFO << "  " << ctx << " -> " << sc << endmsg;
                entry            = running.erase( entry );
                completed_events = true;
              } else
                ++entry;
            }
            // - if we didn't pop anything from the running events, there's no hurry to try to enqueue anything
            if ( !completed_events ) {
              log << MSG::INFO << "  none, waiting before next iteration" << endmsg;
              std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
            }
          }
        }
        // - nothing else to do on the events
        log << MSG::INFO << "all done" << endmsg;
      }

      // - terminate the application
      app->stop().ignore(); // this stops the AsyncEventLoopMgr processing thread
      app->finalize().ignore();

      // - get and propagate the return code the ApplicationMgr whishes to expose
      return getAppReturnCode( app.as<IProperty>() );
    }
  }; // namespace Gaudi::Examples

  DECLARE_COMPONENT( AsyncApplication )
} // namespace Gaudi::Examples
