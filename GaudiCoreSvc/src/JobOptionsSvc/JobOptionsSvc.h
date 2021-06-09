/***********************************************************************************\
* (c) Copyright 1998-2021 CERN for the benefit of the LHCb and ATLAS collaborations *
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

#include <Gaudi/Interfaces/IOptionsSvc.h>
#include <Gaudi/Property.h>
#include <GaudiKernel/IProperty.h>
#include <GaudiKernel/PropertyHolder.h>
#include <GaudiKernel/Service.h>
#include <GaudiKernel/StatusCode.h>

#include <map>
#include <vector>

namespace Gaudi {
  namespace Parsers {
    class Catalog;
  }
} // namespace Gaudi

class JobOptionsSvc : public extends<Service, Gaudi::Interfaces::IOptionsSvc> {
public:
  typedef std::vector<const Gaudi::Details::PropertyBase*> PropertiesT;

private:
  using StorageType = std::unordered_map<std::string, Gaudi::Details::WeakPropertyRef>;

  StorageType m_options;

  mutable std::map<std::string, std::unique_ptr<Gaudi::Details::PropertyBase>> m_old_iface_compat;
  mutable std::map<std::string, PropertiesT>                                   m_old_iface_compat_2;

protected:
  /// @{
  void        set( const std::string& key, const std::string& value ) override { m_options[key] = value; }
  std::string get( const std::string& key, const std::string& default_ = {} ) const override {
    auto item = m_options.find( key );
    return item != m_options.end() ? std::string{item->second} : default_;
  }
  std::string pop( const std::string& key, const std::string& default_ = {} ) override {
    std::string result = default_;

    auto item = m_options.find( key );
    if ( item != m_options.end() ) {
      result = std::move( item->second );
      m_options.erase( item );
    }
    return result;
  }
  bool has( const std::string& key ) const override { return m_options.find( key ) != m_options.end(); }
  std::vector<std::tuple<std::string, std::string>> items() const override {
    std::vector<std::tuple<std::string, std::string>> v;
    v.reserve( m_options.size() );
    std::for_each( begin( m_options ), end( m_options ), [&v]( const auto& item ) { v.emplace_back( item ); } );
    std::sort( begin( v ), end( v ) );
    return v;
  }
  bool isSet( const std::string& key ) const override {
    auto item = m_options.find( key );
    return item != m_options.end() && item->second.isSet();
  }

  void bind( const std::string& prefix, Gaudi::Details::PropertyBase* property ) override;

  void broadcast( const std::regex& filter, const std::string& value, OnlyDefaults defaults_only ) override;
  /// @}

public:
  // Constructor
  JobOptionsSvc( const std::string& name, ISvcLocator* svc );

  StatusCode initialize() override;
  StatusCode start() override;
  StatusCode stop() override;

private:
  /** look for file 'file' into search path 'path'
   *  and read it to update existing JobOptionsCatalogue
   *  @param file file   name
   *  @param path search path
   *  @return status code
   */
  StatusCode readOptions( const std::string& file, const std::string& path = "" );

  void fillServiceCatalog( const Gaudi::Parsers::Catalog& catalog );

  /// dump properties catalog to file
  void dump( const std::string& file, const Gaudi::Parsers::Catalog& catalog ) const;
  void dump( const std::string& file ) const;

private:
  Gaudi::Property<std::string> m_source_type{this, "TYPE"};
  Gaudi::Property<std::string> m_source_path{this, "PATH"};
  Gaudi::Property<std::string> m_dir_search_path{this, "SEARCHPATH"};
  Gaudi::Property<std::string> m_dump{this, "DUMPFILE"};
  Gaudi::Property<std::string> m_pythonAction{this, "PYTHONACTION"};
  Gaudi::Property<std::string> m_pythonParams{this, "PYTHONPARAMS"};

  Gaudi::Property<std::vector<std::pair<std::string, std::string>>> m_globalDefaultsProp{
      this, "GlobalDefaults", {}, "Allow definition of global defaults for properties as list of pairs (regex, value)"};

  Gaudi::Property<bool> m_reportUnused{this, "ReportUnused", false, "Print report of properties set, but not used"};

  std::vector<std::pair<std::regex, std::string>> m_globalDefaults;
};
#endif /* JOBOPTIONSSVC_H_ */
