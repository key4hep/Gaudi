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
#ifndef GAUDIKERNEL_LINKMANAGER_H
#define GAUDIKERNEL_LINKMANAGER_H

// STL includes
#include <memory>
#include <string>
#include <vector>
// Gaudi
#include "GaudiKernel/Kernel.h"

class DataObject;
class IOpaqueAddress;

/** @class LinkManager LinkManager.h GaudiKernel/LinkManager.h
 *
 *  A LinkManager is the object aggregated into a DataObject,
 *  which is responsible for the handling of non-tree like links.
 *
 *  @author M.Frank
 */
class GAUDI_API LinkManager final {

public:
  /// Directory link types
  enum DirLinkType { INVALID, VALID };

  /** Embedded class defining a symbolic link
   * Note: No explicit copy constructor; implicit compiler generated one
   *       is just fine.
   */
  class Link final {
    /// String containing path of symbolic link
    std::string m_path;
    /// Pointer to object behind the link
    DataObject* m_pObject = nullptr;
    /// Link ID
    long m_id = INVALID;

  public:
    /// Standard constructor
    Link( long id, std::string path, DataObject* pObject = nullptr )
        : m_path( std::move( path ) ), m_pObject( pObject ), m_id( id ) {}
    /// Standard constructor for Root I/O
    Link() = default;
    /// Equality operator: check paths only
    bool operator==( const Link& link ) const { return link.m_path == m_path; }
    /// Update object pointer
    void setObject( const DataObject* pObject ) { m_pObject = const_cast<DataObject*>( pObject ); }
    /// Const access to data object
    const DataObject* object() const { return m_pObject; }
    DataObject*       object() { return m_pObject; }
    /// Access to path of object
    const std::string& path() const { return m_path; }
    /// Link identifier
    long ID() const { return m_id; }
    /// Access to the object's address
    IOpaqueAddress* address();
  };

private:
  ///@ TODO: replace by std::vector<std::unique_ptr<Link>> once
  ///        ROOT does 'automatic' schema conversion from T* to
  ///        std::unique_ptr<T>...
  ///        Or, even better, just std::vector<Link>, given that
  ///        Link is barely larger than a pointer (40 vs. 8 bytes)
  ///        -- but that requires more invasive schema evolution.
  ///
  /// The vector containing all links which are non-tree like
  std::vector<Link*> m_linkVector;

public:
  /// Standard Constructor
  LinkManager()                = default;
  LinkManager( LinkManager&& ) = default;
  LinkManager& operator=( LinkManager&& ) = default;
  LinkManager( LinkManager const& )       = delete;
  LinkManager& operator=( LinkManager const& ) = delete;
  /// Standard Destructor
  ~LinkManager();
  /// Retrieve number of link present
  long size() const { return m_linkVector.size(); }
  bool empty() const { return m_linkVector.empty(); }
  /// Retrieve symbolic link identified by ID
  const Link* link( long id ) const;
  Link*       link( long id );
  /// Retrieve symbolic link identified by object
  const Link* link( const DataObject* pObject ) const;
  Link*       link( const DataObject* pObject );
  /// Retrieve symbolic link identified by path
  const Link* link( std::string_view path ) const;
  Link*       link( std::string_view path );
  /// Add link by object reference and path
  long addLink( const std::string& path, const DataObject* pObject );

  struct Sentinel {};
  Sentinel end() const { return {}; }
  auto     begin() const {
    class Iterator {
      int                i;
      LinkManager const* parent;

    public:
      Iterator( LinkManager const* p, int i ) : i{ i }, parent{ p } {}
      bool      operator==( Sentinel ) const { return i == parent->size(); }
      bool      operator!=( Sentinel ) const { return !( *this == Sentinel{} ); }
      Iterator& operator++() {
        ++i;
        return *this;
      }
      const Link& operator*() const { return *parent->link( i ); }
    };
    return Iterator{ this, 0 };
  }
};
#endif // GAUDIKERNEL_LINKMANAGER_H
