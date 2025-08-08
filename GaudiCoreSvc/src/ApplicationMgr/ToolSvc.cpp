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
#include <GaudiKernel/AlgTool.h>
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/IAlgorithm.h>
#include <GaudiKernel/IHistorySvc.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/IToolSvc.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/Service.h>
#include <algorithm>
#include <boost/circular_buffer.hpp>
#include <cassert>
#include <functional>
#include <map>
#include <mutex>
#include <numeric>
#include <string>
#include <vector>

#define ON_DEBUG if ( msgLevel( MSG::DEBUG ) )
#define ON_VERBOSE if ( msgLevel( MSG::VERBOSE ) )

/** @class ToolSvc ToolSvc.h
 *  This service manages tools.
 *  Tools can be common, in which case a single instance
 *  can be shared by different algorithms, or private
 *  in which case it is necessary to specify the parent
 *  when requesting it.
 *  The parent of a tool can be an algortihm or a Service
 *  The environment of a tool is set by using that of the
 *  parent. A common tool is considered to belong to the
 *  ToolSvc itself.
 *
 *  @author G. Corti, P. Mato
 */
class ToolSvc : public extends<Service, IToolSvc> {

public:
  using extends::extends;

  ~ToolSvc() override;

  /// Finalize the service.
  StatusCode finalize() override;

  // Start transition for tools
  StatusCode start() override;

  // Stop transition for tools
  StatusCode stop() override;

  /// Retrieve tool, create it by default as common tool if it does not already exist
  StatusCode retrieve( std::string_view type, const InterfaceID& iid, IAlgTool*& tool, const IInterface* parent,
                       bool createIf ) override;

  /// Retrieve tool, create it by default as common tool if it does not already exist
  StatusCode retrieve( std::string_view tooltype, std::string_view toolname, const InterfaceID& iid, IAlgTool*& tool,
                       const IInterface* parent, bool createIf ) override;

  /// Get names of all tool instances of a given type
  std::vector<std::string> getInstances( std::string_view toolType ) override;

  /// Get names of all tool instances
  std::vector<std::string> getInstances() const override;

  /// Get pointers to all tool instances
  std::vector<IAlgTool*> getTools() const override;

  /// Release tool
  StatusCode releaseTool( IAlgTool* tool ) override;

  /// Create Tool standard way with automatically assigned name
  StatusCode create( const std::string& type, const IInterface* parent, IAlgTool*& tool );

  /// Create Tool standard way with specified name
  StatusCode create( const std::string& type, const std::string& name, const IInterface* parent, IAlgTool*& tool );

  /// Check if the tool instance exists
  bool existsTool( std::string_view fullname ) const;

  /// Get Tool full name by combining nameByUser and "parent" part
  std::string nameTool( std::string_view nameByUser, const IInterface* parent );

  void registerObserver( IToolSvc::Observer* obs ) override;

private:
  /// Finalize the given tool, with exception handling
  StatusCode finalizeTool( IAlgTool* itool ) const;

  Gaudi::Property<bool> m_checkNamedToolsConfigured{
      this, "CheckedNamedToolsConfigured", false,
      "Check that tools which do not have the default name have some explicit configuration." };

  Gaudi::Property<bool> m_showToolDataDeps{ this, "ShowDataDeps", false, "show the data dependencies of AlgTools" };

  /// Common Tools
  class ToolList {
    std::vector<IAlgTool*> m_tools; // List of all instances of tools
    struct Hash {
      using is_transparent = void;
      std::size_t operator()( IAlgTool const* s ) const noexcept { return std::hash<std::string_view>{}( s->name() ); }
      std::size_t operator()( std::string_view s ) const noexcept { return std::hash<std::string_view>{}( s ); }
    };
    struct Equal {
      using is_transparent = void;
      bool operator()( IAlgTool const* lhs, IAlgTool const* rhs ) const { return lhs->name() == rhs->name(); }
      bool operator()( IAlgTool const* lhs, std::string_view rhs ) const { return lhs->name() == rhs; }
      bool operator()( std::string_view lhs, IAlgTool const* rhs ) const { return lhs == rhs->name(); }
    };
    std::unordered_multiset<IAlgTool*, Hash, Equal> m_map;

  public:
    void remove( IAlgTool* tool ) {
      m_tools.erase( std::remove( std::begin( m_tools ), std::end( m_tools ), tool ), std::end( m_tools ) );
      auto range = m_map.equal_range( tool->name() );
      auto itm   = std::find_if( range.first, range.second, [&]( auto const& p ) { return p == tool; } );
      if ( itm != range.second ) m_map.erase( itm );
    }
    void push_back( IAlgTool* tool ) {
      m_tools.push_back( tool );
      m_map.emplace( tool );
    }

    bool contains( std::string_view name ) const { return m_map.find( name ) != m_map.end(); }
    bool contains( IAlgTool const* tool ) const {
      return m_tools.rend() != std::find( m_tools.rbegin(), m_tools.rend(), tool );
      // return tool == find( tool->name(), tool->parent() ); //FIXME: is this faster?
    }
    auto size() const { return m_tools.size(); }
    auto begin() const { return m_tools.begin(); }
    auto end() const { return m_tools.end(); }
    auto find( std::string_view name, const IInterface* parent ) const {
      auto range = m_map.equal_range( name );
      auto it    = std::find_if( range.first, range.second, [&]( auto const& p ) { return p->parent() == parent; } );
      return it != range.second ? *it : nullptr;
    }
    std::vector<IAlgTool*> grab() && {
      m_map.clear();
      auto tools = std::move( m_tools );
      return tools;
    }
  };
  mutable std::recursive_mutex m_mut; // protect m_instancesTools
  ToolList                     m_instancesTools;

  /// Pointer to HistorySvc
  SmartIF<IHistorySvc> m_pHistorySvc;

  std::vector<IToolSvc::Observer*> m_observers;
};

namespace {
  /** The total number of refCounts on all tools in the instancesTools list */
  template <typename C>
  unsigned long totalRefCount( const C& toolList ) {
    return std::accumulate( begin( toolList ), end( toolList ), 0ul,
                            [&]( unsigned long count, const IAlgTool* tool ) { return count + tool->refCount(); } );
  }

  /** The minimum number of refCounts of all tools */
  template <typename C>
  unsigned long minimumRefCount( const C& toolList ) {
    return std::accumulate( begin( toolList ), end( toolList ), ~0ul,
                            []( unsigned long c, const IAlgTool* tool ) { return std::min( c, tool->refCount() ); } );
  }

} // namespace

DECLARE_COMPONENT( ToolSvc )

ToolSvc::~ToolSvc() {
  // tell the remaining observers that we're gone, and forget about unregistering..
  std::for_each( std::begin( m_observers ), std::end( m_observers ),
                 [&]( IToolSvc::Observer* obs ) { obs->setUnregister( {} ); } );
}

StatusCode ToolSvc::finalize() {
  // Finalize and delete all left-over tools. Normally all tools created with
  // ToolSvc are left over, since ToolSvc holds a refCount (via AlgTool ctor).
  // Several cases need to be covered:
  // 1) Simple dependencies: no circular dependencies between tools,
  //    and tools not using other tools
  // 2) Tools-using-tools (but no circular dependencies)
  //   a) release() is called in the tool::finalize() (e.g. via base class)
  //   b) release() is called in the tool destructor  (e.g. via ToolHandle)
  // 3) Circular dependencies between tools
  //   a) release() is called in the tool::finalize() (e.g. via base class)
  //   b) release() is called in the tool destructor  (e.g. via ToolHandle)
  // 4) In addition to each of the above cases, refCounting of a particular tool
  //    might not have been done correctly in the code. Typically release()
  //    is not called, so we are left with a too high refCount.
  //    What to do with those, and how to avoid a crash while handling them...

  /** Algorithm: 2 passes.
      First pass:
        - do not explicitly release any tools
        - finalize tools, in the order of increasing number of refCounts
      Second pass:
        - explicitly release all tools, one release() on all tools per loop.
        -> tools are deleted in the order of increasing number of refCounts.
  */
  info() << "Removing all tools created by ToolSvc" << endmsg;
  auto tools = std::move( m_instancesTools ).grab();

  // Print out list of tools
  ON_DEBUG {
    auto& log = debug();
    log << "  Tool List : ";
    for ( const auto& iTool : tools ) { log << iTool->name() << ":" << iTool->refCount() << " "; }
    log << endmsg;
  }

  //
  // first pass: Finalize all tools (but don't delete them)
  //
  /** Inner loop: full loop over all left-over tools
        + finalize tools with the minimum number of refCounts in the list.
        + Remove finalized tools from list of tools, and add them to the
          list of finalized tools, to be deleted at the end. This way,
          any non-finalized tools who still reference already finalized
          tools in their finalize() will still find a live tool.
      Outer loop: keep on going until nothing changes in the list of tools.
        Checking for:
        + number of left-over tools
        + total number of refcounts
        + minimum number of refcounts
  */
  boost::circular_buffer<IAlgTool*> finalizedTools( tools.size() ); // list of tools that have been finalized
  bool                              fail( false );
  size_t                            toolCount        = tools.size();
  unsigned long                     startRefCount    = 0;
  unsigned long                     endRefCount      = totalRefCount( tools );
  unsigned long                     startMinRefCount = 0;
  unsigned long                     endMinRefCount   = minimumRefCount( tools );
  while ( toolCount > 0 && endRefCount > 0 && ( endRefCount != startRefCount || endMinRefCount != startMinRefCount ) ) {
    ON_DEBUG if ( endMinRefCount != startMinRefCount ) {
      debug() << toolCount << " tools left to finalize. Summed refCounts: " << endRefCount << endmsg;
      debug() << "Will finalize tools with refCount <= " << endMinRefCount << endmsg;
    }
    startMinRefCount      = endMinRefCount;
    startRefCount         = endRefCount;
    unsigned long maxLoop = toolCount + 1;
    while ( --maxLoop > 0 && !tools.empty() ) {
      IAlgTool* pTool = tools.back();
      // removing tool from list makes ToolSvc::releaseTool( IAlgTool* ) a noop
      tools.pop_back();
      unsigned long count = pTool->refCount();
      // cache tool name
      const std::string& toolName = pTool->name();
      if ( count <= startMinRefCount ) {
        ON_DEBUG debug() << "  Performing finalization of " << toolName << " (refCount " << count << ")" << endmsg;
        // finalize of one tool may trigger a release of another tool
        //   pTool->sysFinalize().ignore();
        if ( !finalizeTool( pTool ).isSuccess() ) {
          warning() << "    FAILURE finalizing " << toolName << endmsg;
          fail = true;
        }
        // postpone deletion
        finalizedTools.push_back( pTool );
      } else {
        // Place back at the front of the list to try again later
        // ToolSvc::releaseTool( IAlgTool* ) remains active for this tool
        ON_DEBUG debug() << "  Delaying   finalization of " << toolName << " (refCount " << count << ")" << endmsg;
        tools.insert( std::begin( tools ), pTool );
      }
    } // end of inner loop
    toolCount      = tools.size();
    endRefCount    = totalRefCount( tools );
    endMinRefCount = minimumRefCount( tools );
  }; // end of outer loop

  //
  // Second pass: Delete all finalized tools
  //
  // Delete in the order of increasing number of refCounts.
  // Loop over tools in the same order as the order in which they were finalized.
  // All tools in the list of finalized tools are no longer in the instancesTools list.
  // If a tool destructor calls releaseTool() on another tool, this will have no
  // effect on the 'other tool' if this 'other tool' is in the list of finalized tools.
  // If this 'other tool' is still in the instancesTools list, it may trigger finalization
  // (in releaseTool()), deletion and removal from the instancesTools list.
  // Therefore, the check on non-finalised tools should happen *after* the deletion
  // of the finalized tools.
  ON_DEBUG debug() << "Deleting " << finalizedTools.size() << " finalized tools" << endmsg;
  auto     maxLoop = totalRefCount( finalizedTools ) + 1;
  while ( --maxLoop > 0 && !finalizedTools.empty() ) {
    IAlgTool* pTool = finalizedTools.front();
    finalizedTools.pop_front();
    auto count = pTool->refCount();
    if ( count == 1 ) {
      ON_DEBUG debug() << "  Performing deletion of " << pTool->name() << endmsg;
    } else {
      ON_VERBOSE verbose() << "  Delaying   deletion of " << pTool->name() << " (refCount " << count << ")" << endmsg;
      // Move to the end when refCount still not zero
      finalizedTools.push_back( pTool );
    }
    // do a forced release
    pTool->release();
  }

  // Error if by now not all tools are properly finalised
  if ( !tools.empty() ) {
    error() << "Unable to finalize and delete the following tools : ";
    for ( const auto& iTool : tools ) { error() << iTool->name() << ": " << iTool->refCount() << " "; }
    error() << endmsg;
  }

  // by now, all tools should be deleted and removed.
  if ( !finalizedTools.empty() ) {
    error() << "Failed to delete the following " << finalizedTools.size()
            << " finalized tools. Bug in ToolSvc::finalize()?: ";
    for ( const auto& iTool : finalizedTools ) { error() << iTool->name() << ": " << iTool->refCount() << " "; }
    error() << endmsg;
  }

  // Finalize this specific service
  return ( Service::finalize().isSuccess() && !fail ) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

namespace {
  constexpr std::string_view s_PUBLIC = ":PUBLIC";
}

StatusCode ToolSvc::retrieve( std::string_view tooltype, const InterfaceID& iid, IAlgTool*& tool,
                              const IInterface* parent, bool createIf ) {
  // check for tools, which by name are required to be public:
  if ( tooltype.ends_with( s_PUBLIC ) ) {
    // parent for PUBLIC tool is 'this', i.e. ToolSvc
    tooltype.remove_suffix( s_PUBLIC.size() );
    return retrieve( tooltype, iid, tool, this, createIf );
  }

  // protect against empty type
  if ( tooltype.empty() ) {
    error() << "retrieve(): No Tool Type/Name given" << endmsg;
    return StatusCode::FAILURE;
  }
  auto pos = tooltype.find( '/' );
  if ( std::string_view::npos == pos ) { return retrieve( tooltype, tooltype, iid, tool, parent, createIf ); }
  return retrieve( tooltype.substr( 0, pos ), tooltype.substr( pos + 1 ), iid, tool, parent, createIf );
}

StatusCode ToolSvc::retrieve( std::string_view tooltype, std::string_view toolname, const InterfaceID& iid,
                              IAlgTool*& tool, const IInterface* parent, bool createIf ) {
  // check the applicability of another method:
  // ignore the provided name if it is empty or the type contains a name
  if ( toolname.empty() || ( std::string_view::npos != tooltype.find( '/' ) ) ) {
    return retrieve( tooltype, iid, tool, parent, createIf );
  }

  // check for tools, which by name are required to be public:
  if ( toolname.ends_with( s_PUBLIC ) ) {
    // parent for PUBLIC tool is this, i.e. ToolSvc
    toolname.remove_suffix( s_PUBLIC.size() );
    return retrieve( tooltype, toolname, iid, tool, this, createIf );
  }

  StatusCode sc( StatusCode::FAILURE );

  tool = nullptr;

  // If parent is not specified it means it is the ToolSvc itself
  if ( !parent ) parent = this;
  const std::string fullname = nameTool( toolname, parent );

  // Find tool in list of those already existing, and tell its
  // interface that it has been used one more time
  auto      lock  = std::scoped_lock{ m_mut };
  IAlgTool* itool = m_instancesTools.find( fullname, parent );
  if ( itool ) { ON_DEBUG debug() << "Retrieved tool " << toolname << " with parent " << parent << endmsg; }

  if ( !itool ) {
    // Instances of this tool do not exist, create an instance if desired
    // otherwise return failure
    if ( !createIf ) {
      warning() << "Tool " << toolname << " not found and creation not requested" << endmsg;
      return sc;
    }
    sc = create( std::string{ tooltype }, std::string{ toolname }, parent, itool );
    if ( sc.isFailure() ) { return sc; }
  }

  // Get the right interface of it
  sc = itool->queryInterface( iid, pp_cast<void>( &tool ) );
  if ( sc.isFailure() ) {
    error() << "Tool " << toolname << " either does not implement the correct interface, or its version is incompatible"
            << endmsg;
    return sc;
  }

  ///////////////
  /// invoke retrieve callbacks...
  ///////////////
  std::for_each( std::begin( m_observers ), std::end( m_observers ),
                 [&]( IToolSvc::Observer* obs ) { obs->onRetrieve( itool ); } );
  return sc;
}

std::vector<std::string> ToolSvc::getInstances( std::string_view toolType ) {

  std::vector<std::string> tools;
  auto                     lock = std::scoped_lock{ m_mut };
  for ( const auto& tool : m_instancesTools ) {
    if ( tool->type() == toolType ) tools.push_back( tool->name() );
  }
  return tools;
}

std::vector<std::string> ToolSvc::getInstances() const {
  auto                     lock = std::scoped_lock{ m_mut };
  std::vector<std::string> tools{ m_instancesTools.size() };
  std::transform( std::begin( m_instancesTools ), std::end( m_instancesTools ), std::begin( tools ),
                  []( const IAlgTool* t ) { return t->name(); } );
  return tools;
}

std::vector<IAlgTool*> ToolSvc::getTools() const {
  auto lock = std::scoped_lock{ m_mut };
  return { std::begin( m_instancesTools ), std::end( m_instancesTools ) };
}

StatusCode ToolSvc::releaseTool( IAlgTool* tool ) {
  auto       lock = std::scoped_lock{ m_mut };
  StatusCode sc( StatusCode::SUCCESS );
  // test if tool is in known list (protect trying to access a previously deleted tool)
  if ( m_instancesTools.contains( tool ) ) {
    unsigned long count = tool->refCount();
    if ( count == 1 ) {
      // finalize the tool

      if ( Gaudi::StateMachine::OFFLINE == m_targetState ) {
        // We are being called during ToolSvc::finalize()
        // message format matches the one in ToolSvc::finalize()
        debug() << "  Performing finalization of " << tool->name() << " (refCount " << count << ")" << endmsg;
        // message format matches the one in ToolSvc::finalize()
        debug() << "  Performing     deletion of " << tool->name() << endmsg;
      } else {
        debug() << "Performing finalization and deletion of " << tool->name() << endmsg;
      }
      sc = finalizeTool( tool );
      // remove from known tools...
      m_instancesTools.remove( tool );
    }
    tool->release();
  }
  return sc;
}

StatusCode ToolSvc::create( const std::string& tooltype, const IInterface* parent, IAlgTool*& tool ) {
  const std::string& toolname = tooltype;
  return create( tooltype, toolname, parent, tool );
}

namespace {
  /// Small class to allow a safe roll-back if the tool is not
  /// correctly initialized or there are problems.
  template <typename T>
  class ToolCreateGuard {
    /// list of tools
    T& m_tools;
    /// pointer to be set
    std::unique_ptr<IAlgTool> m_tool;

  public:
    explicit ToolCreateGuard( T& tools ) : m_tools( tools ) {}
    // we don't get a move constructor by default because we
    // have a non-trivial destructor... However, the default
    // one is just fine...
    ToolCreateGuard( ToolCreateGuard&& ) noexcept = default;
    /// Set the internal pointer (delete any previous one). Get ownership of the tool.
    void create( const std::string& tooltype, const std::string& fullname, const IInterface* parent ) {
      // remove previous content
      if ( m_tool ) { m_tools.remove( m_tool.get() ); };
      m_tool = AlgTool::Factory::create( tooltype, tooltype, fullname, parent );
      // set new content
      if ( m_tool ) { m_tools.push_back( m_tool.get() ); }
    }
    /// Get the internal pointer
    IAlgTool* get() { return m_tool.get(); }
    IAlgTool* operator->() const {
      assert( m_tool );
      return m_tool.get();
    }
    /// Return the internal pointer and give ownership.
    IAlgTool* release() { return m_tool.release(); }
    /// remove it from the list.
    ~ToolCreateGuard() {
      if ( m_tool ) m_tools.remove( m_tool.get() );
    }
  };

  template <typename C>
  ToolCreateGuard<C> make_toolCreateGuard( C& c ) {
    return ToolCreateGuard<C>{ c };
  }
} // namespace

/**
 * Now able to handle clones. The test of tool existence is performed according to
 * three criteria: name, type and parent.
 * If a tool is private, i.e. the parent is not the tool Svc, and it exist but
 * the parent is not the specified one, a clone is handed over.
 * No clones of public tools are allowed since they would be undistinguishable.
 **/
StatusCode ToolSvc::create( const std::string& tooltype, const std::string& toolname, const IInterface* parent,
                            IAlgTool*& tool ) {

  // protect against empty type
  if ( tooltype.empty() ) {
    error() << "create(): No Tool Type given" << endmsg;
    return StatusCode::FAILURE;
  }

  // If parent has not been specified, assume it is the ToolSvc
  if ( !parent ) parent = this;

  tool = nullptr;
  // Automatically deletes the tool if not explicitly kept (i.e. on success).
  // The tool is removed from the list of known tools too.
  auto lock      = std::scoped_lock{ m_mut };
  auto toolguard = make_toolCreateGuard( m_instancesTools );

  // Check if the tool already exist : this could happen with clones
  std::string fullname = nameTool( toolname, parent );
  if ( existsTool( fullname ) ) {
    // Now check if the parent is the same. This allows for clones
    for ( IAlgTool* iAlgTool : m_instancesTools ) {
      if ( iAlgTool->name() == toolname && iAlgTool->parent() == parent ) {
        // The tool exist with this name, type and parent: this is bad!
        // This excludes the possibility of cloning public tools intrinsecally
        error() << "Tool " << fullname << " already exists with the same parent" << endmsg;
        if ( parent == this )
          error() << "... In addition, the parent is the ToolSvc: public tools cannot be cloned!" << endmsg;

        return StatusCode::FAILURE;
      }
    }
    ON_DEBUG debug() << "Creating clone of " << fullname << endmsg;
  }
  // instantiate the tool using the factory
  try {
    toolguard.create( tooltype, fullname, parent );
    if ( !toolguard.get() ) {
      error() << "Cannot create tool " << tooltype << " (No factory found)" << endmsg;
      return StatusCode::FAILURE;
    }
  } catch ( const GaudiException& Exception ) {
    // (1) perform the printout of message
    fatal() << "Exception with tag=" << Exception.tag() << " is caught whilst instantiating tool '" << tooltype << "'"
            << endmsg;
    // (2) print  the exception itself
    // (NB!  - GaudiException is a linked list of all "previous exceptions")
    fatal() << Exception << endmsg;
    return StatusCode::FAILURE;
  } catch ( const std::exception& Exception ) {
    // (1) perform the printout of message
    fatal() << "Standard std::exception is caught whilst instantiating tool '" << tooltype << "'" << endmsg;
    // (2) print  the exception itself
    // (NB!  - GaudiException is a linked list of all "previous exceptions")
    fatal() << Exception.what() << endmsg;
    return StatusCode::FAILURE;
  } catch ( ... ) {
    // (1) perform the printout
    fatal() << "UNKNOWN Exception is caught whilst instantiating tool '" << tooltype << "'" << endmsg;
    return StatusCode::FAILURE;
  }
  ON_VERBOSE verbose() << "Created tool " << tooltype << "/" << fullname << endmsg;

  // Since only AlgTool has the setProperties() method it is necessary to cast
  // to downcast IAlgTool to AlgTool in order to set the properties via the JobOptions
  // service
  AlgTool* mytool = dynamic_cast<AlgTool*>( toolguard.get() );
  if ( mytool ) mytool->bindPropertiesTo( serviceLocator()->getOptsSvc() );

  // Initialize the Tool
  StatusCode sc = StatusCode::FAILURE;
  try {
    sc = toolguard->sysInitialize();
  }
  // Catch any exceptions
  catch ( const GaudiException& Exception ) {
    error() << "GaudiException with tag=" << Exception.tag() << " caught whilst initializing tool '" << fullname << "'"
            << endmsg << Exception << endmsg;
    return StatusCode::FAILURE;
  } catch ( const std::exception& Exception ) {
    error() << "Standard std::exception caught whilst initializing tool '" << fullname << "'" << endmsg
            << Exception.what() << endmsg;
    return StatusCode::FAILURE;
  } catch ( ... ) {
    error() << "UNKNOWN Exception caught whilst initializing tool '" << fullname << "'" << endmsg;
    return StatusCode::FAILURE;
  }

  // Status of tool initialization
  if ( sc.isFailure() ) {
    error() << "Error initializing tool '" << fullname << "'" << endmsg;
    return sc;
  }

  if ( m_checkNamedToolsConfigured ) {
    // Check to ensure that non-default named tools are configured.
    if ( toolname != tooltype ) {
      bool                            propsSet = false;
      Gaudi::Interfaces::IOptionsSvc& joSvc    = this->serviceLocator()->getOptsSvc();
      // Check that at least one of the properties has been set:
      for ( const auto prop : mytool->getProperties() ) {
        bool isSet = joSvc.isSet( mytool->name() + "." + prop->name() );
        if ( isSet ) propsSet = true;
      }
      if ( !propsSet ) {
        warning() << tooltype << "/" << fullname
                  << " : Explicitly named tools should be configured! (assigned name=" << toolname << ", default is "
                  << tooltype << ")" << endmsg;
      }
    }
  }

  // Start the tool if we are running.
  if ( m_state == Gaudi::StateMachine::RUNNING ) {
    sc = toolguard->sysStart();

    if ( sc.isFailure() ) {
      error() << "Error starting tool '" << fullname << "'" << endmsg;
      return sc;
    }
  }

  // The tool has been successfully created and initialized,
  // so we inform the guard that it can release it
  tool = toolguard.release();

  ///////////////
  /// invoke create callbacks...
  ///////////////
  std::for_each( m_observers.begin(), m_observers.end(), [&]( IToolSvc::Observer* obs ) { obs->onCreate( tool ); } );
  // TODO: replace by generic callback
  // Register the tool with the HistorySvc
  m_pHistorySvc = service( "HistorySvc", false );
  if ( m_pHistorySvc ) { m_pHistorySvc->registerAlgTool( *tool ).ignore(); }
  return StatusCode::SUCCESS;
}

std::string ToolSvc::nameTool( std::string_view toolname, const IInterface* parent ) {

  if ( !parent ) { return std::string{ this->name() }.append( "." ).append( toolname ); }

  // check that parent has a name!
  auto named_parent = SmartIF<INamedInterface>( const_cast<IInterface*>( parent ) );
  if ( named_parent ) {
    auto fullname = std::string{ named_parent->name() }.append( "." ).append( toolname );
    return fullname;
  }

  error() << "Private Tools only allowed for components implementing INamedInterface" << endmsg;
  //
  return std::string{ "." }.append( toolname );
}

bool ToolSvc::existsTool( std::string_view fullname ) const {
  auto lock = std::scoped_lock{ m_mut };
  return m_instancesTools.contains( fullname );
}

StatusCode ToolSvc::finalizeTool( IAlgTool* itool ) const {

  // Cache tool name in case of errors
  const auto& toolName = itool->name();
  StatusCode  sc;

  // Finalise the tool inside a try block
  try {
    sc = itool->sysFinalize();
  }
  // Catch any exceptions
  catch ( const GaudiException& Exception ) {
    error() << "GaudiException with tag=" << Exception.tag() << " caught whilst finalizing tool '" << toolName << "'"
            << endmsg << Exception << endmsg;
    sc = StatusCode::FAILURE;
  } catch ( const std::exception& Exception ) {
    error() << "Standard std::exception caught whilst finalizing tool '" << toolName << "'" << endmsg
            << Exception.what() << endmsg;
    sc = StatusCode::FAILURE;
  } catch ( ... ) {
    error() << "UNKNOWN Exception caught whilst finalizing tool '" << toolName << "'" << endmsg;
    sc = StatusCode::FAILURE;
  }

  return sc;
}

void ToolSvc::registerObserver( IToolSvc::Observer* obs ) {
  if ( !obs ) throw GaudiException( "Received NULL pointer", this->name() + "::registerObserver", StatusCode::FAILURE );

  auto lock = std::scoped_lock{ m_mut };
  obs->setUnregister( [this, obs]() {
    auto lock = std::scoped_lock{ m_mut };
    auto i    = std::find( m_observers.begin(), m_observers.end(), obs );
    if ( i != m_observers.end() ) m_observers.erase( i );
  } );
  m_observers.push_back( obs );
}

StatusCode ToolSvc::start() {

  ON_DEBUG debug() << "START transition for AlgTools" << endmsg;

  if ( m_showToolDataDeps.value() ) {
    info() << "Listing Data Dependencies of all Tools";
    for ( auto& iTool : m_instancesTools ) {
      IDataHandleHolder* idh = dynamic_cast<IDataHandleHolder*>( iTool );
      if ( idh ) {
        std::ostringstream ost;
        for ( auto& dh : idh->inputHandles() ) { ost << "\n   INPUT       " << dh->fullKey(); }
        for ( auto& id : idh->extraInputDeps() ) { ost << "\n   EXTRA INPUT " << id; }
        for ( auto& dh : idh->outputHandles() ) { ost << "\n   OUTPUT       " << dh->fullKey(); }
        for ( auto& id : idh->extraOutputDeps() ) { ost << "\n   EXTRA OUTPUT " << id; }
        if ( ost.str().length() > 0 ) { info() << "\n" << iTool->name() << ost.str(); }
      } else {
        error() << "can't cast " << iTool->name() << " to IDataHandleHolder!" << endmsg;
      }
    }
    info() << endmsg;
  }

  bool fail( false );
  for ( auto& iTool : m_instancesTools ) {
    ON_VERBOSE verbose() << iTool->name() << "::start()" << endmsg;

    if ( !iTool->sysStart().isSuccess() ) {
      fail = true;
      error() << iTool->name() << " failed to start()" << endmsg;
    }
  }

  if ( fail ) {
    error() << "One or more AlgTools failed to start()" << endmsg;
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode ToolSvc::stop() {

  ON_DEBUG debug() << "STOP transition for AlgTools" << endmsg;

  bool fail( false );
  for ( auto& iTool : m_instancesTools ) {
    ON_VERBOSE verbose() << iTool->name() << "::stop()" << endmsg;

    if ( !iTool->sysStop().isSuccess() ) {
      fail = true;
      error() << iTool->name() << " failed to stop()" << endmsg;
    }
  }

  if ( fail ) {
    error() << "One or more AlgTools failed to stop()" << endmsg;
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}
