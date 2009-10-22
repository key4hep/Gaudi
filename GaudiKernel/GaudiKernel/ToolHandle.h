#ifndef GAUDIKERNEL_TOOLHANDLE_H
#define GAUDIKERNEL_TOOLHANDLE_H

//Includes
#include "GaudiKernel/GaudiHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/INamedInterface.h"
//#include "GaudiKernel/SmartIF.h"

#include <string>
#include <vector>
#include <stdexcept>

// forward declarations
class IInterface;
class IAlgTool;
class IToolSvc;

/** General info and helper functions for toolhandles and arrays */
class ToolHandleInfo {
protected:
  ToolHandleInfo( const IInterface* parent = 0, bool createIf = true )
    : m_parent(parent), m_createIf(createIf)
    {}

public:
  virtual ~ToolHandleInfo() {};

  bool isPublic() const {
     return !m_parent;
  }

  bool createIf() const {
     return m_createIf;
  }

  const IInterface* parent() const {
     return m_parent;
  }

  //
  // Some helper functions
  //
  const std::string toolComponentType( const IInterface* parent ) const {
    return parent ? "PrivateTool" : "PublicTool";
  }

  const std::string toolParentName( const IInterface* parent ) const {
    if (parent) {
      //SmartIF<INamedInterface> pNamed(const_cast<IInterface*>(parent));
      //if (pNamed.isValid()) {
      const INamedInterface* pNamed = dynamic_cast<const INamedInterface*>(parent);
      if (pNamed) {
        return pNamed->name();
      } else {
        return "";
      }
    } else {
      return "ToolSvc";
    }
  }

private:
  const IInterface* m_parent;
  bool m_createIf;
};

/** @class ToolHandle ToolHandle.h GaudiKernel/ToolHandle.h

    Handle to be used in lieu of naked pointers to tools. This allows
    better control through the framework of tool loading and usage.
    @paramater T is the AlgTool interface class (or concrete class) of
    the tool to use, and must derive from IAlgTool.

    @author Wim Lavrijsen <WLavrijsen@lbl.gov>
    @author Martin.Woudstra@cern.ch
*/
template< class T >
class ToolHandle : public ToolHandleInfo, public GaudiHandle<T> {
public:
  //
  // Constructors etc.
  //
  /** Constructor for a tool with default tool type and name.
      Can be called only if the type T is a concrete tool type (not an interface),
      and you want to use the default name. */
  ToolHandle( const IInterface* parent = 0, bool createIf = true )
    : ToolHandleInfo(parent,createIf),
      GaudiHandle<T>( GaudiHandle<T>::getDefaultType(),
		      ToolHandleInfo::toolComponentType(parent),
		      ToolHandleInfo::toolParentName(parent) ),
      m_pToolSvc( "ToolSvc", GaudiHandleBase::parentName() )
  {}

  /** Create a handle ('smart pointer') to a tool.
      The arguments are passed on to ToolSvc, and have the same meaning:
      @code
      StatusCode ToolSvc::retrieveTool ( const std::string& type            ,
                                         T*&                tool            ,
					 const IInterface*  parent   = 0    ,
					 bool               createIf = true )
      @endcode
      @param toolType: "MyToolType/MyToolName"
                       "MyToolType" is short for "MyToolType/MyToolType"
                       'MyToolType' is the name of the class of the concrete tool
                       'MyToolName' is to distinguish several tool instances of the same class
      @param parent: the parent Algorithm,Tool or Service of which this tool is a member.
                     If non-zero, the the tool is a private tool of the parent, otherwise it is
                     a public (shared) tool.
      @param createIf: if true, create tool if not yet existing.
  */
  ToolHandle( const std::string& toolTypeAndName, const IInterface* parent = 0, bool createIf = true )
    : ToolHandleInfo(parent,createIf),
      GaudiHandle<T>( toolTypeAndName,
		      ToolHandleInfo::toolComponentType(parent),
		      ToolHandleInfo::toolParentName(parent) ),
      m_pToolSvc( "ToolSvc", GaudiHandleBase::parentName() )
  {}

  /** Retrieve the AlgTool. Release existing tool if needed.
      Function must be repeated here to avoid hiding the function retrieve( T*& ) */
  StatusCode retrieve() const { // not really const, because it updates m_pObject
    return GaudiHandle<T>::retrieve();
  }

  /** Release the AlgTool.
      Function must be repeated here to avoid hiding the function release( T*& ) */
  StatusCode release() const { // not really const, because it updates m_pObject
    return GaudiHandle<T>::release();
  }

  /** Do the real retrieval of the AlgTool. */
  virtual StatusCode retrieve( T*& algTool ) const {
    return m_pToolSvc->retrieve( GaudiHandleBase::typeAndName(), T::interfaceID(),
				 (IAlgTool*&)(algTool),
				 ToolHandleInfo::parent(), ToolHandleInfo::createIf() );
  }

  /** Do the real release of the AlgTool. */
  virtual StatusCode release( T* algTool ) const {
    return m_pToolSvc->releaseTool( algTool );
  }

private:
  //
  // Private data members
  //
  mutable ServiceHandle<IToolSvc> m_pToolSvc;
};

/** @class ToolHandleArray ToolHandle.h GaudiKernel/ToolHandle.h

    Array of Handles to be used in lieu of vector of naked pointers to tools.
    This allows better control through the framework of tool loading and usage.
    @parameter T is the AlgTool interface class (or concrete class) of
    the tool to use, and must derive from IAlgTool.

    @author Martin.Woudstra@cern.ch
*/

template < class T >
class ToolHandleArray : public ToolHandleInfo, public GaudiHandleArray< ToolHandle<T> > {
public:
  //
  // Constructors
  //
  /** Generic constructor. Probably not very useful...
      @param typesAndNamesList : a vector of strings with the concrete "type/name" strings
                                 for the list of tools
      @param parent   : passed on to ToolHandle, so has the same meaning as for ToolHandle
      @param createIf : passed on to ToolHandle, so has the same meaning as for ToolHandle
 */
  ToolHandleArray( const std::vector< std::string >& myTypesAndNames,
		   const IInterface* parent = 0, bool createIf = true )
    : ToolHandleInfo( parent, createIf ),
      GaudiHandleArray< ToolHandle<T> >( myTypesAndNames,
					 ToolHandleInfo::toolComponentType(parent),
					 ToolHandleInfo::toolParentName(parent) )
  {}

  /** Constructor which creates and empty list.
      @param parent   : passed on to ToolHandle, so has the same meaning as for ToolHandle
      @param createIf : passed on to ToolHandle, so has the same meaning as for ToolHandle
  */
  ToolHandleArray( const IInterface* parent = 0, bool createIf = true )
    : ToolHandleInfo( parent, createIf ),
      GaudiHandleArray< ToolHandle<T> >( ToolHandleInfo::toolComponentType(parent),
					 ToolHandleInfo::toolParentName(parent) )
  { }

  /** Add a handle to the array with given tool type and name.
      This function overrides the one in GaudiHandleArray<T>, as this is a special case.
      The private/public choice and createIf is determined by what was given
      in the constructor of the ToolHandleArray. */
  virtual bool push_back( const std::string& toolTypeAndName ) {
    ToolHandle<T> handle( toolTypeAndName,
			  ToolHandleInfo::parent(),
			  ToolHandleInfo::createIf() );
    GaudiHandleArray< ToolHandle<T> >::push_back( handle );
    return true;
  }

  /** Ensure that for added handles the parent and creatIf are taken from this array. */
  virtual bool push_back( const ToolHandle<T>& myHandle ) {
    return push_back( myHandle.typeAndName() );
  }

};


template <class T>
inline std::ostream& operator<<( std::ostream& os, const ToolHandle<T>& handle ) {
  return operator<<(os, static_cast<const GaudiHandleInfo&>(handle) );
}


template <class T>
inline std::ostream& operator<<( std::ostream& os, const ToolHandleArray<T>& handle ) {
  return operator<<(os, static_cast<const GaudiHandleInfo&>(handle) );
}


#endif // ! GAUDIKERNEL_TOOLHANDLE_H
