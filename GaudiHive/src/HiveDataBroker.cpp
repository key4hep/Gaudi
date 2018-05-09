#include "HiveDataBroker.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/System.h"
#include "boost/lexical_cast.hpp"
#include "boost/tokenizer.hpp"
#include "range/v3/algorithm/for_each.hpp"
#include "range/v3/view/remove_if.hpp"
#include "range/v3/view/reverse.hpp"
#include "range/v3/view/transform.hpp"
#include <algorithm>

DECLARE_COMPONENT( HiveDataBrokerSvc )

namespace
{
  struct AlgorithmRepr {
    const Algorithm& parent;

    friend std::ostream& operator<<( std::ostream& s, const AlgorithmRepr& a )
    {
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
  std::vector<const DataObjID*> sortedDataObjIDColl( const DataObjIDColl& coll )
  {
    std::vector<const DataObjID*> v;
    v.reserve( coll.size() );
    for ( const DataObjID& id : coll ) v.push_back( &id );
    std::sort( v.begin(), v.end(), DataObjIDSorter() );
    return v;
  }

  SmartIF<IAlgorithm> createAlgorithm( IAlgManager& am, const std::string& type, const std::string& name )
  {
    // Maybe modify the AppMgr interface to return Algorithm* ??
    IAlgorithm* tmp;
    StatusCode  sc = am.createAlgorithm( type, name, tmp );
    return {sc.isSuccess() ? dynamic_cast<Algorithm*>( tmp ) : nullptr};
  }
}

StatusCode HiveDataBrokerSvc::initialize()
{
  auto sc = Service::initialize();
  if ( sc.isFailure() ) return sc;
  // populate m_algorithms
  m_algorithms = instantiateAndInitializeAlgorithms( m_producers );
  if ( sc.isFailure() ) return sc;

  // warn about non-reentrant algorithms
  ranges::for_each( m_algorithms | ranges::view::transform( []( const auto& entry ) { return entry.alg; } ) |
                        ranges::view::remove_if( []( const auto* alg ) { return alg->cardinality() == 0; } ),
                    [&]( const Algorithm* alg ) {
                      this->warning() << "non-reentrant algorithm: " << AlgorithmRepr{*alg} << endmsg;
                    } );
  //== Print the list of the created algorithms
  MsgStream& msg = info();
  msg << "Available DataProducers: ";
  GaudiUtils::details::ostream_joiner( msg, m_algorithms, ", ", []( auto& os, const AlgEntry& e ) -> decltype( auto ) {
    return os << AlgorithmRepr{*e.alg};
  } );
  msg << endmsg;

  // populate m_dependencies
  m_dependencies = mapProducers( m_algorithms );

  return sc;
}

StatusCode HiveDataBrokerSvc::finalize()
{
  ranges::for_each(
      m_algorithms | ranges::view::remove_if( []( const auto& entry ) { return entry.requestCount != 0; } ) |
          ranges::view::transform( []( const auto& entry ) { return entry.alg; } ),
      [&]( Algorithm* alg ) { this->warning() << "Unused algorithm: " << AlgorithmRepr{*alg} << endmsg; } );
  m_algorithms.clear();
  return Service::finalize();
}

// populate m_algorithms
std::vector<HiveDataBrokerSvc::AlgEntry>
HiveDataBrokerSvc::instantiateAndInitializeAlgorithms( const std::vector<std::string>& names ) const
{
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
HiveDataBrokerSvc::mapProducers( std::vector<AlgEntry>& algorithms ) const
{
  // figure out all outputs
  std::map<DataObjID, AlgEntry*> producers;
  for ( AlgEntry& alg : algorithms ) {
    const auto& output = alg.alg->outputDataObjs();
    if ( output.empty() ) {
      error() << AlgorithmRepr{*algorithms.back().alg} << " does not produce any data -- should not appear in list of "
                                                          "producers. Ignoring."
              << endmsg;
      continue;
    }
    for ( auto id : output ) {
      if ( id.key().find( ":" ) != std::string::npos ) {
        error() << " in Alg " << AlgorithmRepr{*alg.alg} << " alternatives are NOT allowed for outputs! id: " << id
                << endmsg;
      }

      auto r = producers.emplace( id, &alg );
      if ( !r.second ) {
        if ( output.size() == 1 ) {
          warning() << "multiple algorithms declare " << id << " as output! -- IGNORING " << AlgorithmRepr{*alg.alg}
                    << endmsg;
        } else {
          error() << "multiple algorithms declate " << id << " as output; given that " << AlgorithmRepr{*alg.alg}
                  << " produces multiple outputs ";
          //<< output <<
          error() << " this could lead to clashes in case any of the other "
                     "items is ever requested"
                  << endmsg;
        }
      }
    }
  }

  // resolve dependencies
  for ( auto& algEntry : algorithms ) {
    auto input = sortedDataObjIDColl( algEntry.alg->inputDataObjs() );
    for ( const DataObjID* idp : input ) {
      DataObjID id = *idp;
      if ( id.key().find( ":" ) != std::string::npos ) {
        info() << " contains alternatives which require resolution...\n";
        auto tokens = boost::tokenizer<boost::char_separator<char>>{id.key(), boost::char_separator<char>{":"}};
        auto itok   = std::find_if( tokens.begin(), tokens.end(),
                                  [&]( DataObjID t ) { return producers.find( t ) != producers.end(); } );
        if ( itok != tokens.end() ) {
          info() << "found matching output for " << *itok << " -- updating info\n";
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
        // TODO: assign to dataloader!
        // algEntry.dependsOn.insert(dataloader.alg);
        // dataloader.data.emplace( id ); // TODO: we may ask to much of the
        // dataloader this way...
      }
    }
  }
  return producers;
}

std::vector<Algorithm*> HiveDataBrokerSvc::algorithmsRequiredFor( const DataObjIDColl& requested ) const
{
  std::vector<Algorithm*> result;

  std::vector<const AlgEntry*> deps;
  deps.reserve( requested.size() );

  // start with seeding from the initial request
  for ( const auto& req : requested ) {
    auto i = m_dependencies.find( req );
    if ( i == m_dependencies.end() ) throw GaudiException( "unknown requested input", __func__, StatusCode::FAILURE );
    deps.push_back( i->second );
  }
  // insert the (direct) dependencies of 'current' right after 'current', and
  // interate until done...
  for ( auto current = deps.begin(); current != deps.end(); ++current ) {
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

std::vector<Algorithm*> HiveDataBrokerSvc::algorithmsRequiredFor( const Gaudi::Utils::TypeNameString& requested ) const
{
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
  auto result = algorithmsRequiredFor( alg->alg->inputDataObjs() );
  result.push_back( alg->alg );
  info() << " requested " << requested << " returning ";
  GaudiUtils::details::ostream_joiner( info(), result, ", ", []( auto& os, const Algorithm* a ) -> decltype( auto ) {
    return os << AlgorithmRepr{*a};
  } );
  info() << endmsg;
  return result;
}
