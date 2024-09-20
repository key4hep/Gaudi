/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
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

#include <GaudiKernel/NTuple.h>
#include <GaudiKernel/Selector.h>

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
