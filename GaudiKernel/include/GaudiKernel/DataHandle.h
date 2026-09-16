/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/DataHandleProperty.h>
#include <GaudiKernel/DataObjID.h>
#include <GaudiKernel/System.h>

#include <span>
#include <sstream>
#include <tuple>
#include <vector>

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
        : m_key( std::move( k ) ), m_owner( owner ), m_mode( a ) {}

    DataHandle( DataObjID k, bool isCond, Mode a = Reader, IDataHandleHolder* owner = nullptr )
        : m_key( std::move( k ) ), m_owner( owner ), m_mode( a ), m_isCond( isCond ) {}

    using PropertyType = DataHandleProperty;

    virtual ~DataHandle() = default;

    virtual void               setOwner( IDataHandleHolder* o ) { m_owner = o; }
    virtual IDataHandleHolder* owner() const { return m_owner; }

    virtual Mode mode() const { return m_mode; }

    virtual void setKey( DataObjID key ) { m_key = std::move( key ); }
    virtual void updateKey( std::string key ) { m_key.updateKey( std::move( key ) ); }

    virtual const std::string& objKey() const { return m_key.key(); }
    virtual const DataObjID&   fullKey() const { return m_key; }

    virtual void reset( bool ) {}

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
    DataObjID          m_key   = { "NONE" };
    IDataHandleHolder* m_owner = nullptr;

    static const std::string default_type;

  private:
    Mode m_mode   = Reader;
    bool m_isCond = false;
  };

  /** Type-erased interface for a configurable sequence of data handles.
   *
   *  The associated property stores the TES keys. Updating it rebuilds
   *  the typed handles owned by the concrete DataHandleVector.
   */
  class DataHandleVectorBase {
  public:
    using PropertyType = DataHandleVectorProperty;

    virtual ~DataHandleVectorBase() = default;

    /// Replace the configured TES keys and rebuild the typed handles.
    virtual void setKeys( std::span<const DataObjID> keys ) = 0;
    /// Configured TES keys, in property order.
    virtual const std::vector<DataObjID>& keys() const = 0;
    /// Access mode shared by all handles in the vector.
    virtual DataHandle::Mode mode() const = 0;
    /// Demangled C++ type of the data accessed by each handle.
    virtual std::string type() const = 0;

    /// Python class used to represent the property value.
    std::string pythonPropertyClassName() const { return "DataHandleVector"; }
    /// GaudiConfig2 semantics name including handle mode and value type.
    std::string semantics() const;
    /// Python expression reproducing the configured value.
    std::string pythonRepr() const;
  };

  /** A configurable, dynamically sized sequence of homogeneous data handles.
   *
   *  Every contained handle is registered with the parent IDataHandleHolder,
   *  so schedulers see each configured location as an individual dependency.
   *
   *  @tparam Handle data-handle template, such as DataObjectReadHandle
   *  @tparam T data type accessed by every handle
   */
  template <template <typename> class Handle, typename T>
  class DataHandleVector : public DataHandleVectorBase {
  public:
    /** Declare a vector data-handle property on an owning component.
     *
     *  @param parent component which owns the property and contained handles
     *  @param name property name
     *  @param keys initial sequence of TES keys
     *  @param doc property documentation
     */
    template <typename Owner>
    DataHandleVector( Owner* parent, std::string name, const std::vector<std::string>& keys, std::string doc = "" )
        : m_owner{ parent } {
      std::vector<DataObjID> ids;
      ids.reserve( keys.size() );
      for ( const auto& key : keys ) ids.emplace_back( key );
      setKeys( ids );
      auto p = parent->declareProperty( std::move( name ), *this, std::move( doc ) );
      p->template setOwnerType<Owner>();
    }

    /** Construct from the property-name/default-value pair used by the
     *  Functional algorithm data-handle machinery.
     *
     *  @param parent component which owns the property and contained handles
     *  @param keys property name and initial sequence of TES keys
     */
    template <typename Owner>
    DataHandleVector( Owner* parent, const std::pair<std::string, std::vector<std::string>>& keys )
        : DataHandleVector{ parent, keys.first, keys.second } {}

    /** Adapt the owner/name-and-default tuple produced internally by
     *  DataHandleMixin to the public constructor.
     *
     *  @param keys owner followed by a property-name/default-value pair
     */
    template <typename Owner, typename Keys>
    DataHandleVector( std::tuple<Owner, Keys>&& keys ) : DataHandleVector{ std::get<0>( keys ), std::get<1>( keys ) } {}

    /** Adapt the owner/name/default tuple produced internally by
     *  DataHandleMixin to the public constructor.
     *
     *  @param keys owner, property name, and initial sequence of TES keys
     */
    template <typename Owner, typename Name, typename Keys>
    DataHandleVector( std::tuple<Owner, Name, Keys>&& keys )
        : DataHandleVector{ std::get<0>( keys ), std::get<1>( keys ), std::get<2>( keys ) } {}

    /// Replace the configured TES keys and their registered typed handles.
    void setKeys( std::span<const DataObjID> keys ) override {
      m_keys.assign( keys.begin(), keys.end() );
      m_handles.clear();
      m_handles.reserve( m_keys.size() );
      for ( const auto& key : m_keys ) m_handles.emplace_back( key, m_owner );
    }

    /// Remove all configured keys and unregister their typed handles.
    void clear() { setKeys( std::span<const DataObjID>{} ); }

    /// Configured TES keys, in property order.
    const std::vector<DataObjID>& keys() const override { return m_keys; }
    /// Typed handles, in property order.
    const std::vector<Handle<T>>& handles() const { return m_handles; }
    /// Configured TES key at @p i.
    const DataObjID& at( std::size_t i ) const { return m_keys.at( i ); }
    /// Number of configured handles.
    auto size() const { return m_handles.size(); }

    /// Access mode shared by every contained handle.
    DataHandle::Mode mode() const override { return Handle<T>::mode_v; }
    /// Demangled C++ type accessed by every contained handle.
    std::string type() const override { return System::typeinfoName( typeid( T ) ); }

  protected:
    IDataHandleHolder*     m_owner = nullptr;
    std::vector<DataObjID> m_keys;
    std::vector<Handle<T>> m_handles;
  };

  namespace Parsers {
    StatusCode parse( DataHandle&, std::string_view );
  }
  namespace Utils {
    GAUDI_API std::ostream& toStream( const DataHandle& v, std::ostream& o );
  }
} // namespace Gaudi
