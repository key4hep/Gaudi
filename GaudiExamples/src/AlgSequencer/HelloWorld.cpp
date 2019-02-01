// Include files
#include "HelloWorld.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"

// Static Factory declaration

DECLARE_COMPONENT( HelloWorld )

// Constructor
//------------------------------------------------------------------------------
HelloWorld::HelloWorld( const std::string& name, ISvcLocator* ploc ) : Algorithm( name, ploc ) {
  //------------------------------------------------------------------------------
  m_initialized = false;
}

//------------------------------------------------------------------------------
StatusCode HelloWorld::initialize() {
  //------------------------------------------------------------------------------
  // avoid calling initialize more than once
  if ( m_initialized ) return StatusCode::SUCCESS;

  info() << "initializing...." << endmsg;

  Gaudi::Property<int>         m_int;
  Gaudi::Property<double>      m_double;
  Gaudi::Property<std::string> m_string;

  declareInfo( "int_value", t_int, "description" );
  declareInfo( "double_value", t_double, "description" );
  declareInfo( "string_value", t_string, "description" );

  m_initialized = true;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode HelloWorld::execute() {
  //------------------------------------------------------------------------------
  info() << "executing...." << endmsg;

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode HelloWorld::finalize() {
  //------------------------------------------------------------------------------
  info() << "finalizing...." << endmsg;

  m_initialized = false;
  return StatusCode::SUCCESS;
}
