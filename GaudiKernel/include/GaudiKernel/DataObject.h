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
#ifndef GAUDIKERNEL_DATAOBJECT_H
#define GAUDIKERNEL_DATAOBJECT_H

// Framework include files
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/StatusCode.h"

// STL includes
#include <memory>
#include <ostream>
#include <string>

// Forward declarations
class IOpaqueAddress;
class StreamBuffer;
class LinkManager;
class IRegistry;

// Definition of the CLID for this class
static const CLID CLID_DataObject = 1;

/** @class DataObject DataObject.h GaudiKernel/DataObject.h

    A DataObject is the base class of any identifiable object
    on any data store.
    The base class supplies the implementation of data streaming.

    @author M.Frank
*/
class GAUDI_API DataObject {
private:
  /// Reference count
  unsigned long m_refCount = 0;
  /// Version number
  unsigned char m_version = 0;
  /// Pointer to the Registry Object
  IRegistry* m_pRegistry = nullptr;
  /// Store of symbolic links
  std::unique_ptr<LinkManager> m_pLinkMgr;

public:
  /// Standard Constructor
  DataObject();
  /// Copy Constructor
  DataObject( const DataObject& rhs );
  /// Assignment Operator
  DataObject& operator=( const DataObject& rhs );
  /// Move Constructor
  DataObject( DataObject&& rhs );
  /// Move Assignment Operator
  DataObject& operator=( DataObject&& rhs );
  /// Standard Destructor
  virtual ~DataObject();
  /// Add reference to object
  virtual unsigned long addRef();
  /// release reference to object
  virtual unsigned long release();
  /// Retrieve reference to class definition structure
  virtual const CLID& clID() const;
  /// Retrieve reference to class definition structure (static access)
  static const CLID& classID();
  /// Retreive DataObject name. It is the name when registered in the store.
  const std::string& name() const;

  /// Provide empty placeholder for internal object reconfiguration callback
  virtual StatusCode update();

  /**@name inline code of class DataObject    */
  /// Set pointer to Registry
  void setRegistry( IRegistry* pRegistry ) { m_pRegistry = pRegistry; }
  /// Get pointer to Registry
  IRegistry* registry() const { return m_pRegistry; }
  /// Retrieve Link manager
  LinkManager*       linkMgr() { return m_pLinkMgr.get(); }
  const LinkManager* linkMgr() const { return m_pLinkMgr.get(); }
  /// Retrieve version number of this object representation
  unsigned char version() const { return m_version; }
  /// Set version number of this object representation
  void setVersion( unsigned char vsn ) { m_version = vsn; }
  /// Return the refcount
  unsigned long refCount() const { return m_refCount; }
  /// Fill the output stream (ASCII)
  virtual std::ostream& fillStream( std::ostream& s ) const {
    s << "DataObject at " << std::hex << this;
    return s;
  }
  /// Output operator (ASCII)
  friend std::ostream& operator<<( std::ostream& s, const DataObject& obj ) { return obj.fillStream( s ); }
};

// Additional functions to support the Serialization of objects in the transient store

namespace Gaudi {
  GAUDI_API void        pushCurrentDataObject( DataObject** pobjAddr );
  GAUDI_API void        popCurrentDataObject();
  GAUDI_API DataObject* getCurrentDataObject();
} // namespace Gaudi

#endif // GAUDIKERNEL_DATAOBJECT_H
