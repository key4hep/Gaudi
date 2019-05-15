#include "HiveDataBroker.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/System.h"
#include "boost/lexical_cast.hpp"
#include "boost/tokenizer.hpp"
#include "range/v3/algorithm/for_each.hpp"
#include "range/v3/view/remove_if.hpp"
#include "range/v3/view/reverse.hpp"
#include "range/v3/view/transform.hpp"
#include <Gaudi/Algorithm.h>
#include <algorithm>

DECLARE_COMPONENT( HiveDataBrokerSvc )

namespace {
  struct AlgorithmRepr {
    const Gaudi::Algorithm& parent;

    friend std::ostream& operator<<( std::ostream& s, const AlgorithmRepr& a ) {
      std::string typ = System::typeinfoName( typeid( a.parent ) );
      s << typ;
      if ( a.parent.name() != typ ) s << "/" << a.parent.name();
      return s;
    }
  };

  struct DataObjIDSorter {
    bool operator()( const DataObjID* a, const DataObjID* b ) { return a->fullKey() < b->fullKey(); }
  };

  // Sort a DataObjIDColl in a well-defined, reproducible manner.
  // Used for making debugging dumps.
  std::vector<const DataObjID*> sortedDataObjIDColl( const DataObjIDColl& coll ) {
    std::vector<const DataObjID*> v;
    v.reserve( coll.size() );
    for ( const DataObjID& id : coll ) v.push_back( &id );
    std::sort( v.begin(), v.end(), DataObjIDSorter() );
    return v;
  }

  SmartIF<IAlgorithm> createAlgorithm( IAlgManager& am, const std::string& type, const std::string& name ) {
    // Maybe modify the AppMgr interface to return Algorithm* ??
    IAlgorithm* tmp;
    StatusCode  sc = am.createAlgorithm( type, name, tmp );
    return {sc.isSuccess() ? dynamic_cast<Gaudi::Algorithm*>( tmp ) : nullptr};
  }
} // namespace

StatusCode HiveDataBrokerSvc::initialize() {
  auto sc = Service::initialize();
  if ( sc.isFailure() ) return sc;
  // populate m_algorithms
  m_algorithms = instantiateAndInitializeAlgorithms( m_producers );
  if ( sc.isFailure() ) return sc;

  // warn about non-reentrant algorithms
  ranges::for_each( m_algorithms | ranges::view::transform( []( const auto& entry ) { return entry.alg; } ) |
                        ranges::view::remove_if( []( const auto* alg ) { return alg->cardinality() == 0; } ),
                    [&]( const Gaudi::Algorithm* alg ) {
                      this->warning() << "non-reentrant algorithm: " << AlgorithmRepr{*alg} << endmsg;
                    } );
  //== Print the list of the created algorithms
  if ( msgLevel( MSG::DEBUG ) ) {
    MsgStream& msg = debug();
    msg << "Available DataProducers: ";
    GaudiUtils::details::ostream_joiner(
        msg, m_algorithms, ", ",
        []( auto& os, const AlgEntry& e ) -> decltype( auto ) { return os << AlgorithmRepr{*e.alg}; } );
    msg << endmsg;
  }

  // populate m_dependencies
  m_dependencies = mapProducers( m_algorithms );
  return sc;
}

StatusCode HiveDataBrokerSvc::start() {

  StatusCode ss = Service::start();
  if ( !ss.isSuccess() ) return ss;

  // sysStart for m_algorithms
  for ( AlgEntry& algEntry : m_algorithms ) {
    ss = algEntry.alg->sysStart();
    if ( ss.isFailure() ) {
      error() << "Unable to start Algorithm: " << algEntry.alg->name() << endmsg;
      return ss;
    }
  }
  // sysStart for m_cfnodes
  for ( AlgEntry& algEntry : m_cfnodes ) {
    ss = algEntry.alg->sysStart();
    if ( ss.isFailure() ) {
      error() << "Unable to start Algorithm: " << algEntry.alg->name() << endmsg;
      return ss;
    }
  }
  return ss;
}

StatusCode HiveDataBrokerSvc::stop() {
  StatusCode ss = Service::stop();
  if ( !ss.isSuccess() ) return ss;

  // sysStart for m_algorithms
  for ( AlgEntry& algEntry : m_algorithms ) {
    ss = algEntry.alg->sysStop();
    if ( ss.isFailure() ) {
      error() << "Unable to stop Algorithm: " << algEntry.alg->name() << endmsg;
      return ss;
    }
  }
  // sysStart for m_cfnodes
  for ( AlgEntry& algEntry : m_cfnodes ) {
    ss = algEntry.alg->sysStop();
    if ( ss.isFailure() ) {
      error() << "Unable to stop Algorithm: " << algEntry.alg->name() << endmsg;
      return ss;
    }
  }
  return ss;
}

StatusCode HiveDataBrokerSvc::finalize() {
  ranges::for_each( m_algorithms | ranges::view::transform( &AlgEntry::alg ),
                    []( Gaudi::Algorithm* alg ) { alg->sysFinalize(); } );
  m_algorithms.clear();
  return Service::finalize();
}

// populate m_algorithms
std::vector<HiveDataBrokerSvc::AlgEntry>
HiveDataBrokerSvc::instantiateAndInitializeAlgorithms( const std::vector<std::string>& names ) const {
  std::vector<AlgEntry> algorithms;

  //= Get the Application manager, to see if algorithm exist
  auto appMgr = service<IAlgManager>( "ApplicationMgr" );
  for ( const Gaudi::Utils::TypeNameString item : names ) {
    const std::string& theName = item.name();
    const std::string& theType = item.type();

    //== Check wether the specified algorithm already exists. If not, create it
    SmartIF<IAlgorithm> myIAlg = appMgr->algorithm( item, false ); // do not create it now
    if ( !myIAlg ) {
      myIAlg = createAlgorithm( *appMgr, theType, theName );
    } else {
      // when the algorithm is not created, the ref count is short by one, so we
      // have to fix it.
      myIAlg->addRef();
    }

    if ( !myIAlg ) {
      throw GaudiException{"Failed to create " + boost::lexical_cast<std::string>( item ), __func__,
                           StatusCode::FAILURE};
    }

    // propagate the sub-algorithm into own state.
    StatusCode sc = myIAlg->sysInitialize();
    if ( sc.isFailure() ) {
      throw GaudiException{"Failed to initialize " + boost::lexical_cast<std::string>( item ), __func__,
                           StatusCode::FAILURE};
    }

    algorithms.emplace_back( std::move( myIAlg ) );
  }

  return algorithms;
}

std::map<DataObjID, HiveDataBrokerSvc::AlgEntry*>
HiveDataBrokerSvc::mapProducers( std::vector<AlgEntry>& algorithms ) const {
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "Data Dependencies for Algorithms:";
    for ( const auto& entry : m_algorithms ) {
      debug() << "\n " << entry.alg->name() << " :";
      for ( const auto* id : sortedDataObjIDColl( entry.alg->inputDataObjs() ) ) {
        debug() << "\n    o INPUT  " << id->key();
      }
      for ( const auto* id : sortedDataObjIDColl( entry.alg->outputDataObjs() ) ) {
        debug() << "\n    o OUTPUT " << id->key();
      }
    }
    debug() << endmsg;
  }

  // figure out all outputs
  std::map<DataObjID, AlgEntry*> producers;
  for ( AlgEntry& alg : algorithms ) {
    const auto& output = alg.alg->outputDataObjs();
    if ( output.empty() ) { continue; }
    for ( auto id : output ) {
      if ( id.key().find( ":" ) != std::string::npos ) {
        error() << " in Alg " << AlgorithmRepr{*alg.alg} << " alternatives are NOT allowed for outputs! id: " << id
                << endmsg;
      }

      auto r = producers.emplace( id, &alg );
      if ( !r.second ) {
        throw GaudiException( "multiple algorithms declare " + id.key() + " as output. This is not allowed", __func__,
                              StatusCode::FAILURE );
      }
    }
  }

  // resolve dependencies
  for ( auto& algEntry : algorithms ) {
    auto input = sortedDataObjIDColl( algEntry.alg->inputDataObjs() );
    for ( const DataObjID* idp : input ) {
      DataObjID id = *idp;
      if ( id.key().find( ":" ) != std::string::npos ) {
        warning() << " contains alternatives which require resolution...\n";
        auto tokens = boost::tokenizer<boost::char_separator<char>>{id.key(), boost::char_separator<char>{":"}};
        auto itok   = std::find_if( tokens.begin(), tokens.end(),
                                  [&]( DataObjID t ) { return producers.find( t ) != producers.end(); } );
        if ( itok != tokens.end() ) {
          warning() << "found matching output for " << *itok << " -- updating info\n";
          id.updateKey( *itok );
          warning() << "Please update input to not require alternatives, and "
                       "instead properly configure the dataloader"
                    << endmsg;
        } else {
          error() << "failed to find alternate in global output list"
                  << " for id: " << id << " in Alg " << algEntry.alg << endmsg;
        }
      }
      auto iproducer = producers.find( id );
      if ( iproducer != producers.end() ) {
        algEntry.dependsOn.insert( iproducer->second );
      } else {
        std::ostringstream error_message;
        error_message << "\nUnknown requested input by " << AlgorithmRepr{*( algEntry.alg )} << " : " << id.key()
                      << " .\n";
        error_message << "You can set the OutputLevel of HiveDataBrokerSvc to DEBUG to get a list of inputs and "
                         "outputs of every algorithm.\n";
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

std::vector<Gaudi::Algorithm*>
HiveDataBrokerSvc::algorithmsRequiredFor( const DataObjIDColl&            requested,
                                          const std::vector<std::string>& stoppers ) const {
  std::vector<Gaudi::Algorithm*> result;

  std::vector<const AlgEntry*> deps;
  deps.reserve( requested.size() );

  // start with seeding from the initial request
  for ( const auto& req : requested ) {
    DataObjID id = req;
    if ( id.key().find( ":" ) != std::string::npos ) {
      warning() << req.key() << " contains alternatives which require resolution...\n";
      auto tokens = boost::tokenizer<boost::char_separator<char>>{id.key(), boost::char_separator<char>{":"}};
      auto itok   = std::find_if( tokens.begin(), tokens.end(),
                                [&]( DataObjID t ) { return m_dependencies.find( t ) != m_dependencies.end(); } );
      if ( itok != tokens.end() ) {
        warning() << "found matching output for " << *itok << " -- updating info\n";
        id.updateKey( *itok );
        warning() << "Please update input to not require alternatives, and "
                     "instead properly configure the dataloader"
                  << endmsg;
      } else {
        error() << "failed to find alternate in global output list"
                << " for id: " << id << endmsg;
      }
    }
    auto i = m_dependencies.find( id );
    if ( i == m_dependencies.end() )
      throw GaudiException( "unknown requested input: " + id.key(), __func__, StatusCode::FAILURE );
    deps.push_back( i->second );
  }
  // insert the (direct) dependencies of 'current' right after 'current', and
  // interate until done...
  for ( auto current = deps.begin(); current != deps.end(); ++current ) {
    if ( std::any_of( std::begin( stoppers ), std::end( stoppers ),
                      [current]( auto& stopper ) { return ( *current )->alg->name() == stopper; } ) ) {
      continue;
    }
    for ( auto* entry : ( *current )->dependsOn ) {
      if ( std::find( std::next( current ), deps.end(), entry ) != deps.end() ) continue; // already there downstream...

      auto dup = std::find( deps.begin(), current, entry );
      // if present upstream, move it downstream. Otherwise, insert
      // downstream...
      current = std::prev( dup != current ? std::rotate( dup, std::next( dup ), std::next( current ) )
                                          : deps.insert( std::next( current ), entry ) );
    }
  }
  auto range = ( deps | ranges::view::transform( []( auto& i ) { return i->alg; } ) | ranges::view::reverse );
  return {begin( range ), end( range )};
}

std::vector<Gaudi::Algorithm*>
HiveDataBrokerSvc::algorithmsRequiredFor( const Gaudi::Utils::TypeNameString& requested,
                                          const std::vector<std::string>&     stoppers ) const {
  std::vector<Gaudi::Algorithm*> result;

  auto alg = std::find_if( begin( m_cfnodes ), end( m_cfnodes ),
                           [&]( const AlgEntry& ae ) { return ae.alg->name() == requested.name(); } );

  if ( alg != end( m_cfnodes ) && alg->alg->type() != requested.type() ) {
    error() << "requested " << requested << " but have matching name with different type: " << alg->alg->type()
            << endmsg;
  }
  if ( alg == end( m_cfnodes ) ) {
    auto av = instantiateAndInitializeAlgorithms( {requested.type() + '/' + requested.name()} );
    assert( av.size() == 1 );
    m_cfnodes.push_back( std::move( av.front() ) );
    alg = std::next( m_cfnodes.rbegin() ).base();
  }
  assert( alg != end( m_cfnodes ) );
  assert( alg->alg != nullptr );
  if ( std::find_if( std::begin( stoppers ), std::end( stoppers ),
                     [&requested]( auto& stopper ) { return requested.name() == stopper; } ) == std::end( stoppers ) ) {
    result = algorithmsRequiredFor( alg->alg->inputDataObjs(), stoppers );
  }
  result.push_back( alg->alg );
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << std::endl << "requested " << requested << " returning " << std::endl << "  ";
    GaudiUtils::details::ostream_joiner(
        debug(), result, ",\n  ",
        []( auto& os, const Gaudi::Algorithm* a ) -> decltype( auto ) { return os << AlgorithmRepr{*a}; } );
    debug() << std::endl << endmsg;
  }
  return result;
}
