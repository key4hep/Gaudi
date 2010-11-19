// $Id: TransientFastContainer.h,v 1.6 2008/10/27 16:41:33 marcocle Exp $
#ifndef GAUDIKERNEL_TRANSIENTFASTCONTAINER_H
#define GAUDIKERNEL_TRANSIENTFASTCONTAINER_H 1

// Include files
#include "GaudiKernel/DataObject.h"
#include <vector>
#include <stdexcept>
#include <typeinfo>
#include <functional>


/** @class TransientFastContainerBase TransientFastContainer.h GaudiKernel/TransientFastContainer.h
 *
 *  Base class for TransientFastContainer. This class allow generic access to the reset method without
 *  the need to know the actual contained type.
 *
 *  @author Marco Clemencic
 *  @date   2006-05-02
 */
class GAUDI_API TransientFastContainerBase: public DataObject {
public:
  /// Empty virtual destructor
  virtual ~TransientFastContainerBase();
  /// Resets the internal counter of used objects.
  virtual void clear() = 0;
  /// Deletes all the cached objects.
  virtual void free() = 0;
  /// Hosted type id
  virtual const std::type_info &containedType() const = 0;
};


/** @struct DefaultObjectCleaner TransientFastContainer.h GaudiKernel/TransientFastContainer.h
 *
 *  Basic implementation for the object cleaner functor used by TransientFastContainer
 *  when returning a recycled object.
 *
 *  @author Marco Clemencic
 *  @date   2006-05-02
 */
template <class T>
struct DefaultObjectCleaner: public std::unary_function<T*,T*>{
  /// Get a pointer to an object of type T, clean the content of the object
  /// (by calling the placement destructor and constructor) and returns the same pointer.
  inline T* operator() (T* obj ) const {
    obj->T::~T(); // call the destructor without deallocating memory
    return new(obj) T(); // call the constructor on the memory
  }
  /// Get a pointer to an object of type T, clean the content of the object
  /// (by calling the placement destructor), copy the content of the provided object
  /// and returns the same pointer.
  inline T* operator() (T* obj, const T &rhs ) const {
    obj->T::~T(); // call the destructor without deallocating memory
    return new(obj) T(rhs); // call the copy constructor on the memory
  }
  /// Call the destructor of the object before passing back its pointer.
  inline T* destruct(T* obj) const {
    obj->T::~T(); // call the destructor without deallocating memory
    return obj; // just return the pointer to the non-initialized chunk
  }
  //inline T* operator() (T* obj ) { /* obj->clean(); */ return obj; }
};


/** @class TransientFastContainer TransientFastContainer.h GaudiKernel/TransientFastContainer.h
 *
 *  Container providing internal memory management.
 *
 *  New objects are added to the container using the member function
 *  New(). Those are not deleted until destruction of the container.
 *  When the method clear() is called, the internal  of the container
 *  is set to 0 and it behaves as if it is, but it still has memory of
 *  all the created object. When the New() is invoked on a "cleared" container,
 *  it returns the pointer of the first non-used of the known objects
 *  after calling an instance of CLEANER on it, or a new object if
 *  all the know objects are already in use.
 *
 *  @author Marco Clemencic
 *  @date   2006-05-02
 */
template <class T, class CLEANER = DefaultObjectCleaner<T> >
class TransientFastContainer: public TransientFastContainerBase {
public:
  // ---- types
  typedef T                                  contained_type;
  typedef contained_type*                    value_type;

  typedef value_type                         pointer;
  typedef const value_type                   const_pointer;

  typedef value_type                         reference;
  typedef const value_type                   const_reference;

  typedef std::vector<value_type>            storage_type;

  typedef typename storage_type::size_type               size_type;

  typedef typename storage_type::iterator                iterator;
  typedef typename storage_type::const_iterator          const_iterator;

  typedef CLEANER                            cleaner_type;

  /// Standard constructor. If the argument n is not zero, n objects are instantiated.
  TransientFastContainer(size_type n=0);

  /// Delete all the recorded objects.
  virtual ~TransientFastContainer( );

  // ---- TransientFastContainerBase implementation ----

  /// Set the internal counter to 0 without actually deleting the objects.
  virtual void clear();

  /// Delete all the contained objects to free memory.
  virtual void free();

  /// Return type information on the contained objects.
  virtual const std::type_info &containedType() const { return typeid(contained_type); }

  // ---- inlined functions ----

  /// Add a new object to the container or reuse one of the already available objects (after cleaning),
  /// and return a pointer to it.
  inline pointer New() { return ( m_counter++ < m_current_size ) ? m_cleaner(*(m_end++)) : i_new(); }

  /// Add a new object to the container or reuse one of the already available objects (after cleaning),
  /// and return a pointer to it.
  inline pointer Add(const T &rhs) { return ( m_counter++ < m_current_size ) ? m_cleaner(*(m_end++),rhs) : i_new(rhs); }

  /// Get a pointer to a non-initialized chunk of memory either destructing an already existing object
  /// or creating a new one and destructing it.
  inline pointer NewPointer()
  { return ( m_counter++ < m_current_size ) ?  m_cleaner.destruct(*(m_end++)) : m_cleaner.destruct(i_new()); }

  /// Iterator pointing to the first object of the container.
  inline iterator begin() { return m_storage.begin(); }
  /// Const iterator pointing to the first object of the container.
  inline const_iterator begin() const { return m_storage.begin(); }

  /// Iterator pointing after the last used object in the container.
  inline iterator end() { return m_end; }
  /// Const iterator pointing after the last used object in the container.
  inline const_iterator end() const { return m_end; }

  /// Returns the number of used objects in the container.
  inline size_type size() const { return m_counter; }

  /// Accessor to the object at the given index. Throws std::out_of_range if index is greater than size().
  inline pointer operator[] (size_type index)
  { return (index < size()) ? m_storage[index] : throw std::out_of_range("index out of range"), (pointer)NULL; }
  /// Const accessor to the object at the given index. Throws std::out_of_range if index is greater than size().
  inline const contained_type* operator[] (size_type index) const
  { return (index < size()) ? m_storage[index] : throw std::out_of_range("index out of range"), (const_pointer)NULL; }

  /// Same as operator[]().
  inline pointer at(size_type index) { return operator[] (index); }
  /// Same as operator[]() const.
  inline const contained_type* at(size_type index) const { return operator[] (index); }

private:

  /// Implementation function that instantiate a new object.
  pointer i_new();

  /// Implementation function that instantiate a new object using the copy constructor.
  pointer i_new(const T &rhs);

  /// Internal counter.
  size_type      m_counter;

  /// Number of already allocated objects.
  size_type      m_current_size;

  /// Repository of allocated objects.
  storage_type m_storage;

  /// Iterator that is returned by end().
  iterator m_end;

  /// Instance of the class that cleans new objects.
  cleaner_type m_cleaner;
};

template <class T, class CLEANER>
TransientFastContainer<T,CLEANER>::TransientFastContainer(size_type n):
  TransientFastContainerBase(),
  m_counter(0),
  m_current_size(0),
  m_storage(),
  m_end(m_storage.begin()),
  m_cleaner()
{
  if (n>0) {
    m_storage.reserve(n);
    for(;n>0;--n) New();
    clear();
  }
}

template <class T, class CLEANER>
TransientFastContainer<T,CLEANER>::~TransientFastContainer()
{
  free();
}


template <class T, class CLEANER>
void TransientFastContainer<T,CLEANER>::clear() {
  m_counter = 0;
  m_end = m_storage.begin();
}

template <class T, class CLEANER>
void TransientFastContainer<T,CLEANER>::free() {
  for( typename storage_type::iterator i = m_storage.begin();
       i != m_storage.end();
       ++i ) {
    delete *i;
  }
  m_storage.clear();
  m_counter = m_current_size = 0;
  m_end = m_storage.begin();
}

template <class T, class CLEANER>
typename TransientFastContainer<T,CLEANER>::pointer TransientFastContainer<T,CLEANER>::i_new() {
  //++m_counter; // already incremented in New()
  ++m_current_size;
  pointer ptr = new T();
  m_storage.push_back(ptr);
  m_end = m_storage.end();
  return ptr;
}

template <class T, class CLEANER>
typename TransientFastContainer<T,CLEANER>::pointer TransientFastContainer<T,CLEANER>::i_new(const T &rhs) {
  //++m_counter; // already incremented in New()
  ++m_current_size;
  pointer ptr = new T(rhs);
  m_storage.push_back(ptr);
  m_end = m_storage.end();
  return ptr;
}

#endif // GAUDIKERNEL_TRANSIENTFASTCONTAINER_H
