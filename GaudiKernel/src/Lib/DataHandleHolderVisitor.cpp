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
#include "GaudiKernel/DataHandleHolderVisitor.h"
#include "GaudiKernel/DataHandle.h"
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/IDataHandleHolder.h"
#include <functional>
#include <tuple>
#include <typeinfo>

namespace {
  using std::make_tuple;
}

DHHVisitor::DHHVisitor( DataObjIDColl& ido, DataObjIDColl& odo ) : m_ido( ido ), m_odo( odo ) {}

void DHHVisitor::visit( const IDataHandleHolder* v ) {
  if ( !v ) { return; }

  // Loop over inputs and outputs of handles, extra dependiencies and objects to
  // collect all of them.
  // Handles and extra dependencies are those of a specific algorith or tool, while
  // the collection of data objects also contains those of the tree of tools and
  // algorithms below it.

  // Loop over input handles and output handles and store those with with non-empty keys
  // in the container of input objects passed to us and the others in a container of those
  // we ignore for debug purposes. To avoid multiple for loops, make some tuples and loop
  // over those.
  for ( auto& hs : { make_tuple( v->inputHandles(), std::ref( m_ido ), std::ref( m_ign_i ) ),
                     make_tuple( v->outputHandles(), std::ref( m_odo ), std::ref( m_ign_o ) ) } ) {
    for ( const auto& h : std::get<0>( hs ) ) {
      if ( h->mode() == Gaudi::DataHandle::Mode::Reader ) {
        m_src_i[h->fullKey()].emplace( v );
      } else {
        m_src_i[h->fullKey()].emplace( v );
      }
      if ( !h->objKey().empty() ) {
        std::get<1>( hs ).emplace( h->fullKey() );
      } else {
        std::get<2>( hs ).emplace( h->fullKey() );
      }
      m_all.push_back( std::make_pair( h->fullKey(), h->owner() ) );
    }
  }

  for ( auto& id : v->extraInputDeps() ) { m_src_i[id].emplace( v ); }
  for ( auto& id : v->extraOutputDeps() ) { m_src_o[id].emplace( v ); }

  // The containers of handles are a different type than the on of input deps and input
  // objects, so we need another loop here.
  // NOTE: perhaps a view from range v3 can be used to also avoid this second loop.
  for ( auto& hs : { make_tuple( v->extraInputDeps(), std::ref( m_ido ), std::ref( m_ign_i ) ),
                     make_tuple( v->extraOutputDeps(), std::ref( m_odo ), std::ref( m_ign_o ) ),
                     make_tuple( v->inputDataObjs(), std::ref( m_ido ), std::ref( m_ign_i ) ),
                     make_tuple( v->outputDataObjs(), std::ref( m_odo ), std::ref( m_ign_o ) ) } ) {
    for ( const auto& h : std::get<0>( hs ) ) {
      if ( !h.key().empty() ) {
        std::get<1>( hs ).emplace( h );
      } else {
        std::get<2>( hs ).emplace( h );
      }
      m_all.push_back( std::make_pair( h, v ) );
    }
  }
}
