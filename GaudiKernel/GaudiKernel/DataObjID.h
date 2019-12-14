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
#ifndef GAUDIKERNEL_DATAOBJID
#define GAUDIKERNEL_DATAOBJID 1

#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/StatusCode.h"

#include <iostream>
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

  DataObjID()                   = default;
  DataObjID( const DataObjID& ) = default;
  DataObjID( const std::string& key );
  DataObjID( const CLID& clid, const std::string& key );
  DataObjID( const std::string& className, const std::string& key );

  // only return the last part of the key
  const std::string& key() const { return m_key; }

  // combination of the key and the ClassName, mostly for debugging
  std::string fullKey() const;

  CLID clid() const { return m_clid; }

  void updateKey( const std::string& key );

  friend bool operator<( const DataObjID& lhs, const DataObjID& rhs ) { return lhs.m_hash < rhs.m_hash; }
  friend bool operator==( const DataObjID& lhs, const DataObjID& rhs ) { return lhs.m_hash == rhs.m_hash; }
  friend bool operator!=( const DataObjID& lhs, const DataObjID& rhs ) { return !( lhs == rhs ); }

  friend StatusCode    parse( DataObjID& dest, const std::string& src );
  friend std::ostream& toStream( const DataObjID& v, std::ostream& o );
  friend std::ostream& operator<<( std::ostream& os, const DataObjID& d ) { return toStream( d, os ); }

private:
  void hashGen();
  void setClid();
  void setClassName();

  CLID        m_clid{0};
  std::size_t m_hash{0};

  std::string m_key{"INVALID"};
  std::string m_className;

  static IClassIDSvc*   p_clidSvc;
  static std::once_flag m_ip;
};

inline DataObjID::DataObjID( const std::string& key ) : m_key( key ) { hashGen(); }

inline DataObjID::DataObjID( const CLID& clid, const std::string& key ) : m_clid( clid ), m_key( key ) {
  setClassName();
  hashGen();
}

inline DataObjID::DataObjID( const std::string& className, const std::string& key )
    : m_key( key ), m_className( className ) {
  setClid();
  hashGen();
}

inline void DataObjID::updateKey( const std::string& key ) {
  m_key = key;
  hashGen();
}

struct DataObjID_Hasher {
  std::size_t operator()( const DataObjID& k ) const { return k.m_hash; }
};

using DataObjIDColl = std::unordered_set<DataObjID, DataObjID_Hasher>;

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
    } // namespace Property
  }   // namespace Details
} // namespace Gaudi

#endif
