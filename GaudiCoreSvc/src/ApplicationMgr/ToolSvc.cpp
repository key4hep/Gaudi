// Include Files
#include "ToolSvc.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IHistorySvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Service.h"
#include "boost/algorithm/string/erase.hpp"
#include "boost/algorithm/string/predicate.hpp"
#include "boost/circular_buffer.hpp"
#include <algorithm>
#include <cassert>
#include <functional>
#include <map>
#include <numeric>
#include <string>
namespace ba = boost::algorithm;

#define ON_DEBUG if ( msgLevel( MSG::DEBUG ) )
#define ON_VERBOSE if ( msgLevel( MSG::VERBOSE ) )

namespace
{
  //------------------------------------------------------------------------------
  template <typename C>
  unsigned long totalRefCount( const C& toolList )
  //------------------------------------------------------------------------------
  {
    return std::accumulate( std::begin( toolList ), std::end( toolList ), 0ul,
                            [&]( unsigned long count, const IAlgTool* tool ) { return count + tool->refCount(); } );
  }

  /// small helper functions
  template <typename C>
  void remove( C& c, typename C::const_reference i )
  {
    c.erase( std::remove( std::begin( c ), std::end( c ), i ), std::end( c ) );
  }
}

// Instantiation of a static factory class used by clients to create
//  instances of this service
DECLARE_COMPONENT( ToolSvc )

//------------------------------------------------------------------------------
ToolSvc::~ToolSvc()
{
  // tell the remaining observers that we're gone, and forget about unregistering..
  std::for_each( std::begin( m_observers ), std::end( m_observers ),
                 [&]( IToolSvc::Observer* obs ) { obs->setUnregister( {} ); } );
}
//------------------------------------------------------------------------------
StatusCode ToolSvc::initialize()
//------------------------------------------------------------------------------
{

  // initialize the Service Base class
  StatusCode status = Service::initialize();
  if ( UNLIKELY( status.isFailure() ) ) {
    error() << "Unable to initialize the Service" << endmsg;
    return status;
  }

  // set my own (ToolSvc) properties via the jobOptionService
  if ( UNLIKELY( setProperties().isFailure() ) ) {
    error() << "Unable to set base properties" << endmsg;
    return StatusCode::FAILURE;
  }

  return status;
}

//------------------------------------------------------------------------------
StatusCode ToolSvc::finalize()
//------------------------------------------------------------------------------
{
  // Finalize and delete all left-over tools. Normally all tools created with
  // ToolSvc are left over, since ToolSvc holds a refCount (via AlgTool ctor).
  // Several cases need to be covered:
  // 1) Simple dependencies: no circular dependencies between tools,
  //    and tools not using other tools
  // 2) Tools-using-tools (but no circular dependencies)
  //   a) release() is called in the tool::finalize() (e.g. via GaudiAlgorithm)
  //   b) release() is called in the tool destructor  (e.g. via ToolHandle)
  // 3) Circular dependencies between tools
  //   a) release() is called in the tool::finalize() (e.g. via GaudiAlgorithm)
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

  // Print out list of tools
  ON_DEBUG
  {
    auto& log = debug();
    log << "  Tool List : ";
    for ( const auto& iTool : m_instancesTools ) {
      log << iTool->name() << ":" << iTool->refCount() << " ";
    }
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
  boost::circular_buffer<IAlgTool*> finalizedTools( m_instancesTools.size() ); // list of tools that have been finalized
  bool fail( false );
  size_t toolCount               = m_instancesTools.size();
  unsigned long startRefCount    = 0;
  unsigned long endRefCount      = totalToolRefCount();
  unsigned long startMinRefCount = 0;
  unsigned long endMinRefCount   = minimumToolRefCount();
  while ( toolCount > 0 && endRefCount > 0 && ( endRefCount != startRefCount || endMinRefCount != startMinRefCount ) ) {
    ON_DEBUG if ( endMinRefCount != startMinRefCount )
    {
      debug() << toolCount << " tools left to finalize. Summed refCounts: " << endRefCount << endmsg;
      debug() << "Will finalize tools with refCount <= " << endMinRefCount << endmsg;
    }
    startMinRefCount      = endMinRefCount;
    startRefCount         = endRefCount;
    unsigned long maxLoop = toolCount + 1;
    while ( --maxLoop > 0 && !m_instancesTools.empty() ) {
      IAlgTool* pTool = m_instancesTools.back();
      // removing tool from list makes ToolSvc::releaseTool( IAlgTool* ) a noop
      m_instancesTools.pop_back();
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
        m_instancesTools.insert( std::begin( m_instancesTools ), pTool );
      }
    } // end of inner loop
    toolCount      = m_instancesTools.size();
    endRefCount    = totalToolRefCount();
    endMinRefCount = minimumToolRefCount();
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
  auto maxLoop = totalRefCount( finalizedTools ) + 1;
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
  if ( !m_instancesTools.empty() ) {
    error() << "Unable to finalize and delete the following tools : ";
    for ( const auto& iTool : m_instancesTools ) {
      error() << iTool->name() << ": " << iTool->refCount() << " ";
    }
    error() << endmsg;
  }

  // by now, all tools should be deleted and removed.
  if ( !finalizedTools.empty() ) {
    error() << "Failed to delete the following " << finalizedTools.size()
            << " finalized tools. Bug in ToolSvc::finalize()?: ";
    for ( const auto& iTool : finalizedTools ) {
      error() << iTool->name() << ": " << iTool->refCount() << " ";
    }
    error() << endmsg;
  }

  if ( m_pHistorySvc ) m_pHistorySvc->release();

  // Finalize this specific service
  return ( Service::finalize().isSuccess() && !fail ) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

// ===================================================================================
/** the indicator for the tool to be "PUBLIC"
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 */
// ===================================================================================
namespace
{
  static const std::string s_PUBLIC = ":PUBLIC";
}

//------------------------------------------------------------------------------
StatusCode ToolSvc::retrieve( const std::string& tooltype, const InterfaceID& iid, IAlgTool*& tool,
                              const IInterface* parent, bool createIf )
//------------------------------------------------------------------------------
{
  // check for tools, which by name are required to be public:
  if ( ba::ends_with( tooltype, s_PUBLIC ) ) {
    // parent for PUBLIC tool is 'this', i.e. ToolSvc
    return retrieve( ba::erase_tail_copy( tooltype, s_PUBLIC.size() ), iid, tool, this, createIf );
  }

  // protect against empty type
  if ( tooltype.empty() ) {
    error() << "retrieve(): No Tool Type/Name given" << endmsg;
    return StatusCode::FAILURE;
  }
  auto pos = tooltype.find( '/' );
  if ( std::string::npos == pos ) {
    return retrieve( tooltype, tooltype, iid, tool, parent, createIf );
  }
  return retrieve( tooltype.substr( 0, pos ), tooltype.substr( pos + 1 ), iid, tool, parent, createIf );
}

// ===================================================================================

//------------------------------------------------------------------------------
StatusCode ToolSvc::retrieve( const std::string& tooltype, const std::string& toolname, const InterfaceID& iid,
                              IAlgTool*& tool, const IInterface* parent, bool createIf )
//------------------------------------------------------------------------------
{
  // check the applicability of another method:
  // ignore the provided name if it is empty or the type contains a name
  if ( toolname.empty() || ( std::string::npos != tooltype.find( '/' ) ) ) {
    return retrieve( tooltype, iid, tool, parent, createIf );
  }

  // check for tools, which by name are required to be public:
  if ( ba::ends_with( toolname, s_PUBLIC ) ) {
    // parent for PUBLIC tool is this, i.e. ToolSvc
    return retrieve( tooltype, ba::erase_tail_copy( toolname, s_PUBLIC.size() ), iid, tool, this, createIf );
  }

  std::lock_guard<CallMutex> lock( m_mut );

  IAlgTool* itool = nullptr;
  StatusCode sc( StatusCode::FAILURE );

  tool = nullptr;

  // If parent is not specified it means it is the ToolSvc itself
  if ( !parent ) parent      = this;
  const std::string fullname = nameTool( toolname, parent );

  // Find tool in list of those already existing, and tell its
  // interface that it has been used one more time
  auto it = std::find_if( std::begin( m_instancesTools ), std::end( m_instancesTools ),
                          [&]( const IAlgTool* i ) { return i->name() == fullname && i->parent() == parent; } );
  if ( it != std::end( m_instancesTools ) ) {
    ON_DEBUG debug() << "Retrieved tool " << toolname << " with parent " << parent << endmsg;
    itool = *it;
  }

  if ( !itool ) {
    // Instances of this tool do not exist, create an instance if desired
    // otherwise return failure
    if ( UNLIKELY( !createIf ) ) {
      warning() << "Tool " << toolname << " not found and creation not requested" << endmsg;
      return sc;
    }
    sc = create( tooltype, toolname, parent, itool );
    if ( sc.isFailure() ) {
      return sc;
    }
  }

  // Get the right interface of it
  sc = itool->queryInterface( iid, pp_cast<void>( &tool ) );
  if ( UNLIKELY( sc.isFailure() ) ) {
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
//------------------------------------------------------------------------------
std::vector<std::string> ToolSvc::getInstances( const std::string& toolType )
//------------------------------------------------------------------------------
{

  std::lock_guard<CallMutex> lock( m_mut );
  std::vector<std::string> tools;
  for ( const auto& tool : m_instancesTools ) {
    if ( tool->type() == toolType ) tools.push_back( tool->name() );
  }
  return tools;
}
//------------------------------------------------------------------------------
std::vector<std::string> ToolSvc::getInstances() const
//------------------------------------------------------------------------------
{
  std::lock_guard<CallMutex> lock( m_mut );
  std::vector<std::string> tools{m_instancesTools.size()};
  std::transform( std::begin( m_instancesTools ), std::end( m_instancesTools ), std::begin( tools ),
                  []( const IAlgTool* t ) { return t->name(); } );
  return tools;
}
//------------------------------------------------------------------------------
std::vector<IAlgTool*> ToolSvc::getTools() const
//------------------------------------------------------------------------------
{
  std::lock_guard<CallMutex> lock( m_mut );
  return std::vector<IAlgTool*>{std::begin( m_instancesTools ), std::end( m_instancesTools )};
}
//------------------------------------------------------------------------------
StatusCode ToolSvc::releaseTool( IAlgTool* tool )
//------------------------------------------------------------------------------
{
  std::lock_guard<CallMutex> lock( m_mut );
  StatusCode sc( StatusCode::SUCCESS );
  // test if tool is in known list (protect trying to access a previously deleted tool)
  if ( m_instancesTools.rend() != std::find( m_instancesTools.rbegin(), m_instancesTools.rend(), tool ) ) {
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
      remove( m_instancesTools, tool );
    }
    tool->release();
  }
  return sc;
}

//------------------------------------------------------------------------------
StatusCode ToolSvc::create( const std::string& tooltype, const IInterface* parent, IAlgTool*& tool )
//------------------------------------------------------------------------------
{
  const std::string& toolname = tooltype;
  return create( tooltype, toolname, parent, tool );
}

namespace
{
  /// Small class to allow a safe roll-back if the tool is not
  /// correctly initialized or there are problems.
  template <typename T>
  class ToolCreateGuard
  {
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
    void create( const std::string& tooltype, const std::string& fullname, const IInterface* parent )
    {
      // remove previous content
      if ( m_tool ) remove( m_tools, m_tool.get() );
      m_tool.reset( AlgTool::Factory::create( tooltype, tooltype, fullname, parent ) );
      // set new content
      if ( m_tool ) m_tools.push_back( m_tool.get() );
    }
    /// Get the internal pointer
    IAlgTool* get() { return m_tool.get(); }
    IAlgTool* operator->() const
    {
      assert( m_tool );
      return m_tool.get();
    }
    /// Return the internal pointer and give ownership.
    IAlgTool* release() { return m_tool.release(); }
    /// remove it from the list.
    ~ToolCreateGuard()
    {
      if ( m_tool ) remove( m_tools, m_tool.get() );
    }
  };

  template <typename C>
  ToolCreateGuard<C> make_toolCreateGuard( C& c )
  {
    return ToolCreateGuard<C>{c};
  }
}

//------------------------------------------------------------------------------
/**
 * Now able to handle clones. The test of tool existence is performed according to
 * three criteria: name, type and parent.
 * If a tool is private, i.e. the parent is not the tool Svc, and it exist but
 * the parent is not the specified one, a clone is handed over.
 * No clones of public tools are allowed since they would be undistinguishable.
**/
StatusCode ToolSvc::create( const std::string& tooltype, const std::string& toolname, const IInterface* parent,
                            IAlgTool*& tool )
//------------------------------------------------------------------------------
{

  std::lock_guard<CallMutex> lock( m_mut );
  // protect against empty type
  if ( UNLIKELY( tooltype.empty() ) ) {
    error() << "create(): No Tool Type given" << endmsg;
    return StatusCode::FAILURE;
  }

  // If parent has not been specified, assume it is the ToolSvc
  if ( !parent ) parent = this;

  tool = nullptr;
  // Automatically deletes the tool if not explicitly kept (i.e. on success).
  // The tool is removed from the list of known tools too.
  auto toolguard = make_toolCreateGuard( m_instancesTools );

  // Check if the tool already exist : this could happen with clones
  std::string fullname = nameTool( toolname, parent );
  if ( UNLIKELY( existsTool( fullname ) ) ) {
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
    if ( UNLIKELY( !toolguard.get() ) ) {
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
  if ( mytool ) {
    StatusCode sc = mytool->setProperties();
    if ( UNLIKELY( sc.isFailure() ) ) {
      error() << "Error setting properties for tool '" << fullname << "'" << endmsg;
      return sc;
    }
  }

  // Initialize the Tool
  StatusCode sc( StatusCode::FAILURE, true );
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
  if ( UNLIKELY( sc.isFailure() ) ) {
    error() << "Error initializing tool '" << fullname << "'" << endmsg;
    return sc;
  }

  // Start the tool if we are running.
  if ( m_state == Gaudi::StateMachine::RUNNING ) {
    sc = toolguard->sysStart();

    if ( UNLIKELY( sc.isFailure() ) ) {
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
  if ( m_pHistorySvc || service( "HistorySvc", m_pHistorySvc, false ).isSuccess() ) {
    m_pHistorySvc->registerAlgTool( *tool ).ignore();
  }
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
std::string ToolSvc::nameTool( const std::string& toolname, const IInterface* parent )
//------------------------------------------------------------------------------
{

  if ( !parent ) {
    return this->name() + "." + toolname;
  } // RETURN

  // check that parent has a name!
  auto named_parent = SmartIF<INamedInterface>( const_cast<IInterface*>( parent ) );
  if ( named_parent ) {
    auto fullname = named_parent->name() + "." + toolname;
    return fullname; // RETURN
  }

  error() << "Private Tools only allowed for components implementing INamedInterface" << endmsg;
  //
  return "." + toolname;
}

//------------------------------------------------------------------------------
bool ToolSvc::existsTool( const std::string& fullname ) const
//------------------------------------------------------------------------------
{
  std::lock_guard<CallMutex> lock( m_mut );
  auto i = std::find_if( std::begin( m_instancesTools ), std::end( m_instancesTools ),
                         [&]( const IAlgTool* tool ) { return tool->name() == fullname; } );
  return i != std::end( m_instancesTools );
}

//------------------------------------------------------------------------------
StatusCode ToolSvc::finalizeTool( IAlgTool* itool ) const
//------------------------------------------------------------------------------
{

  // Cache tool name in case of errors
  const std::string toolName = itool->name();
  StatusCode sc;

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

//------------------------------------------------------------------------------
unsigned long ToolSvc::totalToolRefCount() const
//------------------------------------------------------------------------------
{
  return totalRefCount( m_instancesTools );
}
//------------------------------------------------------------------------------
unsigned long ToolSvc::minimumToolRefCount() const
//------------------------------------------------------------------------------
{
  auto i =
      std::min_element( std::begin( m_instancesTools ), std::end( m_instancesTools ),
                        []( const IAlgTool* lhs, const IAlgTool* rhs ) { return lhs->refCount() < rhs->refCount(); } );
  return i != std::end( m_instancesTools ) ? ( *i )->refCount() : 0;
}

//------------------------------------------------------------------------------
void ToolSvc::registerObserver( IToolSvc::Observer* obs )
{
  //------------------------------------------------------------------------------
  if ( !obs ) throw GaudiException( "Received NULL pointer", this->name() + "::registerObserver", StatusCode::FAILURE );

  std::lock_guard<CallMutex> lock( m_mut );
  obs->setUnregister( [this, obs]() {
    std::lock_guard<CallMutex> lock( m_mut );
    auto i = std::find( m_observers.begin(), m_observers.end(), obs );
    if ( i != m_observers.end() ) m_observers.erase( i );
  } );
  m_observers.push_back( obs );
}

//------------------------------------------------------------------------------
StatusCode ToolSvc::start()
//------------------------------------------------------------------------------
{

  ON_DEBUG debug() << "START transition for AlgTools" << endmsg;

  bool fail( false );
  for ( auto& iTool : m_instancesTools ) {
    ON_VERBOSE verbose() << iTool->name() << "::start()" << endmsg;

    if ( UNLIKELY( !iTool->sysStart().isSuccess() ) ) {
      fail = true;
      error() << iTool->name() << " failed to start()" << endmsg;
    }
  }

  if ( UNLIKELY( fail ) ) {
    error() << "One or more AlgTools failed to start()" << endmsg;
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode ToolSvc::stop()
//------------------------------------------------------------------------------
{

  ON_DEBUG debug() << "STOP transition for AlgTools" << endmsg;

  bool fail( false );
  for ( auto& iTool : m_instancesTools ) {
    ON_VERBOSE verbose() << iTool->name() << "::stop()" << endmsg;

    if ( UNLIKELY( !iTool->sysStop().isSuccess() ) ) {
      fail = true;
      error() << iTool->name() << " failed to stop()" << endmsg;
    }
  }

  if ( UNLIKELY( fail ) ) {
    error() << "One or more AlgTools failed to stop()" << endmsg;
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}
