// $Id: HistogramPersistencySvc.cpp,v 1.9 2008/10/09 13:40:18 marcocle Exp $
// ============================================================================
//	HistogramPersistencySvc.cpp
//--------------------------------------------------------------------
//
//	Package    : System ( The LHCb Offline System)
//
//  Description: implementation of the Event data persistency service
//               This specialized service only deals with event related
//               data
//
//	Author     : M.Frank
//  History    :
// +---------+----------------------------------------------+---------
// |    Date |                 Comment                      | Who
// +---------+----------------------------------------------+---------
// | 29/10/98| Initial version                              | MF
// +---------+----------------------------------------------+---------
//
//====================================================================
#define  PERSISTENCYSVC_HISTOGRAMPERSISTENCYSVC_CPP
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/IDataProviderSvc.h"
// ============================================================================
// local
// ============================================================================
#include "HistogramPersistencySvc.h"
// ============================================================================
// AIDA
// ============================================================================
/// @FIXME: AIDA interfaces visibility
#define class class GAUDI_API
#include "AIDA/IBaseHistogram.h"
#undef class
// ============================================================================
// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT(HistogramPersistencySvc)

// ============================================================================
// Finalize the service.
StatusCode HistogramPersistencySvc::finalize()
{
  //
  MsgStream log ( msgSvc() , name() );
  if ( !(m_convert.empty() && m_exclude.empty()) )
  { // print message if any of the two properties is used
    log << MSG::INFO  << "Histograms Converted/Excluded: "
        << m_converted.size() << "/" << m_excluded.size() << endmsg ;
  }
  if (msgLevel(MSG::DEBUG)) {
    if ( !m_excluded.empty() )
    {
      log << MSG::DEBUG << "Excluded  Histos : #" << m_excluded.size() ;
      for ( Set::const_iterator item = m_excluded.begin() ;
          m_excluded.end() != item ; ++item )
      { log << std::endl << "  '" << (*item) << "'" ; }
      log << endmsg ;
    }
    //
    if ( !m_converted.empty() )
    {
      log << MSG::DEBUG << "Converted Histos : #" << m_converted.size() ;
      for ( Set::const_iterator item = m_converted.begin() ;
          m_converted.end() != item ; ++item )
      { log << std::endl << "  '" << (*item) << "'" ; }
      log << endmsg ;
    }
  }
  return PersistencySvc::finalize();
}
// ============================================================================
// Initialize the service.
// ============================================================================
StatusCode HistogramPersistencySvc::initialize()     {
  StatusCode status = PersistencySvc::initialize();
  if ( status.isSuccess() )   {
    status = reinitialize();
  }
  return status;
}
// ============================================================================
// Reinitialize the service.
// ============================================================================
StatusCode HistogramPersistencySvc::reinitialize()
{
  MsgStream log(msgSvc(), name());
  // Obtain the IProperty of the ApplicationMgr
  SmartIF<IProperty> prpMgr(serviceLocator());
  if ( !prpMgr.isValid() )   {
    log << MSG::FATAL << "IProperty interface not found in ApplicationMgr." << endmsg;
    return StatusCode::FAILURE;
  }
  else {
    setProperty(prpMgr->getProperty("HistogramPersistency")).ignore();
  }

  // To keep backward compatibility, we set the property of conversion service
  // into JobOptions catalogue
  if( m_outputFile != "" ) {
    SmartIF<IJobOptionsSvc> joptsvc(serviceLocator()->service("JobOptionsSvc"));
    if( joptsvc.isValid() ) {
      StringProperty p("OutputFile", m_outputFile);
      if ( m_histPersName == "ROOT" ) {
        joptsvc->addPropertyToCatalogue("RootHistSvc", p).ignore();
      } else if (m_histPersName == "HBOOK" ) {
        joptsvc->addPropertyToCatalogue("HbookHistSvc", p).ignore();
      }
    }
  }

  // Load the Histogram persistency service that's required as default
  setConversionSvc(0).ignore();
  if ( m_histPersName == "ROOT" ) {
    setConversionSvc(service("RootHistSvc")).ignore();
    if ( !conversionSvc() ) {
      return StatusCode::FAILURE;
    }
    enable(true);
  }
  else if ( m_histPersName == "HBOOK" ) {
    setConversionSvc(service("HbookHistSvc")).ignore();
    if ( !conversionSvc() ) {
      return StatusCode::FAILURE;
    }
    enable(true);
  }
  else if ( m_histPersName == "NONE" ) {
    enable(false);
    if ( m_warnings ) {
      log << MSG::WARNING << "Histograms saving not required." << endmsg;
    }
  }
  else {
    setConversionSvc(service(m_histPersName)).ignore();
    if ( !conversionSvc() ) {
      return StatusCode::FAILURE;
    }
    enable(true);
    if ( m_warnings ) {
      log << MSG::WARNING << "Unknown Histogram Persistency Mechanism " << m_histPersName << endmsg;
    }
  }
  return StatusCode::SUCCESS;
}
// ============================================================================
namespace
{
  // ==========================================================================
  /// invalid name
  const std::string s_NULL = "<NULL>" ;
  // ==========================================================================
  /** check the match of the full name and the pattern
   *  @param obj the object
   *  @param pat the pattern
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   */
  // ==========================================================================
  inline bool match
  ( const std::string& name ,
    const std::string& pat  )
  {
    // the most primitive match
    return std::string::npos != name.find ( pat );
  }
  // ==========================================================================
  /** get the ful name of data object
   *  @param obj the object
   *  @return the full name
   */
  inline const std::string& oname ( const DataObject* obj )
  {
    if ( 0 == obj ) { return s_NULL ; }
    const IRegistry* reg = obj->registry() ;
    return ( 0 == reg ) ? obj -> name () : reg -> identifier () ;
  }
  // ==========================================================================
  /** check the match of the full name of data object with the pattern
   *  @param obj the object
   *  @param pat the pattern
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   */
  inline bool match ( const DataObject*  obj ,
                      const std::string& pat )
  {
    if ( 0 == obj ) { return false ; }
    return match ( oname ( obj ) , pat ) ;
  }
  // ==========================================================================
}
// ============================================================================
// Convert the transient object to the requested representation.
// ============================================================================
StatusCode HistogramPersistencySvc::createRep
( DataObject*      pObj     ,
  IOpaqueAddress*& refpAddr )
{
  // enable the conversion
  enable ( true ) ;
  // conversion is possible ?
  if ( "NONE" == m_histPersName )
  {
    enable ( false ) ;
    return PersistencySvc::createRep ( pObj , refpAddr ) ;   // RETURN
  }
  // histogram ?
  if ( 0 != dynamic_cast<AIDA::IBaseHistogram*> ( pObj ) )
  {
    bool select = false ;
    // Empty ConvertHistos property means convert all
    if ( m_convert.empty() ) { select = true ; }
    else
    {
      for ( Strings::const_iterator item = m_convert.begin() ;
            m_convert.end() != item ; ++item )
      { if ( match ( pObj , *item ) ) { select = true ; break ; } }
    }
    // exclude ?
    for ( Strings::const_iterator item = m_exclude.begin() ;
          m_exclude.end() != item && select ; ++item )
    { if ( match ( pObj , *item ) ) { select = false ; break ; } }
    //
    enable ( select ) ;
    //
    const std::string& path = oname ( pObj ) ;
    //
    if ( !select ) { m_excluded.insert  ( path ) ; }
    else           { m_converted.insert ( path ) ; }
  }
  //
  return PersistencySvc::createRep ( pObj , refpAddr ) ;     // RETURN
}
// ============================================================================
// Standard Constructor
// ============================================================================
HistogramPersistencySvc::HistogramPersistencySvc
( const std::string& name ,
  ISvcLocator*       svc  )
  :  PersistencySvc(name, svc)
  //
  , m_convert   ()
  , m_exclude   ()
  , m_converted ()
  , m_excluded  ()
  //
{
  std::vector<std::string> defServices;
  defServices.push_back("RootHistSvc");
  m_svcNames.set(defServices);
  declareProperty ("HistogramPersistency", m_histPersName = "");
  declareProperty ("OutputFile", m_outputFile = "");
  //
  declareProperty
    ("ConvertHistos" , m_convert ,
     "The list of patterns to be accepted for conversion" ) ;
  //
  declareProperty
    ("ExcludeHistos" , m_exclude ,
     "The list of patterns to be excluded for conversion" ) ;
  declareProperty("Warnings",m_warnings=true,
		  "Set this property to false to suppress warning messages");
}
// ============================================================================
// Standard Destructor
// ============================================================================
HistogramPersistencySvc::~HistogramPersistencySvc()   {}
// ============================================================================


// ============================================================================
// The END
// ============================================================================
