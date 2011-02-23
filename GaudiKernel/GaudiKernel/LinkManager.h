#ifndef GAUDIKERNEL_LINKMANAGER_H
#define GAUDIKERNEL_LINKMANAGER_H

// STL includes
#include <vector>
#include <string>
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
class GAUDI_API LinkManager {

public:
  /// Directory link types
  enum DirLinkType {INVALID, VALID};

  /** Embedded class defining a symbolic link
    * Note: No copy constructor; bitwise copy (done by the compiler)
    *       is just fine.
    */
  class Link  {
    /// DataObject is a friend
    friend class LinkManager;
  protected:
    /// String containing path of symbolic link
    std::string m_path;
    /// Pointer to object behind the link
    DataObject* m_pObject;
    /// Link ID
    long        m_id;
  public:
    /// Standard constructor
    Link(long id, const std::string& path, const DataObject* pObject=0)
    : m_path(path), m_id(id) {
      setObject(pObject);
    }
    /// Standard constructor
    Link() : m_path(""), m_pObject(0), m_id(INVALID) {
    }
    /// Equality operator: check pathes only
    Link& operator=(const Link& link)  {
      setObject(link.m_pObject);
      m_path = link.m_path;
      m_id   = link.m_id;
      return *this;
    }
    /// Default destructor
    virtual ~Link() {
    }
    /// Update the link content
    void set(long id, const std::string& path, const DataObject* pObject)   {
      setObject(pObject);
      m_path = path;
      m_id   = id;
    }
    /// Equality operator: check paths only
    bool operator==(const Link& link)  const  {
      return link.m_path == m_path;
    }
    /// Update object pointer
    void setObject(const DataObject* pObject)   {
      m_pObject = const_cast<DataObject*>(pObject);
    }
    /// Const access to data object
    DataObject* object()  const  {
      return m_pObject;
    }
    /// Access to path of object
    const std::string& path()   const   {
      return m_path;
    }
    /// Link identifier
    long ID()  const    {
      return m_id;
    }
    /// Access to the object's address
    virtual IOpaqueAddress* address();
  };
  typedef std::vector<Link*>           LinkVector;
  /// Data type: iterator over leaf links
  typedef LinkVector::iterator         LinkIterator;
  /// Data type: iterator over leaf links (CONST)
  typedef LinkVector::const_iterator   ConstLinkIterator;

  /// The vector containing all links which are non-tree like
  mutable LinkVector m_linkVector;

public:
  /// Standard Constructor
  LinkManager();
  /// Standard Destructor
  virtual ~LinkManager();
  /// Static instantiation
  static LinkManager* newInstance();
  /// Assign new instantiator
  static void setInstantiator( LinkManager* (*newInstance)() );
  /// Retrieve number of link present
  long size()   const   {
    return m_linkVector.size();
  }
  /// Retrieve symbolic link identified by ID
  Link* link(long id);
  /// Retrieve symbolic link identified by object
  Link* link(const DataObject* pObject);
  /// Retrieve symbolic link identified by path
  Link* link(const std::string& path);
  /// Add link by object reference and path
  long addLink(const std::string& path, const DataObject* pObject)   const;
  /// Remove link by object reference
  long removeLink(const DataObject* pObject)   const;
  /// Remove link by object reference
  long removeLink(const std::string& fullPath)   const;
  /// Remove link by link ID
  long removeLink(long id)   const;
  /// Remove all possibly existing symbolic links
  void clearLinks();
};
#endif // GAUDIKERNEL_LINKMANAGER_H
