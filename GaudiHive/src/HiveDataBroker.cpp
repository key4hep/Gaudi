/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "GraphDumper.h"

#include <Gaudi/Algorithm.h>
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/IAlgManager.h>
#include <GaudiKernel/IDataBroker.h>
#include <GaudiKernel/Service.h>
#include <GaudiKernel/System.h>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <iomanip>
#include <ranges>
#include <stdexcept>

namespace {
  struct AlgEntry {
    size_t                        index;
    SmartIF<IAlgorithm>           ialg;
    Gaudi::Algorithm*             alg;
    std::set<AlgEntry const*>     dependsOn;
    std::vector<DataObjID const*> inputs;
    std::vector<DataObjID const*> outputs;

    friend bool operator<( AlgEntry const& lhs, AlgEntry const& rhs ) { return lhs.index < rhs.index; }

    friend bool operator==( AlgEntry const& lhs, AlgEntry const& rhs ) { return lhs.index == rhs.index; }

    AlgEntry( size_t i, SmartIF<IAlgorithm>&& p )
        : index{ i }, ialg{ std::move( p ) }, alg{ dynamic_cast<Gaudi::Algorithm*>( ialg.get() ) } {
      if ( !alg ) throw std::runtime_error( "algorithm pointer == nullptr???" );
      // gather _all_ the inputs and outputs in a well-defined, reproducible manner,
      // removing any duplication (sometimes, extra{Out,In}putDeps entries will already appear in {Out,In}putDataObjs)
      constexpr auto gather = []( auto& c, auto const& in1, auto const& in2 ) {
        for ( const DataObjID& id : in1 ) c.push_back( &id );
        for ( const DataObjID& id : in2 ) c.push_back( &id );
        constexpr auto by_key = []( const DataObjID* id ) { return id->fullKey(); };
        std::ranges::sort( c, std::less{}, by_key );
        auto od = std::ranges::unique( c, std::equal_to{}, by_key );
        c.erase( od.begin(), od.end() );
      };
      gather( outputs, alg->outputDataObjs(), alg->extraOutputDeps() );
      gather( inputs, alg->inputDataObjs(), alg->extraInputDeps() );
    }
  };

  template <std::ranges::range R>
    requires std::common_reference_with<std::ranges::range_reference_t<R>, const AlgEntry&>
  void dumpGraphFile( std::string const& fname, R const& algorithms ) {
    Gaudi::Hive::Graph g{ fname };

    // loop over all algorithms to create nodes
    for ( const AlgEntry& entry : algorithms ) { g.addNode( entry.alg->name(), std::to_string( entry.index ) ); }

    // loop over all algorithms to create list of outputs with corresponding alg indexes
    std::unordered_map<std::string, size_t> output2Idx;
    for ( const AlgEntry& entry : algorithms ) {
      for ( const auto* id : entry.outputs ) { output2Idx[id->key()] = entry.index; }
    }

    // loop over all algorithms to create edges
    for ( const AlgEntry& entry : algorithms ) {
      for ( const auto* id : entry.inputs ) {
        g.addEdge( entry.alg->name(), std::to_string( entry.index ), id->key(), std::to_string( output2Idx[id->key()] ),
                   id->key() );
      }
    }
  }

  struct AlgorithmRepr {
    const Gaudi::Algorithm& parent;

    friend std::ostream& operator<<( std::ostream& s, const AlgorithmRepr& a ) {
      std::string typ = System::typeinfoName( typeid( a.parent ) );
      s << typ;
      if ( a.parent.name() != typ ) s << "/" << a.parent.name();
      return s;
    }
  };

  // Used for making debugging dumps.
  template <typename T>
  std::vector<const T*> sorted_( const std::set<T*>& s ) {
    std::vector<const T*> v{ s.begin(), s.end() };
    std::sort( v.begin(), v.end(), []( const auto* lhs, const auto* rhs ) { return *lhs < *rhs; } );
    return v;
  }

  SmartIF<IAlgorithm> createAlgorithm( IAlgManager& am, const std::string& type, const std::string& name ) {
    // Maybe modify the AppMgr interface to return Algorithm* ??
    IAlgorithm* tmp = nullptr;
    StatusCode  sc  = am.createAlgorithm( type, name, tmp );
    return sc.isSuccess() ? dynamic_cast<Gaudi::Algorithm*>( tmp ) : nullptr;
  }
} // namespace

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
  Gaudi::Property<std::string>              m_dataLoader{ this, "DataLoader", "",
                                             "Attribute any unmet input dependencies to this Algorithm" };
  Gaudi::Property<std::vector<std::string>> m_producers{
      this, "DataProducers", {}, "List of algorithms to be used to resolve data dependencies" };

  Gaudi::Property<std::string> m_dataDepsGraphFile{
      this, "DataDepsGraphFile", "",
      "Name of the output file (.dot or .md extensions allowed) containing the data dependency graph. If empty, no "
      "graph is dumped" };

  std::map<std::string, AlgEntry>
  instantiateAndInitializeAlgorithms( const std::vector<std::string>& names ) const; // algorithms must be fully
                                                                                     // initialized first, as
                                                                                     // doing so may create
                                                                                     // additional data
                                                                                     // dependencies...

  std::map<std::string, AlgEntry> m_algorithms;

  std::map<DataObjID, AlgEntry const*> mapProducers( std::map<std::string, AlgEntry>& algorithms ) const;

  std::map<DataObjID, AlgEntry const*> m_dependencies;

  void visit( AlgEntry const& alg, std::vector<std::string> const& stoppers, std::vector<Gaudi::Algorithm*>& sorted,
              std::vector<bool>& visited, std::vector<bool>& visiting ) const;
};

DECLARE_COMPONENT( HiveDataBrokerSvc )

StatusCode HiveDataBrokerSvc::initialize() {
  return Service::initialize().andThen( [&] {
    // populate m_algorithms
    m_algorithms = instantiateAndInitializeAlgorithms( m_producers );

    // warn about non-reentrant algorithms
    std::ranges::for_each( m_algorithms | std::ranges::views::transform( []( const auto& entry ) -> decltype( auto ) {
                             return entry.second.alg;
                           } ) |
                               std::ranges::views::filter( []( const auto* alg ) { return alg->cardinality() > 0; } ),
                           [&]( const Gaudi::Algorithm* alg ) {
                             this->warning() << "non-reentrant algorithm: " << AlgorithmRepr{ *alg } << endmsg;
                           } );
    //== Print the list of the created algorithms
    if ( msgLevel( MSG::DEBUG ) ) {
      debug() << "Available DataProducers:\n";
      std::ranges::for_each( m_algorithms | std::ranges::views::transform( []( const auto& entry ) -> decltype( auto ) {
                               return entry.second.alg;
                             } ),
                             [&]( const Gaudi::Algorithm* alg ) {
                               this->debug() << "  " << AlgorithmRepr{ *alg } << " " << alg->outputDataObjs() << " "
                                             << alg->extraOutputDeps() << endmsg;
                             } );
    }

    // populate m_dependencies and set AlgEntry::dependsOn
    m_dependencies = mapProducers( m_algorithms );
  } );
}

StatusCode HiveDataBrokerSvc::start() {

  StatusCode ss = Service::start();
  if ( !ss.isSuccess() ) return ss;

  // sysStart for m_algorithms
  for ( auto& [name, algEntry] : m_algorithms ) {
    ss = algEntry.alg->sysStart();
    if ( ss.isFailure() ) {
      error() << "Unable to start Algorithm: " << name << endmsg;
      return ss;
    }
  }
  return ss;
}

StatusCode HiveDataBrokerSvc::stop() {
  StatusCode ss = Service::stop();
  if ( !ss.isSuccess() ) return ss;

  // sysStart for m_algorithms
  for ( auto& [name, algEntry] : m_algorithms ) {
    ss = algEntry.alg->sysStop();
    if ( ss.isFailure() ) {
      error() << "Unable to stop Algorithm: " << name << endmsg;
      return ss;
    }
  }
  return ss;
}

StatusCode HiveDataBrokerSvc::finalize() {
  for ( auto& [name, algEntry] : m_algorithms ) {
    algEntry.alg->sysFinalize().ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
  }
  m_algorithms.clear();
  return Service::finalize();
}

std::map<std::string, AlgEntry>
HiveDataBrokerSvc::instantiateAndInitializeAlgorithms( const std::vector<std::string>& names ) const {
  std::map<std::string, AlgEntry> algorithms;

  //= Get the Application manager, to see if algorithm exist
  auto   appMgr = service<IAlgManager>( "ApplicationMgr" );
  size_t index  = 0;
  for ( const std::string& item : names ) {
    const Gaudi::Utils::TypeNameString tn( item );

    //== Check wether the specified algorithm already exists. If not, create it
    SmartIF<IAlgorithm> myIAlg = appMgr->algorithm( item, false ); // do not create it now
    if ( !myIAlg ) {
      myIAlg = createAlgorithm( *appMgr, tn.type(), tn.name() );
    } else {
      // when the algorithm is not created, the ref count is short by one, so we
      // have to fix it.
      myIAlg->addRef();
    }

    if ( !myIAlg ) {
      throw GaudiException{ "Failed to create " + boost::lexical_cast<std::string>( item ), __func__,
                            StatusCode::FAILURE };
    }

    // propagate the sub-algorithm into own state.
    StatusCode sc = myIAlg->sysInitialize();
    if ( sc.isFailure() ) {
      throw GaudiException{ "Failed to initialize " + boost::lexical_cast<std::string>( item ), __func__,
                            StatusCode::FAILURE };
    }

    algorithms.emplace( tn.name(), AlgEntry{ index++, std::move( myIAlg ) } );
  }

  return algorithms;
}

std::map<DataObjID, AlgEntry const*>
HiveDataBrokerSvc::mapProducers( std::map<std::string, AlgEntry>& algorithms ) const {
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "Data Dependencies for Algorithms:";
    for ( const auto& [name, entry] : m_algorithms ) {
      debug() << "\n " << name << " :";
      for ( const auto* id : entry.inputs ) { debug() << "\n    o INPUT  " << id->key(); }
      for ( const auto* id : entry.outputs ) { debug() << "\n    o OUTPUT " << id->key(); }
    }
    debug() << endmsg;
  }

  // If requested, dump a graph of the data dependencies in a .dot or .md file
  if ( !m_dataDepsGraphFile.empty() ) {
    info() << "Dumping data dependencies graph to file: " << m_dataDepsGraphFile.value() << endmsg;
    dumpGraphFile( m_dataDepsGraphFile, m_algorithms | std::views::values );
  }

  // figure out all outputs
  std::map<DataObjID, const AlgEntry*> producers;
  for ( auto& [name, alg] : algorithms ) {
    for ( auto id : alg.outputs ) {
      auto r = producers.emplace( *id, &alg );
      if ( !r.second ) {
        throw GaudiException( "multiple algorithms declare " + id->key() + " as output (" + name + " and " +
                                  producers[*id]->alg->name() + " at least). This is not allowed",
                              __func__, StatusCode::FAILURE );
      }
    }
  }

  // resolve dependencies
  for ( auto& [name, algEntry] : algorithms ) {
    for ( const DataObjID* idp : algEntry.inputs ) {
      DataObjID id        = *idp;
      auto      iproducer = producers.find( id );
      if ( iproducer != producers.end() ) {
        algEntry.dependsOn.insert( iproducer->second );
      } else {
        std::ostringstream error_message;
        error_message << "\nUnknown requested input by " << AlgorithmRepr{ *( algEntry.alg ) } << " : "
                      << std::quoted( id.key(), '\'' ) << ".\n";
        error_message << "You can set the OutputLevel of HiveDataBrokerSvc to DEBUG to get a list of inputs and "
                         "outputs of every registered algorithm.\n";
        throw GaudiException( error_message.str(), __func__, StatusCode::FAILURE );
        // TODO: assign to dataloader!
        // algEntry.dependsOn.insert(dataloader.alg);
        // dataloader.data.emplace( id ); // TODO: we may ask to much of the
        // dataloader this way...
      }
    }
  }
  return producers;
}

/// Implements DFS topological sorting
void HiveDataBrokerSvc::visit( AlgEntry const& alg, std::vector<std::string> const& stoppers,
                               std::vector<Gaudi::Algorithm*>& sorted, std::vector<bool>& visited,
                               std::vector<bool>& visiting ) const {
  assert( visited.size() == m_algorithms.size() );
  assert( visiting.size() == m_algorithms.size() );
  if ( visited[alg.index] ) { return; }
  if ( visiting[alg.index] ) { throw GaudiException( "Cycle detected ", __func__, StatusCode::FAILURE ); }

  if ( std::none_of( std::begin( stoppers ), std::end( stoppers ),
                     [alg]( auto& stopper ) { return alg.alg->name() == stopper; } ) ) {
    visiting[alg.index] = true;
    for ( auto* dep : sorted_( alg.dependsOn ) ) { visit( *dep, stoppers, sorted, visited, visiting ); }
    visiting[alg.index] = false;
  }

  visited[alg.index] = true;
  sorted.push_back( alg.alg );
}

std::vector<Gaudi::Algorithm*>
HiveDataBrokerSvc::algorithmsRequiredFor( const DataObjIDColl&            requested,
                                          const std::vector<std::string>& stoppers ) const {
  std::vector<Gaudi::Algorithm*> result;

  std::vector<const AlgEntry*> deps;
  deps.reserve( requested.size() );

  // start with seeding from the initial request
  for ( const auto& id : requested ) {
    auto i = m_dependencies.find( id );
    if ( i == m_dependencies.end() )
      throw GaudiException( "unknown requested input: " + id.key(), __func__, StatusCode::FAILURE );
    deps.push_back( i->second );
  }
  // producers may be responsible for multiple requested DataObjID -- make sure they are only mentioned once
  std::sort( deps.begin(), deps.end(), []( auto const* lhs, auto const* rhs ) { return *lhs < *rhs; } );
  deps.erase( std::unique( deps.begin(), deps.end(), []( auto const& lhs, auto const& rhs ) { return *lhs == *rhs; } ),
              deps.end() );

  std::vector<bool> visited( m_algorithms.size() );
  std::vector<bool> visiting( m_algorithms.size() );
  for ( auto* alg : deps ) { visit( *alg, stoppers, result, visited, visiting ); }
  return result;
}

std::vector<Gaudi::Algorithm*>
HiveDataBrokerSvc::algorithmsRequiredFor( const Gaudi::Utils::TypeNameString& requested,
                                          const std::vector<std::string>&     stoppers ) const {
  std::vector<Gaudi::Algorithm*> result;

  auto it = m_algorithms.find( requested.name() );
  if ( it == end( m_algorithms ) ) {
    throw GaudiException{ "No algorithm with name " + requested.name() + " in DataProducers. Type is " +
                              ( requested.haveType() ? requested.type() : "not specified" ),
                          __func__, StatusCode::FAILURE };
  }
  auto const& alg = it->second;
  if ( requested.haveType() && alg.alg->type() != requested.type() ) {
    error() << "requested " << requested << " but have matching name with different type: " << alg.alg->type()
            << endmsg;
  }
  assert( alg.alg != nullptr );

  std::vector<bool> visited( m_algorithms.size() );
  std::vector<bool> visiting( m_algorithms.size() );
  visit( alg, stoppers, result, visited, visiting );

  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << std::endl << "requested " << requested << " returning " << std::endl << "  ";
    GaudiUtils::details::ostream_joiner(
        debug(), result, ",\n  ",
        []( auto& os, const Gaudi::Algorithm* a ) -> decltype( auto ) { return os << AlgorithmRepr{ *a }; } );
    debug() << std::endl << endmsg;
  }
  return result;
}
