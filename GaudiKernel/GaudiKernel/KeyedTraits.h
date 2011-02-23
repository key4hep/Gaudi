// $Id: KeyedTraits.h,v 1.3 2006/12/10 20:29:17 leggett Exp $
#ifndef GAUDIKERNEL_KEYEDTRAITS_H
#define GAUDIKERNEL_KEYEDTRAITS_H

#define CHECK_KEYED_CONTAINER

// Include files
#include <vector>

// Forward declarations
template <class K>          class KeyedObject;
template <class T, class M> class KeyedContainer;

/*
  Namespace for Key classes used by the Keyed Container
*/
namespace Containers    {

  // Status enumeration
  enum {
    OBJ_NOT_FOUND,    /**< Object not present in the container.              */
    OBJ_DELETED,      /**< Object was removed from the container and deleted */
    OBJ_ERASED,       /**< Object was removed, but not deleted               */
    OBJ_INSERTED,     /**< Object was inserted into the container.           */
    OBJ_CANNOT_INSERT /**< Cannot insert object into container.              */
  };

  // Forward declarations

  /// Container traits class
  template <class CONTAINER, class DATATYPE>  struct traits;
  /// Key traits class
  template <class KEY>                        struct key_traits;
  /// Object manager class
  template <class SETUP>                      class  KeyedObjectManager;

  /** Function to be called to indicate that an object cannot be inserted
      to the container. Internally an exception is thrown.                    */
  GAUDI_API void cannotInsertToContainer();

  /** Function to be called to indicate that the container is found to be
      inconsistent. Internally an exception is thrown.                        */
  GAUDI_API void containerIsInconsistent();

  /** Function to be called to indicate that an operation should be
      performed on the container or it's contained data, which is not
      allowed. Internally an exception is thrown.                             */
  GAUDI_API void invalidContainerOperation();

  /** Function to be called when an object key cannot be assigned.            */
  GAUDI_API void cannotAssignObjectKey();

  /** Definition of the key traits class.

      This is the default class for keys. This implementation
      works "as is" for integer, long etc. keys.

      For all other types of keys, this class must be either
      partially or completely specialized.

      @author   M.Frank CERN/LHCb
      @version  1.0
  */
  template < class KEY > struct key_traits   {
    /// Declaration of key-type
    typedef KEY                   key_type;
    /// Declaration of keyed object type
    typedef KeyedObject<key_type> obj_type;
    /** Create key from its full integer representation.
        Not implementing on specialization may inhibit the creation
        of keys, i.e. then a key must be supplied at insertion time.
    */
    static key_type makeKey(long k)                    {return key_type(k); }
    static key_type makeKey(int k)                     {return key_type(k); }
    /// Full unhashed key identifier
    static long identifier(const key_type& k)          {return k;           }
    /// Hash function for this key
    static long hash(const key_type& key_value)        {return key_value;   }
    /// Set object key when inserted into the container
    static void setKey(obj_type* v, const key_type& k) {if(v)v->setKey(k);  }
    /** Check the validity of the object's key.
      Select if key-checks should be performed by
      switching on/off the macro CHECK_KEYED_CONTAINER.
    */
    static bool checkKey(obj_type* v,const key_type& k) {
#ifdef CHECK_KEYED_CONTAINER
      return (v) ? (hash(v->key())==hash(k)) : false;
#else
      return true;
#endif
    }
    /// Add reference counter to object when inserted into the container
    static long addRef(obj_type* v)      {   return (v) ? v->addRef()  : 0; }
    /// Release reference to object
    static long release(obj_type* v)     {   return (v) ? v->release() : 0; }
  };

  /** Definition of the container traits class.

      Select if container-checks should be performed by
      switching on/off the macro CHECK_KEYED_CONTAINER.

      @author   M.Frank CERN/LHCb
      @version  1.0
  */
  template < class CONTAINER, class DATATYPE >
  struct traits : public key_traits < typename DATATYPE::key_type >
  {
    /// Allow to check the access to container elements for consistency
    static bool checkBounds(const std::vector<DATATYPE*>* cnt,
                            const typename DATATYPE::key_type& k) {
#ifdef CHECK_KEYED_CONTAINER
      return size_t(cnt->size()) > size_t(traits::hash(k));
#else
      return true;
#endif
    }
  };
}
#endif  // GAUDIKERNEL_KEYEDTRAITS_H

