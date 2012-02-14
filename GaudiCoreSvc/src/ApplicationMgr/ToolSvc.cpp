// Include Files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IHistorySvc.h"
#include "GaudiKernel/ToolFactory.h"
#include "ToolSvc.h"
#include <algorithm>
#include <map>
#include <string>
#include <cassert>
#ifdef __ICC
// disable icc remark #177: declared but never referenced
// TODO: Remove. Problem with boost::lambda
#pragma warning(disable:177)
#endif
#include "boost/lambda/bind.hpp"

#define ON_DEBUG if (UNLIKELY(outputLevel() <= MSG::DEBUG))
#define ON_VERBOSE if (UNLIKELY(outputLevel() <= MSG::VERBOSE))

// Instantiation of a static factory class used by clients to create
//  instances of this service
DECLARE_SERVICE_FACTORY(ToolSvc)

using ROOT::Reflex::PluginService;
namespace bl = boost::lambda;

//------------------------------------------------------------------------------
ToolSvc::ToolSvc( const std::string& name, ISvcLocator* svc )
  //------------------------------------------------------------------------------
  : base_class(name, svc),
    m_pHistorySvc(0)
 { }

//------------------------------------------------------------------------------
ToolSvc::~ToolSvc()
  //------------------------------------------------------------------------------
{

}

//------------------------------------------------------------------------------
StatusCode ToolSvc::initialize()
  //------------------------------------------------------------------------------
{

  // initialize the Service Base class
  StatusCode status = Service::initialize();
  if (UNLIKELY(status.isFailure()))
  {
    error() << "Unable to initialize the Service" << endmsg;
    return status;
  }

  // set my own (ToolSvc) properties via the jobOptionService
  if (UNLIKELY(setProperties().isFailure())) {
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
  ListTools finalizedTools; // list of tools that have been finalized
  info()  << "Removing all tools created by ToolSvc" << endmsg;

  // Print out list of tools
  ON_DEBUG {
    MsgStream &log = debug();
    log << "  Tool List : ";
    for ( ListTools::const_iterator iTool = m_instancesTools.begin();
        iTool != m_instancesTools.end(); ++iTool ) {
      log << (*iTool)->name() << ":" << refCountTool( *iTool ) << " ";
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
  bool fail(false);
  size_t toolCount = m_instancesTools.size();
  unsigned long startRefCount = 0;
  unsigned long endRefCount = totalToolRefCount();
  unsigned long startMinRefCount = 0;
  unsigned long endMinRefCount = minimumToolRefCount();
  while ( toolCount > 0 &&
          endRefCount > 0 &&
          (endRefCount != startRefCount || endMinRefCount != startMinRefCount) ) {
    ON_DEBUG if ( endMinRefCount != startMinRefCount ) {
      debug() << toolCount << " tools left to finalize. Summed refCounts: "
              << endRefCount << endmsg;
      debug() << "Will finalize tools with refCount <= "
              << endMinRefCount << endmsg;
    }
    startMinRefCount = endMinRefCount;
    startRefCount = endRefCount;
    unsigned long maxLoop = toolCount + 1;
    while ( --maxLoop > 0 &&  m_instancesTools.size() > 0 ) {
      IAlgTool* pTool = m_instancesTools.back();
      // removing tool from list makes ToolSvc::releaseTool( IAlgTool* ) a noop
      m_instancesTools.pop_back();
      unsigned long count = refCountTool( pTool );
      // cache tool name
      std::string toolName = pTool->name();
      if ( count <= startMinRefCount ) {
        ON_DEBUG debug() << "  Performing finalization of " << toolName
                         << " (refCount " << count << ")" << endmsg;
        // finalize of one tool may trigger a release of another tool
        //   pTool->sysFinalize().ignore();
        if (!finalizeTool(pTool).isSuccess()) {
          warning() << "    FAILURE finalizing " << toolName << endmsg;
          fail = true;
        }
        // postpone deletion
        finalizedTools.push_back(pTool);
      } else {
        // Place back in list to try again later
        // ToolSvc::releaseTool( IAlgTool* ) remains active for this tool
        ON_DEBUG debug() << "  Delaying   finalization of " << toolName
                         << " (refCount " << count << ")" << endmsg;
        m_instancesTools.push_front(pTool);
      }
    } // end of inner loop
    toolCount = m_instancesTools.size();
    endRefCount = totalToolRefCount();
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
  unsigned long maxLoop = totalToolRefCount( finalizedTools ) + 1;
  while ( --maxLoop > 0 && finalizedTools.size() > 0 ) {
    IAlgTool* pTool = finalizedTools.front();
    finalizedTools.pop_front();
    unsigned long count = refCountTool( pTool );
    if ( count == 1 ) {
      ON_DEBUG debug() << "  Performing deletion of " << pTool->name() << endmsg;
    } else {
      ON_VERBOSE verbose() << "  Delaying   deletion of " << pTool->name()
          << " (refCount " << count << ")" << endmsg;
      // Put it back at the end of the list if refCount still not zero
      finalizedTools.push_back(pTool);
    }
    // do a forced release
    pTool->release();
  }

  // Error if by now not all tools are properly finalised
  if ( !m_instancesTools.empty() ) {
    error() << "Unable to finalize and delete the following tools : ";
    for ( ListTools::const_iterator iTool = m_instancesTools.begin();
          iTool != m_instancesTools.end(); ++iTool ) {
      error() << (*iTool)->name() << ": " << refCountTool( *iTool ) << " ";
    }
    error() << endmsg;
  }

  // by now, all tools should be deleted and removed.
  if ( finalizedTools.size() > 0 ) {
    error() << "Failed to delete the following " <<  finalizedTools.size()
            << " finalized tools. Bug in ToolSvc::finalize()?: ";
    for ( ListTools::const_iterator iTool = finalizedTools.begin();
          iTool != finalizedTools.end(); ++iTool ) {
      error() << (*iTool)->name() << ": " << refCountTool( *iTool ) << " ";
    }
    error() << endmsg;
  }

  if ( 0 != m_pHistorySvc ) {
    m_pHistorySvc->release();
  }

  // Finalize this specific service
  if (! Service::finalize().isSuccess() || fail) {
    return StatusCode::FAILURE;
  } else {
    return StatusCode::SUCCESS;
  }


}

// ===================================================================================
/** the indicator for the tool to be "PUBLIC"
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 */
// ===================================================================================
namespace
{
  const std::string s_PUBLIC = ":PUBLIC" ;
}

//------------------------------------------------------------------------------
StatusCode ToolSvc::retrieve ( const std::string& tooltype ,
                               const InterfaceID& iid      ,
                               IAlgTool*&         tool     ,
                               const IInterface*  parent   ,
                               bool               createIf )
//------------------------------------------------------------------------------
{

  // protect against empty type
  if ( tooltype.empty() ) {
    error() << "retrieve(): No Tool Type/Name given" << endmsg;
    return StatusCode::FAILURE;
  }

  {
    // check for tools, which by name is required to be public:
    const std::string::size_type pos = tooltype.find ( s_PUBLIC ) ;
    if ( std::string::npos != pos )
    {
      // set parent for PUBLIC tool
      parent = this ;
      return retrieve ( std::string( tooltype , 0 , pos ) ,
                        iid , tool , parent , createIf ) ;
    }
  }

  const std::string::size_type pos = tooltype.find('/');
  if( std::string::npos == pos ) {
    return retrieve ( tooltype , tooltype , iid , tool , parent , createIf );
  }
  const std::string newtype ( tooltype ,       0 , pos               ) ;
  const std::string newname ( tooltype , pos + 1 , std::string::npos ) ;
  return retrieve ( newtype , newname , iid , tool , parent , createIf ) ;
}

// ===================================================================================

//------------------------------------------------------------------------------
StatusCode ToolSvc::retrieve ( const std::string& tooltype ,
                               const std::string& toolname ,
                               const InterfaceID& iid      ,
                               IAlgTool*&         tool     ,
                               const IInterface*  parent   ,
                               bool               createIf )
  //------------------------------------------------------------------------------
{
  // check the applicability of another method:
  // ignore the provided name if it is empty or the type contains a name
  if( toolname.empty() || (std::string::npos != tooltype.find('/')) )
  { return retrieve ( tooltype , iid , tool , parent , createIf ) ; }

  {
    // check for tools, which by name is required to be public:
    const std::string::size_type pos = toolname.find ( s_PUBLIC ) ;
    if ( std::string::npos != pos )
    {
      // set parent for PUBLIC tool
      parent = this ;
      return retrieve ( tooltype , std::string( toolname , 0 , pos ) ,
                        iid , tool , parent , createIf ) ;
    }
  }

  IAlgTool* itool = 0;
  StatusCode sc(StatusCode::FAILURE);

  tool = 0;

  // If parent is not specified it means it is the ToolSvc itself
  if( 0 == parent ) {
    parent = this;
  }
  const std::string fullname = nameTool( toolname, parent );

  // Find tool in list of those already existing, and tell its
  // interface that it has been used one more time
  ListTools::const_iterator it;
  for( it = m_instancesTools.begin(); it != m_instancesTools.end(); ++it ) {
    if( (*it)->name() == fullname ) {
      ON_DEBUG debug() << "Retrieved tool " << toolname << endmsg;
      itool = *it;
      break;
    }
  }

  if ( 0 == itool ) {
    // Instances of this tool do not exist, create an instance if desired
    // otherwise return failure
    if( UNLIKELY(!createIf) ) {
      warning() << "Tool " << toolname
                << " not found and creation not requested" << endmsg;
      return sc;
    }
    else {
      sc = create( tooltype, toolname, parent, itool );
      if ( sc.isFailure() ) { return sc; }
    }
  }

  // Get the right interface of it
  sc = itool->queryInterface( iid, (void**)&tool);
  if( UNLIKELY(sc.isFailure()) ) {
    error() << "Tool " << toolname
        << " either does not implement the correct interface, or its version is incompatible"
        << endmsg;
    return sc;
  }
  ///////////////
  /// invoke retrieve callbacks...
  ///////////////
  if (!m_observers.empty()) {
     std::for_each( m_observers.begin(),
                    m_observers.end(),
                    bl::bind(&IToolSvc::Observer::onRetrieve,
                             bl::_1,
                             itool));
  }

  return sc;
}
//------------------------------------------------------------------------------
std::vector<std::string> ToolSvc::getInstances( const std::string& toolType )
//------------------------------------------------------------------------------
{

  std::vector<std::string> tools;

  ListTools::const_iterator it;
  for (it = m_instancesTools.begin(); it != m_instancesTools.end(); ++it) {
    if ((*it)->type() == toolType) {
      tools.push_back( (*it)->name() );
    }
  }

  return tools;

}
//------------------------------------------------------------------------------
StatusCode ToolSvc::releaseTool( IAlgTool* tool )
  //------------------------------------------------------------------------------
{
  StatusCode sc(StatusCode::SUCCESS);
  // test if tool is in known list (protect trying to access a previously deleted tool)
  if ( m_instancesTools.rend() != std::find( m_instancesTools.rbegin(),
                                              m_instancesTools.rend(),
                                              tool ) ) {
    unsigned long count = refCountTool(tool);
    if ( count == 1 ) {
      MsgStream log( msgSvc(), name() );
      // finalize the tool

      if ( Gaudi::StateMachine::OFFLINE == m_targetState ) {
        // We are being called during ToolSvc::finalize()
        // message format matches the one in ToolSvc::finalize()
        log << MSG::DEBUG << "  Performing finalization of " << tool->name()
            << " (refCount " << count << ")" << endmsg;
        // message format matches the one in ToolSvc::finalize()
        log << MSG::DEBUG << "  Performing     deletion of " << tool->name() << endmsg;
      } else {
        log << MSG::DEBUG << "Performing finalization and deletion of " << tool->name() << endmsg;
      }
      sc = finalizeTool(tool);
      // remove from known tools...
      m_instancesTools.remove(tool);
    }
    tool->release();
  }

  return sc;
}

//------------------------------------------------------------------------------
StatusCode ToolSvc::create(const std::string& tooltype,
                           const IInterface* parent,
                           IAlgTool*& tool)
  //------------------------------------------------------------------------------
{
  const std::string & toolname = tooltype;
  return create( tooltype, toolname, parent, tool);
}

namespace {
/// Small class to allow a safe roll-back if the tool is not
/// correctly initialized or there are problems.
class ToolCreateGuard {
public:
  ToolCreateGuard(ToolSvc::ListTools &listTools):
    m_list(listTools),
    m_tool(0)
  {}
  /// Set the internal pointer (delete any previous one). Get ownership of the tool.
  void set(IAlgTool* tool) {
    if (m_tool) { // remove previous content
      m_list.remove(m_tool);
      delete m_tool;
    }
    if (tool) { // set new content
      m_tool = tool;
      m_list.push_back(m_tool);
    }
  }
  ToolCreateGuard& operator=(IAlgTool* tool) {
    set(tool);
    return *this;
  }
  /// Get the internal pointer
  IAlgTool* get() {
    return m_tool;
  }
  IAlgTool* operator->() const {
    assert(m_tool != 0);
    return m_tool;
  }
  /// Return the internal pointer and give ownership.
  IAlgTool* release() {
    IAlgTool* tool = m_tool;
    m_tool = 0;
    return tool;
  }
  /// Delete the owned tool and remove it from the list.
  ~ToolCreateGuard(){
    set(0);
  }
private:
  /// list of tools
  ToolSvc::ListTools& m_list;
  /// pointer to be set
  IAlgTool* m_tool;
};
}
//------------------------------------------------------------------------------
StatusCode ToolSvc::create(const std::string& tooltype,
                           const std::string& toolname,
                           const IInterface* parent,
                           IAlgTool*& tool)
  //------------------------------------------------------------------------------
{
  // protect against empty type
  if ( UNLIKELY(tooltype.empty()) ) {
    error() << "create(): No Tool Type given" << endmsg;
    return StatusCode::FAILURE;
  }

  // If parent has not been specified, assume it is the ToolSvc
  if ( 0 == parent ) parent = this;

  tool = 0;
  // Automatically deletes the tool if not explicitly kept (i.e. on success).
  // The tool is removed from the list of known tools too.
  ToolCreateGuard toolguard(m_instancesTools);

  // Check if the tool already exist : this should never happen
  const std::string fullname = nameTool(toolname, parent);
  if( UNLIKELY(existsTool(fullname)) ) {
    error() << "Tool " << fullname << " already exists" << endmsg;
    return StatusCode::FAILURE;
  }
  // instantiate the tool using the factory
  try {
    toolguard = PluginService::Create<IAlgTool*>(tooltype, tooltype, fullname, parent);
    if ( UNLIKELY(! toolguard.get()) ){
       error() << "Cannot create tool " << tooltype << " (No factory found)" << endmsg;
       return StatusCode::FAILURE;
    }
  }
  catch ( const GaudiException& Exception )  {
    // (1) perform the printout of message
    fatal() << "Exception with tag=" << Exception.tag()
            << " is caught whilst instantiating tool '" << tooltype << "'" << endmsg;
    // (2) print  the exception itself
    // (NB!  - GaudiException is a linked list of all "previous exceptions")
    fatal() << Exception  << endmsg;
    return StatusCode::FAILURE;
  }
  catch( const std::exception& Exception ) {
    // (1) perform the printout of message
    fatal() << "Standard std::exception is caught whilst instantiating tool '"
            << tooltype << "'" << endmsg;
    // (2) print  the exception itself
    // (NB!  - GaudiException is a linked list of all "previous exceptions")
    fatal() << Exception.what()  << endmsg;
    return StatusCode::FAILURE;
  }
  catch(...) {
    // (1) perform the printout
    fatal() << "UNKNOWN Exception is caught whilst instantiating tool '"
            << tooltype << "'" << endmsg;
    return StatusCode::FAILURE;
  }
  ON_VERBOSE verbose() << "Created tool " << tooltype << "/" << fullname << endmsg;

  // Since only AlgTool has the setProperties() method it is necessary to cast
  // to downcast IAlgTool to AlgTool in order to set the properties via the JobOptions
  // service
  AlgTool* mytool = dynamic_cast<AlgTool*> (toolguard.get());
  if ( mytool != 0 ) {
    StatusCode sc = mytool->setProperties();
    if ( UNLIKELY(sc.isFailure()) ) {
      error() << "Error setting properties for tool '"
              << fullname << "'" << endmsg;
      return sc;
    }
  }

  // Initialize the Tool
  StatusCode sc (StatusCode::FAILURE,true);
  try {
    sc = toolguard->sysInitialize();
  }
  // Catch any exceptions
  catch ( const GaudiException & Exception )
    {
      error()
          << "GaudiException with tag=" << Exception.tag()
          << " caught whilst initializing tool '" << fullname << "'" << endmsg
          << Exception << endmsg;
      return StatusCode::FAILURE;
    }
  catch( const std::exception & Exception )
    {
      error()
          << "Standard std::exception caught whilst initializing tool '"
          << fullname << "'" << endmsg << Exception.what() << endmsg;
      return StatusCode::FAILURE;
    }
  catch (...)
    {
      error()
          << "UNKNOWN Exception caught whilst initializing tool '"
          << fullname << "'" << endmsg;
      return StatusCode::FAILURE;
    }

  // Status of tool initialization
  if ( UNLIKELY(sc.isFailure()) ) {
    error() << "Error initializing tool '" << fullname << "'" << endmsg;
    return sc;
  }

  // Start the tool if we are running.
  if (m_state == Gaudi::StateMachine::RUNNING) {
    sc = toolguard->sysStart();

    if (UNLIKELY(sc.isFailure())) {
      error() << "Error starting tool '" << fullname << "'" << endmsg;
      return sc;
    }
  }


  // The tool has been successfully created and initialized,
  // so we the guard can be released
  tool = toolguard.release();

  ///////////////
  /// invoke create callbacks...
  ///////////////
  if (!m_observers.empty()) {
      std::for_each( m_observers.begin(),
                     m_observers.end(),
                     bl::bind(&IToolSvc::Observer::onCreate,
                              bl::_1,
                              tool));
  }
  // TODO: replace by generic callback
  // Register the tool with the HistorySvc
  if (m_pHistorySvc != 0 ||
      service("HistorySvc",m_pHistorySvc,false).isSuccess() ) {
    m_pHistorySvc->registerAlgTool(*tool).ignore();
  }

  return StatusCode::SUCCESS;

}

//------------------------------------------------------------------------------
std::string ToolSvc::nameTool( const std::string& toolname,
                               const IInterface* parent )
  //------------------------------------------------------------------------------
{

  std::string fullname = "";
  if ( parent == 0 ) { return this->name() + "." + toolname; }    // RETURN


  IInterface* cparent = const_cast<IInterface*>( parent ) ;
  // check that parent has a name!
  INamedInterface* _p = 0 ;
  StatusCode sc = cparent->queryInterface( INamedInterface::interfaceID() , pp_cast<void>(&_p) ) ;
  if ( sc.isSuccess() )
  {
    fullname = _p->name() + "." + toolname ;
    _p->release() ;
    return fullname ;                                          // RETURN
  }

  MsgStream log ( msgSvc(), name() );
  log << MSG::ERROR
      << "Private Tools only allowed for components implementing INamedInterface"
      << endmsg;
  //
  return "." + toolname ;
}

//------------------------------------------------------------------------------
bool ToolSvc::existsTool( const std::string& fullname) const
  //------------------------------------------------------------------------------
{
  for ( ListTools::const_iterator it = m_instancesTools.begin();
        it != m_instancesTools.end(); ++it ) {
    if ( (*it)->name() == fullname ) { return true; }
  }
  return false;
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
  catch ( const GaudiException & Exception )
  {
    error()
        << "GaudiException with tag=" << Exception.tag()
        << " caught whilst finalizing tool '" << toolName << "'" << endmsg
        << Exception << endmsg;
    sc = StatusCode::FAILURE;
  }
  catch( const std::exception & Exception )
  {
    error()
        << "Standard std::exception caught whilst finalizing tool '"
        << toolName << "'" << endmsg << Exception.what() << endmsg;
    sc = StatusCode::FAILURE;
  }
  catch (...)
  {
    error()
        << "UNKNOWN Exception caught whilst finalizing tool '"
        << toolName << "'" << endmsg;
    sc = StatusCode::FAILURE;
  }

  return sc;

}

//------------------------------------------------------------------------------
unsigned long ToolSvc::totalToolRefCount( const ToolSvc::ListTools& toolList ) const
//------------------------------------------------------------------------------
{
  unsigned long count = 0;
  for ( ListTools::const_iterator iTool = toolList.begin();
        iTool != toolList.end(); ++iTool ) {
    count += refCountTool( *iTool );
  }
  return count;
}

//------------------------------------------------------------------------------
unsigned long ToolSvc::totalToolRefCount() const
//------------------------------------------------------------------------------
{
  return totalToolRefCount( m_instancesTools );
}
//------------------------------------------------------------------------------
unsigned long ToolSvc::minimumToolRefCount() const
//------------------------------------------------------------------------------
{
  unsigned long count = 0;
  if ( m_instancesTools.size() > 0 ) {
    ListTools::const_iterator iTool = m_instancesTools.begin();
    // start with first
    count = refCountTool( *iTool );
    // then compare the others
    for( ++iTool; iTool != m_instancesTools.end(); ++iTool ) {
      count = std::min( count, refCountTool( *iTool ) );
    }
  }
  return count;
}

void ToolSvc::registerObserver(IToolSvc::Observer* obs) {
  if ( 0 == obs )
    throw GaudiException( "Received NULL pointer", this->name() + "::registerObserver", StatusCode::FAILURE );
  m_observers.push_back(obs);
}

void ToolSvc::unRegisterObserver(IToolSvc::Observer* obs) {
  std::vector<IToolSvc::Observer*>::iterator i =
    find(m_observers.begin(),m_observers.end(),obs);
  if (i!=m_observers.end()) m_observers.erase(i);
}

//------------------------------------------------------------------------------
StatusCode
ToolSvc::start()
//------------------------------------------------------------------------------
{

  ON_DEBUG debug() << "START transition for AlgTools" << endmsg;

  bool fail(false);
  for ( ListTools::const_iterator iTool = m_instancesTools.begin();
        iTool != m_instancesTools.end(); ++iTool ) {
    ON_VERBOSE verbose() << (*iTool)->name() << "::start()" << endmsg;

    if (UNLIKELY(!(*iTool)->sysStart().isSuccess())) {
      fail = true;
      error() << (*iTool)->name() << " failed to start()" << endmsg;
    }

  }

  if (UNLIKELY(fail)) {
    error() << "One or more AlgTools failed to start()" << endmsg;
    return StatusCode::FAILURE;
  } else {
    return StatusCode::SUCCESS;
  }

}

//------------------------------------------------------------------------------
StatusCode
ToolSvc::stop()
//------------------------------------------------------------------------------
{

  ON_DEBUG debug() << "STOP transition for AlgTools" << endmsg;

  bool fail(false);
  for ( ListTools::const_iterator iTool = m_instancesTools.begin();
        iTool != m_instancesTools.end(); ++iTool ) {
    ON_VERBOSE verbose() << (*iTool)->name() << "::stop()" << endmsg;

    if (UNLIKELY(!(*iTool)->sysStop().isSuccess())) {
      fail = true;
      error() << (*iTool)->name() << " failed to stop()" << endmsg;
    }

  }

  if (UNLIKELY(fail)) {
    error() << "One or more AlgTools failed to stop()" << endmsg;
    return StatusCode::FAILURE;
  } else {
    return StatusCode::SUCCESS;
  }

}
