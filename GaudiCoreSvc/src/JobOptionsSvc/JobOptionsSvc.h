/***********************************************************************************\
* (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef JOBOPTIONSSVC_H_
#define JOBOPTIONSSVC_H_

#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/PropertyHolder.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/StatusCode.h"

#include "SvcCatalog.h"

#include <vector>

namespace Gaudi {
  namespace Parsers {
    class Catalog;
  }
} // namespace Gaudi

class JobOptionsSvc : public extends<Service, IJobOptionsSvc> {
public:
  typedef std::vector<const Gaudi::Details::PropertyBase*> PropertiesT;
  // Constructor
  JobOptionsSvc( const std::string& name, ISvcLocator* svc );

  StatusCode initialize() override;
  StatusCode start() override;

  /** Override default properties of the calling client
         @param client Name of the client algorithm or service
         @param me Address of the interface IProperty of the client
   */
  StatusCode setMyProperties( const std::string& client, IProperty* me ) override;

  /// Add a property into the JobOptions catalog
  StatusCode addPropertyToCatalogue( const std::string& client, const Gaudi::Details::PropertyBase& property ) override;

  /// Remove a property from the JobOptions catalog
  StatusCode removePropertyFromCatalogue( const std::string& client, const std::string& name ) override;

  using Service::getProperties;
  /// Get the properties associated to a given client
  const std::vector<const Gaudi::Details::PropertyBase*>* getProperties( const std::string& client ) const override;

  using Service::getProperty;
  /// Get a property for a client
  const Gaudi::Details::PropertyBase* getClientProperty( const std::string& client,
                                                         const std::string& name ) const override {
    return m_svc_catalog.getProperty( client, name );
  }
  /// Get the list of clients
  std::vector<std::string> getClients() const override;

  /** look for file 'file' into search path 'path'
   *  and read it to update existing JobOptionsCatalogue
   *  @param file file   name
   *  @param path search path
   *  @return status code
   */
  StatusCode readOptions( const std::string& file, const std::string& path = "" ) override;

private:
  void fillServiceCatalog( const Gaudi::Parsers::Catalog& catalog );

  /// dump properties catalog to file
  template <typename C>
  void dump( const std::string& file, const C& catalog ) const;

private:
  Gaudi::Property<std::string> m_source_type{this, "TYPE"};
  Gaudi::Property<std::string> m_source_path{this, "PATH"};
  Gaudi::Property<std::string> m_dir_search_path{this, "SEARCHPATH"};
  Gaudi::Property<std::string> m_dump{this, "DUMPFILE"};
  Gaudi::Property<std::string> m_pythonAction{this, "PYTHONACTION"};
  Gaudi::Property<std::string> m_pythonParams{this, "PYTHONPARAMS"};

  SvcCatalog m_svc_catalog;
};
#endif /* JOBOPTIONSSVC_H_ */
