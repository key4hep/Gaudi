// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/ContainedObject.h,v 1.7 2004/01/19 16:56:44 mato Exp $
#ifndef GAUDIKERNEL_CONTAINEDOBJECT_H
#define GAUDIKERNEL_CONTAINEDOBJECT_H

// Include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/StreamBuffer.h"
#include "GaudiKernel/ObjectContainerBase.h"
#include <iostream>

// Forward declarations
template <class TYPE> class ObjectVector;
template <class TYPE> class ObjectList;

// Typedefs
typedef ObjectVector<ContainedObject> ContainedObjectVector;
typedef ObjectList<ContainedObject> ContainedObjectList;

// Definition of the CLID for this class
static const CLID CLID_ContainedObject = 190;

/** @class ContainedObject ContainedObject.h GaudiKernel/ContainedObject.h

    All classes that their objects may be contained in an LHCb
    ObjectContainer (e.g. ObjectVector, or ObjectList),
    have to inherit from the class ContainedObject. It guarantees
    the navigability from the contained object back to its container.
    @author Pavel Binko
*/
class GAUDI_API ContainedObject {

  /// Allow the container classes access to protected members
  friend class ObjectVector<ContainedObject>;
  friend class ObjectList<ContainedObject>;

protected:
  /// Constructors
  ContainedObject() : m_parent(0)  { }
  /// Copy constructor
  ContainedObject(const ContainedObject&) : m_parent(0)  { }
  /// Destructor
  virtual ~ContainedObject();

public:
  /// Retrieve pointer to class identifier
  virtual const CLID& clID() const { return classID(); }
  static const CLID& classID()     { return CLID_ContainedObject; }

  /// Access to parent object
  const ObjectContainerBase* parent () const  { return m_parent;        }
  /// Update parent member
  void setParent (ObjectContainerBase* value) { m_parent = value;       }

  /// Distance in the parent container
  virtual long index() const
  {
    return (m_parent) ? m_parent->index(this) : -1;
  }
  /// Serialize the object for writing
  virtual StreamBuffer& serialize( StreamBuffer& s ) const  { return s; }
  /// Serialize the object for reading
  virtual StreamBuffer& serialize( StreamBuffer& s )        { return s; }
  /// Fill the output stream (ASCII)
  virtual std::ostream& fillStream( std::ostream& s ) const { return s; }
  /// Output operator (ASCII)
  friend std::ostream& operator<< ( std::ostream& s, const ContainedObject& obj ) {
    return obj.fillStream(s);
  }

private:
  /// Pointer to the parent
  ObjectContainerBase*   m_parent;
};

#endif  // GAUDIKERNEL_CONTAINEDOBJECT_H
