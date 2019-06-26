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
