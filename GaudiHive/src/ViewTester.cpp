#include "ViewTester.h"
#include "GaudiKernel/ThreadLocalContext.h"

DECLARE_COMPONENT( Test::ViewTester )

#define ON_DEBUG if ( msgLevel( MSG::DEBUG ) )
#define DEBUG_MSG ON_DEBUG debug()

#define ON_VERBOSE if ( msgLevel( MSG::VERBOSE ) )
#define VERBOSE_MSG ON_VERBOSE verbose()

using namespace Test;

//------------------------------------------------------------------------------

ViewTester::ViewTester( const std::string& name, // the algorithm instance name
                        ISvcLocator*       pSvc )
    : GaudiAlgorithm( name, pSvc )
{
}

ViewTester::~ViewTester()
{
  for ( uint i = 0; i < m_inputHandles.size(); ++i ) delete m_inputHandles[i];

  for ( uint i = 0; i < m_outputHandles.size(); ++i ) delete m_outputHandles[i];
}

StatusCode ViewTester::initialize()
{
  auto sc = GaudiAlgorithm::initialize();
  if ( !sc ) return sc;

  // This is a bit ugly. There is no way to declare a vector of DataObjectHandles, so
  // we need to wait until initialize when we've read in the input and output key
  // properties, and know their size, and then turn them
  // into Handles and register them with the framework by calling declareProperty. We
  // could call declareInput/declareOutput on them too.

  int i = 0;
  for ( auto k : m_inpKeys ) {
    DEBUG_MSG << "adding input key " << k << endmsg;
    m_inputHandles.push_back( new DataObjectHandle<DataObject>( k, Gaudi::DataHandle::Reader, this ) );
    declareProperty( "dummy_in_" + std::to_string( i ), *( m_inputHandles.back() ) );
    i++;
  }

  i = 0;
  for ( auto k : m_outKeys ) {
    DEBUG_MSG << "adding output key " << k << endmsg;
    m_outputHandles.push_back( new DataObjectHandle<DataObject>( k, Gaudi::DataHandle::Writer, this ) );
    declareProperty( "dummy_out_" + std::to_string( i ), *( m_outputHandles.back() ) );
    i++;
  }

  return sc;
}

//------------------------------------------------------------------------------

StatusCode ViewTester::execute() // the execution of the algorithm
{
  SmartIF<IScheduler> scheduler( serviceLocator()->service( "AvalancheSchedulerSvc" ) );
  if ( !scheduler ) {
    fatal() << "Unable to load AvalancheSchedulerSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  // Report if currently running in a view
  std::string const* contextName = getContext().getExtension<std::string>();
  if ( contextName == nullptr ) {
    info() << "Running in whole event context" << endmsg;
  } else {
    info() << "Running in view " << *contextName << endmsg;
  }

  // If a node name is specified (and not already in view), do view scheduling
  if ( m_viewNodeName != "" && contextName == nullptr ) {
    if ( m_viewNumber > 0 ) {
      // Make views
      for ( unsigned int viewIndex = 0; viewIndex < m_viewNumber; ++viewIndex ) {
        // Make event context for the view
        EventContext* viewContext = new EventContext( getContext().evt(), getContext().slot() );
        std::string   viewName    = m_baseViewName + std::to_string( viewIndex );
        viewContext->setExtension<std::string>( viewName );

        StatusCode sc = scheduler->scheduleEventView( &getContext(), m_viewNodeName, viewContext );
        if ( sc.isSuccess() ) {
          info() << "Attached " << viewName << " to node " << m_viewNodeName.toString() << endmsg;
        } else {
          error() << "Unable to attach " << viewName << " to node " << m_viewNodeName.toString() << endmsg;
        }
      }
    } else {
      // Disable the view node if there are no views
      scheduler->scheduleEventView( &getContext(), m_viewNodeName, 0 );
    }
  }

  VERBOSE_MSG << "outputs number: " << m_outputHandles.size() << endmsg;
  for ( auto& outputHandle : m_outputHandles ) {
    if ( !outputHandle->isValid() ) continue;

    VERBOSE_MSG << "put to TS: " << outputHandle->objKey() << endmsg;
    outputHandle->put( new DataObject() );
  }

  setFilterPassed( true );

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------

StatusCode ViewTester::finalize() // the finalization of the algorithm
{
  return GaudiAlgorithm::finalize();
}

//------------------------------------------------------------------------------
