/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/ClassID.h>
#include <GaudiKernel/Kernel.h>
#include <GaudiKernel/ObjectContainerBase.h>
#include <GaudiKernel/StreamBuffer.h>
#include <iomanip>
#include <vector>

// Definition of the CLID for this class defined in ClassID.h
// static const CLID CLID_ObjectList = (1<<17);  // ObjectVector   (bit 17 set)

/** @class ObjectVector ObjectVector.h GaudiKernel/ObjectVector.h
    ObjectVector is one of the basic Gaudi container classes capable of being
    registered in Data Stores.
    It is based on Standard Library (STL) std::vector
    (see <A HREF="http://www.sgi.com/Technology/STL/">STL Programmer's Guide</A>)
    ObjectVector has all functions of the std::vector interface,
    and in addition a number of Gaudi specific functions.

    Each object is allowed to belong into a single container only.
    After inserting the object into the container, it takes over
    all responsibilities for the object.  E.g. erasing the object
    from its container causes removing the object's pointer from
    the container and deleting the object itself.


    @author  Pavel Binko
    @author  Pere Mato
    @date    19/10/1999, 30/11/2000
*/
template <class TYPE>
class ObjectVector : public ObjectContainerBase {

public:
  typedef TYPE                                    contained_type;
  typedef typename std::vector<TYPE*>::value_type value_type;

  typedef typename std::vector<TYPE*>::reference       reference;
  typedef typename std::vector<TYPE*>::const_reference const_reference;

  typedef typename std::vector<TYPE*>::iterator       iterator;
  typedef typename std::vector<TYPE*>::const_iterator const_iterator;

  typedef typename std::vector<TYPE*>::reverse_iterator       reverse_iterator;
  typedef typename std::vector<TYPE*>::const_reverse_iterator const_reverse_iterator;

  typedef typename std::vector<TYPE*>::pointer       pointer;
  typedef typename std::vector<TYPE*>::const_pointer const_pointer;

public:
  ObjectVector()                                       = default;
  ObjectVector( const ObjectVector<TYPE>& )            = delete;
  ObjectVector& operator=( const ObjectVector<TYPE>& ) = delete;
  ObjectVector( ObjectVector&& rhs ) : ObjectContainerBase( std::move( rhs ) ), m_vector{ std::move( rhs.m_vector ) } {
    std::for_each( begin(), end(), [this]( TYPE* obj ) { obj->setParent( this ); } );
  }

  ~ObjectVector() override {
    for ( auto& i : m_vector ) {
      // Set the back pointer to 0 to avoid repetitional searching
      // for the object in the container, and deleting the object
      i->setParent( nullptr );
      delete i;
    }
  }

  /// Retrieve class ID
  const CLID& clID() const override { return ObjectVector<TYPE>::classID(); }
  /// Retrieve class ID
  static const CLID& classID() {
    static const CLID clid = TYPE::classID() + CLID_ObjectVector;
    return clid;
  }

  /// Return an iterator pointing to the beginning of the container
  typename ObjectVector<TYPE>::iterator begin() { return m_vector.begin(); }

  /// Return a const_iterator pointing to the beginning of the container
  typename ObjectVector<TYPE>::const_iterator begin() const { return m_vector.begin(); }

  /// Return an iterator pointing to the end of the container
  typename ObjectVector<TYPE>::iterator end() { return m_vector.end(); }

  /// Return a const_iterator pointing to the end of the container
  typename ObjectVector<TYPE>::const_iterator end() const { return m_vector.end(); }

  /// Return a reverse_iterator pointing to the beginning of the reversed container
  typename ObjectVector<TYPE>::reverse_iterator rbegin() { return m_vector.rbegin(); }

  /// Return a const_reverse_iterator pointing to the beginning
  ///   of the reversed container
  typename ObjectVector<TYPE>::const_reverse_iterator rbegin() const { return m_vector.rbegin(); }

  /// Return a reverse_iterator pointing to the end of the reversed container
  typename ObjectVector<TYPE>::reverse_iterator rend() { return m_vector.rend(); }

  /// Return a const_reverse_iterator pointing to the end of the reversed container
  typename ObjectVector<TYPE>::const_reverse_iterator rend() const { return m_vector.rend(); }

  /** Return the size of the container.
      Size means the number of objects stored in the container,
      independently on the amount of information stored in each object
  */
  typename ObjectVector<TYPE>::size_type size() const { return m_vector.size(); }

  /// The same as size(), return number of objects in the container
  typename ObjectVector<TYPE>::size_type numberOfObjects() const override { return m_vector.size(); }

  /// Return the largest possible size of the container
  typename ObjectVector<TYPE>::size_type max_size() const { return m_vector.max_size(); }

  /// Return number of elements for which memory has been allocated
  /// It is always greater than or equal to size()
  typename ObjectVector<TYPE>::size_type capacity() const { return m_vector.capacity(); }

  /** Reserve place for "value" objects in the container.
      If "value" is less than or equal to capacity(), this call has no effect,
      otherwise, it is a request for allocation of additional memory.
      If the request is successful, then capacity() is >= n,
      otherwise, capacity() is unchanged.
      In either case, size() is unchanged
  */
  void reserve( typename ObjectVector<TYPE>::size_type value ) { m_vector.reserve( value ); }

  /// Return true if the size of the container is 0
  bool empty() const { return m_vector.empty(); }

  /// Return reference to the first element
  typename ObjectVector<TYPE>::reference front() { return m_vector.front(); }

  /// Return const_reference to the first element
  typename ObjectVector<TYPE>::const_reference front() const { return m_vector.front(); }

  /// Return reference to the last element
  typename ObjectVector<TYPE>::reference back() { return m_vector.back(); }

  /// Return const_reference to the last element
  typename ObjectVector<TYPE>::const_reference back() const { return m_vector.back(); }

  /// push_back = append = insert a new element at the end of the container
  void push_back( typename ObjectVector<TYPE>::const_reference value ) {
    if ( value->parent() ) { const_cast<ObjectContainerBase*>( value->parent() )->remove( value ); }
    value->setParent( this );
    m_vector.push_back( value );
  }

  /// Add an object to the container
  long add( ContainedObject* pObject ) override {
    try {
      auto ptr = dynamic_cast<typename ObjectVector<TYPE>::value_type>( pObject );
      if ( ptr ) {
        push_back( ptr );
        return m_vector.size() - 1;
      }
    } catch ( ... ) {}
    return -1;
  }

  /// pop_back = remove the last element from the container
  /// The removed object will be deleted (see the method release)
  void pop_back() {
    auto position = m_vector.back();
    // Set the back pointer to 0 to avoid repetitional searching
    // for the object in the container, and deleting the object
    position->setParent( nullptr );
    delete position;
    // Removing from the container itself
    m_vector.pop_back();
  }

  /// Release object from the container (the poiter will be removed
  /// from the container, but the object itself will remain alive) (see the method pop_back)
  long remove( ContainedObject* value ) override {
    // Find the object of the value value
    auto i = std::find_if( begin(), end(), [&]( const ContainedObject* j ) { return j == value; } );
    if ( i == end() ) {
      // Object cannot be released from the conatiner,
      // as it is not contained in it
      return 0;
    }
    long idx = std::distance( begin(), i );
    // Set the back pointer to 0 to avoid repetitional searching
    // for the object in the container and deleting the object
    ( *i )->setParent( nullptr );
    erase( i );
    return idx;
  }

  /// Insert "value" before "position"
  typename ObjectVector<TYPE>::iterator insert( typename ObjectVector<TYPE>::iterator        position,
                                                typename ObjectVector<TYPE>::const_reference value ) {
    value->setParent( this );
    return m_vector.insert( position, value );
  }

  /// Erase the object at "position" from the container. The removed object will be deleted.
  void erase( typename ObjectVector<TYPE>::iterator position ) {
    if ( ( *position )->parent() ) {
      // Set the back pointer to 0 to avoid repetitional searching
      // for the object in the container, and deleting the object
      ( *position )->setParent( nullptr );
      delete *position;
    }
    // Removing from the container itself
    m_vector.erase( position );
  }

  /// Erase the range [first, last) from the container. The removed object will be deleted
  void erase( typename ObjectVector<TYPE>::iterator first, typename ObjectVector<TYPE>::iterator last ) {
    for ( auto i = first; i != last; i++ ) {
      // Set the back pointer to 0 to avoid repetitional searching
      // for the object in the container, and deleting the object
      ( *i )->setParent( nullptr );
      delete *i;
    }
    // Removing from the container itself
    m_vector.erase( first, last );
  }

  /// Clear the entire content of the container and delete all contained objects
  void clear() { erase( begin(), end() ); }

  /// Return the reference to the n'th object in the container
  typename ObjectVector<TYPE>::reference operator[]( typename ObjectVector<TYPE>::size_type n ) { return m_vector[n]; }

  /// Return the const_reference to the n'th object in the container
  typename ObjectVector<TYPE>::const_reference operator[]( typename ObjectVector<TYPE>::size_type n ) const {
    return m_vector[n];
  }

  /// Return distance of a given object from the beginning of its container
  /// It correcponds to the "index" ( from 0 to size()-1 )
  /// If "obj" not fount, return -1
  long index( const ContainedObject* obj ) const override {
    auto i = std::find_if( begin(), end(), [&]( const ContainedObject* o ) { return o == obj; } );
    return i != end() ? std::distance( begin(), i ) : -1;
  }

  /// Return const pointer to an object of a given distance (index)
  ContainedObject const* containedObject( long dist ) const override { return m_vector[dist]; }
  ContainedObject*       containedObject( long dist ) override { return m_vector[dist]; }

  /// Fill the output stream (ASCII)
  std::ostream& fillStream( std::ostream& s ) const override {
    s << "class ObjectVector :    size = " << std::setw( 12 ) << size() << "\n";
    // Output the base class
    // ObjectContainerBase::fillStream(s);
    if ( !empty() ) {
      s << "\nContents of the STL vector :";
      long count = 0;
      for ( const auto& i : m_vector ) { s << "\nIndex " << std::setw( 12 ) << count++ << " of object of type " << *i; }
    }
    return s;
  }

private:
  std::vector<TYPE*> m_vector;
};
