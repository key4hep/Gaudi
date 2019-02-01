#pragma once
#include "GaudiKernel/IDataBroker.h"
#include "GaudiKernel/Service.h"
#include <Gaudi/Algorithm.h>
#include <stdexcept>

class HiveDataBrokerSvc final : public extends<Service, IDataBroker> {
public:
  using extends::extends;

  std::vector<Gaudi::Algorithm*> algorithmsRequiredFor( const DataObjIDColl&            requested,
                                                        const std::vector<std::string>& stoppers = {} ) const override;
  std::vector<Gaudi::Algorithm*> algorithmsRequiredFor( const Gaudi::Utils::TypeNameString& alg,
                                                        const std::vector<std::string>& stoppers = {} ) const override;

  StatusCode initialize() override;
  StatusCode start() override;
  StatusCode stop() override;
  StatusCode finalize() override;

private:
  Gaudi::Property<std::string>              m_dataLoader{this, "DataLoader", "",
                                            "Attribute any unmet input dependencies to this Algorithm"};
  Gaudi::Property<std::vector<std::string>> m_producers{
      this, "DataProducers", {}, "List of algorithms to be used to resolve data dependencies"};

  struct AlgEntry {
    SmartIF<IAlgorithm> ialg;
    Gaudi::Algorithm*   alg;
    std::set<AlgEntry*> dependsOn;
    int                 requestCount = 0;

    AlgEntry( SmartIF<IAlgorithm>&& p ) : ialg{std::move( p )}, alg{dynamic_cast<Gaudi::Algorithm*>( ialg.get() )} {
      if ( !alg ) throw std::runtime_error( "algorithm pointer == nullptr???" );
    }
  };

  std::vector<AlgEntry>
  instantiateAndInitializeAlgorithms( const std::vector<std::string>& names ) const; // algorithms must be fully
                                                                                     // initialized first, as
                                                                                     // doing so may create
                                                                                     // additional data
                                                                                     // dependencies...

  std::vector<AlgEntry> m_algorithms;

  mutable std::vector<AlgEntry> m_cfnodes;

  std::map<DataObjID, AlgEntry*> mapProducers( std::vector<AlgEntry>& algorithms ) const;

  std::map<DataObjID, AlgEntry*> m_dependencies;
};
