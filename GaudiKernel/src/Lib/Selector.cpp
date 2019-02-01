//====================================================================
//  NTuple name space: Selector class implementation
//--------------------------------------------------------------------
//
//  Package    : Gaudi/NTupleSvc ( The LHCb Offline System)
//  Author     : M.Frank
//
//  +---------+----------------------------------------------+--------+
//  |    Date |                 Comment                      | Who    |
//  +---------+----------------------------------------------+--------+
//  | 10/10/00| Initial version.                             | MF     |
//  +---------+----------------------------------------------+--------+
//====================================================================
#define GAUDI_NTUPLEITEMS_CPP 1

#include "GaudiKernel/Selector.h"
#include "GaudiKernel/NTuple.h"

/// Selector Initialisation
StatusCode NTuple::Selector::initialize( NTuple::Tuple* /* nt */ ) { return StatusCode::SUCCESS; }

/// Overloaded callback from SelectStatement
bool NTuple::Selector::operator()( void* nt ) {
  DataObject* p      = static_cast<DataObject*>( nt );
  bool        result = false;
  try {
    NTuple::Tuple* tuple = dynamic_cast<NTuple::Tuple*>( p );
    if ( tuple ) {
      if ( m_firstCall ) {
        m_status    = initialize( tuple );
        m_firstCall = false;
      }
      if ( m_status.isSuccess() ) { result = this->operator()( tuple ); }
    }
  } catch ( ... ) {}
  return result;
}

/// Specialized overload for N-tuples
bool NTuple::Selector::operator()( NTuple::Tuple* /* nt */ ) { return true; }
