#ifndef JOBOPTIONSSVC_H_
#define JOBOPTIONSSVC_H_

#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/PropertyHolder.h"
#include "GaudiKernel/Service.h"

#include "SvcCatalog.h"

#include <vector>

namespace Gaudi { namespace Parsers {
  class Catalog;
}}


class JobOptionsSvc : public extends<Service,
                                     IJobOptionsSvc> {
 public:
    typedef std::vector<const Property*> PropertiesT;
  // Constructor
  JobOptionsSvc(const std::string& name,ISvcLocator* svc);
  /// destructor
  ~JobOptionsSvc() override = default;
  StatusCode initialize () override;

  /** Override default properties of the calling client
         @param client Name of the client algorithm or service
         @param me Address of the interface IProperty of the client
   */
  StatusCode setMyProperties(const std::string& client,
      IProperty* me ) override ;

  /// Add a property into the JobOptions catalog
  StatusCode addPropertyToCatalogue( const std::string& client,
      const Property& property ) override;

  /// Remove a property from the JobOptions catalog
  StatusCode removePropertyFromCatalogue( const std::string& client,
      const std::string& name ) override;

  using Service::getProperties;
  /// Get the properties associated to a given client
  const std::vector<const Property*>*
  getProperties( const std::string& client) const override;

  using Service::getProperty;
  /// Get a property for a client
  const Property* getClientProperty(const std::string& client, const std::string& name) const override {
    return m_svc_catalog.getProperty(client, name);
  }
  /// Get the list of clients
  std::vector<std::string> getClients() const override;

  /** look for file 'file' into search path 'path'
   *  and read it to update existing JobOptionsCatalogue
   *  @param file file   name
   *  @param path search path
   *  @return status code
   */
  StatusCode readOptions ( const std::string& file,
      const std::string& path = "" ) override;

 private:
  void fillServiceCatalog(const Gaudi::Parsers::Catalog& catalog);
  void dump (const std::string& file,
      const Gaudi::Parsers::Catalog& catalog) const ;
  /// dump the content of Properties catalog to the predefined file

 private:
  StringProperty m_source_type{this, "TYPE"};
  StringProperty m_source_path{this, "PATH"};
  StringProperty m_dir_search_path{this, "SEARCHPATH"};
  StringProperty m_dump{this, "DUMPFILE"};
  StringProperty m_pythonAction{this, "PYTHONACTION"};
  StringProperty m_pythonParams{this, "PYTHONPARAMS"};

  SvcCatalog  m_svc_catalog;
};
#endif /* JOBOPTIONSSVC_H_ */
