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
