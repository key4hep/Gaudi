/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_KEYEDOBJECT_H
#define GAUDIKERNEL_KEYEDOBJECT_H

namespace GaudiDict {
  template <class T>
  struct KeyedObjectDict;
}

// Framework include files
#include <GaudiKernel/ContainedObject.h>
#include <GaudiKernel/KeyedContainer.h>
#include <GaudiKernel/KeyedTraits.h>

/** Definition of the templated KeyedObject class.
 *
 *  This object, which is of the basic containedObject
 *  type allows to be identified by key.
 *
 *  This implementation uses a reference count mechanism
 *  for insertion into multiple containers; Once the
 *  reference count is NULL, the object will automatically
 *  be deleted.
 *
 *  @author   M.Frank CERN/LHCb
 *  @version  1.0
 *
 */
template <class KEY>
class GAUDI_API KeyedObject : public ContainedObject {
  friend struct GaudiDict::KeyedObjectDict<KEY>;

public:
  /// Definition of the key-type to access object
  typedef KEY key_type;

protected:
  /// definition of the container key traits to be made friend
  typedef typename Containers::key_traits<key_type> traits;
  /// Allow the container traits to access full object properties
  // #ifdef _WIN32
  //   friend traits;
  // #else
  friend struct Containers::key_traits<key_type>;
  // #endif

  /// Object Key
  key_type m_key{};
  /// Reference counter
  long m_refCount = 0;
  /// Boolean to indicate wether a key was already assigned.
  bool m_hasKey = false;
  /// Add reference to object (Increase reference counter).
  unsigned long addRef();
  /// Release reference. If the reference count is ZERO, delete the object.
  unsigned long release();
  /** Set object key. The key for consistency reasons
      can be set only once for the object. Any attempt to
      redefine the key results in an exception.
  */
  void setKey( const key_type& key );

public:
  /// Standard Constructor. The object key is preset to the invalid value.
  KeyedObject() = default;
  /** Standard Constructor accepting the object's key.
      The key must be valid and cannot be changed later.
  */
  KeyedObject( const key_type& kval ) : m_key( kval ), m_refCount( 0 ), m_hasKey( true ) {}
  /// Standard destructor.
  ~KeyedObject() override;
  /// Retrieve Key of the object.
  const key_type& key() const { return m_key; }
  /// Check if the object has a key assigned or not.
  bool hasKey() const { return m_hasKey; } /// Distance in the parent container
  long index() const override { return traits::identifier( m_key ); }
  /// Serialize the object for writing
  StreamBuffer& serialize( StreamBuffer& s ) const override;
  /// Serialize the object for reading
  StreamBuffer& serialize( StreamBuffer& s ) override;

private:
  /// NOBODY may copy these objects
  KeyedObject( const KeyedObject& copy ) : ContainedObject( copy ), m_refCount( 0 ), m_hasKey( true ) {}
};

/*
 *
 *
 *  Inline code for keyed KeyedObject class
 *
 */

// Standard destructor.
template <class KEY>
inline KeyedObject<KEY>::~KeyedObject() {
  ObjectContainerBase* p = const_cast<ObjectContainerBase*>( parent() );
  if ( p ) {
    setParent( nullptr );
    p->remove( this );
  }
}

// Add reference to object (Increase reference counter).
template <class KEY>
inline unsigned long KeyedObject<KEY>::addRef() {
  return ++m_refCount;
}

// Release reference. If the reference count is ZERO, delete the object.
template <class KEY>
inline unsigned long KeyedObject<KEY>::release() {
  long cnt = --m_refCount;
  if ( cnt <= 0 ) delete this;
  return cnt;
}

/*  Set object key. The key for consistency reasons
    can be set only once for the object. Any attempt to
    redefine the key results in an exception.
*/
template <class KEY>
inline void KeyedObject<KEY>::setKey( const key_type& key ) {
  if ( !m_hasKey ) {
    m_key    = key;
    m_hasKey = true;
    return;
  }
  Containers::cannotAssignObjectKey();
}

// Serialize the object for writing
template <class KEY>
inline StreamBuffer& KeyedObject<KEY>::serialize( StreamBuffer& s ) const {
  return ContainedObject::serialize( s ) << traits::identifier( m_key );
}

// Serialize the object for reading
template <class KEY>
inline StreamBuffer& KeyedObject<KEY>::serialize( StreamBuffer& s ) {
  long k;
  ContainedObject::serialize( s ) >> k;
  m_key    = traits::makeKey( k );
  m_hasKey = true;
  return s;
}
#endif // GAUDIKERNEL_KEYEDOBJECT_H
