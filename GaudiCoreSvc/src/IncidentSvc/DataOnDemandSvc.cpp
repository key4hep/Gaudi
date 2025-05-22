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
#include "DataOnDemandSvc.h"
#include <GaudiKernel/AttribStringParser.h>
#include <GaudiKernel/Chrono.h>
#include <GaudiKernel/DataIncident.h>
#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/IAlgManager.h>
#include <GaudiKernel/IAlgorithm.h>
#include <GaudiKernel/IDataProviderSvc.h>
#include <GaudiKernel/IIncidentSvc.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/IToolSvc.h>
#include <GaudiKernel/LockedChrono.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/ThreadLocalContext.h>
#include <GaudiKernel/ToStream.h>
#include <GaudiKernel/TypeNameString.h>
#include <fmt/format.h>
#include <map>
#include <math.h>
#include <set>
#include <string>

namespace {

  /** remove the prefix from the string
   *  @param value input string
   *  @param prefix prefix to be removed
   *  @return input string without prefix
   */
  inline std::string no_prefix( const std::string& value, const std::string& prefix ) {
    return boost::algorithm::starts_with( value, prefix ) ? value.substr( prefix.size() ) : value;
  }
  /** add a prefix (if needed) to all keys of the map
   *  the previous key is removed
   *  @param _map map to be modified
   *  @param prefix prefix to be added
   *  @return number of modified keys
   */
  template <class MAP>
  inline size_t add_prefix( MAP& _map, const std::string& prefix ) {
    // empty  prefix
    if ( prefix.empty() ) { return 0; }
    /// loop over all entries to find the  proper keys
    auto it = std::find_if_not( _map.begin(), _map.end(), [&]( typename MAP::ValueType::const_reference i ) {
      return boost::algorithm::starts_with( i.first, prefix );
    } );
    if ( it == _map.end() ) return 0;
    std::string key   = prefix + it->first;
    std::string value = std::move( it->second ); // steal the value we're about to erase..
    _map.erase( it );
    _map[key] = std::move( value );        // and move it into its new location
    return 1 + add_prefix( _map, prefix ); // RETURN, recursion
  }
  /** get the list of directories for a certain TES location
   *  @param object to be inspected
   *  @param _set (output) the set with directory list
   *  @return increment of the output set size
   */
  template <class SET>
  inline size_t get_dir( const std::string& object, SET& _set ) {
    std::string::size_type ifind = object.rfind( '/' );
    // stop recursion
    if ( std::string::npos == ifind ) { return 0; }
    if ( 0 == ifind ) { return 0; }
    const std::string top = std::string( object, 0, ifind );
    _set.insert( top );
    return 1 + get_dir( top, _set ); // RETURN, recursion
  }
  /** get the list of directories for a all keys of the input map
   *  @param _map (input) map to be inspected
   *  @param _set (output) the set with directoiry list
   *  @return incrment of the output set size
   */
  template <class MAP, class SET>
  inline size_t get_dirs( const MAP& _map, SET& _set ) {
    size_t size = _set.size();
    for ( const auto& item : _map ) { get_dir( item.first, _set ); }
    return _set.size() - size;
  }
} // namespace

void DataOnDemandSvc::i_setNodeHandler( const std::string& name, const std::string& type ) {
  ClassH cl = TClass::GetClass( type.c_str() );
  if ( !cl ) { warning() << "Failed to access dictionary class for " << name << " of type:" << type << endmsg; }
  m_nodes[name] = Node( cl, false, type );
}

StatusCode DataOnDemandSvc::i_setAlgHandler( const std::string& name, const Gaudi::Utils::TypeNameString& alg ) {
  Leaf leaf( alg.type(), alg.name() );
  if ( m_init ) {
    StatusCode sc = configureHandler( leaf );
    if ( sc.isFailure() ) {
      if ( m_allowInitFailure ) {
        // re-store the content of the leaf object to try again to initialize
        // the algorithm later (on demand)
        leaf = Leaf( alg.type(), alg.name() );
      } else
        return sc;
    }
  }
  m_algs[name] = leaf;
  return StatusCode::SUCCESS;
}

StatusCode DataOnDemandSvc::update() {
  if ( !m_updateRequired ) { return StatusCode::SUCCESS; }

  // convert obsolete "Nodes"      into new "NodeMap"
  StatusCode sc = setupNodeHandlers(); // convert "Nodes"      new "NodeMap"
  if ( sc.isFailure() ) {
    error() << "Failed to setup old \"Nodes\"" << endmsg;
    return sc;
  }
  // convert obsolete "Algorithms" into new "AlgMap"
  sc = setupAlgHandlers(); // convert "Algorithms" into "AlgMap"
  if ( sc.isFailure() ) {
    error() << "Failed to setup old \"Algorithms\"" << endmsg;
    return sc;
  }
  // add the default prefix
  add_prefix( m_algMap, m_prefix );
  // add the default prefix
  add_prefix( m_nodeMap, m_prefix );
  // get all directories
  std::set<std::string> dirs;
  if ( m_partialPath ) { get_dirs( m_algMap, dirs ); }
  if ( m_partialPath ) { get_dirs( m_nodeMap, dirs ); }

  auto _e = dirs.find( "/Event" );
  if ( dirs.end() != _e ) { dirs.erase( _e ); }
  // add all directories as nodes
  for ( const auto& dir : dirs ) {
    if ( m_algMap.end() == m_algMap.find( dir ) && m_nodeMap.end() == m_nodeMap.find( dir ) )
      m_nodeMap[dir] = "DataObject";
  }

  m_algs.clear();
  m_nodes.clear();

  // setup algorithms
  for ( const auto& alg : m_algMap ) {
    if ( i_setAlgHandler( alg.first, alg.second ).isFailure() ) return StatusCode::FAILURE;
  }
  // setup nodes
  for ( const auto& node : m_nodeMap ) { i_setNodeHandler( node.first, node.second ); }

  m_updateRequired = false;

  return StatusCode::SUCCESS;
}
StatusCode DataOnDemandSvc::initialize() {
  // initialize the Service Base class
  StatusCode sc = Service::initialize();
  if ( sc.isFailure() ) { return sc; }
  sc = setup();
  if ( sc.isFailure() ) { return sc; }
  //
  if ( m_dump ) {
    dump( MSG::INFO );
  } else if ( msgLevel( MSG::DEBUG ) ) {
    dump( MSG::DEBUG );
  }

  if ( m_init ) { return update(); }

  return StatusCode::SUCCESS;
}
StatusCode DataOnDemandSvc::finalize() {
  info() << "Handled \"" << m_trapType.value() << "\" incidents: " << m_statAlg << "/" << m_statNode << "/" << m_stat
         << "(Alg/Node/Total)." << endmsg;
  if ( m_dump || msgLevel( MSG::DEBUG ) ) {
    info() << m_total.outputUserTime( "Algorithm timing: Mean(+-rms)/Min/Max:%3%(+-%4%)/%6%/%7%[ms] ",
                                      System::milliSec )
           << m_total.outputUserTime( "Total:%2%[s]", System::Sec ) << endmsg;
    info() << m_timer_nodes.outputUserTime( "Nodes     timing: Mean(+-rms)/Min/Max:%3%(+-%4%)/%6%/%7%[ms] ",
                                            System::milliSec )
           << m_timer_nodes.outputUserTime( "Total:%2%[s]", System::Sec ) << endmsg;
    info() << m_timer_algs.outputUserTime( "Algs      timing: Mean(+-rms)/Min/Max:%3%(+-%4%)/%6%/%7%[ms] ",
                                           System::milliSec )
           << m_timer_algs.outputUserTime( "Total:%2%[s]", System::Sec ) << endmsg;
    info() << m_timer_all.outputUserTime( "All       timing: Mean(+-rms)/Min/Max:%3%(+-%4%)/%6%/%7%[ms] ",
                                          System::milliSec )
           << m_timer_all.outputUserTime( "Total:%2%[s]", System::Sec ) << endmsg;
  }
  // dump it!
  if ( m_dump ) {
    dump( MSG::INFO, false );
  } else if ( msgLevel( MSG::DEBUG ) ) {
    dump( MSG::DEBUG, false );
  }

  if ( m_incSvc ) {
    m_incSvc->removeListener( this, m_trapType );
    m_incSvc.reset();
  }
  m_algMgr.reset();
  m_dataSvc.reset();
  if ( m_toolSvc ) { // we may not have retrieved the ToolSvc
    // Do not call releaseTool if the ToolSvc was already finalized.
    if ( SmartIF<IStateful>( m_toolSvc )->FSMState() > Gaudi::StateMachine::CONFIGURED ) {
      for ( const auto& i : m_nodeMappers ) m_toolSvc->releaseTool( i ).ignore();
      for ( const auto& i : m_algMappers ) m_toolSvc->releaseTool( i ).ignore();
    } else {
      warning() << "ToolSvc already finalized: cannot release tools. Check options." << endmsg;
    }
    m_nodeMappers.clear();
    m_algMappers.clear();
    m_toolSvc.reset();
  }
  return Service::finalize();
}
StatusCode DataOnDemandSvc::reinitialize() {
  // reinitialize the Service Base class
  if ( m_incSvc ) {
    m_incSvc->removeListener( this, m_trapType );
    m_incSvc.reset();
  }
  m_algMgr.reset();
  m_dataSvc.reset();
  for ( const auto& i : m_nodeMappers ) m_toolSvc->releaseTool( i ).ignore();
  m_nodeMappers.clear();
  for ( const auto& i : m_algMappers ) m_toolSvc->releaseTool( i ).ignore();
  m_algMappers.clear();
  m_toolSvc.reset();

  StatusCode sc = Service::reinitialize();
  if ( sc.isFailure() ) { return sc; }

  sc = setup();
  if ( sc.isFailure() ) { return sc; }

  if ( m_dump ) {
    dump( MSG::INFO );
  } else if ( msgLevel( MSG::DEBUG ) ) {
    dump( MSG::DEBUG );
  }

  return StatusCode::SUCCESS;
}
StatusCode DataOnDemandSvc::setup() {
  if ( !( m_algMgr = serviceLocator() ) ) // assignment meant
  {
    error() << "Failed to retrieve the IAlgManager interface." << endmsg;
    return StatusCode::FAILURE;
  }

  if ( !( m_incSvc = serviceLocator()->service( "IncidentSvc" ) ) ) // assignment meant
  {
    error() << "Failed to retrieve Incident service." << endmsg;
    return StatusCode::FAILURE;
  }
  m_incSvc->addListener( this, m_trapType );

  if ( !( m_dataSvc = serviceLocator()->service( m_dataSvcName ) ) ) // assignment meant
  {
    error() << "Failed to retrieve the data provider interface of " << m_dataSvcName << endmsg;
    return StatusCode::FAILURE;
  }

  // No need to get the ToolSvc if we are not using tools
  if ( !( m_nodeMapTools.empty() && m_algMapTools.empty() ) ) {
    if ( !( m_toolSvc = serviceLocator()->service( "ToolSvc" ) ) ) // assignment meant
    {
      error() << "Failed to retrieve ToolSvc" << endmsg;
      return StatusCode::FAILURE;
    }

    // load the node mapping tools
    IDODNodeMapper* nodetool = nullptr;
    for ( const auto& i : m_nodeMapTools ) {
      const StatusCode sc = m_toolSvc->retrieveTool( i, nodetool );
      if ( sc.isFailure() ) return sc;
      m_nodeMappers.push_back( nodetool );
    }
    IDODAlgMapper* algtool = nullptr;
    for ( const auto& i : m_algMapTools ) {
      const StatusCode sc = m_toolSvc->retrieveTool( i, algtool );
      if ( sc.isFailure() ) return sc;
      m_algMappers.push_back( algtool );
    }
  }
  return update();
}
StatusCode DataOnDemandSvc::setupNodeHandlers() {
  std::string nam, typ, tag;
  StatusCode  sc = StatusCode::SUCCESS;
  // Setup for node leafs, where simply a constructor is called...
  for ( auto node : m_nodeMapping ) {
    using Parser = Gaudi::Utils::AttribStringParser;
    for ( auto attrib : Parser( node ) ) {
      switch ( ::toupper( attrib.tag[0] ) ) {
      case 'D':
        tag = std::move( attrib.value );
        break;
      case 'T':
        nam = std::move( attrib.value );
        break;
      }
    }
    if ( m_algMap.end() != m_algMap.find( tag ) || m_nodeMap.end() != m_nodeMap.find( tag ) ) {
      warning() << "The obsolete property 'Nodes' redefines the action for '" + tag + "' to be '" + nam + "'" << endmsg;
    }
    m_nodeMap[tag] = nam;
  }

  m_updateRequired = true;

  return sc;
}
StatusCode DataOnDemandSvc::setupAlgHandlers() {
  std::string typ, tag;

  for ( auto alg : m_algMapping ) {
    using Parser = Gaudi::Utils::AttribStringParser;
    for ( auto attrib : Parser( alg ) ) {
      switch ( ::toupper( attrib.tag[0] ) ) {
      case 'D':
        tag = std::move( attrib.value );
        break;
      case 'T':
        typ = std::move( attrib.value );
        break;
      }
    }
    Gaudi::Utils::TypeNameString item( typ );
    if ( m_algMap.end() != m_algMap.find( tag ) || m_nodeMap.end() != m_nodeMap.find( tag ) ) {
      warning() << "The obsolete property 'Algorithms' redefines the action for '" + tag + "' to be '" + item.type() +
                       "/" + item.name() + "'"
                << endmsg;
    }
    m_algMap[tag] = item.type() + "/" + item.name();
  }
  m_updateRequired = true;
  return StatusCode::SUCCESS;
}
StatusCode DataOnDemandSvc::configureHandler( Leaf& l ) {
  if ( l.algorithm ) { return StatusCode::SUCCESS; }
  if ( !m_algMgr ) { return StatusCode::FAILURE; }
  l.algorithm = m_algMgr->algorithm( l.name, false );
  if ( l.algorithm ) { return StatusCode::SUCCESS; }
  // create it!
  StatusCode sc = m_algMgr->createAlgorithm( l.type, l.name, l.algorithm, true );
  if ( sc.isFailure() ) {
    error() << "Failed to create algorithm " << l.type << "('" << l.name << "')" << endmsg;
    l.algorithm = nullptr;
    return sc;
  }
  if ( l.algorithm->isInitialized() ) { return StatusCode::SUCCESS; }
  // initialize it!
  sc = l.algorithm->sysInitialize();
  if ( sc.isFailure() ) {
    error() << "Failed to initialize algorithm " << l.type << "('" << l.name << "')" << endmsg;
    l.algorithm = nullptr;
    return sc;
  }
  if ( Gaudi::StateMachine::RUNNING == l.algorithm->FSMState() ) { return StatusCode::SUCCESS; }
  // run it!
  sc = l.algorithm->sysStart();
  if ( sc.isFailure() ) {
    error() << "Failed to 'run'      algorithm " << l.type << "('" << l.name << "')" << endmsg;
    l.algorithm = nullptr;
    return sc;
  }
  return StatusCode::SUCCESS;
}

// local algorithms
namespace {
  /// Helper class to uniform the access to the main method of IDODNodeMapper
  /// and IDODAlgMapper.
  struct ToolGetter {
    /// local copy of the path to look for in the tool
    std::string path;
    /// constructor
    ToolGetter( std::string _path ) : path( std::move( _path ) ) {}
    /// find the node from a node mapper tool
    inline std::string operator()( IDODNodeMapper* t ) const { return t->nodeTypeForPath( path ); }
    /// find the algorithm from an algorithm mapper tool
    inline Gaudi::Utils::TypeNameString operator()( IDODAlgMapper* t ) const { return t->algorithmForPath( path ); }
  };

  ///@{
  /// Helper function to uniform the check on std::string and Gaudi::Utils::TypeNameString.
  inline bool isGood( const std::string& r ) { return !r.empty(); }
  inline bool isGood( const Gaudi::Utils::TypeNameString& r ) { return !r.name().empty(); }
  ///@}

  /// Simple algorithmic class to get the first non-empty node type or algorithm
  /// type/name given a path and a list of mapping tools.
  class Finder {
    const ToolGetter                    getter;
    const std::vector<IDODNodeMapper*>& nodes;
    const std::vector<IDODAlgMapper*>&  algs;
    /// Looping algorithm.
    template <class R, class C>
    R find( const C& l ) const {
      for ( auto& i : l ) {
        auto result = getter( i );
        if ( isGood( result ) ) return result;
      }
      return R{ "" };
    }

  public:
    /// Constructor.
    Finder( std::string _path, const std::vector<IDODNodeMapper*>& _nodes, const std::vector<IDODAlgMapper*>& _algs )
        : getter( std::move( _path ) ), nodes( _nodes ), algs( _algs ) {}
    /// Find the node type for the requested path.
    inline std::string node() const { return find<std::string>( nodes ); }
    /// Find the algorithm for the requested path.
    inline Gaudi::Utils::TypeNameString alg() const { return find<Gaudi::Utils::TypeNameString>( algs ); }
  };
} // namespace

void DataOnDemandSvc::handle( const Incident& incident ) {

  Gaudi::Utils::LockedChrono timer( m_timer_all, m_locked_all );

  ++m_stat;
  // proper incident type?
  if ( incident.type() != m_trapType ) { return; }
  const DataIncident* inc = dynamic_cast<const DataIncident*>( &incident );
  if ( !inc ) { return; }
  // update if needed!
  if ( m_updateRequired ) {
    if ( !update() ) throw GaudiException( "Failed to update", name(), StatusCode::FAILURE );
  }

  if ( msgLevel( MSG::VERBOSE ) ) {
    verbose() << "Incident: [" << incident.type() << "] "
              << " = " << incident.source() << " Location:" << inc->tag() << endmsg;
  }

  Gaudi::StringKey tag( inc->tag() );

  auto icl = m_nodes.find( tag );
  if ( icl != m_nodes.end() ) {
    StatusCode sc = execHandler( tag, icl->second );
    if ( sc.isSuccess() ) { ++m_statNode; }
    return;
  }

  auto ialg = m_algs.find( tag );
  if ( ialg != m_algs.end() ) {
    StatusCode sc = execHandler( tag, ialg->second );
    if ( sc.isSuccess() ) { ++m_statAlg; }
    return;
  }

  // Fall back on the tools
  if ( m_toolSvc ) {
    if ( msgLevel( MSG::VERBOSE ) ) verbose() << "Try to find mapping with mapping tools" << endmsg;
    Finder finder( no_prefix( inc->tag(), m_prefix ), m_nodeMappers, m_algMappers );
    //  - try the node mappers
    std::string node = finder.node();
    if ( isGood( node ) ) {
      // if one is found update the internal node mapping and try again.
      if ( msgLevel( MSG::VERBOSE ) ) verbose() << "Found Node handler: " << node << endmsg;
      i_setNodeHandler( inc->tag(), node );
      handle( incident );
      --m_stat; // avoid double counting because of recursion
      return;
    }
    //  - try alg mappings
    Gaudi::Utils::TypeNameString alg = finder.alg();
    if ( isGood( alg ) ) {
      // we got an algorithm, update alg map and try to handle again
      if ( msgLevel( MSG::VERBOSE ) ) verbose() << "Found Algorithm handler: " << alg << endmsg;
      i_setAlgHandler( inc->tag(), alg ).ignore();
      handle( incident );
      --m_stat; // avoid double counting because of recursion
      return;
    }
  }
}
StatusCode DataOnDemandSvc::execHandler( const std::string& tag, Node& n ) {

  Gaudi::Utils::LockedChrono timer( m_timer_nodes, m_locked_nodes );

  if ( n.executing ) { return StatusCode::FAILURE; }

  Protection p( n.executing );

  std::unique_ptr<DataObject> object;

  if ( n.dataObject ) {
    object.reset( new DataObject() );
  } else {
    // try to recover the handler
    if ( !n.clazz ) { n.clazz = TClass::GetClass( n.name.c_str() ); }
    if ( !n.clazz ) {
      error() << "Failed to get dictionary for class '" << n.name << "' for location:" << tag << endmsg;
      return StatusCode::FAILURE;
    }

    object.reset( reinterpret_cast<DataObject*>( n.clazz->New() ) );

    if ( !object ) {
      error() << "Failed to create an object of type:" << n.clazz->GetName() << " for location:" << tag << endmsg;
      return StatusCode::FAILURE;
    }
  }
  //
  StatusCode sc = m_dataSvc->registerObject( tag, object.release() );
  if ( sc.isFailure() ) {
    error() << "Failed to register an object of type:" << n.name << " at location:" << tag << endmsg;
    return sc;
  }
  ++n.num;
  //
  return StatusCode::SUCCESS;
}
StatusCode DataOnDemandSvc::execHandler( const std::string& tag, Leaf& l ) {
  Gaudi::Utils::LockedChrono timer( m_timer_algs, m_locked_algs );

  if ( l.executing ) { return StatusCode::FAILURE; }

  if ( !l.algorithm ) {
    StatusCode sc = configureHandler( l );
    if ( sc.isFailure() ) {
      error() << "Failed to configure handler for: " << l.name << "[" << l.type << "] " << tag << endmsg;
      return sc;
    }
  }

  Chrono atimer( m_total );

  Protection p( l.executing );
  // FIXME: this will cause problems for Hive, as we need to set
  // the EventContext of the called Algorithm.
  // if (!l.algorithm->getContext()) {
  //   l.algorithm->setContext( &Gaudi::Hive::currentContext() );
  // }
  StatusCode sc = l.algorithm->sysExecute( Gaudi::Hive::currentContext() );
  if ( sc.isFailure() ) {
    error() << "Failed to execute the algorithm:" << l.algorithm->name() << " for location:" << tag << endmsg;
    return sc;
  }
  ++l.num;

  return StatusCode::SUCCESS;
}
void DataOnDemandSvc::dump( const MSG::Level level, const bool mode ) const {
  if ( m_algs.empty() && m_nodes.empty() ) { return; }

  typedef std::pair<std::string, std::string> Pair;
  std::map<std::string, Pair>                 _m;
  for ( auto& alg : m_algs ) {
    auto check = _m.find( alg.first );
    if ( _m.end() != check ) {
      warning() << " The data item is activated for '" << check->first << "' as '" << check->second.first << "'"
                << endmsg;
    }
    const Leaf& l   = alg.second;
    std::string nam = ( l.name == l.type ? l.type : ( l.type + "/" + l.name ) );

    if ( !mode && 0 == l.num ) { continue; }

    std::string val;
    if ( mode ) {
      val = ( !l.algorithm ) ? "F" : "T";
    } else {
      val = std::to_string( l.num );
    }

    _m[no_prefix( alg.first, m_prefix )] = { nam, val };
  }
  // nodes:
  for ( const auto& node : m_nodes ) {
    auto check = _m.find( node.first );
    if ( _m.end() != check ) {
      warning() << " The data item is already activated for '" << check->first << "' as '" << check->second.first << "'"
                << endmsg;
    }
    const Node& n   = node.second;
    std::string nam = "'" + n.name + "'";

    std::string val;

    if ( !mode && 0 == n.num ) { continue; }

    if ( mode ) {
      val = ( 0 == n.clazz ) ? "F" : "T";
    } else {
      val = std::to_string( n.num );
    }

    _m[no_prefix( node.first, m_prefix )] = { nam, val };
  }

  if ( _m.empty() ) { return; }

  // set width of the columns
  size_t n1 = 10; // minimum width
  size_t n2 = 10; // minimum width
  size_t n3 = 0;
  for ( const auto& i : _m ) {
    n1 = std::max( n1, i.first.size() );
    n2 = std::max( n2, i.second.first.size() );
    n3 = std::max( n3, i.second.second.size() );
  }
  n1 = std::min( n1, size_t{ 60 } ); // maximum width
  n2 = std::min( n2, size_t{ 60 } ); // maximum width

  auto& msg = msgStream( level );

  if ( mode ) {
    msg << "Data-On-Demand Actions enabled for:";
  } else {
    msg << "Data-On-Demand Actions has been used for:";
  }

  const auto header = fmt::format( " | {3:<{0}.{0}s} | {4:<{1}.{1}s} | {5:>{2}.{2}s} |", n1, n2, n3, "Address",
                                   "Creator", ( mode ? "S" : "#" ) );
  const auto line   = fmt::format( " {0:-^{1}}", "", header.size() - 1 );
  msg << '\n' << line << '\n' << header << '\n' << line;

  // make the actual printout:
  for ( const auto& item : _m ) {
    msg << fmt::format( "\n | {3:<{0}.{0}s} | {4:<{1}.{1}s} | {5:>{2}.{2}s} |", n1, n2, n3, item.first,
                        item.second.first, item.second.second );
  }

  msg << '\n' << line << endmsg;
}

DECLARE_COMPONENT( DataOnDemandSvc )
