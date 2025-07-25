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
//	====================================================================
//	SmartRef.h
//	--------------------------------------------------------------------
//
//	Package   : Kernel
//
//	Author    : Markus Frank
//
//	====================================================================
#ifndef KERNEL_SMARTREF_H
#define KERNEL_SMARTREF_H 1

// Include files
#include <GaudiKernel/ContainedObject.h>
#include <GaudiKernel/SmartRefBase.h>

#include <typeinfo>

// Forward declarations
template <class TYPE>
class SmartRefArray;
template <class TYPE>
class SmartRefList;
template <class TYPE>
class SmartRefMap;

/** Kernel objects: SmartRef

    Description:
    The SmartRef class allows transparent handling of object links
    within the data store. Links are unloaded a priori and will only be
    loaded "on demand", i.e. when dereferenced.

    SmartRefs should behave in the same way as normal pointers;
    The SmartRef object in fact is a smart pointer construct intercepting
    the dereference operators.

    When loading, the executed code resides in the non templated base class.
    This ensures, that the templated code is minimized and code blow up is
    inhibited.

    Using SmartRefs StreamBuffers are able to also save the references
    on data conversion requests.

    Base Class:
    SmartRefBase

    Dependencies:
    <UL>
    <LI> Base class:         GaudiKernel/SmartRefBase.h
    </UL>

    <P> History    :
    <PRE>
    +---------+----------------------------------------------+--------+
    |    Date |                 Comment                      | Who    |
    +---------+----------------------------------------------+--------+
    | 21/04/99| Initial version.                             | MF     |
    | 16/01/04| Move base class into aggregation.            | MF     |
    +---------+----------------------------------------------+--------+
    </PRE>
    Author:  M.Frank
    Version: 1.0
*/
template <class TYPE>
class SmartRef final {
  /// The container must be a friend
  friend class SmartRefArray<TYPE>;
  friend class SmartRefList<TYPE>;
  friend class SmartRefMap<TYPE>;

  SmartRefBase m_base;
  /// Pointer to target data object
  mutable const TYPE* m_target = nullptr;

public:
  enum { VALID = StreamBuffer::VALID, INVALID = StreamBuffer::INVALID };
  /// Entry type definition
  typedef TYPE entry_type;

  /// Standard Constructor
  SmartRef( const TYPE* pObject = nullptr ) {
    m_base.m_hintID = INVALID;
    m_base.m_linkID = INVALID;
    m_target        = pObject;
    _setEnvironment( nullptr, nullptr );
  }
  /// Copy Constructor
  SmartRef( const SmartRef& copy ) {
    m_base.m_hintID = copy.m_base.m_hintID;
    m_base.m_linkID = copy.m_base.m_linkID;
    m_target        = copy.m_target;
    _setEnvironment( copy.m_base.m_data, copy.m_base.m_contd );
  }
  /// Constructor
  SmartRef( long hint, long link, const TYPE* obj = nullptr ) {
    m_base.m_hintID = hint;
    m_base.m_linkID = link;
    m_target        = obj;
    _setEnvironment( nullptr, nullptr );
  }
  /// Constructor for references to contained objects passing environment
  SmartRef( const ContainedObject* pObj, long hint, long link, const TYPE* obj = nullptr ) {
    m_base.m_hintID       = hint;
    m_base.m_linkID       = link;
    m_target              = obj;
    const DataObject* src = ( pObj ? pObj->parent() : nullptr );
    _setEnvironment( src, pObj );
  }
  /// Constructor for references to contained objects passing environment
  SmartRef( const DataObject* pObj, long hint, long link, const TYPE* obj = nullptr ) {
    m_base.m_hintID = hint;
    m_base.m_linkID = link;
    m_target        = obj;
    _setEnvironment( pObj, nullptr );
  }
  /// Constructor for references to DataObjects passing environment
  SmartRef( const DataObject* pObj, long hint, const TYPE* obj = nullptr ) {
    m_base.m_hintID = hint;
    m_base.m_linkID = INVALID;
    m_target        = obj;
    _setEnvironment( pObj, nullptr );
  }
  /// Check if link should be followed: link must be valid and object not yet loaded
  bool shouldFollowLink( const DataObject* ) const { return !m_target && m_base.m_hintID != INVALID; }
  /// Check if link should be followed: link must be valid and object not yet loaded
  bool shouldFollowLink( const ContainedObject* ) const {
    return !m_target && m_base.m_hintID != INVALID && m_base.m_linkID != INVALID;
  }
  /// Access hint id:
  long hintID() const { return m_base.m_hintID; }
  /// Access link id:
  long linkID() const { return m_base.m_linkID; }
  /// Setup smart reference when reading. Must be allowed from external sources.
  void set( DataObject* pObj, long hint_id, long link_id ) { m_base.set( pObj, hint_id, link_id ); }
  /// Access to embedded type
  const std::type_info* type() const { return &typeid( TYPE ); }
  /// Access to raw data pointer
  TYPE*       data() { return const_cast<TYPE*>( m_target ); }
  const TYPE* data() const { return m_target; }
  /// Access to the object
  const TYPE* target() const {
    if ( !m_target ) { m_target = dynamic_cast<const TYPE*>( m_base.accessData( m_target ) ); }
    return m_target;
  }
  /// Access to the object
  TYPE* target() {
    if ( !m_target ) { m_target = dynamic_cast<const TYPE*>( m_base.accessData( m_target ) ); }
    return const_cast<TYPE*>( m_target );
  }
  /// Return the path of the linked object inside the data store.
  const std::string& path() const { return m_base.path(); }
  /// Equality operator
  bool operator==( const SmartRef<TYPE>& c ) const {
    if ( m_target && c.m_target ) return m_target == c.m_target;
    if ( !m_target && !c.m_target ) return m_base.isEqual( m_target, c.m_base );
    if ( m_target && !c.m_target ) return m_base.isEqualEx( m_target, c.m_base );
    if ( !m_target && c.m_target ) return c.m_base.isEqualEx( c.m_target, m_base );
    return false;
  }
  friend bool operator==( const SmartRef<TYPE>& ref, std::nullptr_t ) { return ref.target() == nullptr; }

  /// Friend helper to check for object existence (will load object)
  friend bool operator==( std::nullptr_t, const SmartRef<TYPE>& ref ) { return ref.target() == nullptr; }
  /// NON-Equality operator
  bool operator!=( const SmartRef<TYPE>& c ) const { return !( this->operator==( c ) ); }

  friend bool operator!=( const SmartRef<TYPE>& ref, std::nullptr_t ) { return ref.target() != nullptr; }

  /// Friend helper to check for object existence (will load object)
  friend bool operator!=( std::nullptr_t, const SmartRef<TYPE>& ref ) { return ref.target() != nullptr; }

  /// explicit conversion to bool to check for object existence (will load object)
  explicit operator bool() const { return target() != nullptr; }

  /// Set the environment (CONST)
  const SmartRef<TYPE>& _setEnvironment( const DataObject* pObj, const ContainedObject* pContd ) const {
    m_base.m_data  = pObj;
    m_base.m_contd = pContd;
    m_base.setObjectType( data() );
    return *this;
  }
  /// Set the environment (CONST)
  SmartRef<TYPE>& _setEnvironment( const DataObject* pObj, const ContainedObject* pContd ) {
    m_base.m_data  = pObj;
    m_base.m_contd = pContd;
    m_base.setObjectType( data() );
    return *this;
  }
  /// operator(): assigns parent object for serialisation
  SmartRef<TYPE>& operator()( ContainedObject* pObj ) {
    const DataObject* src = ( pObj ? pObj->parent() : nullptr );
    return _setEnvironment( src, pObj );
  }
  /// operator() const: assigns parent object for serialisation
  const SmartRef<TYPE>& operator()( const ContainedObject* pObj ) const {
    const DataObject* src = ( pObj ? pObj->parent() : nullptr );
    return _setEnvironment( src, pObj );
  }
  /// operator(): assigns parent object for serialisation
  SmartRef<TYPE>& operator()( DataObject* pObj ) { return _setEnvironment( pObj, nullptr ); }
  /// operator() const: assigns parent object for serialisation
  const SmartRef<TYPE>& operator()( const DataObject* pObj ) const { return _setEnvironment( pObj, nullptr ); }
  /// Assignment
  SmartRef<TYPE>& operator=( const SmartRef<TYPE>& c ) {
    m_target        = c.m_target;
    m_base.m_hintID = c.m_base.m_hintID;
    m_base.m_linkID = c.m_base.m_linkID;
    return _setEnvironment( c.m_base.m_data, c.m_base.m_contd );
  }
  /// Assignment
  SmartRef<TYPE>& operator=( const TYPE* pObject ) {
    m_target        = pObject;
    m_base.m_hintID = INVALID;
    m_base.m_linkID = INVALID;
    return *this;
  }
  /// Dereference operator
  TYPE& operator*() { return *SmartRef<TYPE>::target(); }
  /// Dereference operator
  const TYPE& operator*() const { return *SmartRef<TYPE>::target(); }
  /// Dereference operator
  TYPE* operator->() { return SmartRef<TYPE>::target(); }
  /// Dereference operator to const object
  const TYPE* operator->() const { return SmartRef<TYPE>::target(); }
  /// Implicit type conversion to const object
  operator const TYPE*() const { return SmartRef<TYPE>::target(); }
  /// Implicit type conversion
  operator TYPE*() { return SmartRef<TYPE>::target(); }
  /// Write the reference to the stream buffer (needed due to stupid G++ compiler)
  StreamBuffer& writeRef( StreamBuffer& s ) const {
    m_base.writeObject( m_target, s );
    return s;
  }
  /// Read the reference from the stream buffer (needed due to stupid G++ compiler)
  StreamBuffer& readRef( StreamBuffer& s ) {
    m_target = dynamic_cast<const TYPE*>( m_base.readObject( m_target, s ) );
    return s;
  }
  /// Output Streamer operator
  // MCl: it is "_s" instead of the most common "s" to avoid a fake icc remark #1599
  friend StreamBuffer& operator<<( StreamBuffer& _s, const SmartRef<TYPE>& ptr ) { return ptr.writeRef( _s ); }
  /// Input Streamer operator
  // MCl: it is "_s" instead of the most common "s" to avoid a fake icc remark #1599
  friend StreamBuffer& operator>>( StreamBuffer& _s, SmartRef<TYPE>& ptr ) { return ptr.readRef( _s ); }
};

#endif // KERNEL_SMARTREF_H
