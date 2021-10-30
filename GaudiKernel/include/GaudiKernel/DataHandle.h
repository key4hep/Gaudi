/***********************************************************************************\
* (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_DATAHANDLE
#define GAUDIKERNEL_DATAHANDLE 1

#include "GaudiKernel/DataHandleProperty.h"
#include "GaudiKernel/DataObjID.h"

//---------------------------------------------------------------------------

/** DataHandle.h  GaudiKernel/DataHandle.h
 *
 * Base class for Handles to access data in Event Store, shared between
 * Gaudi and Atlas
 *
 * Objects are identified via a DataObjID
 *
 * Once object is created, Mode is not modifiable
 *
 * @author Charles Leggett
 * @date   2015-09-01
 */

//---------------------------------------------------------------------------

class IDataHandleHolder;

namespace Gaudi {

  class DataHandle {
  public:
    enum Mode { Reader = 1 << 2, Writer = 1 << 4 };

    DataHandle( DataObjID k, Mode a = Reader, IDataHandleHolder* owner = nullptr )
        : m_key( std::move( k ) ), m_owner( owner ), m_mode( a ){};

    DataHandle( DataObjID k, bool isCond, Mode a = Reader, IDataHandleHolder* owner = nullptr )
        : m_key( std::move( k ) ), m_owner( owner ), m_mode( a ), m_isCond( isCond ){};

    using PropertyType = DataHandleProperty;

    virtual ~DataHandle() = default;

    virtual void               setOwner( IDataHandleHolder* o ) { m_owner = o; }
    virtual IDataHandleHolder* owner() const { return m_owner; }

    virtual Mode mode() const { return m_mode; }

    virtual void setKey( DataObjID key ) const { m_key = std::move( key ); }
    virtual void updateKey( std::string key ) const { m_key.updateKey( std::move( key ) ); }

    virtual const std::string& objKey() const { return m_key.key(); }
    virtual const DataObjID&   fullKey() const { return m_key; }

    virtual void reset( bool ){};

    virtual std::string pythonRepr() const;
    virtual bool        init() { return true; }

    // is this a ConditionHandle?
    virtual bool isCondition() const { return m_isCond; }

    friend std::ostream& operator<<( std::ostream& str, const DataHandle& d );

  protected:
    /**
     * The key of the object behind this DataHandle
     * Although it may look strange to have it mutable, this can actually
     * change in case the object had alternative names, and it should not
     * be visible to the end user, for which the Handle is still the same
     */
    mutable DataObjID  m_key   = { "NONE" };
    IDataHandleHolder* m_owner = nullptr;

    static const std::string default_type;

  private:
    Mode m_mode   = Reader;
    bool m_isCond = false;
  };

  namespace Parsers {
    StatusCode parse( DataHandle&, const std::string& );
  }
  namespace Utils {
    GAUDI_API std::ostream& toStream( const DataHandle& v, std::ostream& o );
  }
} // namespace Gaudi

#endif
