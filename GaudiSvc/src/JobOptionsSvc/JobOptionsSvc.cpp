//$Id: JobOptionsSvc.cpp,v 1.32 2007/09/25 09:49:26 marcocle Exp $
// ============================================================================
// CVS tag $Name:  $ , version $Revision: 1.32 $
// ============================================================================
//  Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <fstream>
#include <string>
#include <algorithm>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/System.h"
#include "GaudiKernel/Parsers.h"
// ============================================================================
// Local
// ============================================================================
#include "ParserUtils.h"
#include "JobOptionsSvc.h"
// ============================================================================
/** @file
 *  Implementation file for class JobOptionsSvc
 */
// ============================================================================
// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_SERVICE_FACTORY(JobOptionsSvc)
// ============================================================================
JobOptionsSvc::JobOptionsSvc( const std::string& name,
                              ISvcLocator* svc)
  : base_class(name,svc)
  , m_source_path     ()
  , m_source_type     ()
  , m_dirSearchPath   ()
  , m_dump            ()
  , m_dumped ( false )
{
  std::string tmp ;
  tmp = System::getEnv ( "JOBOPTSEARCHPATH" ) ;
  if ( !tmp.empty() && ("UNKNOWN" != tmp) ) { m_dirSearchPath = tmp ; }
  tmp = System::getEnv ( "JOBOPTSDUMPFILE"  ) ;
  if ( !tmp.empty() && ("UNKNOWN" != tmp) ) { m_dump          = tmp ; }

  m_pmgr.declareProperty( "TYPE"       , m_source_type   ) ;
  m_pmgr.declareProperty( "PATH"       , m_source_path   ) ;
  m_pmgr.declareProperty( "SEARCHPATH" , m_dirSearchPath ) ;
  m_pmgr.declareProperty( "DUMPFILE"   , m_dump          ) ;
}
// ============================================================================
StatusCode JobOptionsSvc::setProperty( const Property &p )
{
	return m_pmgr.setProperty( p );
}
// ============================================================================
StatusCode JobOptionsSvc::getProperty( Property *p ) const
{
	return m_pmgr.getProperty( p );
}
// ============================================================================
StatusCode JobOptionsSvc::setMyProperties( const std::string& client,
                                           IProperty* myInt )
{
  const JobOptionsCatalogue::PropertiesT* props =
    m_catalogue.getProperties(client);

  if ( NULL == props ){ return StatusCode::SUCCESS; }

  bool fail = false;
  for ( std::vector<const Property*>::const_iterator cur = props->begin();
    cur != props->end(); cur++)
  {
    StatusCode sc = myInt->setProperty (**cur ) ;
    if ( sc.isFailure() )
    {
      MsgStream my_log( this->msgSvc(), this->name() );
      my_log
        << MSG::ERROR
        << "Unable to set the property '" << (*cur)->name() << "'"
        <<                        " of '" << client         << "'. "
        << "Check option and algorithm names, type and bounds."
        << endmsg;
      fail = true;
    }
  }
  return fail ? StatusCode::FAILURE : StatusCode::SUCCESS ;
}
// ============================================================================
StatusCode JobOptionsSvc::initialize()
{
  // Call base class initializer
  StatusCode sc = Service::initialize();
  if( !sc.isSuccess() ) return sc;
  // Read the job options if needed
  if ( this->m_source_type == "NONE" ) {
    sc =  StatusCode::SUCCESS;
  }
  else {
    sc = this->readOptions( m_source_path , m_dirSearchPath);
  }
  return sc;
}
// ============================================================================
StatusCode JobOptionsSvc::readOptions( const std::string& file ,
                                       const std::string& path )
{
  // use the default non-empty path (if any) if no path specified
  if ( path.empty() && !m_dirSearchPath.empty() )
  { return readOptions ( file , m_dirSearchPath ) ; }
  //
  MsgStream my_log( this->msgSvc(), this->name() );
  // --------------------------------------------------------------------------
  std::vector<Gaudi::Parsers::Message> messages;
  // --------------------------------------------------------------------------
  // fillParserCatalogue();
  // --------------------------------------------------------------------------
  my_log << MSG::DEBUG                             // debug
         << "Job-options read from the file "
         << "'" << file << "'"
         << MSG::INFO                              // switch
         << std::endl ;
  // create the parser & parse the file
  Gaudi::Parsers::Parser parser (m_pcatalogue, m_included, path, my_log.stream() ) ;
  StatusCode sc = Gaudi::Parsers::parse
    ( parser , file , messages ) ;
  my_log << endmsg ;

  // --------------------------------------------------------------------------
  if ( sc.isSuccess() )
  {
    my_log << MSG::INFO
           << "Job options successfully read in from " << file << endmsg;
    fillServiceCatalogue();
  }
  else
  {
    my_log << MSG::FATAL << "Job options errors."<< endmsg;
  }
  // ----------------------------------------------------------------------------
  for ( std::vector<Gaudi::Parsers::Message>::const_iterator
          cur=messages.begin(); cur!=messages.end();cur++)
  {
    my_log << convertSeverity(cur->severity()) << cur->message() << endmsg;
  }
  // ----------------------------------------------------------------------------
  return sc;
}
// ============================================================================
StatusCode JobOptionsSvc::addPropertyToCatalogue
( const std::string& client,
  const Property& property )
{
  Property* p = new StringProperty ( property.name(), "" ) ;
  if ( !property.load( *p ) ) { delete p ; return StatusCode::FAILURE ; }
  return m_catalogue.addProperty( client , p );
}
// ============================================================================
StatusCode
JobOptionsSvc::removePropertyFromCatalogue
( const std::string& client,
  const std::string& name )
{
  return m_catalogue.removeProperty(client,name);
}
// ============================================================================
const JobOptionsSvc::PropertiesT*
JobOptionsSvc::getProperties( const std::string& client) const
{
  return m_catalogue.getProperties(client);
}
// ============================================================================
std::vector<std::string>  JobOptionsSvc::getClients() const
{
  return m_catalogue.getClients();
}
// ============================================================================
/// Fill parser catalogue before parse
void JobOptionsSvc::fillParserCatalogue(){
  std::vector<std::string> clients = getClients();
  for(std::vector<std::string>::const_iterator client = clients.begin();
    client != clients.end(); client++){
       const PropertiesT* props = getProperties(*client);
       for(PropertiesT::const_iterator prop = props->begin();
        prop != props->end(); prop++){
         m_pcatalogue.addProperty(*client,(*prop)->name(),
           (*prop)->toString());
        }
   }
}
// ============================================================================
/// Fill service catalogue after parse
void JobOptionsSvc::fillServiceCatalogue()
{
  typedef Gaudi::Parsers::Catalogue::CatalogueT      _Catalogue ;
  typedef std::vector<Gaudi::Parsers::PropertyEntry> _Entries   ;
  _Catalogue catalogue = m_pcatalogue.catalogue();
  for  ( _Catalogue::const_iterator curObj = catalogue.begin() ;
         curObj!=catalogue.end(); ++curObj)
  {
    for ( _Entries::const_iterator curProp = curObj->second.begin() ;
          curProp!=curObj->second.end(); ++curProp)
    {
      StringProperty tmp (curProp->name(), curProp->value() ) ;
      addPropertyToCatalogue ( curObj->first , tmp ) ;
    }
  }
}
// ============================================================================
MSG::Level
JobOptionsSvc::convertSeverity
( const Gaudi::Parsers::Message::Severity& severity )
{
  switch ( severity )
  {
  case Gaudi::Parsers::Message::E_ERROR   :
    return MSG::FATAL   ;
  case Gaudi::Parsers::Message::E_WARNING :
    return MSG::WARNING ;
  case Gaudi::Parsers::Message::E_NOTICE  :
    return MSG::INFO    ;
  default:
    return MSG::DEBUG   ;
  }
}
// ============================================================================
StatusCode JobOptionsSvc::finalize()
{
  // dump the properties catalogue if needed
  if ( !m_dump.empty() && !m_dumped ) { dump() ;}
   // finalize the base class
  return Service::finalize() ;
}
// ============================================================================
void JobOptionsSvc::dump ()
{
  // avoid double dump
  if ( m_dump.empty() || m_dumped ) { return ; }               // RETURN
  dump ( m_dump ) ;
  // avoid double dump
  m_dumped = true ; ///< avoid double dump
}
// ============================================================================
void JobOptionsSvc::dump( const std::string& file ) const
{
  MsgStream log ( msgSvc() , name() ) ;
  std::ofstream out
    ( file.c_str() , std::ios_base::out | std::ios_base::trunc ) ;
  // perform the actual dumping
  if ( !out )
  {
    log << MSG::ERROR << "Unable to open dump-file \""+file+"\"" << endmsg ;
    return ;                                                   // RETURN
  }
  else
  { log << MSG::INFO << "Properties are dumped into \""+file+"\"" << endmsg ; }
  // perform the actual dump:
  fillStream ( out ) ;
}
// ============================================================================
std::ostream& JobOptionsSvc::fillStream ( std::ostream& o ) const
{ return  o << m_pcatalogue ; }
// ============================================================================
JobOptionsSvc::~JobOptionsSvc()
{
  // dump needed? @attention JobOptionsSvc::finalize is never invoked!
  if ( !m_dump.empty() && !m_dumped ) { dump() ;}
}
// ============================================================================


// ============================================================================
// The END
// ============================================================================



