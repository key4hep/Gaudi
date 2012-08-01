// $Id: GaudiCommonImp.h,v 1.11 2008/10/10 08:06:33 marcocle Exp $
// ============================================================================
#ifndef GAUDIALG_GAUDICOMMONIMP_H
#define GAUDIALG_GAUDICOMMONIMP_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GetData.h"
#include "GaudiAlg/GaudiCommon.h"
// ============================================================================
/** @file
 *  The implementation of inline/templated methods for class GaudiCommon
 *  @see    GaudiCommon
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   2004-01-19
 */
// ============================================================================
// Returns the full correct event location given the rootInTes settings
// ============================================================================
template < class PBASE >
inline const std::string
GaudiCommon<PBASE>::fullTESLocation( const std::string & location,
                                     const bool useRootInTES ) const
{
  // The logic is:
  // if no R.I.T., give back location
  // if R.I.T., this is the mapping:
  // (note that R.I.T. contains a trailing '/')
  //  location       -> result
  //  -------------------------------------------------
  //  ""             -> R.I.T.[:-1]      ("rit")
  //  "/Event"       -> R.I.T.[:-1]      ("rit")
  //  "/Event/MyObj" -> R.I.T. + "MyObj" ("rit/MyObj")
  //  "MyObj"        -> R.I.T. + "MyObj" ("rit/MyObj")
  return ( !useRootInTES || rootInTES().empty() ?
           location
         :
           location.empty() || ( location == "/Event" ) ?
             rootInTES().substr(0,rootInTES().size()-1)
           :
             0 == location.find("/Event/") ?
               rootInTES() + location.substr(7)
             :
               rootInTES() + location );
}
// ============================================================================
// Templated access to the data in Gaudi Transient Store
// ============================================================================
template < class PBASE >
template < class TYPE  >
inline typename Gaudi::Utils::GetData<TYPE>::return_type
GaudiCommon<PBASE>::get( IDataProviderSvc*  service ,
                         const std::string& location,
                         const bool useRootInTES ) const
{
  // check the environment
  Assert( 0 != service ,    "get():: IDataProvider* points to NULL!"      ) ;
  // get the helper object:
  Gaudi::Utils::GetData<TYPE> getter ;
  return getter ( *this    ,
                  service  ,
                  fullTESLocation ( location , useRootInTES ) ) ;
}
// ============================================================================
// Templated access to the data in Gaudi Transient Store, no check on the content
// ============================================================================
template < class PBASE >
template < class TYPE  >
inline typename Gaudi::Utils::GetData<TYPE>::return_type
GaudiCommon<PBASE>::getIfExists( IDataProviderSvc*  service ,
                                 const std::string& location,
                                 const bool useRootInTES ) const
{
  // check the environment
  Assert( 0 != service ,    "get():: IDataProvider* points to NULL!"      ) ;
  // get the helper object:
  Gaudi::Utils::GetData<TYPE> getter ;
  return getter ( *this    ,
                  service  ,
                  fullTESLocation ( location , useRootInTES ),
                  false) ;
}
// ============================================================================
// check the existence of objects in Gaudi Transient Store
// ============================================================================
template < class PBASE >
template < class TYPE  >
inline bool GaudiCommon<PBASE>::exist( IDataProviderSvc*  service  ,
                                       const std::string& location ,
                                       const bool useRootInTES ) const
{
  // check the environment
  Assert( 0 != service , "exist():: IDataProvider* points to NULL!"      ) ;
  // check the data object
  Gaudi::Utils::CheckData<TYPE> checker ;
  return checker ( service,
                   fullTESLocation ( location , useRootInTES ) ) ;
}
// ============================================================================
// get the existing object from Gaudi Event Transient store
// or create new object register in in TES and return if object
// does not exist
// ============================================================================
template <class PBASE>
template <class TYPE,class TYPE2>
inline typename Gaudi::Utils::GetData<TYPE>::return_type
GaudiCommon<PBASE>::getOrCreate( IDataProviderSvc*  service  ,
                                 const std::string& location ,
                                 const bool useRootInTES  ) const
{
  // check the environment
  Assert ( 0 != service , "getOrCreate():: svc points to NULL!" ) ;
  // get the helper object
  Gaudi::Utils::GetOrCreateData<TYPE,TYPE2> getter ;
  return getter ( *this                                     ,
                  service                                   ,
                  fullTESLocation( location, useRootInTES ) ,
                  location                                  ) ;
}
// ============================================================================
// the useful method for location of tools.
// ============================================================================
template < class PBASE >
template < class TOOL  >
inline TOOL* GaudiCommon<PBASE>::tool( const std::string& type           ,
                                       const std::string& name           ,
                                       const IInterface*  parent         ,
                                       bool               create         ) const
{
  TOOL* Tool = 0 ;
  // for empty names delegate to another method
  if ( name.empty() )
  {
    Tool = tool<TOOL>( type , parent , create ) ;
  }
  else
  {
    Assert( this->toolSvc() != 0, "tool():: IToolSvc* points to NULL!" ) ;
    // get the tool from Tool Service
    const StatusCode sc =
      this->toolSvc()->retrieveTool ( type , name , Tool , parent , create ) ;
    if ( sc.isFailure() )
    { Exception("tool():: Could not retrieve Tool '" + type + "'/'" + name + "'", sc ) ; }
    if ( 0 == Tool )
    { Exception("tool():: Could not retrieve Tool '" + type + "'/'" + name + "'"     ) ; }
    // add the tool into list of known tools to be properly released
    addToToolList( Tool );
  }
  // return *VALID* located tool
  return Tool ;
}
// ============================================================================
// the useful method for location of tools.
// ============================================================================
template < class PBASE >
template < class TOOL  >
inline TOOL* GaudiCommon<PBASE>::tool( const std::string& type   ,
                                       const IInterface*  parent ,
                                       bool               create ) const
{
  // check the environment
  Assert ( PBASE::toolSvc() != 0, "IToolSvc* points to NULL!" );
  // retrieve the tool from Tool Service
  TOOL* Tool = 0 ;
  const StatusCode sc =
    this->toolSvc() -> retrieveTool ( type, Tool, parent , create   );
  if ( sc.isFailure() )
  { Exception("tool():: Could not retrieve Tool '" + type + "'", sc ) ; }
  if ( 0 == Tool )
  { Exception("tool():: Could not retrieve Tool '" + type + "'"     ) ; }
  // add the tool into the list of known tools to be properly released
  addToToolList( Tool );
  // return *VALID* located tool
  return Tool ;
}
// ============================================================================
// the useful method for location of services
// ============================================================================
template < class PBASE   >
template < class SERVICE >
inline SmartIF<SERVICE> GaudiCommon<PBASE>::svc( const std::string& name   ,
                                                  const bool         create ) const
{
  Assert ( this->svcLoc() != 0, "ISvcLocator* points to NULL!" );
  SmartIF<SERVICE> s;
  // check if we already have this service
  Services::iterator it = m_services.find(name);
  if (it != m_services.end()) {
    // Try to get the requested interface
    s = it->second;
    // check the results
    if ( !s.isValid() ) {
      Exception ("svc():: Could not retrieve Svc '" + name + "'", StatusCode::FAILURE);
    }
  } else {
    SmartIF<IService>& baseSvc = this->svcLoc()->service(name, create);
    // Try to get the requested interface
    s = baseSvc;
    // check the results
    if ( !baseSvc.isValid() || !s.isValid() ) {
      Exception ("svc():: Could not retrieve Svc '" + name + "'", StatusCode::FAILURE);
    }
    // add the tool into list of known tools, to be properly released
    addToServiceList(baseSvc);
  }
  // return *VALID* located service
  return s;
}
// ============================================================================
// Short-cut  to get a pointer to the UpdateManagerSvc
// ============================================================================
template <class PBASE>
inline IUpdateManagerSvc *
GaudiCommon<PBASE>::updMgrSvc() const
{
  if ( !m_updMgrSvc )
  { m_updMgrSvc = svc<IUpdateManagerSvc>("UpdateManagerSvc",true); }
  return m_updMgrSvc ;
}
// ============================================================================
// predefined configurable message stream for the effective printouts
// ============================================================================
template <class PBASE>
inline MsgStream&
GaudiCommon<PBASE>::msgStream ( const MSG::Level level ) const
{
  if ( !m_msgStream )
  { m_msgStream = new MsgStream ( PBASE::msgSvc() , this->name() ) ; }
  return *m_msgStream << level ;
}
// ============================================================================
// Assertion - throw exception, if condition is not fulfilled
// ============================================================================
template <class PBASE>
inline void GaudiCommon<PBASE>::Assert( const bool         ok  ,
                                        const std::string& msg ,
                                        const StatusCode   sc  ) const
{
  if (!ok) Exception( msg , sc );
}
// ============================================================================
// Delete the current message stream object
// ============================================================================
template <class PBASE>
inline void GaudiCommon<PBASE>::resetMsgStream() const
{
  if ( 0 != m_msgStream ) { delete m_msgStream; m_msgStream = 0; }
}
// ============================================================================
// Assertion - throw exception, if condition is not fulfilled
// ============================================================================
template <class PBASE>
inline void GaudiCommon<PBASE>::Assert( const bool        ok  ,
                                        const char*       msg ,
                                        const StatusCode  sc  ) const
{
  if (!ok) Exception( msg , sc );
}
// ============================================================================
/** @def ALG_ERROR
 *  Small and simple macro to add into error message the file name
 *  and the line number for easy location of the problematic lines.
 *
 *  @code
 *
 *  if ( a < 0 ) { return ALG_ERROR( "'a' is negative" , 301 ) ; }
 *
 *  @endcode
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date 2002-10-29
 */
// ============================================================================
#define ALG_ERROR( message , code )                                     \
  ( Error( message                                   +                  \
           std::string             ( " [ at line " ) +                  \
           GaudiAlg::fileLine      (   __LINE__    ) +                  \
           std::string             ( " in file '"  ) +                  \
           std::string             (   __FILE__    ) + "']" , code ) )

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_GAUDICOMMONIMP_H
