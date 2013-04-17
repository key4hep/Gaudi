// $Id:$
#ifndef JOBOPTIONSSVC_H_
#define JOBOPTIONSSVC_H_

#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/PropertyMgr.h"
#include "GaudiKernel/Service.h"

#include "SvcCatalog.h"

#include <vector>

namespace Gaudi { namespace Parsers {
  class Catalog;
}}


class JobOptionsSvc : public extends2<Service, IProperty, IJobOptionsSvc> {
 public:
    typedef std::vector<const Property*> PropertiesT;
  // unhides some of Service's methods
   using Service::setProperty;
   using Service::getProperty;
   using Service::getProperties;
  // Constructor
  JobOptionsSvc(const std::string& name,ISvcLocator* svc);
  /// destructor
  virtual ~JobOptionsSvc() {};
  virtual StatusCode initialize ();
  /// Service finalization   method
  virtual StatusCode finalize   ();

  /** Override default properties of the calling client
         @param client Name of the client algorithm or service
         @param me Address of the interface IProperty of the client
   */
  virtual StatusCode setMyProperties(const std::string& client,
      IProperty* me ) ;

  /// Add a property into the JobOptions catalog
  virtual StatusCode addPropertyToCatalogue( const std::string& client,
      const Property& property ) ;

  /// Remove a property from the JobOptions catalog
  virtual StatusCode removePropertyFromCatalogue( const std::string& client,
      const std::string& name );
  /// Get the properties associated to a given client
  virtual const std::vector<const Property*>*
  getProperties( const std::string& client) const;

  /// Get the list of clients
  virtual std::vector<std::string> getClients() const;

  /** look for file 'file' into search path 'path'
   *  and read it to update existing JobOptionsCatalogue
   *  @param file file   name
   *  @param path search path
   *  @return status code
   */
  virtual StatusCode readOptions ( const std::string& file,
      const std::string& path = "" );

  /// IProperty implementation (needed for initialisation)
  StatusCode setProperty(const Property& p);
  StatusCode getProperty(Property *p) const;
 private:
  void fillServiceCatalog(const Gaudi::Parsers::Catalog& catalog);
  void dump (const std::string& file,
      const Gaudi::Parsers::Catalog& catalog) const ;
  /// dump the content of Properties catalog to the predefined file

 private:
  PropertyMgr m_pmgr;
  std::string m_source_path;
  std::string m_source_type;
  std::string m_dir_search_path;
  std::string m_dump;
  SvcCatalog  m_svc_catalog;
};
#endif /* JOBOPTIONSSVC_H_ */
