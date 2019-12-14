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
#include "GaudiKernel/DataHandleFinder.h"

DataHandleFinder::DataHandleFinder( DataObjID const& target ) : m_target( target ) {}

void DataHandleFinder::visit( const IDataHandleHolder* visitee ) {
  if ( !visitee ) { return; }

  // See if the handle holder we're visiting contains the target
  for ( auto& handle : visitee->inputHandles() ) {
    if ( handle->fullKey() == m_target ) {
      m_holderNames.push_back( visitee->name() );
      return;
    }
  }

  for ( auto& handle : visitee->extraInputDeps() ) {
    if ( handle == m_target ) {
      m_holderNames.push_back( visitee->name() );
      return;
    }
  }

  for ( auto& handle : visitee->inputDataObjs() ) {
    if ( handle == m_target ) {
      m_holderNames.push_back( visitee->name() );
      return;
    }
  }
}
