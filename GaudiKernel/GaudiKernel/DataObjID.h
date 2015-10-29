#ifndef GAUDIKERNEL_DATAOBJID
#define GAUDIKERNEL_DATAOBJID 1

#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/StatusCode.h"

#include <string>
#include <unordered_set>
#include <iostream>
#include <mutex>

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
 * allocator DataObjID_Hasher which the hash function
 *
 * @author Charles Leggett
 * @date   2015-09-01
 */

//---------------------------------------------------------------------------


class DataObjID_Hasher;
class IClassIDSvc;

class DataObjID {
public:
  friend DataObjID_Hasher;

  DataObjID();
  DataObjID(const std::string& key);
  DataObjID(const CLID& clid, const std::string& key);
  DataObjID(const std::string& className, const std::string& key);
  DataObjID(const DataObjID& d);

  // only return the last part of the key
  const std::string& key() const { return m_key; }

  // combination of the key and the ClassName, mostly for debugging
  std::string fullKey() const;

  CLID clid() const { return m_clid; }

  void updateKey(const std::string& key);

  friend std::ostream& operator<< (std::ostream& str, const DataObjID& d);

  bool operator< ( const DataObjID& other ) const {
    return (m_hash < other.m_hash);
  }

  bool operator==( const DataObjID& other ) const {
    return (m_hash == other.m_hash);
  }

  bool operator!=( const DataObjID& other ) const {
    return (m_hash != other.m_hash);
  }

private:

  void hashGen();
  void parse(const std::string& key);
  void setClid();
  void setClassName();

  CLID m_clid;
  std::size_t m_hash;

  std::string m_className;
  std::string m_key;

  static void getClidSvc();
  static IClassIDSvc* p_clidSvc;
  static std::once_flag m_ip;

};

inline DataObjID::DataObjID(): 
  m_clid(0), m_hash(0), m_className(""), m_key("INVALID") {}

inline DataObjID::DataObjID(const std::string& key): 
  m_clid(0), m_className(""), m_key(key) { 
  hashGen();

}

inline DataObjID::DataObjID(const CLID& clid, const std::string& key): 
  m_clid(clid), m_key(key) {
  setClassName();
  hashGen();
}

inline DataObjID::DataObjID(const std::string& className, const std::string& key): 
  m_className(className), m_key(key) {
  setClid();
  hashGen();
}


inline DataObjID::DataObjID(const DataObjID& d): 
  m_clid(d.m_clid), m_hash(d.m_hash), m_className(d.m_className), m_key(d.m_key) {}

inline void DataObjID::updateKey(const std::string& key) {
  m_key = key;
  hashGen();
}

class DataObjID_Hasher {
public:
  std::size_t operator()(const DataObjID& k) const {
    return (k.m_hash);
  }
};

typedef std::unordered_set<DataObjID, DataObjID_Hasher> DataObjIDColl;

namespace Gaudi { 
  namespace Parsers {
    StatusCode parse(DataObjID&, const std::string&);
    StatusCode parse(DataObjIDColl&, const std::string&);
  }
  namespace Utils {
    GAUDI_API std::ostream& toStream(const DataObjID& v, std::ostream& o);
    GAUDI_API std::ostream& toStream(const DataObjIDColl& v, std::ostream& o);
  }
}


#endif
