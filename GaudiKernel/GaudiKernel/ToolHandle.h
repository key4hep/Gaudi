#ifndef GAUDIKERNEL_TOOLHANDLE_H
#define GAUDIKERNEL_TOOLHANDLE_H

// Includes
#include "GaudiKernel/GaudiHandle.h"
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/INamedInterface.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/ServiceHandle.h"

#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

// forward declarations
class IInterface;
class IToolSvc;

class Algorithm;
class AlgTool;
class Service;

/** General info and helper functions for toolhandles and arrays */
class ToolHandleInfo
{

protected:
  ToolHandleInfo( const IInterface* parent = nullptr, bool createIf = true )
      : m_parent( parent ), m_createIf( createIf )
  {
  }

public:
  virtual ~ToolHandleInfo() = default;

  bool isPublic() const noexcept { return !m_parent; }

  bool createIf() const noexcept { return m_createIf; }

  const IInterface* parent() const noexcept { return m_parent; }

  //
  // Some helper functions
  //

  static std::string toolComponentType( const IInterface* parent ) { return parent ? "PrivateTool" : "PublicTool"; }

  static std::string toolParentName( const IInterface* parent )
  {
    auto* pNamed = ( parent ? dynamic_cast<const INamedInterface*>( parent ) : nullptr );
    return ( !parent ? "ToolSvc" : ( pNamed ? pNamed->name() : "" ) );
  }

protected:
  const IInterface* m_parent = nullptr;
  bool m_createIf{true};
};

/** @class BaseToolHandle ToolHandle.h GaudiKernel/ToolHandle.h

    Non-templated base class for actual ToolHandle<T>.
    Defines the interface for the management of ToolHandles in the Algorithms and Tools

    @author Daniel Funke <daniel.funke@cern.ch>
*/
class BaseToolHandle : public ToolHandleInfo
{

protected:
  BaseToolHandle( const IInterface* parent = nullptr, bool createIf = true ) : ToolHandleInfo( parent, createIf ) {}

  virtual StatusCode i_retrieve( IAlgTool*& ) const = 0;

public:
  StatusCode retrieve( IAlgTool*& tool ) const { return i_retrieve( tool ); }

  virtual StatusCode retrieve() const = 0;

  const IAlgTool* get() const { return getAsIAlgTool(); }

  IAlgTool* get() { return getAsIAlgTool(); }

  virtual std::string typeAndName() const = 0;

  /// Helper to check if the ToolHandle should be retrieved.
  inline bool isEnabled() const
  {
    // the handle is considered enabled if the type/name is valid and the
    // enabled flag was set to true, or it was already retrieved
    return ( m_enabled && !typeAndName().empty() ) || get();
  }

  inline void enable() { m_enabled = true; }

  inline void disable() { m_enabled = false; }

  inline bool setEnabled( const bool flag )
  {
    auto old  = m_enabled;
    m_enabled = flag;
    return old;
  }

protected:
  virtual const IAlgTool* getAsIAlgTool() const = 0;

  virtual IAlgTool* getAsIAlgTool() = 0;

  bool m_enabled = true;
};

/** @class ToolHandle ToolHandle.h GaudiKernel/ToolHandle.h

    Handle to be used in lieu of naked pointers to tools. This allows
    better control through the framework of tool loading and usage.
    @paramater T is the AlgTool interface class (or concrete class) of
    the tool to use, and must derive from IAlgTool.

    @author Wim Lavrijsen <WLavrijsen@lbl.gov>
    @author Martin.Woudstra@cern.ch
*/
template <class T>
class ToolHandle : public BaseToolHandle, public GaudiHandle<T>
{

  friend class Algorithm;
  friend class AlgTool;
  friend class Service;

public:
  /** Constructor for a tool with default tool type and name.
      Can be called only if the type T is a concrete tool type (not an interface),
      and you want to use the default name. */
  ToolHandle( const IInterface* parent = nullptr, bool createIf = true )
      : BaseToolHandle( parent, createIf )
      , GaudiHandle<T>( "", toolComponentType( parent ), toolParentName( parent ) )
      , m_pToolSvc( "ToolSvc", GaudiHandleBase::parentName() )
  {
  }

  /** Copy constructor from a non const T to const T tool handle */
  template <typename CT = T, typename NCT = typename std::remove_const<T>::type>
  ToolHandle( const ToolHandle<NCT>& other,
              typename std::enable_if<std::is_const<CT>::value && !std::is_same<CT, NCT>::value>::type* = nullptr )
      : BaseToolHandle( other.parent(), other.createIf() )
      , GaudiHandle<CT>( other )
      , m_pToolSvc( "ToolSvc", GaudiHandleBase::parentName() )
  {
  }

public:
//
// Constructors etc.
//

/** Create a handle ('smart pointer') to a tool.
    The arguments are passed on to ToolSvc, and have the same meaning:
    @code
    StatusCode ToolSvc::retrieveTool ( const std::string& type            ,
                                       T*&                tool            ,
                                       const IInterface*  parent   = 0    ,
                                       bool               createIf = true )
    @endcode
    @param owner: class owning the ToolHandle
    @param toolType: "MyToolType/MyToolName"
                     "MyToolType" is short for "MyToolType/MyToolType"
                     'MyToolType' is the name of the class of the concrete tool
                     'MyToolName' is to distinguish several tool instances of the same class
    @param parent: the parent Algorithm,Tool or Service of which this tool is a member.
                   If non-zero, the the tool is a private tool of the parent, otherwise it is
                   a public (shared) tool.
    @param createIf: if true, create tool if not yet existing.
*/

#if defined( TOOLHANDLE_DEPR_WARN )
// warn about using deprecated explicit ToolHandle construction
#pragma message( "Untracked ToolHandle: Migrate explicit DataHandle constructor to declareTool Algorithm Property" )

  __attribute__( ( deprecated ) )

#endif
  ToolHandle( const std::string& toolTypeAndName, const IInterface* parent = nullptr, bool createIf = true )
      : BaseToolHandle( parent, createIf )
      , GaudiHandle<T>( toolTypeAndName, toolComponentType( parent ), toolParentName( parent ) )
      , m_pToolSvc( "ToolSvc", GaudiHandleBase::parentName() )
  {
  }

  /// Autodeclaring constructor with property name, tool type/name and documentation.
  /// @note the use std::enable_if is required to avoid ambiguities
  template <class OWNER, typename = std::enable_if_t<std::is_base_of<IProperty, OWNER>::value>>
  inline ToolHandle( OWNER* owner, std::string name, std::string toolType, std::string doc = "" ) : ToolHandle( owner )
  {
    // convert name and type to a valid type/name string
    // - if type does not contain '/' use type/name
    // - otherwise type is already a type/name string
    if ( !toolType.empty() and toolType.find( '/' ) == std::string::npos ) {
      toolType += '/';
      toolType += name;
    }
    owner->declareTool( *this, std::move( toolType ) ).ignore();
    auto p = owner->OWNER::PropertyHolderImpl::declareProperty( std::move( name ), *this, std::move( doc ) );
    p->template setOwnerType<OWNER>();
  }

public:
  StatusCode initialize( const std::string& toolTypeAndName, const IInterface* parent = nullptr, bool createIf = true )
  {

    GaudiHandleBase::setTypeAndName( toolTypeAndName );
    GaudiHandleBase::setComponentType( toolComponentType( parent ) );
    GaudiHandleBase::setParentName( toolParentName( parent ) );

    m_parent   = parent;
    m_createIf = createIf;

    return m_pToolSvc.initialize( "ToolSvc", GaudiHandleBase::parentName() );
  }

  /** Retrieve the AlgTool. Release existing tool if needed.
      Function must be repeated here to avoid hiding the function retrieve( T*& ) */
  StatusCode retrieve() const override
  { // not really const, because it updates m_pObject
    return GaudiHandle<T>::retrieve();
  }

  /** Release the AlgTool.
      Function must be repeated here to avoid hiding the function release( T*& ) */
  StatusCode release() const
  { // not really const, because it updates m_pObject
    return GaudiHandle<T>::release();
  }

  /** Do the real retrieval of the AlgTool. */
  StatusCode retrieve( T*& algTool ) const override
  {
    IAlgTool* iface = nullptr;
    algTool         = i_retrieve( iface ) ? dynamic_cast<T*>( iface ) : nullptr;
    return algTool ? StatusCode::SUCCESS : StatusCode::FAILURE;
  }

  /** Do the real release of the AlgTool. */
  StatusCode release( T* algTool ) const override { return m_pToolSvc->releaseTool( details::nonConst( algTool ) ); }

  std::string typeAndName() const override { return GaudiHandleBase::typeAndName(); }

  typename std::add_const<T>::type* get() const { return GaudiHandle<T>::get(); }

  T* get() { return GaudiHandle<T>::get(); }

// Allow access to non-const Tool methods of const ToolHandle
#ifdef ALLOW_TOOLHANDLE_NONCONSTNESS
  T* operator->() { return GaudiHandle<T>::operator->(); }
  T& operator*() { return *( GaudiHandle<T>::operator->() ); }

  T* operator->() const { return details::nonConst( GaudiHandle<T>::operator->() ); }
  T& operator*() const { return *( details::nonConst( GaudiHandle<T>::operator->() ) ); }
#endif

#ifdef ATLAS
  [[deprecated( "FIXME!! should not call non-const method from a const ToolHandle" )]] ToolHandle<T>& unConst() const
  {
    return const_cast<ToolHandle<T>&>( *this );
  }
#endif

protected:
  const IAlgTool* getAsIAlgTool() const override
  {
    // const cast to support T being const
    return GaudiHandle<T>::get();
  }

  IAlgTool* getAsIAlgTool() override
  {
    // const cast to support T being const
    return details::nonConst( GaudiHandle<T>::get() );
  }

  StatusCode i_retrieve( IAlgTool*& algTool ) const override
  {
    return m_pToolSvc->retrieve( typeAndName(), IAlgTool::interfaceID(), algTool, ToolHandleInfo::parent(),
                                 ToolHandleInfo::createIf() );
  }

private:
  //
  // Private data members
  //
  mutable ServiceHandle<IToolSvc> m_pToolSvc;
};

/** Helper class to construct ToolHandle instances for public tools via the
 *  auto registering constructor.
 */
template <class T>
class PublicToolHandle : public ToolHandle<T>
{
public:
  PublicToolHandle( bool createIf = true ) : ToolHandle<T>( nullptr, createIf ) {}
  PublicToolHandle( const char* toolTypeAndName, bool createIf = true )
      : PublicToolHandle{std::string{toolTypeAndName}, createIf}
  {
  }
  PublicToolHandle( const std::string& toolTypeAndName, bool createIf = true )
      : ToolHandle<T>( toolTypeAndName, nullptr, createIf )
  {
  }

  /// Copy constructor from a non const T to const T tool handle
  template <typename CT = T, typename NCT = typename std::remove_const<T>::type>
  PublicToolHandle( const PublicToolHandle<NCT>& other,
                    std::enable_if_t<std::is_const<CT>::value && !std::is_same<CT, NCT>::value>* = nullptr )
      : ToolHandle<T>( static_cast<const ToolHandle<NCT>&>( other ) )
  {
  }

  /// Autodeclaring constructor with property name, tool type/name and documentation.
  /// @note the use std::enable_if is required to avoid ambiguities
  template <class OWNER, typename = typename std::enable_if<std::is_base_of<IProperty, OWNER>::value>::type>
  inline PublicToolHandle( OWNER* owner, std::string name, std::string toolType, std::string doc = "" )
      : PublicToolHandle()
  {
    // convert name and type to a valid type/name string
    // - if type does not contain '/' use type/name
    // - otherwise type is already a type/name string
    if ( !toolType.empty() and toolType.find( '/' ) == std::string::npos ) {
      toolType += '/';
      toolType += name;
    }
    owner->declareTool( *this, std::move( toolType ) ).ignore();
    auto p = owner->OWNER::PropertyHolderImpl::declareProperty( std::move( name ), *this, std::move( doc ) );
    p->template setOwnerType<OWNER>();
  }
};

//-------------------------------------------------------------------------//

/** @class ToolHandleArray ToolHandle.h GaudiKernel/ToolHandle.h

    Array of Handles to be used in lieu of vector of naked pointers to tools.
    This allows better control through the framework of tool loading and usage.
    @parameter T is the AlgTool interface class (or concrete class) of
    the tool to use, and must derive from IAlgTool.

    @author Martin.Woudstra@cern.ch
*/

template <class T>
class ToolHandleArray : public ToolHandleInfo, public GaudiHandleArray<ToolHandle<T>>
{
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
  ToolHandleArray( const std::vector<std::string>& myTypesAndNames, const IInterface* parent = nullptr,
                   bool createIf = true )
      : ToolHandleInfo( parent, createIf )
      , GaudiHandleArray<ToolHandle<T>>( myTypesAndNames, ToolHandleInfo::toolComponentType( parent ),
                                         ToolHandleInfo::toolParentName( parent ) )
  {
  }

  /** Constructor which creates and empty list.
      @param parent   : passed on to ToolHandle, so has the same meaning as for ToolHandle
      @param createIf : passed on to ToolHandle, so has the same meaning as for ToolHandle
  */
  ToolHandleArray( const IInterface* parent = nullptr, bool createIf = true )
      : ToolHandleInfo( parent, createIf )
      , GaudiHandleArray<ToolHandle<T>>( ToolHandleInfo::toolComponentType( parent ),
                                         ToolHandleInfo::toolParentName( parent ) )
  {
  }

  /** Add a handle to the array with given tool type and name.
      This function overrides the one in GaudiHandleArray<T>, as this is a special case.
      The private/public choice and createIf is determined by what was given
      in the constructor of the ToolHandleArray. */
  bool push_back( const std::string& toolTypeAndName ) override
  {
    ToolHandle<T> handle( toolTypeAndName, ToolHandleInfo::parent(), ToolHandleInfo::createIf() );
    GaudiHandleArray<ToolHandle<T>>::push_back( handle );
    return true;
  }

  /** Ensure that for added handles the parent and creatIf are taken from this array. */
  bool push_back( const ToolHandle<T>& myHandle ) override { return push_back( myHandle.typeAndName() ); }

  /// Autodeclaring constructor with property name, tool type/name and documentation.
  /// @note the use std::enable_if is required to avoid ambiguities
  template <class OWNER, typename = std::enable_if_t<std::is_base_of<IProperty, OWNER>::value>>
  inline ToolHandleArray( OWNER* owner, std::string name, const std::vector<std::string>& typesAndNames = {},
                          std::string doc = "" )
      : ToolHandleArray( typesAndNames, owner )
  {
    owner->addToolsArray( *this );
    auto p = owner->OWNER::PropertyHolderImpl::declareProperty( std::move( name ), *this, std::move( doc ) );
    p->template setOwnerType<OWNER>();
  }
};

/** Helper class to construct ToolHandle instances for public tools via the
 *  auto registering constructor.
 */
template <class T>
class PublicToolHandleArray : public ToolHandleArray<T>
{
public:
  PublicToolHandleArray( bool createIf = true ) : ToolHandleArray<T>( nullptr, createIf ) {}
  PublicToolHandleArray( const std::vector<std::string>& typesAndNames, bool createIf = true )
      : ToolHandleArray<T>( typesAndNames, nullptr, createIf )
  {
  }

  /// Autodeclaring constructor with property name, tool type/name and documentation.
  /// @note the use std::enable_if is required to avoid ambiguities
  template <class OWNER, typename = typename std::enable_if<std::is_base_of<IProperty, OWNER>::value>::type>
  inline PublicToolHandleArray( OWNER* owner, std::string name, const std::vector<std::string>& typesAndNames = {},
                                std::string doc = "" )
      : PublicToolHandleArray( typesAndNames )
  {
    owner->addToolsArray( *this );
    auto p = owner->OWNER::PropertyHolderImpl::declareProperty( std::move( name ), *this, std::move( doc ) );
    p->template setOwnerType<OWNER>();
  }
};

template <class T>
inline std::ostream& operator<<( std::ostream& os, const ToolHandle<T>& handle )
{
  return operator<<( os, static_cast<const GaudiHandleInfo&>( handle ) );
}

template <class T>
inline std::ostream& operator<<( std::ostream& os, const ToolHandleArray<T>& handle )
{
  return operator<<( os, static_cast<const GaudiHandleInfo&>( handle ) );
}

#endif // ! GAUDIKERNEL_TOOLHANDLE_H
