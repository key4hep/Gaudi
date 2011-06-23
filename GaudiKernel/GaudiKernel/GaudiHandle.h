#ifndef GAUDIKERNEL_GAUDIHANDLE_H
#define GAUDIKERNEL_GAUDIHANDLE_H

//Includes
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/System.h"
#include "GaudiKernel/GaudiException.h"

#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>

class GAUDI_API GaudiHandleInfo {
protected:
  /** Some basic information and helper functions shared between various handles/arrays.
   @param myComponentType: string indicating what type of component the handle is pointing to.
                        For example: "PublicTool", "PrivateTool", "Service".
                        This is used for printout and on the python side for type checking.
			On the python side there are classes with these names with "Handle" appended:
                        PublicToolHandle,PrivateToolHandle,ServiceHandle
   @param myParentName: Name of the parent that has this handle as a member. Used in printout.
  */
  GaudiHandleInfo( const std::string& myComponentType, const std::string& myParentName )
    : m_componentType(myComponentType), m_parentName(myParentName)
  {}
public:
  /** virtual destructor so that derived class destructor is called. */
  virtual ~GaudiHandleInfo() {}
  //
  // Public member functions
  //
  const std::string& componentType() const {
     return m_componentType;
  }

  /** name as used in declareProperty(name,gaudiHandle) */
  const std::string& propertyName() const {
     return m_propertyName;
  }

  /** set name as used in declareProperty(name,gaudiHandle). Used in printout. */
  void setPropertyName( const std::string& propName ) {
    m_propertyName = propName;
  }

  /** The name of the parent */
  const std::string& parentName() const {
    return m_parentName;
  }

  /** The python class name for the property in the genconf-generated configurables.
      The python class is defined in GaudiPython/python/GaudiHandles.py.
      To be implemented in derived class. */
  virtual const std::string pythonPropertyClassName() const = 0;

  /** Python representation of handle, i.e. python class name and argument.
      Can be used in the genconf-generated configurables.
      The corresponding python classes are defined in GaudiPython/GaudiHandles.py.
      To be implemented in derived class. */
  virtual const std::string pythonRepr() const = 0;

private:
  //
  // Data members
  //
  std::string m_componentType; // e.g.: "PublicTool","PrivateTool","Service"
  std::string m_propertyName;  // name as used in declareProperty(name,gaudiHandle)
  std::string m_parentName;    // name of the parent having this handle as a member
};


/** @class GaudiHandleBase GaudiHandle.h GaudiKernel/GaudiHandle.h

    Base class to handles to be used in lieu of naked pointers to various Gaudi components.
    This allows better control through the framework of component loading, configuration and usage.
    This base class implements common features.
    @author Martin.Woudstra@cern.ch
*/
class GAUDI_API GaudiHandleBase : public GaudiHandleInfo {
  //
  // Ctors etc
  //
protected:
  /** Create a handle ('smart pointer') to a gaudi component
   @param myTypeAndName: "MyType/MyName" ("MyType" is short for "MyType/MyType")
                       'MyType' is the name of the concrete class of the component
                       'MyName' is to distinguish several instances of the same concrete class
   @param myComponentType: string indicating what type of component the handle is pointing to.
                        For example: "PublicTool", "PrivateTool", "Service".
                        This is used for printout and on the python side for type checking.
			On the python side there are classes with these names with "Handle" appended:
                        PublicToolHandle,PrivateToolHandle,ServiceHandle
   @param myParentName: Name of the parent that has this handle as a member. Used in printout.
  */
  GaudiHandleBase( const std::string& myTypeAndName, const std::string& myComponentType,
		   const std::string& myParentName )
    : GaudiHandleInfo(myComponentType,myParentName)
  {
    setTypeAndName(myTypeAndName);
  }
public:
  //
  // Public member functions
  //
  /** The full type and name: "type/name" */
  std::string typeAndName() const {
     return m_typeAndName;
  }

  /** The concrete component class name: the part before the '/' */
  std::string type() const;

  /** The instance name: the part after the '/' */
  std::string name() const;

  /** Check if the handle has been set to empty string (i.e. typeAndName string is empty). */
  bool empty() const {
    return m_typeAndName.empty();
  }

  /** The component "type/name" string */
  void setTypeAndName( const std::string& myTypeAndName );

  /** Set the instance name (part after the '/') without changing the class type */
  void setName( const std::string& myName );

  /** Name of the componentType with "Handle" appended. Used as the python class name
      for the property in the genconf-generated configurables.
      The python class is defined in GaudiPython/python/GaudiHandles.py. */
  const std::string pythonPropertyClassName() const;

  /** name used for printing messages */
  const std::string messageName() const;

  /** Python representation of handle, i.e. python class name and argument.
      Can be used in the genconf-generated configurables.
      The corresponding python classes are defined in GaudiPython/GaudiHandles.py */
  virtual const std::string pythonRepr() const;

private:
  //
  // Data member
  //
  std::string m_typeAndName;   // the full type and name: "type/name"
};


/** @class GaudiHandle GaudiHandle.h GaudiKernel/GaudiHandle.h

    Handle to be used in lieu of naked pointers to gaudis. This allows
    better control through the framework of gaudi loading and usage.
    T is the type of the component interface (or concrete class).

    @author Martin.Woudstra@cern.ch
*/
template< class T >
class GAUDI_API GaudiHandle: public GaudiHandleBase {
  //
  // Constructors etc.
  //
protected:
  GaudiHandle( const std::string& myTypeAndName, const std::string& myComponentType,
	       const std::string& myParentName )
    : GaudiHandleBase(myTypeAndName, myComponentType, myParentName), m_pObject(0)
  {}

public:
  /** Copy constructor needed for correct ref-counting */
  GaudiHandle( const GaudiHandle& other )
    : GaudiHandleBase( other ) {
    m_pObject = other.m_pObject;
    if ( m_pObject ) m_pObject->addRef();
  }

  /** Assignment operator for correct ref-counting */
  GaudiHandle& operator=( const GaudiHandle& other ) {
    GaudiHandleBase::operator=( other );
    // release any current tool
    release().ignore();
    m_pObject = other.m_pObject;
    // update ref-counting
    if ( m_pObject ) m_pObject->addRef();
    return *this;
  }

  /** Retrieve the component. Release existing component if needed. */
  StatusCode retrieve() const { // not really const, because it updates m_pObject
    if ( m_pObject && release().isFailure() ) return StatusCode::FAILURE;
    if ( retrieve( m_pObject ).isFailure() ) {
      m_pObject = 0;
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }

  /** Release the component. */
  StatusCode release() const { // not really const, because it updates m_pObject
    if ( m_pObject ) {
      StatusCode sc = release( m_pObject );
      m_pObject = 0;
      return sc;
    }
    return StatusCode::SUCCESS;
  }

  /** For testing if handle has component. Does retrieve() if needed.
      If this returns false, the component could not be retrieved. */
  operator bool() const { // not really const, because it may update m_pObject
    return getObject();
  }

  T& operator*() {
    assertObject();
    return *m_pObject;
  }

  T* operator->() {
    assertObject();
    return m_pObject;
  }

  T& operator*() const { // not really const, because it may update m_pObject
    assertObject();
    return *m_pObject;
  }

  T* operator->() const { // not really const, because it may update m_pObject
    assertObject();
    return m_pObject;
  }

  /** Helper function to get default type string from the class type */
  std::string getDefaultType() {
    return System::typeinfoName( typeid(T) );
  }

  std::string getDefaultName() {
    std::string defName = GaudiHandleBase::type();
    if ( defName.empty() ) defName = getDefaultType();
    return defName;
  }

protected:
  /** Retrieve the component. To be implemented by the derived class. It will pass the pointer */
  virtual StatusCode retrieve( T*& ) const = 0; // not really const, because it updates m_pObject

  /** Release the component. Default implementation calls release() on the component.
      Can be overridden by the derived class if something else if needed. */
  virtual StatusCode release( T* comp ) const { // not really const, because it updates m_pObject
    comp->release();
    return StatusCode::SUCCESS;
  }

private:
  /** Helper function to set default name and type */
  void setDefaultTypeAndName() {
    const std::string& myType = getDefaultType();
    GaudiHandleBase::setTypeAndName(myType+'/'+myType);
  }

  /** Helper function to set default type from the class type T */
  void setDefaultType() {
    GaudiHandleBase::setTypeAndName( getDefaultType() );
  }

  /** Load the pointer to the component. Do a retrieve if needed */
  bool getObject() const { // not really const, because it may update m_pObject
    return m_pObject || retrieve().isSuccess();
  }

  /** Load the pointer to the component. Do a retrieve if needed. Throw an exception if
      retrieval fails. */
  void assertObject() const { // not really const, because it may update m_pObject
    if ( !getObject() ) {
      throw GaudiException("Failed to retrieve " + componentType() + ": " + typeAndName(),
			   componentType() + " retrieve", StatusCode::FAILURE);
    }
  }
  //
  // Data members
  //
  mutable T* m_pObject;
};


/**
   Base class of array's of various gaudihandles. Used in communication with
   GaudiHandleArrayProperty. For that purpose is has some pure virtual functions.
   This is a non-templated class to one Property can handle all kinds of concrete handles.
*/

class GAUDI_API GaudiHandleArrayBase: public GaudiHandleInfo {
protected:
  GaudiHandleArrayBase( const std::string& myComponentType, const std::string& myParentName )
    : GaudiHandleInfo(myComponentType,myParentName)
  {}
public:
  typedef std::vector< GaudiHandleBase* > BaseHandleArray;
  typedef std::vector< const GaudiHandleBase* > ConstBaseHandleArray;

  /** Set the array of handles from list of "type/name" strings in
      &lt;myTypesAndNamesList&gt;. Return whether set was successful or not */
  bool setTypesAndNames( const std::vector< std::string >& myTypesAndNamesList );

  /** Return a vector with "type/name" strings of all handles in the array.
      Inverse of setTypesAndNames() */
  const std::vector< std::string > typesAndNames() const;

  /** Return a vector with "type" strings of all handles in the array. */
  const std::vector< std::string > types() const;

  /** Return a vector with "type/name" strings of all handles in the array. */
  const std::vector< std::string > names() const;

  /** Helper function to get a vector of strings filled with the return value
      of each tool of a member function if GaudiHandleBase */
  const std::vector< std::string > getBaseInfos( std::string (GaudiHandleBase::*pMemFunc)() const ) const;

  /** Name of the componentType with "HandleArray" appended. Used as the python class name
      for the property in the genconf-generated configurables.
      The python class is defined in GaudiPython/python/GaudiHandles.py. */
  virtual const std::string pythonPropertyClassName() const;

  /** Python representation of array of handles, i.e. list of python handles.
      Can be used in the genconf-generated configurables.
      The corresponding python classes are defined in GaudiPython/GaudiHandles.py */
  virtual const std::string pythonRepr() const;

  /** Add a handle to the array with "type/name" given in &lt;myHandleTypeAndName&gt;.
      Return whether addition was successful or not.
      Must be implemented by derived class with concrete handle category. */
  virtual bool push_back( const std::string& myHandleTypeAndName ) = 0;

  /** Clear the list of handles. Implemented in GaudiHandleArray */
  virtual void clear() = 0;

  /** Return whether the list of tools is empty. */
  virtual bool empty() const = 0;

  /** Get a read-only vector of const GaudiHandleBase* pointing to the real handles.
      Implemented in GaudiHandleArray. */
  virtual ConstBaseHandleArray getBaseArray() const = 0;

  /** Get a read-write vector of GaudiHandleBase* pointing to the real handles.
      Implemented in GaudiHandleArray. */
  virtual BaseHandleArray getBaseArray() = 0;
};


/** T is the concrete handle type, e.g. ToolHandle<IMyTool> */
template <class T>
class GAUDI_API GaudiHandleArray: public GaudiHandleArrayBase {
public:
  //
  // public nested types
  //
  typedef std::vector< T > HandleVector;
  typedef typename HandleVector::value_type value_type;
  typedef typename HandleVector::size_type size_type;
  typedef typename HandleVector::reference reference;
  typedef typename HandleVector::const_reference const_reference;
  typedef typename HandleVector::iterator iterator;
  typedef typename HandleVector::const_iterator const_iterator;
  typedef typename HandleVector::reverse_iterator reverse_iterator;
  typedef typename HandleVector::const_reverse_iterator const_reverse_iterator;

protected:
  //
  // Constructors
  //
  /** Generic constructor. Probably not very useful...
      @param typesAndNamesList : a vector of strings with the concrete "type/name" strings
                                 for the list of tools
 */
  GaudiHandleArray( const std::vector< std::string >& myTypesAndNamesList,
		    const std::string& myComponentType, const std::string& myParentName  )
    : GaudiHandleArrayBase(myComponentType,myParentName)
  {
    setTypesAndNames( myTypesAndNamesList );
  }

  /** Constructor creating an empty array.
      @param typesAndNamesList : a vector of strings with the concrete "type/name" strings
                                 for the list of tools
 */
  GaudiHandleArray( const std::string& myComponentType, const std::string& myParentName  )
    : GaudiHandleArrayBase(myComponentType,myParentName)
  {}

public:
  virtual ~GaudiHandleArray() {};

  /**Set the array of GaudiHandles from typeAndNames given in vector of strings. */
  GaudiHandleArray& operator=( const std::vector< std::string >& myTypesAndNamesList ) {
    setTypesAndNames( myTypesAndNamesList );
    return *this;
  }

  virtual GaudiHandleArrayBase::BaseHandleArray getBaseArray() {
    GaudiHandleArrayBase::BaseHandleArray baseArray;
    iterator it = begin(), itEnd = end();
    for (  ; it != itEnd; ++it ) baseArray.push_back( &*it );
    return baseArray;
  }

  virtual GaudiHandleArrayBase::ConstBaseHandleArray getBaseArray() const {
    GaudiHandleArrayBase::ConstBaseHandleArray baseArray;
    const_iterator it = begin(), itEnd = end();
    for (  ; it != itEnd; ++it ) baseArray.push_back( &*it );
    return baseArray;
  }

  //
  // Simulate (part of) an std::vector
  //
  iterator begin() {
    return m_handleArray.begin();
  }

  iterator end() {
    return m_handleArray.end();
  }

  const_iterator begin() const {
    return m_handleArray.begin();
  }

  const_iterator end() const {
    return m_handleArray.end();
  }

  const_iterator rbegin() const {
    return m_handleArray.rbegin();
  }

  const_iterator rend() const {
    return m_handleArray.rend();
  }

  size_type size() const {
    return m_handleArray.size();
  }

  virtual void clear() {
    m_handleArray.clear();
  }

  virtual bool empty() const {
    return m_handleArray.empty();
  }

  T& operator[]( int index ) {
    return m_handleArray[index];
  }

  const T& operator[]( int index ) const {
    return m_handleArray[index];
  }

  /** Get pointer (!) to ToolHandle by instance name. Returns zero pointer if not found */
  T* operator[]( const std::string& name ) {
    iterator it = begin(), itEnd = end();
    for ( ; it != itEnd; ++it ) {
      if ( it->name() == name ) return &*it;
    }
    // not found
    return 0;
  }

  /** Get const pointer (!) to ToolHandle by instance name. Returns zero pointer if not found */
  const T* operator[]( const std::string& name ) const {
    const_iterator it = begin(), itEnd = end();
    for ( ; it != itEnd; ++it ) {
      if ( it->name() == name ) return &*it;
    }
    // not found
    return 0;
  }

/** Add a handle with given type and name. Can be overridden in derived class.
    Return whether addition was successful or not. */
  using GaudiHandleArrayBase::push_back; // avoid compiler warning
  virtual bool push_back( const T& myHandle ) {
    m_handleArray.push_back( myHandle );
    return true;
  }

  /** Retrieve all tools */
  StatusCode retrieve() {
    iterator it = begin(), itEnd = end();
    for ( ; it != itEnd; ++it ) {
      if ( it->retrieve().isFailure() ) {
	// stop at first failure
	return StatusCode::FAILURE;
      }
    }
    return StatusCode::SUCCESS;
  }

  /** Release all tools */
  StatusCode release() {
    StatusCode sc = StatusCode::SUCCESS;
    iterator it = begin(), itEnd = end();
    for ( ; it != itEnd; ++it ) {
      if ( it->release().isFailure() ) {
	// continue trying to release other tools
	sc = StatusCode::FAILURE;
      }
    }
    return sc;
  }

private:
  //
  // Private data members
  //
  HandleVector m_handleArray;
};

// Easy printing out of Handles and HandleArrays
// It prints <propertyName> = <HandleType>( <HandleType(s)AndName(s)> )
std::ostream& operator<<( std::ostream& os, const GaudiHandleInfo& handle );

#endif // ! GAUDIKERNEL_GAUDIHANDLE_H
