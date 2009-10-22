//	====================================================================
//	SmartRefBase.h
//	--------------------------------------------------------------------
//
//	Package   : Gaudi/Kernel
//              Base class for SmartReference objects
//
//	Author    : Markus Frank
//
//	====================================================================
#ifndef KERNEL_SMARTREFBASE_H
#define KERNEL_SMARTREFBASE_H 1

// Framework include files
#include "GaudiKernel/StreamBuffer.h"

class ObjectContainerBase;
class SmartRefBase;
class ContainedObject;
class DataObject;

/** User example objects: SmartRefBase

    Description:
    Base class of SmartReference objects. The base class handles the
    object requests to the data store.
    The separation of the SmartReference and the base class is mainly to
    avoid code blow up, because if the object is not yet present
    serious work must be performed in order to request the object from the
    store and invoke object updates to keep pointers in sync.

    Base Class: None

    Dependencies: None

    <P> History    :
    <PRE>
    +---------+----------------------------------------------+--------+
    |    Date |                 Comment                      | Who    |
    +---------+----------------------------------------------+--------+
    | 21/06/99| Initial version.                             | MF     |
    +---------+----------------------------------------------+--------+
    </PRE>
    Author:  M.Frank
    Version: 1.0
*/
class GAUDI_API SmartRefBase {
public:
  /// Object data: ID of the link hint to the identifiable object
  mutable long                    m_hintID;
  /// Object data: ID of the object within the identifiable container (if any)
  mutable long                    m_linkID;
  /// Object data: Pointer to the identifiable object the link originates
  mutable const DataObject*       m_data;
  /// Object data: Pointer to the Contained object (if applicable)
  mutable const ContainedObject*  m_contd;

  mutable long                    m_type;

  enum { DATAOBJECT, CONTAINEDOBJECT };

  /// Load on demand of ContainedObject like references
  const ContainedObject* accessData ( const ContainedObject* typ )   const;
  /// Load on demand of DataObject like references
  const DataObject* accessData ( const DataObject* typ )   const;

  /// Extended equality check
  bool isEqualEx(const DataObject* pObj, const SmartRefBase& c)  const;
  /// Extended equality check
  bool isEqualEx(const ContainedObject* pObj, const SmartRefBase& c)  const;
  /// Equality operator for ContainedObject like references
  bool isEqual          (const ContainedObject* /* pObj */, const SmartRefBase& c)  const  {
    return (m_hintID == c.m_hintID && m_linkID == c.m_linkID &&
            m_data   == c.m_data   && m_contd  == c.m_contd);
  }
  /// Equality operator for DataObject like references
  bool isEqual          (const DataObject* /* pObj */, const SmartRefBase& c)  const    {
    return m_linkID == c.m_linkID && m_data   == c.m_data;
  }
  /// Output streamer for DataObject like references
  void writeObject  (const DataObject* pObject, StreamBuffer& s)  const   {
    s.addIdentifiedLink(pObject, m_hintID);
  }
  /// Output streamer for ContainedObject like references
  void writeObject  (const ContainedObject* pObject, StreamBuffer& s)  const    {
    s.addContainedLink(pObject, m_hintID, m_linkID);
  }
  /// Input streamer for DataObject like references
  DataObject* readObject   (const DataObject* /* pObject */, StreamBuffer& s)  const   {
    DataObject* pObj = 0;
    s.getIdentifiedLink(pObj, m_hintID);
    return pObj;
  }
  /// Output streamer for ContainedObject like references
  ContainedObject* readObject(const ContainedObject* /* pObject */, StreamBuffer& s)  const    {
    ContainedObject* pObj = 0;
    s.getContainedLink(pObj, m_hintID, m_linkID);
    return pObj;
  }
  void setObjectType(const ContainedObject* /* pobj */) const {
    m_type = CONTAINEDOBJECT;
  }
  void setObjectType(const DataObject* /* pobj */)  const {
    m_type = DATAOBJECT;
  }

  /// Shortcut to access the path to the linked object.
  const std::string &path () const;

public:
  int objectType()  const {
    return m_type;
  }
  /// Setup smart reference when reading. Must be allowed from external sources.
  void set(DataObject* pObj, long hint_id, long link_id);

};
#endif // KERNEL_SMARTREFBASE_H
