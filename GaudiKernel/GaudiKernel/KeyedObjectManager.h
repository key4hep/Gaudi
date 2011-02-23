// $Id $
#ifndef GAUDIKERNEL_KEYEDOBJECTMANAGER_H
#define GAUDIKERNEL_KEYEDOBJECTMANAGER_H

// Framework include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/KeyedTraits.h"

// STL includes
#include <vector>

// Forward declarations
class ObjectContainerBase;
class ContainedObject;

/** Containers namespace
*/
namespace Containers   {

  /// Object manipulator type definition
  typedef long (*MANIPULATOR)(void*);
  /// Parametrisation class for hashmap-like implementation.
  struct hashmap;
  /// Parametrisation class for map-like implementation.
  struct map;
  /// Parametrisation class for redirection array - like implementation.
  struct array;
  /// Parametrisation class for vector-like implementation.
  struct vector;

  /** KeyedObjectManager
   *  Class to manage keyed objects. This class is instantiated for two
   *  container types: map and hashmap. Other types are possible,
   *  but currently not supported. Other implementations may be achieved
   *  by specializing the SETUP class.
   *
   *  As an example below the specialization for a vector like
   *  implementation is shown.
   *
   *  @author   M.Frank CERN/LHCb
   *  @version  1.0
   *
   */
  template <class SETUP>
  class GAUDI_API KeyedObjectManager    {
  private:
    typedef std::vector<void*> seq_type;
    /// Container holding array like container
    seq_type*       m_seq;
    /// Dirty flag
    mutable long    m_direct;
    mutable long    m_keyCtxt;
    union {
      /// Buffer space to hold keyed container
      char          buffer[128];
      SETUP*        s;
    } m_setup;

    /// Callback when the container becomes dirty
    void onDirty()  const;

  public:
    /// Standard Constructor
    KeyedObjectManager();
    /// Standard Destructor
    virtual ~KeyedObjectManager();
    /// Clear all direct access fields
    void clearDirect();
    /// Insert element into direct access map
    long insertDirect(ObjectContainerBase* b,ContainedObject* c,void* o, long k);
    /// Check if the container is dirty
    long isDirect() const {      return m_direct;    }
    /// Retrieve object identified by a key from the container
    void* object(long key)  const;
    /// Insert new object into container
    long insert(ObjectContainerBase* b,ContainedObject* c,void* o, long* k);
    /// Insert new object into container
    long insert(ObjectContainerBase* b,ContainedObject* c,void* o,long k);
    /// Remove object from container (very inefficient if key is invalid)
    void* erase(long key, const void* obj);
    /// Remove object by sequential iterators
    long erase(seq_type::iterator beg, seq_type::iterator end);
    /// Reserve buffer space
    void reserve(long size);
    /// Clear content of the vector
    void clear();
    /// Setup of the Map and the parent object
    void setup(void* seq, void** rndm);
    /// Access CLID for this type of container.
    static CLID classID();
  };

  /// Forward declaration of specialized std::map-like object manager
  typedef KeyedObjectManager< map >     Map;
  /// Forward declaration of specialized std::hashmap-like object manager
  typedef KeyedObjectManager< hashmap > HashMap;
  /// Forward declaration of specialized std::vector-like object manager
  typedef KeyedObjectManager< vector >  Vector;
  /// Forward declaration of specialized redirection array object manager
  typedef KeyedObjectManager< array >   Array;
}
#endif // GAUDIKERNEL_KEYEDOBJECTMANAGER_H
