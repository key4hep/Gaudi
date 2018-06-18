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
class GAUDI_API LinkManager
{

public:
  /// Directory link types
  enum DirLinkType { INVALID, VALID };

  /** Embedded class defining a symbolic link
    * Note: No copy constructor; bitwise copy (done by the compiler)
    *       is just fine.
    */
  class Link final
  {
    /// DataObject is a friend
    // friend class LinkManager;
    /// String containing path of symbolic link
    std::string m_path;
    /// Pointer to object behind the link
    DataObject* m_pObject = nullptr;
    /// Link ID
    long m_id = INVALID;

  public:
    /// Standard constructor
    Link( long id, std::string path, DataObject* pObject = nullptr )
        : m_path( std::move( path ) ), m_pObject( pObject ), m_id( id )
    {
    }
    /// Standard constructor
    Link() = default;
    /// Update the link content
    void set( long id, std::string path, DataObject* pObject )
    {
      setObject( pObject );
      m_path = std::move( path );
      m_id   = id;
    }
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
  ///@ TODO: provide interface to let MergeEvntAlg do its thing
  //         without the need for friendship. It needs to 'shunt'
  //         all contained links to a new prefix...
  //         The steps to get there are
  //           1) provide friend access for backwards compatibility
  //           2) add new interface here
  //           3) once released, update MergeEventAlg to use the new
  //              interface
  //           4) revoke friendship.
  //         Now we're at stage 1...
  friend class MergeEventAlg;
  ///@ TODO: replace by std::vector<std::unique_ptr<Link>> once
  ///        ROOT does 'automatic' schema conversion from T* to
  ///        std::unique_ptr<T>...
  ///        Or, even better, just std::vector<Link>, given that
  ///        Link is barely larger than a pointer (40 vs. 8 bytes)
  ///        -- but that requires more invasive schema evolution.
  ///
  /// The vector containing all links which are non-tree like
  mutable std::vector<Link*> m_linkVector;

public:
  /// Standard Constructor
  LinkManager() = default;
  /// Standard Destructor
  virtual ~LinkManager();
  /// Static instantiation
  static LinkManager* newInstance();
  /// Assign new instantiator
  static void setInstantiator( LinkManager* ( *newInstance )() );
  /// Retrieve number of link present
  long size() const { return m_linkVector.size(); }
  bool empty() const { return m_linkVector.empty(); }
  /// Retrieve symbolic link identified by ID
  Link* link( long id );
  /// Retrieve symbolic link identified by object
  Link* link( const DataObject* pObject );
  /// Retrieve symbolic link identified by path
  Link* link( const std::string& path );
  /// Add link by object reference and path
  long addLink( const std::string& path, const DataObject* pObject ) const;
  /// Remove link by object reference
  long removeLink( const DataObject* pObject ) const;
  /// Remove link by object reference
  long removeLink( const std::string& fullPath ) const;
  /// Remove link by link ID
  long removeLink( long id ) const;
  /// Remove all possibly existing symbolic links
  void clearLinks();
};
#endif // GAUDIKERNEL_LINKMANAGER_H
