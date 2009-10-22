// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/ObjectList.h,v 1.10 2008/10/09 16:46:49 marcocle Exp $
#ifndef GAUDIKERNEL_OBJECTLIST_H
#define GAUDIKERNEL_OBJECTLIST_H


// Include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/StreamBuffer.h"
#include "GaudiKernel/ObjectContainerBase.h"

#include <list>
#include <iomanip>


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
class ObjectList : public ObjectContainerBase {

public:
  typedef TYPE                                                 contained_type;
  typedef typename std::list<TYPE*>::value_type                value_type;

  typedef typename std::list<TYPE*>::reference                 reference;
  typedef typename std::list<TYPE*>::const_reference           const_reference;

  typedef typename std::list<TYPE*>::iterator                  iterator;
  typedef typename std::list<TYPE*>::const_iterator            const_iterator;

  typedef typename std::list<TYPE*>::reverse_iterator          reverse_iterator;
  typedef typename std::list<TYPE*>::const_reverse_iterator    const_reverse_iterator;

#ifdef _WIN32
  typedef typename std::vector<TYPE*>::_Tptr                   pointer;
  typedef typename std::vector<TYPE*>::_Ctptr                  const_pointer;
#else
  typedef typename std::vector<TYPE*>::pointer                 pointer;
  typedef typename std::vector<TYPE*>::const_pointer           const_pointer;
#endif

public:
  /// Constructors
  ObjectList()
    : m_list(0) { }
  ObjectList( const char* name )
    : m_list(0) { }
  /// Copy Constructor
  ObjectList( const ObjectList<TYPE>& value )
    : m_list(value.m_list) { }

  /// Destructor
  virtual ~ObjectList() {
    clear();
  }

  /// Retrieve pointer to class defininition structure
  virtual const CLID& clID() const {
    return ObjectList<TYPE>::classID();
  }
  static const CLID& classID() {
    static CLID clid = TYPE::classID() + CLID_ObjectList;
    return clid;
  }

  /// Clone operator
  const ObjectList<TYPE>& operator = (const ObjectList<TYPE> &right) {
    this->processingVersion = right.m_processingVersion;
    this->detectorDataObject = right.m_detectorDataObject;
    m_list = right.m_list;
    return *this;
  }

  /// Return an iterator pointing to the beginning of the container
  typename ObjectList<TYPE>::iterator begin () {
    return m_list.begin();
  }

  /// Return a const_iterator pointing to the beginning of the container
  typename ObjectList<TYPE>::const_iterator begin () const {
    return m_list.begin();
  }

  /// Return an iterator pointing to the end of the container
  typename ObjectList<TYPE>::iterator end () {
    return m_list.end();
  }

  /// Return a const_iterator pointing to the end of the container
  typename ObjectList<TYPE>::const_iterator end () const {
    return m_list.end();
  }

  /// Return a reverse_iterator pointing to the beginning
  ///   of the reversed container
  typename ObjectList<TYPE>::reverse_iterator rbegin () {
    return m_list.rbegin();
  }

  /// Return a const_reverse_iterator pointing to the beginning of the reversed container
  typename ObjectList<TYPE>::const_reverse_iterator rbegin () const {
    return m_list.rbegin();
  }

  /// Return a reverse_iterator pointing to the end of the reversed container
  typename ObjectList<TYPE>::reverse_iterator rend () {
    return m_list.rend();
  }

  /// Return a const_reverse_iterator pointing to the end of the reversed container
  typename ObjectList<TYPE>::const_reverse_iterator rend () const {
    return m_list.rend();
  }

  /// Return the size of the container
  /// Size means the number of objects stored in the container, independently on the amount of information stored in each object
  typename ObjectList<TYPE>::size_type size () const {
    return m_list.size();
  }
  /// The same as size(), return number of objects in the container
  virtual typename ObjectList<TYPE>::size_type numberOfObjects() const {
    return m_list.size();
  }

  /// Return the largest possible size of the container
  typename ObjectList<TYPE>::size_type max_size () const {
    return m_list.max_size();
  }

  /// Return true if the size of the container is 0
  bool empty () const {
    return m_list.empty();
  }

  /// Return reference to the first element
  typename ObjectList<TYPE>::reference front () {
    return m_list.front();
  }

  /// Return const_reference to the first element
  typename ObjectList<TYPE>::const_reference front () const {
    return m_list.front();
  }

  /// Return reference to the last element
  typename ObjectList<TYPE>::reference back () {
    return m_list.back();
  }

  /// Return const_reference to the last element
  typename ObjectList<TYPE>::const_reference back () const {
    return m_list.back();
  }

  /// push_back = append = insert a new element at the end of the container
  void push_back( typename ObjectList<TYPE>::const_reference value ) {
    if( 0 != value->parent() ) {
      const_cast<ObjectContainerBase*>(value->parent())->remove(value);
    }
    value->setParent(this);
    m_list.push_back(value);
  }

  /// Add an object to the container
  virtual long add(ContainedObject* pObject) {
    try {
      typename ObjectList<TYPE>::value_type ptr =
            dynamic_cast<typename ObjectList<TYPE>::value_type>(pObject);
      if ( 0 != ptr ) {
        push_back(ptr);
        return m_list.size()-1;
      }
    }
    catch(...) {
    }
    return -1;
  }

  /// pop_back = remove the last element from the container
  /// The removed object will be deleted (see the method release)
  void pop_back () {
    typename ObjectList<TYPE>::value_type position = m_list.back();
    // Set the back pointer to 0 to avoid repetitional searching
    // for the object in the container, and deleting the object
    position->setParent (0);
    delete position;
    // Removing from the container itself
    m_list.pop_back();
  }

  /// Release object from the container (the poiter will be removed
  /// from the container, but the object itself will remain alive) (see the method pop_back)
  virtual long remove(ContainedObject* value) {
    // Find the object of value value
    long idx = 0;
    typename ObjectList<TYPE>::iterator   iter;
    for( iter = begin(); iter != end(); iter++, idx++ )  {
      if( value == *iter ) {
        break;
      }
    }
    if( end() == iter )  {
      // Object cannot be released from the conatiner,
      // as it is not contained in it
      return -1;
    }
    else  {
      // Set the back pointer to 0 to avoid repetitional searching
      // for the object in the container and deleting the object
      (*iter)->setParent (0);
      erase(iter);
      return idx;
    }
  }

  /// Insert "value" before "position"
  typename ObjectList<TYPE>::iterator insert( typename ObjectList<TYPE>::iterator position,
                                     typename ObjectList<TYPE>::const_reference value ) {
    value->setParent(this);
    typename ObjectList<TYPE>::iterator i = m_list.insert(position, value);
    return i;
  }

  /// Erase the object at "position" from the container. The removed object will be deleted.
  void erase( typename ObjectList<TYPE>::iterator position ) {
    if( 0 != (*position)->parent() ) {
      // Set the back pointer to 0 to avoid repetitional searching
      // for the object in the container, and deleting the object
      (*position)->setParent (0);
      delete *position;
    }
    // Removing from the container itself
    m_list.erase(position);
  }

  /// Erase the range [first, last) from the container. The removed object will be deleted
  void erase( typename ObjectList<TYPE>::iterator first,
              typename ObjectList<TYPE>::iterator last ) {
    for( typename ObjectList<TYPE>::iterator iter = first; iter != last; iter++ )  {
      // Set the back pointer to 0 to avoid repetitional searching
      // for the object in the container, and deleting the object
      (*iter)->setParent (0);
      delete *iter;
    }
    // Removing from the container itself
    m_list.erase(first, last);
  }

  /// Clear the entire content of the container and delete all contained objects
  void clear()    {
    erase(begin(), end());
  }

  /// Return distance of a given object from the beginning of its container
  /// It correcponds to the "index" ( from 0 to size()-1 ) If "obj" not fount, return -1
  virtual long index( const ContainedObject* obj ) const {
    long i = 0;
    typename ObjectList<TYPE>::const_iterator   iter;
    for( iter = begin(); iter != end(); iter++ )  {
      if( *iter == obj ) {
        return i;
      }
      i++;
    }
    return -1;
  }

  /// Return const pointer to an object of a given distance
  virtual ContainedObject* containedObject( long dist ) const {
    long i = 0;
    typename ObjectList<TYPE>::const_iterator   iter;
    for( iter = begin(); iter != end(); iter++ )  {
      if( dist == i ) {
        return *iter;
      }
      i++;
    }
    return 0;
  }

  /// Fill the output stream (ASCII)
  virtual std::ostream& fillStream( std::ostream& s ) const                    {
    s << "class ObjectList :    size = "
      << std::setw(12)
      << size() << "\n";
    // Output the base class
    //ObjectContainerBase::fillStream(s);
    if ( 0 != size() ) {
      s << "\nContents of the STL list :";
      long   count = 0;
      typename ObjectList<TYPE>::const_iterator iter;
      for( iter = m_list.begin(); iter != m_list.end(); iter++, count++ ) {
        s << "\nIndex "
          << std::setw(12)
          << count
          << " of object of type "<< **iter;
      }
    }
    return s;
  }

private:

  /// The STL list
  std::list<TYPE*> m_list;
};


#endif    // GAUDI_OBJECTLIST_H
