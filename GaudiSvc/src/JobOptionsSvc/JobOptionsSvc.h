//$Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiSvc/src/JobOptionsSvc/JobOptionsSvc.h,v 1.12 2007/05/15 16:46:25 marcocle Exp $
// ===========================================================================
#ifndef GAUDISVC_JOBOPTIONSSVC_H
#define GAUDISVC_JOBOPTIONSSVC_H 1
// ===========================================================================
// Include files
// ===========================================================================
// STD & STL
// ===========================================================================
#include <string>
// ===========================================================================
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/PropertyMgr.h"

#include "Catalogue.h"
#include "JobOptionsCatalogue.h"

// ===========================================================================
class JobOptionsSvc : public extends2<Service, IProperty, IJobOptionsSvc> {
public:

  // unhides some of Service's methods
  using Service::setProperty;
  using Service::getProperty;
  using Service::getProperties;

  typedef std::vector<const Property*> PropertiesT;
  /// Constructor
	JobOptionsSvc(const std::string& name,ISvcLocator* svc);
	/// destructor
  virtual ~JobOptionsSvc() ;
  /// Service initialization method
	virtual StatusCode initialize ();
  /// Service finalization   method
	virtual StatusCode finalize   ();

  /// IJobOptionsSvc::setMyProperties
  virtual StatusCode setMyProperties( const std::string& client,
		                                IProperty* myInt );

  /// implementation of IJobOptionsSvc::addPropertyToCatalogue
  virtual StatusCode addPropertyToCatalogue( const std::string& client,
                                             const Property& property );

  /// implementation of IJobOptionsSvc::removePropertyFromCatalogue
  virtual StatusCode removePropertyFromCatalogue( const std::string& client,
                                                  const std::string& name );

  /// inmplementation of IJobOptionsSvc::getProperties
  virtual const PropertiesT* getProperties( const std::string& client) const;

  /// inmplementation of IJobOptionsSvc::getClients
  virtual std::vector<std::string> getClients() const;

  /** look for file 'File' into search path 'Path'
   *  and read it to update existing JobOptionsCatalogue
   *  @param  File file   name
   *  @param  Path search path
   *  @return status code
   */
  virtual StatusCode readOptions
  ( const std::string& file      ,
    const std::string& path = "" ) ;

  /// IProperty implementation (needed for initialisation)
  StatusCode setProperty(const Property& p);
  StatusCode getProperty(Property *p) const;

public:
  /// dump the content of Properties catalogue to std::ostream
  std::ostream& fillStream ( std::ostream& o ) const ;
  /// dump the content of Properties catalogue to the file
  void dump ( const std::string& file ) const ;
  /// dump the content of Properties catalogue to the predefined file
  void dump () ;
private:
  /// Fill parser catalogue before parse
  void fillParserCatalogue();
  /// Fill service catalogue after parse
  void fillServiceCatalogue();
  /// convert the severity
  MSG::Level convertSeverity(const Gaudi::Parsers::Message::Severity& severity);
private:
  std::string         m_source_path;
  std::string         m_source_type;
  std::string         m_dirSearchPath;
  // optional output file to dump all properties
  std::string         m_dump   ; ///< optional output file to dump all properties
  bool                m_dumped ;
  PropertyMgr         m_pmgr;
  Gaudi::Parsers::Catalogue m_pcatalogue;
  std::vector<std::string> m_included;
  JobOptionsCatalogue m_catalogue;
} ;

#endif
