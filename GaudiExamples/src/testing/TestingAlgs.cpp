/*
 * TestingAlg1.cpp
 *
 *  Created on: Sep 7, 2009
 *      Author: Marco Clemencic
 */

#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/Memory.h"
#include "GaudiKernel/Sleep.h"

#include <iostream>

#include <csignal>

namespace GaudiTesting
{

  class DestructorCheckAlg : public GaudiAlgorithm
  {
  public:
    using GaudiAlgorithm::GaudiAlgorithm;
    ~DestructorCheckAlg() override
    {
      // do not print messages if we are created in genconf
      const std::string cmd = System::cmdLineArgs()[0];
      if ( cmd.find( "genconf" ) != std::string::npos ) return;

      std::cout << "Destructor of " << name() << std::endl;
    }
    StatusCode execute() override
    {
      info() << "Executing " << name() << endmsg;
      return StatusCode::SUCCESS;
    }
  };

  class SleepyAlg : public GaudiAlgorithm
  {
  public:
    using GaudiAlgorithm::GaudiAlgorithm;
    StatusCode execute() override
    {
      info() << "Executing event " << ++m_counter << endmsg;
      info() << "Sleeping for " << m_sleep.value() << " seconds" << endmsg;
      Gaudi::Sleep( m_sleep );
      info() << "Back from sleep" << endmsg;
      return StatusCode::SUCCESS;
    }

  private:
    Gaudi::Property<int> m_sleep{this, "SleepTime", 10, "Seconds to sleep during the execute"};
    int m_counter = 0;
  };

  /**
   * Simple algorithm that raise a signal after N events.
   */
  class SignallingAlg : public GaudiAlgorithm
  {
  public:
    using GaudiAlgorithm::GaudiAlgorithm;
    StatusCode execute() override
    {
      if ( m_eventCount <= 0 ) {
        info() << "Raising signal now" << endmsg;
        std::raise( m_signal );
      } else {
        info() << m_eventCount.value() << " events to go" << endmsg;
      }
      --m_eventCount;
      return StatusCode::SUCCESS;
    }

  private:
    Gaudi::Property<int> m_eventCount{this, "EventCount", 3, "Number of events to let go before raising the signal"};
    Gaudi::Property<int> m_signal{this, "Signal", SIGINT, "Signal to raise"};
  };

  class StopLoopAlg : public GaudiAlgorithm
  {
  public:
    using GaudiAlgorithm::GaudiAlgorithm;
    StatusCode execute() override
    {
      if ( m_eventCount <= 0 ) {
        info() << "Stopping loop with " << m_mode.value() << endmsg;
        if ( m_mode == "exception" ) {
          Exception( "Stopping loop" );
        } else if ( m_mode == "stopRun" ) {
          auto ep = serviceLocator()->as<IEventProcessor>();
          if ( !ep ) {
            error() << "Cannot get IEventProcessor" << endmsg;
            return StatusCode::FAILURE;
          }
          ep->stopRun();
        } else { // "failure"
          return StatusCode::FAILURE;
        }
      } else {
        info() << m_eventCount.value() << " events to go" << endmsg;
      }
      --m_eventCount;
      return StatusCode::SUCCESS;
    }

  private:
    Gaudi::Property<int> m_eventCount{this, "EventCount", 3,
                                      "Number of events to let go before breaking the event loop"};
    Gaudi::Property<std::string> m_mode{this, "Mode", "failure",
                                        "Type of interruption ['exception', 'stopRun',  'failure']"};
  };

  class CustomIncidentAlg : public GaudiAlgorithm
  {
  public:
    using GaudiAlgorithm::GaudiAlgorithm;
    StatusCode initialize() override
    {
      StatusCode sc = GaudiAlgorithm::initialize();
      if ( sc.isFailure() ) return sc;

      if ( m_incident.empty() ) {
        error() << "The incident type (property Incident) must be declared." << endmsg;
        return StatusCode::FAILURE;
      }

      m_incidentSvc = service( "IncidentSvc" );
      if ( !m_incidentSvc ) return StatusCode::FAILURE;

      return StatusCode::SUCCESS;
    }
    StatusCode execute() override
    {
      if ( m_eventCount == 0 ) {
        info() << "Firing incident " << m_incident.value() << endmsg;
        m_incidentSvc->fireIncident( Incident( name(), m_incident ) );
      } else if ( m_eventCount > 0 ) {
        info() << m_eventCount.value() << " events to go" << endmsg;
      } else {
        info() << "keep processing events..." << endmsg;
      }
      --m_eventCount;
      return StatusCode::SUCCESS;
    }
    StatusCode finalize() override
    {
      m_incidentSvc.reset();
      return GaudiAlgorithm::finalize();
    }

  private:
    Gaudi::Property<int> m_eventCount{this, "EventCount", 3, "Number of events to let go before firing the incident."};
    Gaudi::Property<std::string> m_incident{this, "Incident", "", "Type of incident to fire."};
    /// Incident service.
    SmartIF<IIncidentSvc> m_incidentSvc;
  };

  /**
   * Simple algorithm that creates dummy objects in the transient store.
   */
  class PutDataObjectAlg : public GaudiAlgorithm
  {
  public:
    using GaudiAlgorithm::GaudiAlgorithm;

    StatusCode initialize() override
    {
      StatusCode sc = GaudiAlgorithm::initialize();
      if ( sc.isFailure() ) return sc;

      m_dataProvider = service( m_dataSvc );
      if ( !m_dataProvider ) return StatusCode::FAILURE;

      return StatusCode::SUCCESS;
    }

    StatusCode execute() override
    {
      StatusCode sc = StatusCode::SUCCESS;
      info() << "Adding " << m_paths.size() << " objects to " << m_dataSvc.value() << endmsg;
      for ( auto& p : m_paths ) {
        info() << "Adding '" << p << "'" << endmsg;
        DataObject* obj = new DataObject();
        sc              = m_dataProvider->registerObject( p, obj );
        if ( sc.isFailure() ) warning() << "Cannot register object '" << p << "'" << endmsg;
      }

      return sc;
    }

    StatusCode finalize() override
    {
      m_dataProvider.reset();
      return GaudiAlgorithm::finalize();
    }

  private:
    Gaudi::Property<std::vector<std::string>> m_paths{
        this, "Paths", {}, "List of paths in the transient store to load"};
    Gaudi::Property<std::string> m_dataSvc{this, "DataSvc", "EventDataSvc", "Name of the data service to use"};
    SmartIF<IDataProviderSvc> m_dataProvider;
  };

  /**
   * Simple algorithm that retrieves objects from the transient store.
   */
  class GetDataObjectAlg : public GaudiAlgorithm
  {
  public:
    using GaudiAlgorithm::GaudiAlgorithm;

    StatusCode initialize() override
    {
      StatusCode sc = GaudiAlgorithm::initialize();
      if ( sc.isFailure() ) return sc;

      m_dataProvider = service( m_dataSvc );
      if ( !m_dataProvider ) return StatusCode::FAILURE;

      return StatusCode::SUCCESS;
    }

    StatusCode execute() override
    {
      info() << "Getting " << m_paths.size() << " objects from " << m_dataSvc.value() << endmsg;
      bool missing = false;
      for ( auto& p : m_paths ) {
        info() << "Getting '" << p << "'" << endmsg;
        DataObject* obj;
        StatusCode sc = m_dataProvider->retrieveObject( p, obj );
        if ( sc.isFailure() ) {
          warning() << "Cannot retrieve object '" << p << "'" << endmsg;
          missing = true;
        }
      }

      return ( missing && !m_ignoreMissing ) ? StatusCode::FAILURE : StatusCode::SUCCESS;
    }

    StatusCode finalize() override
    {
      m_dataProvider.reset();
      return GaudiAlgorithm::finalize();
    }

  private:
    Gaudi::Property<std::vector<std::string>> m_paths{
        this, "Paths", {}, "List of paths in the transient store to load"};
    Gaudi::Property<std::string> m_dataSvc{this, "DataSvc", "EventDataSvc", "Name of the data service to use"};
    Gaudi::Property<bool> m_ignoreMissing{this, "IgnoreMissing", false,
                                          "if True,  missing objects will not beconsidered an error"};
    SmartIF<IDataProviderSvc> m_dataProvider;
  };

  class OddEventsFilter : public GaudiAlgorithm
  {
  public:
    using GaudiAlgorithm::GaudiAlgorithm;
    StatusCode initialize() override
    {
      m_counter = 0;
      return GaudiAlgorithm::initialize();
    }
    StatusCode execute() override
    {
      setFilterPassed( ( ++m_counter ) % 2 );
      return StatusCode::SUCCESS;
    }

  protected:
    int m_counter = 0;
  };

  class EvenEventsFilter : public OddEventsFilter
  {
  public:
    using OddEventsFilter::OddEventsFilter;
    StatusCode execute() override
    {
      setFilterPassed( ( ( ++m_counter ) % 2 ) == 0 );
      return StatusCode::SUCCESS;
    }
  };

  /**
   * Simple algorithm that creates dummy objects in the transient store.
   */
  class ListTools : public GaudiAlgorithm
  {
  public:
    using GaudiAlgorithm::GaudiAlgorithm;

    StatusCode execute() override
    {
      StatusCode sc = StatusCode::SUCCESS;
      info() << "All tool instances:" << endmsg;
      for ( auto& tool : toolSvc()->getTools() ) {
        info() << "  " << tool->name() << endmsg;
      }
      return sc;
    }
  };

  /**
   * Simple algorithm that prints the memory usage every N events (property "Frequency").
   */
  class PrintMemoryUsage : public GaudiAlgorithm
  {
  public:
    using GaudiAlgorithm::GaudiAlgorithm;
    StatusCode initialize() override
    {
      m_counter = 0;
      return GaudiAlgorithm::initialize();
    }
    StatusCode execute() override
    {
      if ( ( m_frequency <= 1 ) || ( ( m_counter ) % m_frequency == 0 ) ) print();
      return StatusCode::SUCCESS;
    }
    StatusCode finalize() override
    {
      print();
      return GaudiAlgorithm::finalize();
    }

  protected:
    Gaudi::Property<int> m_frequency{this, "Frequency", 1, "How often to print the memory usage (number of events)"};
    int m_counter = 0;
    void print()
    {
      info() << "vmem: " << System::virtualMemory() << " kB" << endmsg;
      info() << "rss:  " << System::mappedMemory() << " kB" << endmsg;
    }
  };
}

namespace GaudiTesting
{
  DECLARE_COMPONENT( DestructorCheckAlg )
  DECLARE_COMPONENT( SleepyAlg )
  DECLARE_COMPONENT( SignallingAlg )
  DECLARE_COMPONENT( StopLoopAlg )
  DECLARE_COMPONENT( CustomIncidentAlg )
  DECLARE_COMPONENT( PutDataObjectAlg )
  DECLARE_COMPONENT( GetDataObjectAlg )
  DECLARE_COMPONENT( OddEventsFilter )
  DECLARE_COMPONENT( EvenEventsFilter )
  DECLARE_COMPONENT( ListTools )
  DECLARE_COMPONENT( PrintMemoryUsage )
}
