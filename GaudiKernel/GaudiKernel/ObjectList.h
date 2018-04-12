#ifndef GAUDIKERNEL_OBJECTLIST_H
#define GAUDIKERNEL_OBJECTLIST_H

// Include files
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/ObjectContainerBase.h"
#include "GaudiKernel/StreamBuffer.h"

#include <iomanip>
#include <list>

// Definition of the CLID for this class  defined in ClassID.h
// static const CLID CLID_ObjectList = (1<<18);  // ObjectList   (bit 18 set)

/** @class ObjectList ObjectList.h GaudiKernel/ObjectList.h

    ObjectList is one of the basic Gaudi container classes capable of being
    registered in Data Stores.
    It is based on Standard Library (STL) std::list
    (see <A HREF="http://www.sgi.com/Technology/STL/">STL Programmer's Guide</A>)
    ObjectList has all functions of the std::list interface,

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
class ObjectList : public ObjectContainerBase
{

public:
  typedef TYPE                                  contained_type;
  typedef typename std::list<TYPE*>::value_type value_type;

  typedef typename std::list<TYPE*>::reference       reference;
  typedef typename std::list<TYPE*>::const_reference const_reference;

  typedef typename std::list<TYPE*>::iterator       iterator;
  typedef typename std::list<TYPE*>::const_iterator const_iterator;

  typedef typename std::list<TYPE*>::reverse_iterator       reverse_iterator;
  typedef typename std::list<TYPE*>::const_reverse_iterator const_reverse_iterator;

#ifdef _WIN32
  typedef typename std::vector<TYPE*>::_Tptr  pointer;
  typedef typename std::vector<TYPE*>::_Ctptr const_pointer;
#else
  typedef typename std::vector<TYPE*>::pointer       pointer;
  typedef typename std::vector<TYPE*>::const_pointer const_pointer;
#endif

public:
  /// Constructors
  ObjectList() = default;

  ObjectList( const ObjectList<TYPE>& ) = delete;
  ObjectList& operator=( const ObjectList<TYPE>& ) = delete;

  /// Destructor
  ~ObjectList() override { clear(); }

  /// Retrieve pointer to class definition structure
  const CLID&        clID() const override { return ObjectList<TYPE>::classID(); }
  static const CLID& classID()
  {
    static CLID clid = TYPE::classID() + CLID_ObjectList;
    return clid;
  }

  /// Return an iterator pointing to the beginning of the container
  typename ObjectList<TYPE>::iterator begin() { return m_list.begin(); }

  /// Return a const_iterator pointing to the beginning of the container
  typename ObjectList<TYPE>::const_iterator begin() const { return m_list.begin(); }

  /// Return an iterator pointing to the end of the container
  typename ObjectList<TYPE>::iterator end() { return m_list.end(); }

  /// Return a const_iterator pointing to the end of the container
  typename ObjectList<TYPE>::const_iterator end() const { return m_list.end(); }

  /// Return a reverse_iterator pointing to the beginning
  ///   of the reversed container
  typename ObjectList<TYPE>::reverse_iterator rbegin() { return m_list.rbegin(); }

  /// Return a const_reverse_iterator pointing to the beginning of the reversed container
  typename ObjectList<TYPE>::const_reverse_iterator rbegin() const { return m_list.rbegin(); }

  /// Return a reverse_iterator pointing to the end of the reversed container
  typename ObjectList<TYPE>::reverse_iterator rend() { return m_list.rend(); }

  /// Return a const_reverse_iterator pointing to the end of the reversed container
  typename ObjectList<TYPE>::const_reverse_iterator rend() const { return m_list.rend(); }

  /// Return the size of the container
  /// Size means the number of objects stored in the container, independently on the amount of information stored in
  /// each object
  typename ObjectList<TYPE>::size_type size() const
  {
    // C++11: std::list::size is constant (pre C++11 it could be linear!)
    return m_list.size();
  }
  /// The same as size(), return number of objects in the container
  typename ObjectList<TYPE>::size_type numberOfObjects() const override { return m_list.size(); }

  /// Return the largest possible size of the container
  typename ObjectList<TYPE>::size_type max_size() const { return m_list.max_size(); }

  /// Return true if the size of the container is 0
  bool empty() const { return m_list.empty(); }

  /// Return reference to the first element
  typename ObjectList<TYPE>::reference front() { return m_list.front(); }

  /// Return const_reference to the first element
  typename ObjectList<TYPE>::const_reference front() const { return m_list.front(); }

  /// Return reference to the last element
  typename ObjectList<TYPE>::reference back() { return m_list.back(); }

  /// Return const_reference to the last element
  typename ObjectList<TYPE>::const_reference back() const { return m_list.back(); }

  /// push_back = append = insert a new element at the end of the container
  void push_back( typename ObjectList<TYPE>::const_reference value )
  {
    if ( value->parent() ) {
      const_cast<ObjectContainerBase*>( value->parent() )->remove( value );
    }
    value->setParent( this );
    m_list.push_back( value );
  }

  /// Add an object to the container
  long add( ContainedObject* pObject ) override
  {
    try {
      auto ptr = dynamic_cast<typename ObjectList<TYPE>::value_type>( pObject );
      if ( ptr ) {
        push_back( ptr );
        return m_list.size() - 1;
      }
    } catch ( ... ) {
    }
    return -1;
  }

  /// pop_back = remove the last element from the container
  /// The removed object will be deleted (see the method release)
  void pop_back()
  {
    auto position = m_list.back();
    // Set the back pointer to 0 to avoid repetitional searching
    // for the object in the container, and deleting the object
    position->setParent( nullptr );
    delete position;
    // Removing from the container itself
    m_list.pop_back();
  }

  /// Release object from the container (the pointer will be removed
  /// from the container, but the object itself will remain alive) (see the method pop_back)
  long remove( ContainedObject* value ) override
  {
    // Find the object of value value
    long idx  = 0;
    auto iter = std::find_if( begin(), end(), [&]( const ContainedObject* i ) { return i == value; } );
    if ( iter == end() ) {
      // Object cannot be released from the container,
      // as it is not contained in it
      return -1;
    }

    // Set the back pointer to 0 to avoid repetitional searching
    // for the object in the container and deleting the object
    ( *iter )->setParent( nullptr );
    erase( iter );
    return idx;
  }

  /// Insert "value" before "position"
  typename ObjectList<TYPE>::iterator insert( typename ObjectList<TYPE>::iterator        position,
                                              typename ObjectList<TYPE>::const_reference value )
  {
    value->setParent( this );
    return m_list.insert( position, value );
  }

  /// Erase the object at "position" from the container. The removed object will be deleted.
  void erase( typename ObjectList<TYPE>::iterator position )
  {
    if ( ( *position )->parent() ) {
      // Set the back pointer to 0 to avoid repetitional searching
      // for the object in the container, and deleting the object
      ( *position )->setParent( nullptr );
      delete *position;
    }
    // Removing from the container itself
    m_list.erase( position );
  }

  /// Erase the range [first, last) from the container. The removed object will be deleted
  void erase( typename ObjectList<TYPE>::iterator first, typename ObjectList<TYPE>::iterator last )
  {
    for ( auto iter = first; iter != last; ++iter ) {
      // Set the back pointer to 0 to avoid repetitional searching
      // for the object in the container, and deleting the object
      ( *iter )->setParent( nullptr );
      delete *iter;
    }
    // Removing from the container itself
    m_list.erase( first, last );
  }

  /// Clear the entire content of the container and delete all contained objects
  void clear() { erase( begin(), end() ); }

  /// Return distance of a given object from the beginning of its container
  /// It corresponds to the "index" ( from 0 to size()-1 ) If "obj" not fount, return -1
  long index( const ContainedObject* obj ) const override
  {
    auto i = std::find_if( begin(), end(), [&]( const ContainedObject* o ) { return o == obj; } );
    return i != end() ? std::distance( begin(), i ) : -1;
  }

  /// Return const pointer to an object of a given distance
  ContainedObject* containedObject( long dist ) const override
  {
    return dist < size() ? *std::next( begin(), dist ) : nullptr;
  }

  /// Fill the output stream (ASCII)
  std::ostream& fillStream( std::ostream& s ) const override
  {
    s << "class ObjectList :    size = " << std::setw( 12 ) << size() << "\n";
    // Output the base class
    // ObjectContainerBase::fillStream(s);
    if ( !empty() ) {
      s << "\nContents of the STL list :";
      long count = 0;
      for ( const auto& iter : m_list ) {
        s << "\nIndex " << std::setw( 12 ) << count++ << " of object of type " << *iter;
      }
    }
    return s;
  }

private:
  /// The STL list
  std::list<TYPE*> m_list;
};

#endif // GAUDI_OBJECTLIST_H
