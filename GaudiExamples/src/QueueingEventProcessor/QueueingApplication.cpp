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
/** @file Example of implementation of a Gaudi::Application using IQueueingEventProcessor.

In this example we use the Gaudi::Interfaces::IQueueingEventProcessor implementation QueueingEventLoopMgr,
which simply enqueues events to be processed by a dedicated thread, independent from the main thread.

After creating, configuring, initializing and starting the ApplicationMgr instance, the QueueingEventLoopMgr
is ready to process events.

For each event we want to process we have to create a new EventContext via the createEventContext() method,
then we enqueue it with push(EventContext&&). Completed events can be extracted from the output queue with
the non blocking function pop() (one can use the method empty() to check if there's anything to be popped or
the system is busy).

In this example we produce events to be processed in batches of `evts_in_batch` events, the QueueingEventLoopMgr
limits the number of events it can hold at a given time (`Capacity` property), and we stop after `n_of_batches`
complete batches of events.

See the comments in the code for the details.
*/

#include <Gaudi/Application.h>
#include <Gaudi/Interfaces/IQueueingEventProcessor.h>
#include <Gaudi/Property.h>
#include <GaudiKernel/AppReturnCode.h>
#include <GaudiKernel/IAppMgrUI.h>
#include <GaudiKernel/IMessageSvc.h>
#include <GaudiKernel/IProperty.h>
#include <GaudiKernel/IStateful.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/SmartIF.h>
#include <queue>
#include <thread>

using namespace std::literals::chrono_literals;
using std::this_thread::sleep_for;

namespace Gaudi::Examples {
  class QueueingApplication : public Gaudi::Application {
    // - nothing special to add to the base constructor (which implements the default configuration)
    using Application::Application;

    // this method is used to implement the main application logic (prepare to run, loop over events, terminate)
    int run() override {
      // - parameters for the job
      const std::size_t n_of_batches  = 2;
      const std::size_t evts_in_batch = 5;

      // - get ready to process events
      app->initialize().ignore();

      // - this MsgStream is useful to have uniform printout
      MsgStream log( app.as<IMessageSvc>(), "<main>" );

      // we print the parameters here so that they appear close to the Capacity in the log
      log << MSG::INFO << " n_of_batches: " << n_of_batches << endmsg;
      log << MSG::INFO << " evts_in_batch: " << evts_in_batch << endmsg;

      app->start().ignore(); // this starts the QueueingEventLoopMgr processing thread

      // - main processing loop
      {
        // - get the IQueueingEventProcessor interface of the application
        SmartIF<Gaudi::Interfaces::IQueueingEventProcessor> qep{ app };

        // - processing state informations
        //   - events ready to be processed
        std::queue<EventContext> ready;
        //   - count of events enqueued
        std::size_t evt_count = 0;

        // - loop over input batches
        for ( std::size_t batch = 1; batch <= n_of_batches; ++batch ) {
          // - prepare the batch
          log << MSG::INFO << "prepare batch of events n. " << batch << endmsg;
          if ( batch == 2 ) {
            log << MSG::INFO << "  (pretend we need time so that the processing thread drains the input queue)"
                << endmsg;
            sleep_for( 4s );
            log << MSG::INFO << "  (all events in the queue should have been processed by now)" << endmsg;
          }
          for ( std::size_t i = 0; i < evts_in_batch; ++i ) {
            // - create a new EventContext for each event in the batch
            auto ctx = qep->createEventContext();
            // ... here you can do something with the context ... like setting I/O related stuff
            // - declare the event as ready to be enqueued
            ready.push( std::move( ctx ) );
          }

          // - once the batch is ready we can push all events, relying on the push to block if needed
          log << MSG::INFO << "looping over the batch" << endmsg;
          while ( !ready.empty() ) {
            ++evt_count;
            log << MSG::INFO << "- pushing event " << evt_count << " (" << ready.front() << ")..." << endmsg;
            qep->push( std::move( ready.front() ) ); // this blocks if the system is full
            ready.pop();
            log << MSG::INFO << "  ... event " << evt_count << " pushed" << endmsg;

            // - for each push we try a pop, to avoid that the output queue gets too big
            log << MSG::INFO << "- checking for completed events" << endmsg;
            if ( auto result = qep->pop() ) { // this never blocks, but evaulates to false if there was nothing to pop
              // - if an event completed, we can do something (e.g. I/O)
              auto&& [sc, ctx] = std::move( *result );
              log << MSG::INFO << "  " << ctx << " -> " << sc << endmsg;
            }
          }
        }
        log << MSG::INFO << "no more inputs: let's drain the output queue" << endmsg;
        while ( !qep->empty() ) {
          if ( auto result = qep->pop() ) {
            auto&& [sc, ctx] = std::move( *result );
            log << MSG::INFO << "  " << ctx << " -> " << sc << endmsg;
          } else {
            sleep_for( 10ms );
          }
        }

        // - nothing else to do on the events
        log << MSG::INFO << "all done" << endmsg;
      }

      // - terminate the application
      app->stop().ignore(); // this stops the QueueingEventLoopMgr processing thread
      app->finalize().ignore();

      // - get and propagate the return code the ApplicationMgr whishes to expose
      return getAppReturnCode( app.as<IProperty>() );
    }
  }; // namespace Gaudi::Examples

  DECLARE_COMPONENT( QueueingApplication )
} // namespace Gaudi::Examples
