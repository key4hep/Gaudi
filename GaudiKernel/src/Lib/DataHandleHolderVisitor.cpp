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
#include <GaudiKernel/DataHandle.h>
#include <GaudiKernel/DataHandleHolderVisitor.h>
#include <GaudiKernel/DataObjID.h>
#include <GaudiKernel/IDataHandleHolder.h>
#include <GaudiKernel/SerializeSTL.h>
#include <functional>
#include <tuple>
#include <typeinfo>

namespace {
  using std::make_tuple;
}

DHHVisitor::DHHVisitor( DataObjIDColl& ido, DataObjIDColl& odo ) : m_ido( ido ), m_odo( odo ) {}

void DHHVisitor::visit( const IDataHandleHolder* v ) {
  if ( !v ) { return; }

  // Record the name of the first IDataHandleHolder we encounter, for cleaner report
  if ( m_initialName.empty() ) m_initialName = v->name();

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
      if ( !h->objKey().empty() ) {
        std::get<1>( hs ).emplace( h->fullKey() );
      } else {
        std::get<2>( hs ).emplace( h->fullKey() );
      }
      m_owners[h->fullKey()].emplace( v );
    }
  }

  for ( auto& id : v->extraInputDeps() ) { m_owners[id].emplace( v ); }
  for ( auto& id : v->extraOutputDeps() ) { m_owners[id].emplace( v ); }

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
      m_owners[h].emplace( v );
    }
  }
}

std::vector<const IDataHandleHolder*> DHHVisitor::owners_of( const DataObjID& id ) const {
  if ( auto item = m_owners.find( id ); item != m_owners.end() ) {
    return { item->second.begin(), item->second.end() };
  }
  return {};
};

std::vector<std::string> DHHVisitor::owners_names_of( const DataObjID& id, bool with_main ) const {
  std::vector<std::string> tmp;
  for ( auto owner : owners_of( id ) ) {
    if ( with_main || owner->name() != m_initialName ) tmp.push_back( owner->name() );
  }
  if ( !tmp.empty() ) { std::sort( tmp.begin(), tmp.end() ); }
  return tmp;
};

MsgStream& DHHVisitor::report( MsgStream& stream ) const {
  // sort DataObjects by path so that logging is reproducible
  // we define a little helper creating an ordered set from a non ordered one
  auto sort     = []( const DataObjID& a, const DataObjID& b ) -> bool { return a.fullKey() < b.fullKey(); };
  auto orderset = [&sort]( const DataObjIDColl& in ) -> std::set<DataObjID, decltype( sort )> {
    return { in.begin(), in.end(), sort };
  };
  auto write_owners_of = [this]( auto& stream, const DataObjID& id ) {
    auto tmp = owners_names_of( id );
    if ( !tmp.empty() ) { stream << ' ' << tmp; }
  };

  for ( auto h : orderset( m_ido ) ) {
    stream << "\n  + INPUT  " << h;
    write_owners_of( stream, h );
  }
  for ( auto id : orderset( m_ign_i ) ) {
    stream << "\n  + INPUT IGNORED " << id;
    write_owners_of( stream, id );
  }
  for ( auto h : orderset( m_odo ) ) {
    stream << "\n  + OUTPUT " << h;
    write_owners_of( stream, h );
  }
  for ( auto id : orderset( m_ign_o ) ) {
    stream << "\n  + OUTPUT IGNORED " << id;
    write_owners_of( stream, id );
  }
  return stream;
}

bool DHHVisitor::empty() const {
  // any data handle would have as side effect to add something to m_owners,
  // so it's enough to check this instead of all the other containers
  return m_owners.empty();
}
