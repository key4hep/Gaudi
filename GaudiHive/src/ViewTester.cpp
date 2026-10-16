/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

// Framework includes
#include <GaudiKernel/Algorithm.h>
#include <GaudiKernel/DataObjectHandle.h>
#include <GaudiKernel/IScheduler.h>
#include <GaudiKernel/RegistryEntry.h>
#include <GaudiKernel/RndmGenerators.h>

// STL includes
#include <memory>
#include <string>
#include <vector>

//------------------------------------------------------------------------------

/** @class ViewTester
 *
 *
 */
namespace Test {

  class ViewTester : public Algorithm {

  public:
    bool isClonable() const override { return true; }

    /// the execution of the algorithm
    StatusCode execute() override;
    /// Its initialization
    StatusCode initialize() override;

    ViewTester( const std::string& name, // the algorithm instance name
                ISvcLocator*       pSvc );     // the Service Locator

  private:
    /// the default constructor is disabled
    ViewTester(); // no default constructor
    /// the copy constructor is disabled
    ViewTester( const ViewTester& ); // no copy constructor
    /// the assignement operator is disabled
    ViewTester& operator=( const ViewTester& ); // no assignement

    Gaudi::Property<std::vector<std::string>>                  m_inpKeys{ this, "inpKeys", {}, "" };
    Gaudi::Property<std::vector<std::string>>                  m_outKeys{ this, "outKeys", {}, "" };
    std::vector<std::unique_ptr<DataObjectHandle<DataObject>>> m_inputHandles;
    std::vector<std::unique_ptr<DataObjectHandle<DataObject>>> m_outputHandles;

    // View config
    Gaudi::Property<std::string>  m_baseViewName{ this, "baseViewName", "view",
                                                 "Views to be named this, plus a numerical index" };
    Gaudi::Property<unsigned int> m_viewNumber{ this, "viewNumber", 0, "How many views to make" };
    Gaudi::Property<std::string>  m_viewNodeName{ this, "viewNodeName", "viewNode", "Name of node to attach views to" };
  };
} // namespace Test
#include <GaudiKernel/ThreadLocalContext.h>

DECLARE_COMPONENT( Test::ViewTester )

#define ON_DEBUG if ( msgLevel( MSG::DEBUG ) )
#define DEBUG_MSG ON_DEBUG debug()

#define ON_VERBOSE if ( msgLevel( MSG::VERBOSE ) )
#define VERBOSE_MSG ON_VERBOSE verbose()

using namespace Test;

//------------------------------------------------------------------------------

ViewTester::ViewTester( const std::string& name, // the algorithm instance name
                        ISvcLocator*       pSvc )
    : Algorithm( name, pSvc ) {}

StatusCode ViewTester::initialize() {
  auto sc = Algorithm::initialize();
  if ( !sc ) return sc;

  // This is a bit ugly. There is no way to declare a vector of DataObjectHandles, so
  // we need to wait until initialize when we've read in the input and output key
  // properties, and know their size, and then turn them
  // into Handles and register them with the framework by calling declareProperty. We
  // could call declareInput/declareOutput on them too.

  int i = 0;
  for ( auto k : m_inpKeys ) {
    DEBUG_MSG << "adding input key " << k << endmsg;
    m_inputHandles.emplace_back( std::make_unique<DataObjectHandle<DataObject>>( k, Gaudi::DataHandle::Reader, this ) );
    declareProperty( "dummy_in_" + std::to_string( i ), *( m_inputHandles.back() ) );
    i++;
  }

  i = 0;
  for ( auto k : m_outKeys ) {
    DEBUG_MSG << "adding output key " << k << endmsg;
    m_outputHandles.emplace_back(
        std::make_unique<DataObjectHandle<DataObject>>( k, Gaudi::DataHandle::Writer, this ) );
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

  const auto& context = getContext();

  // Report if currently running in a view
  if ( !context.hasExtension<std::string>() )
    info() << "Running in whole event context" << endmsg;
  else
    info() << "Running in view " << context.getExtension<std::string>() << endmsg;

  // If a node name is specified (and not already in view), do view scheduling
  if ( !m_viewNodeName.empty() && !context.hasExtension<std::string>() ) {
    if ( m_viewNumber > 0 ) {
      // Make views
      for ( unsigned int viewIndex = 0; viewIndex < m_viewNumber; ++viewIndex ) {
        // Make event context for the view
        auto               viewContext = std::make_unique<EventContext>( context.evt(), context.slot() );
        const std::string& viewName    = viewContext->setExtension( m_baseViewName + std::to_string( viewIndex ) );

        StatusCode sc = scheduler->scheduleEventView( &context, m_viewNodeName, std::move( viewContext ) );
        if ( sc.isSuccess() )
          info() << "Attached view " << viewName << " to node " << m_viewNodeName.value() << " for " << context
                 << endmsg;
        else
          error() << "Unable to attach view " << viewName << " to node " << m_viewNodeName.value() << " for " << context
                  << endmsg;
      }
    } else {
      // Disable the view node if there are no views
      scheduler->scheduleEventView( &context, m_viewNodeName, nullptr ).ignore();
    }
  }

  setFilterPassed( true );

  return StatusCode::SUCCESS;
}
