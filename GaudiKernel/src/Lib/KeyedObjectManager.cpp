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
// Include files
#include <GaudiKernel/ContainedObject.h>
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/HashMap.h>
#include <GaudiKernel/Kernel.h>
#include <GaudiKernel/KeyedObjectManager.h>
#include <algorithm>
#include <map>

namespace Containers {
  struct hashmap {
    typedef GaudiUtils::HashMap<long, void*> map_type;
    map_type                                 m;
    std::vector<void*>                       v;
    bool                                     insert( void* obj, long key ) {
      auto p = m.insert( map_type::value_type( key, obj ) );
      return p.second;
    }
    hashmap()            = default;
    hashmap( hashmap&& ) = default;
  };
  struct map {
    typedef std::map<long, void*> map_type;
    map_type                      m;
    std::vector<void*>            v;
    bool                          insert( void* obj, long key ) {
      auto p = m.insert( map_type::value_type( key, obj ) );
      return p.second;
    }
    map()        = default;
    map( map&& ) = default;
  };
  struct array {
    typedef std::vector<long> map_type;
    /// Indirection array
    std::vector<long> m_idx;
    /// Direct access array
    std::vector<void*> v;
    struct decrement {
      long m_min;
      decrement( long m ) : m_min( m ) {}
      bool operator()( long& j ) const {
        if ( j > m_min ) --j;
        return true;
      }
    };
    array()          = default;
    array( array&& ) = default;
  };
  struct vector {
    typedef std::vector<void*> map_type;
    /// Direct access array
    std::vector<void*> v;
    vector()           = default;
    vector( vector&& ) = default;
  };

  template <class CONT>
  class find {
    const void*                       m_obj;
    typedef typename CONT::value_type v_type;

  public:
    find( const void* o ) : m_obj( o ) {}
    bool operator()( const void* cmp ) const { return cmp == m_obj; }
    bool operator()( const v_type& cmp ) const { return ( *this )( cmp.second ); }
  };
} // namespace Containers
void Containers::cannotAssignObjectKey() {
  throw GaudiException( "Cannot assign key to keyed object! Object already has a key.", "KeyedObject",
                        StatusCode::FAILURE );
}
void Containers::cannotInsertToContainer() {
  throw GaudiException( "Cannot insert element to Keyed Container!", "KeyedContainer", StatusCode::FAILURE );
}

void Containers::containerIsInconsistent() {
  throw GaudiException( "Keyed Container structures are inconsistent - severe problem!", "KeyedContainer",
                        StatusCode::FAILURE );
}

void Containers::invalidContainerOperation() {
  throw GaudiException( "Keyed Container cannot satisfy request - severe problem!", "KeyedContainer",
                        StatusCode::FAILURE );
}

template <class T>
Containers::KeyedObjectManager<T>::KeyedObjectManager() : m_seq( nullptr ), m_direct( 0 ) {
  if ( sizeof( typename T::map_type ) > sizeof( m_setup.buffer ) ) {
    throw GaudiException( "Basic STL contaier sizes are incompatible", "KeyedContainer", StatusCode::FAILURE );
  }
  m_setup.s = ::new ( m_setup.buffer + sizeof( m_setup.s ) ) T();
  m_keyCtxt = -1;
}

template <class T>
Containers::KeyedObjectManager<T>::KeyedObjectManager( KeyedObjectManager&& other )
    : m_seq( nullptr ), m_direct( other.m_direct ), m_keyCtxt( other.m_keyCtxt ) {
  m_setup.s = ::new ( m_setup.buffer + sizeof( m_setup.s ) ) T( std::move( *other.m_setup.s ) );

  other.m_keyCtxt = -1;
  other.m_seq     = nullptr;
  other.m_direct  = 0;
}

// the usage of namespace here is only to avoid a clang11/12 compiler warning
// due to a miswording in the specification of C++ and fixed in C++20, see
// https://stackoverflow.com/questions/68751682/is-a-class-templates-name-in-scope-for-a-qualified-out-of-line-destructors-def
namespace Containers {
  template <class T>
  KeyedObjectManager<T>::~KeyedObjectManager() {
    m_setup.s->~T();
  }
} // namespace Containers

/// Setup of the Map and the parent object
template <class T>
void Containers::KeyedObjectManager<T>::setup( void* seq, void** rndm ) {
  m_seq = (seq_type*)seq;
  *rndm = &m_setup.s->v;
}

template <class T>
void Containers::KeyedObjectManager<T>::onDirty() const {
  m_direct = 1;
  auto& s  = *m_setup.s;
  long  i  = 0;
  for ( auto p : s.v ) s.insert( p, i++ );
  s.v.clear();
}

template <class T>
long Containers::KeyedObjectManager<T>::insert( ObjectContainerBase* pBase, ContainedObject* pObject, void* obj,
                                                long* key ) {
  *key = ++m_keyCtxt;
  return insert( pBase, pObject, obj, *key );
}

template <class T>
long Containers::KeyedObjectManager<T>::insert( ObjectContainerBase* pBase, ContainedObject* pObject, void* obj,
                                                long key ) {
  /// Keep major key value
  if ( key > m_keyCtxt ) { m_keyCtxt = key; }
  if ( 1 == m_direct ) {
    if ( m_setup.s->insert( obj, key ) ) {
      if ( !pObject->parent() ) { pObject->setParent( pBase ); }
      m_seq->push_back( obj );
      return OBJ_INSERTED;
    }
  } else if ( key == long( m_setup.s->v.size() ) ) {
    m_setup.s->v.push_back( obj );
    if ( !pObject->parent() ) { pObject->setParent( pBase ); }
    m_seq->push_back( obj );
    return OBJ_INSERTED;
  } else {
    // Document is dirty now...
    // need to copy all pointers from the vector to the map
    onDirty();
    return insert( pBase, pObject, obj, key );
  }
  cannotInsertToContainer();
  return OBJ_CANNOT_INSERT;
}

template <class T>
long Containers::KeyedObjectManager<T>::insertDirect( ObjectContainerBase* pBase, ContainedObject* pObject, void* obj,
                                                      long key ) {
  /// Keep major key value
  if ( key > m_keyCtxt ) { m_keyCtxt = key; }
  if ( 1 == m_direct ) {
    if ( m_setup.s->insert( obj, key ) ) {
      if ( !pObject->parent() ) { pObject->setParent( pBase ); }
      return OBJ_INSERTED;
    }
  } else if ( key == long( m_setup.s->v.size() ) ) {
    m_setup.s->v.push_back( obj );
    if ( !pObject->parent() ) { pObject->setParent( pBase ); }
    return OBJ_INSERTED;
  } else {
    // Document is dirty now...
    // need to copy all pointers from the vector to the map
    onDirty();
    return insertDirect( pBase, pObject, obj, key );
  }
  cannotInsertToContainer();
  return OBJ_CANNOT_INSERT;
}

// Remove object from container
template <class T>
void* Containers::KeyedObjectManager<T>::erase( long key, const void* obj ) {
  typedef typename T::map_type MTYP;
  typedef find<MTYP>           FND;
  if ( 1 == m_direct ) {
    auto& m = m_setup.s->m;
    auto  i = ( obj ? std::find_if( m.begin(), m.end(), FND( obj ) ) : m_setup.s->m.find( key ) );
    if ( i != m_setup.s->m.end() ) {
      void* o = i->second;
      auto  j = std::find( m_seq->begin(), m_seq->end(), o );
      if ( j != m_seq->end() ) {
        m_seq->erase( j );
        m_setup.s->m.erase( i );
        return o;
      }
    }
    containerIsInconsistent();
  }
  onDirty();
  return erase( key, obj );
}

template <class T>
void* Containers::KeyedObjectManager<T>::object( long key ) const {
  if ( 0 == m_direct ) onDirty();
  auto i = m_setup.s->m.find( key );
  if ( i != m_setup.s->m.end() ) return ( *i ).second;
  return nullptr;
}

template <class T>
void Containers::KeyedObjectManager<T>::reserve( long len ) {
  switch ( m_direct ) {
  case 1:
    break;
  case 0:
    m_setup.s->v.reserve( len );
    break;
  default:
    break;
  }
  m_seq->reserve( len );
}

template <class T>
void Containers::KeyedObjectManager<T>::clear() {
  clearDirect();
  m_seq->clear();
}

template <class T>
void Containers::KeyedObjectManager<T>::clearDirect() {
  switch ( m_direct ) {
  case 1:
    m_setup.s->m.clear();
    break;
  case 0:
    m_setup.s->v.clear();
    break;
  default:
    break;
  }
  m_direct  = 0;
  m_keyCtxt = -1;
}

// Remove object by sequential iterators
template <class T>
long Containers::KeyedObjectManager<T>::erase( seq_type::iterator beg, seq_type::iterator end ) {
  typedef typename T::map_type MTYP;
  typedef find<MTYP>           FND;
  if ( 0 == m_direct ) {
    onDirty();
    return erase( beg, end );
  }
  if ( beg == m_seq->begin() && end == m_seq->end() ) {
    clear();
  } else {
    for ( auto j = beg; j != end; ++j ) {
      //      auto& m = m_setup.s->m;
      auto i = std::find_if( m_setup.s->m.begin(), m_setup.s->m.end(), FND( *j ) );
      if ( i != m_setup.s->m.end() ) {
        m_setup.s->m.erase( i );
        continue;
      }
      containerIsInconsistent();
    }
    m_seq->erase( beg, end );
  }
  return OBJ_ERASED;
}

namespace Containers {

  /*  First specialize static methods and then instantiate templated class to appear as symbols in the library
      This order in needed for gcc 4.0 (MacOSX) */

  template <>
  CLID KeyedObjectManager<Containers::map>::classID() {
    return CLID_ObjectVector + 0x00030000;
  }
  template <>
  CLID KeyedObjectManager<Containers::hashmap>::classID() {
    return CLID_ObjectVector + 0x00040000;
  }

  template class KeyedObjectManager<Containers::hashmap>;
  template class KeyedObjectManager<Containers::map>;
} // namespace Containers

/*
 *
 *
 *  Inline code for indirection array implementation
 *
 */
typedef Containers::array __A;

namespace Containers {

  //__forceinline
  template <>
  void* KeyedObjectManager<__A>::object( long value ) const {
#ifdef CHECK_KEYED_CONTAINER
    unsigned long siz = m_setup.s->m_idx.size();
    if ( value >= 0 && size_t( value ) < siz ) {
      long ent = *( m_setup.s->m_idx.begin() + value );
      if ( ent >= 0 ) { return *( m_setup.s->v.begin() + ent ); }
    }
    return nullptr;
#else
    return *( m_setup.s->v.begin() + ( *( m_setup.s->m_idx.begin() + value ) ) );
#endif
  }

  template <>
  void KeyedObjectManager<__A>::onDirty() const {
    m_direct = 1;
    m_setup.s->m_idx.reserve( m_setup.s->v.size() + 1 );
    for ( size_t i = 0, stop = m_setup.s->v.size(); i < stop; ++i ) {
      if ( !m_setup.s->v[i] ) { containerIsInconsistent(); }
      m_setup.s->m_idx.push_back( i );
    }
  }

  // Insert new object into container
  template <>
  long KeyedObjectManager<__A>::insert( ObjectContainerBase* b, ContainedObject* c, void* o, long* k ) {
    // auto key creation only possible for direct access!
    if ( 0 == m_direct ) {
      m_seq->push_back( o );
      m_setup.s->v.push_back( o );
      if ( !c->parent() ) c->setParent( b );
      *k = ++m_keyCtxt;
      return OBJ_INSERTED;
    }
    cannotInsertToContainer();
    return OBJ_CANNOT_INSERT;
  }

  // Insert new object into container
  template <>
  long KeyedObjectManager<__A>::insert( ObjectContainerBase* b, ContainedObject* c, void* o, long k ) {
    if ( 0 == m_direct ) {
      if ( k == m_keyCtxt + 1 ) { return insert( b, c, o, &k ); }
      onDirty();
      return insert( b, c, o, k );
    }
    /// Keep major key value
    if ( k > m_keyCtxt ) m_keyCtxt = k;
    /// Extend redirection array and insert
    if ( k + 1 > long( m_setup.s->m_idx.size() ) ) { m_setup.s->m_idx.resize( k + 1, -1 ); }
    auto idx = m_setup.s->m_idx.begin() + k;
    if ( *idx == -1 ) {
      *idx = m_setup.s->v.size();
      m_setup.s->v.push_back( o );
      m_seq->push_back( o );
      if ( !c->parent() ) c->setParent( b );
      return OBJ_INSERTED;
    }
    cannotInsertToContainer();
    return OBJ_CANNOT_INSERT;
  }

  // Insert new object into container
  template <>
  long KeyedObjectManager<__A>::insertDirect( ObjectContainerBase* b, ContainedObject* c, void* o, long k ) {
    if ( 0 == m_direct ) {
      if ( k == m_keyCtxt + 1 ) {
        m_setup.s->v.push_back( o );
        if ( !c->parent() ) c->setParent( b );
        ++m_keyCtxt;
        return OBJ_INSERTED;
      }
      onDirty();
      return insertDirect( b, c, o, k );
    }
    /// Keep major key value
    if ( k > m_keyCtxt ) m_keyCtxt = k;
    /// Extend redirection array and insert
    if ( k + 1 > long( m_setup.s->m_idx.size() ) ) { m_setup.s->m_idx.resize( k + 1, -1 ); }
    auto idx = m_setup.s->m_idx.begin() + k;
    if ( *idx == -1 ) {
      *idx = m_setup.s->v.size();
      m_setup.s->v.push_back( o );
      if ( !c->parent() ) c->setParent( b );
      return OBJ_INSERTED;
    }
    cannotInsertToContainer();
    return OBJ_CANNOT_INSERT;
  }

  // Clear content of the vector
  template <>
  void KeyedObjectManager<__A>::clearDirect() {
    m_setup.s->v.clear();
    m_setup.s->m_idx.clear();
    m_direct  = 0;
    m_keyCtxt = -1;
  }

  // Remove object from container (very inefficient if key is invalid)
  template <>
  void* KeyedObjectManager<__A>::erase( long key, const void* obj ) {
    if ( 0 == m_direct ) {
      onDirty();
      return erase( key, obj );
    }
    if ( obj ) {
      auto& idx = m_setup.s->m_idx;
      for ( auto& elem : idx ) {
        auto j = m_setup.s->v.begin() + ( elem );
        auto k = std::find( m_seq->begin(), m_seq->end(), *j );
        if ( *j == obj ) {
          void* o = *j;
          m_seq->erase( k );
          m_setup.s->v.erase( j );
          std::for_each( m_setup.s->m_idx.begin(), m_setup.s->m_idx.end(), array::decrement( elem ) );
          elem = -1;
          return o;
        }
      }
    } else if ( key >= 0 && key < long( m_setup.s->m_idx.size() ) ) {
      auto idx = m_setup.s->m_idx.begin() + key;
      if ( *idx != -1 ) {
        auto i = m_setup.s->v.begin() + ( *idx );
        if ( i == m_setup.s->v.end() ) { containerIsInconsistent(); }
        void* o = *i;
        auto  j = std::find( m_seq->begin(), m_seq->end(), o );
        if ( j == m_seq->end() ) { containerIsInconsistent(); }
        m_seq->erase( j );
        m_setup.s->v.erase( i );
        std::for_each( m_setup.s->m_idx.begin(), m_setup.s->m_idx.end(), array::decrement( *idx ) );
        *idx = -1;
        return o;
      }
    }
    containerIsInconsistent();
    return nullptr;
  }

  // Remove object by sequential iterators
  template <>
  long KeyedObjectManager<__A>::erase( seq_type::iterator beg, seq_type::iterator end ) {
    if ( beg == m_seq->begin() && end == m_seq->end() ) {
      clear();
      return OBJ_ERASED;
    } else if ( 0 == m_direct ) {
      onDirty();
      return erase( beg, end );
    } else {
      long  cnt = 0, nobj = end - beg;
      auto& idx = m_setup.s->m_idx;
      for ( auto& elem : idx ) {
        auto j = m_setup.s->v.begin() + ( elem );
        auto k = std::find( beg, end, *j );
        if ( k != end ) {
          m_setup.s->v.erase( j );
          std::for_each( m_setup.s->m_idx.begin(), m_setup.s->m_idx.end(), array::decrement( elem ) );
          elem = -1;
          cnt++;
          if ( cnt == nobj ) break;
        }
      }
      m_seq->erase( beg, end );
      if ( cnt != nobj ) { containerIsInconsistent(); }
      return OBJ_ERASED;
    }
    // cannot reach this point
  }

  template <>
  CLID KeyedObjectManager<__A>::classID() {
    return CLID_ObjectVector + 0x00050000;
  }
} // namespace Containers
template class Containers::KeyedObjectManager<__A>;
/*
 *
 *
 *  Implementation for objects with vector like access
 *
 *
 **/
typedef Containers::vector __V;

namespace Containers {
  // Access single entry by long(integer) key
  template <>
  void* KeyedObjectManager<__V>::object( long /* value */ ) const {
    invalidContainerOperation();
    return nullptr;
  }

  template <>
  void KeyedObjectManager<__V>::onDirty() const {
    invalidContainerOperation();
  }

  // Insert new object into container
  template <>
  long KeyedObjectManager<__V>::insert( ObjectContainerBase* b, ContainedObject* c, void* o, long* k ) {
    m_seq->push_back( o );
    m_setup.s->v.push_back( o );
    if ( !c->parent() ) c->setParent( b );
    *k = ( m_setup.s->v.size() - 1 );
    return OBJ_INSERTED;
  }

  // Insert new object into container
  template <>
  long KeyedObjectManager<__V>::insert( ObjectContainerBase* b, ContainedObject* c, void* o, long k ) {
    if ( k == long( m_setup.s->v.size() ) ) { return insert( b, c, o, &k ); }
    cannotInsertToContainer();
    return OBJ_CANNOT_INSERT;
  }

  // Insert new object into container
  template <>
  long KeyedObjectManager<__V>::insertDirect( ObjectContainerBase* b, ContainedObject* c, void* o, long k ) {
    if ( k == long( m_setup.s->v.size() ) ) {
      m_setup.s->v.push_back( o );
      if ( !c->parent() ) c->setParent( b );
      return OBJ_INSERTED;
    }
    cannotInsertToContainer();
    return OBJ_CANNOT_INSERT;
  }

  // Clear content of the vector
  template <>
  void KeyedObjectManager<__V>::clearDirect() {
    m_setup.s->v.clear();
    m_direct  = 0;
    m_keyCtxt = -1;
  }

  // Remove object from container (very inefficient if key is invalid)
  template <>
  void* KeyedObjectManager<__V>::erase( long /* key */, const void* /* obj */ ) {
    invalidContainerOperation();
    return nullptr;
  }

  // Remove object by sequential iterators
  template <>
  long KeyedObjectManager<__V>::erase( seq_type::iterator beg, seq_type::iterator end ) {
    if ( beg == m_seq->begin() && end == m_seq->end() ) {
      clear();
      return OBJ_ERASED;
    }
    invalidContainerOperation();
    return OBJ_ERASED;
  }

  template <>
  CLID KeyedObjectManager<__V>::classID() {
    return CLID_ObjectVector + 0x00060000;
  }
} // namespace Containers
template class Containers::KeyedObjectManager<__V>;
