/***********************************************************************************\
* (c) Copyright 1998-2021 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_DATASTOREITEM_H
#define GAUDIKERNEL_DATASTOREITEM_H

// STL include files
#include <string>

/** @class DataStoreItem DataStoreItem.h GaudiKernel/DataStoreItem.h

    Description of the DataStoreItem class.
    DataStoreItems describe in an abstract way an item in the data
    store. Items can be supplied to the services in order to
    e.g. trigger preloading or writing.

    @author M.Frank
    @version 1.0
*/
class DataStoreItem {
protected:
  /// Path of item to be loaded
  std::string m_path;
  /// Depth to be auto-loaded from the requested path onwards
  int m_depth;

public:
  /// Standard Constructor
  DataStoreItem( std::string path, int depth = 1 ) : m_path( std::move( path ) ), m_depth( depth ) { analyse(); }
  /// Copy constructor
  DataStoreItem( const DataStoreItem& item ) : m_path( item.m_path ), m_depth( item.m_depth ) { analyse(); }
  /// Standard Destructor
  virtual ~DataStoreItem() = default;

  /// Equality operator
  bool operator==( const DataStoreItem& cmp ) const { return m_path == cmp.path() && m_depth == cmp.depth(); }
  /// Inequality operator
  bool operator!=( const DataStoreItem& cmp ) const { return !( m_path == cmp.path() && m_depth == cmp.depth() ); }
  /// Equivalence operator
  DataStoreItem& operator=( const DataStoreItem& cmp ) {
    m_path  = cmp.path();
    m_depth = cmp.depth();
    analyse();
    return *this;
  }
  /// Interprete the load path for special options
  void analyse() {
    if ( m_path.empty() ) return;
    const size_t len = m_path.length() - 1;
    if ( m_path[len] == '*' ) {
      m_depth = 99999999;
      ( len > 0 && m_path[len - 1] == '/' ) ? m_path.erase( len - 1, 2 ) : m_path.erase( len, 1 );
    } else if ( m_path[len] == '+' ) {
      ( len > 0 && m_path[len - 1] == '/' ) ? m_path.erase( len - 1, 2 ) : m_path.erase( len, 1 );
      m_depth = 2;
    }
  }
  /// Accessor: Retrieve load path
  const std::string& path() const { return m_path; }
  /// Accessor: Retrieve load depth
  int depth() const { return m_depth; }
};
#endif // GAUDIKERNEL_DATASTOREITEM_H
