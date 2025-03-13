/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_DATAOBJID
#define GAUDIKERNEL_DATAOBJID 1

#include <GaudiKernel/ClassID.h>
#include <GaudiKernel/StatusCode.h>

#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>

//---------------------------------------------------------------------------

/** DataObjID.h GaudiKernel/DataObjID.h
 *
 * Class used to identify an object in the Data Store, with fast lookup
 * using an hash
 *
 * Objects are identified via either a Gaudi style '/path/to/object' key,
 * or ATLAS style (ClassID, 'key') of ('ClassName', 'key')
 *
 * depending on the style, the hash is either a std::hash<string> of the path,
 * or a combination of the std::hash<string> of the key and the ClassID
 *
 * Collections of DataObjIDs are std::unordered_set<DataObjID> with a provided
 * hash function DataObjID_Hasher
 *
 * @author Charles Leggett
 * @date   2015-09-01
 */

//---------------------------------------------------------------------------

struct DataObjID_Hasher;
class IClassIDSvc;

class DataObjID {
public:
  friend DataObjID_Hasher;

  DataObjID() = default;
  DataObjID( const DataObjID& other )
      : m_clid( other.m_clid ), m_hash( other.m_hash ), m_key( other.m_key ), m_className( other.m_className ) {}

  DataObjID( std::string key );
  DataObjID( const CLID& clid, std::string key );
  DataObjID( std::string className, std::string key );

  DataObjID& operator=( const DataObjID& other ) {
    m_clid      = other.m_clid;
    m_hash      = other.m_hash;
    m_key       = other.m_key;
    m_className = other.m_className;
    return *this;
  }

  /// only return the last part of the key
  const std::string& key() const { return m_key; }

  /// return the ClassName (if available)
  const std::string& className() const;

  /// combination of the key and the ClassName, mostly for debugging
  std::string fullKey() const;

  CLID        clid() const { return m_clid; }
  std::size_t hash() const { return m_hash; }

  void updateKey( std::string key );

  friend bool operator<( const DataObjID& lhs, const DataObjID& rhs ) { return lhs.m_hash < rhs.m_hash; }
  friend bool operator==( const DataObjID& lhs, const DataObjID& rhs ) { return lhs.m_hash == rhs.m_hash; }
  friend bool operator!=( const DataObjID& lhs, const DataObjID& rhs ) { return !( lhs == rhs ); }

  friend StatusCode    parse( DataObjID& dest, std::string_view src );
  friend std::ostream& toStream( const DataObjID& v, std::ostream& o );
  friend std::ostream& operator<<( std::ostream& os, const DataObjID& d ) { return toStream( d, os ); }

private:
  void hashGen();
  void setClid();

  CLID        m_clid{ 0 };
  std::size_t m_hash{ 0 };

  std::string            m_key{ "INVALID" };
  mutable std::string    m_className;
  mutable std::once_flag m_setClassName;
};

inline DataObjID::DataObjID( std::string key ) : m_key( std::move( key ) ) { hashGen(); }

inline DataObjID::DataObjID( const CLID& clid, std::string key ) : m_clid( clid ), m_key( std::move( key ) ) {
  hashGen();
}

inline DataObjID::DataObjID( std::string className, std::string key )
    : m_key( std::move( key ) ), m_className( std::move( className ) ) {
  setClid();
  hashGen();
}

inline void DataObjID::updateKey( std::string key ) {
  m_key = std::move( key );
  hashGen();
}

struct DataObjID_Hasher {
  std::size_t operator()( const DataObjID& k ) const { return k.m_hash; }
};

using DataObjIDColl   = std::unordered_set<DataObjID, DataObjID_Hasher>;
using DataObjIDVector = std::vector<DataObjID>;

namespace Gaudi {
  namespace Details {
    namespace Property {
      template <typename T>
      struct StringConverter;

      template <>
      struct StringConverter<DataObjIDColl> {
        std::string   toString( const DataObjIDColl& v );
        DataObjIDColl fromString( const DataObjIDColl&, const std::string& );
      };

      template <>
      struct StringConverter<DataObjIDVector> {
        std::string     toString( const DataObjIDVector& v );
        DataObjIDVector fromString( const DataObjIDVector&, const std::string& );
      };
    } // namespace Property
  }   // namespace Details
} // namespace Gaudi

#endif
