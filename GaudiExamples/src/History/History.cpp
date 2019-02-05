#include "History.h"

// Static Factory declaration
DECLARE_COMPONENT( History )

///////////////////////////////////////////////////////////////////////////

History::History( const std::string& name, ISvcLocator* pSvcLocator ) : Algorithm( name, pSvcLocator ) {}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode History::initialize() {

  m_histSvc = service( "HistorySvc" );
  if ( !m_histSvc ) {
    error() << "Could not get historySvc" << endmsg;
    return StatusCode::FAILURE;
  }
  info() << "got historySvc: " << m_histSvc->name() << endmsg;
  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode History::execute() { return StatusCode::SUCCESS; }
