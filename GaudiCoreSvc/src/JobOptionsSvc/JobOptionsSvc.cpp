// ============================================================================
// Boost:
// ============================================================================
#include <boost/foreach.hpp>
// ============================================================================
// Local:
// ============================================================================
#include "JobOptionsSvc.h"
#include "Analyzer.h"
#include "Messages.h"
#include "Catalog.h"
#include "Units.h"
#include "PragmaOptions.h"
#include "Node.h"
// ============================================================================
// Gaudi:
// ============================================================================
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/Property.h"

#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/System.h"
// ============================================================================
DECLARE_SERVICE_FACTORY(JobOptionsSvc)
// ============================================================================
// Namespace aliases:
// ============================================================================
namespace gp = Gaudi::Parsers;
// ============================================================================
JobOptionsSvc::JobOptionsSvc(const std::string& name,ISvcLocator* svc):
base_class(name,svc),
m_pmgr()
, m_source_path()
, m_source_type()
, m_dir_search_path()
, m_dump()
, m_dumped( false )
{
  std::string tmp ;
  tmp = System::getEnv ( "JOBOPTSEARCHPATH" ) ;
  if ( !tmp.empty() && ("UNKNOWN" != tmp) ) { m_dir_search_path = tmp ; }
  tmp = System::getEnv ( "JOBOPTSDUMPFILE"  ) ;
  if ( !tmp.empty() && ("UNKNOWN" != tmp) ) { m_dump = tmp ; }

  m_pmgr.declareProperty( "TYPE"       , m_source_type   ) ;
  m_pmgr.declareProperty( "PATH"       , m_source_path   ) ;
  m_pmgr.declareProperty( "SEARCHPATH" , m_dir_search_path ) ;
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
    sc = this->readOptions( m_source_path , m_dir_search_path);
  }
  return sc;
}

StatusCode JobOptionsSvc::finalize()
{
  // finalize the base class
  return Service::finalize();
}
// ============================================================================
StatusCode JobOptionsSvc::addPropertyToCatalogue
( const std::string& client,
  const Property& property )
{
  Property* p = new StringProperty ( property.name(), "" ) ;
  if ( !property.load( *p ) ) { delete p ; return StatusCode::FAILURE ; }
  return m_svc_catalog.addProperty( client , p );
}
// ============================================================================
StatusCode
JobOptionsSvc::removePropertyFromCatalogue
( const std::string& client,
  const std::string& name )
{
  return m_svc_catalog.removeProperty(client,name);
}
// ============================================================================
const JobOptionsSvc::PropertiesT*
JobOptionsSvc::getProperties( const std::string& client) const
{
  return m_svc_catalog.getProperties(client);
}
// ============================================================================
StatusCode JobOptionsSvc::setMyProperties( const std::string& client,
                                           IProperty* myInt )
{
  const SvcCatalog::PropertiesT* props =
    m_svc_catalog.getProperties(client);

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

/// Get the list of clients
std::vector<std::string> JobOptionsSvc::getClients() const {
  return m_svc_catalog.getClients();
}


void JobOptionsSvc::dump (const std::string& file,
    const gp::Catalog& catalog) const {
  MsgStream log ( msgSvc() , name() ) ;
  std::ofstream out
    ( file.c_str() , std::ios_base::out | std::ios_base::trunc ) ;
  // perform the actual dumping
  if ( !out ) {
    log << MSG::ERROR << "Unable to open dump-file \""+file+"\"" << endmsg ;
    return ;                                                   // RETURN
  }
  else {
    log << MSG::INFO << "Properties are dumped into \""+file+"\"" << endmsg ;
  }
  // perform the actual dump:
  out << catalog;
}

void JobOptionsSvc::fillServiceCatalog(const gp::Catalog& catalog) {
  BOOST_FOREACH(const gp::Catalog::value_type& client, catalog) {
    for (gp::Catalog::CatalogSet::mapped_type::const_iterator current
          = client.second.begin(); current != client.second.end();
                                                                  ++current) {
      StringProperty tmp (current->NameInClient(), current->ValueAsString()) ;
      addPropertyToCatalogue ( client.first , tmp ) ;
    }
  }
}

StatusCode JobOptionsSvc::readOptions ( const std::string& file,
       const std::string& path) {
    std::string search_path = path;
    if ( search_path.empty() && !m_dir_search_path.empty() )
    { search_path =  m_dir_search_path ; }
    //
    MsgStream my_log( this->msgSvc(), this->name() );
    if (UNLIKELY(outputLevel() <= MSG::DEBUG))
      my_log << MSG::DEBUG                             // debug
             << "Reading options from the file "
             << "'" << file << "'" << endmsg;
    gp::Messages messages(my_log);
    gp::Catalog catalog;
    gp::Units units;
    gp::PragmaOptions pragma;
    gp::Node ast;
    StatusCode sc = gp::ReadOptions(file, path, &messages, &catalog, &units,
        &pragma, &ast);

    // --------------------------------------------------------------------------
    if ( sc.isSuccess() )
    {
      if (pragma.IsPrintOptions()) {
        my_log << MSG::INFO << "Print options" << std::endl << catalog
            << endmsg;
      }
      if (pragma.IsPrintTree()) {
        my_log << MSG::INFO << "Print tree:" << std::endl << ast.ToString()
            << endmsg;
      }
      if (pragma.HasDumpFile()) {
        dump(pragma.dumpFile(), catalog);
      }
      my_log << MSG::INFO
             << "Job options successfully read in from " << file << endmsg;
      fillServiceCatalog(catalog);
    }
    else
    {
      my_log << MSG::FATAL << "Job options errors."<< endmsg;
    }
    // ----------------------------------------------------------------------------
    return sc;
}

