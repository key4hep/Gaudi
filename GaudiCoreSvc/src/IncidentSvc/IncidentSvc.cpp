#ifdef _WIN32
// ============================================================================
// Avoid conflicts between windows and the message service.
// ============================================================================
#define NOMSG
#define NOGDI
#endif
// ============================================================================
// Include Files
// ============================================================================
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/LockedChrono.h"
#include "GaudiKernel/AppReturnCode.h"
// ============================================================================
// Local
// ============================================================================
#include "IncidentSvc.h"
// ============================================================================
// Instantiation of a static factory class used by clients to create
//  instances of this service
DECLARE_COMPONENT(IncidentSvc)
// ============================================================================
namespace
{
  // ==========================================================================
  static const std::string s_unknown = "<unknown>" ;
  // Helper to get the name of the listener
  inline const std::string& getListenerName ( IIncidentListener* lis )
  {
    SmartIF<INamedInterface> iNamed(lis);
    return iNamed.isValid() ? iNamed->name() : s_unknown ;
  }
  // ==========================================================================
}

#define ON_DEBUG if (UNLIKELY(outputLevel() <= MSG::DEBUG))
#define ON_VERBOSE if (UNLIKELY(outputLevel() <= MSG::VERBOSE))

#define DEBMSG ON_DEBUG debug()
#define VERMSG ON_VERBOSE verbose()

// ============================================================================
// Constructors and Destructors
// ============================================================================
IncidentSvc::IncidentSvc( const std::string& name, ISvcLocator* svc )
  : base_class(name, svc)
  , m_currentIncidentType(0)
  , m_timer()
  , m_timerLock ( false )
{}
// ============================================================================
IncidentSvc::~IncidentSvc()
{
  boost::recursive_mutex::scoped_lock lock(m_listenerMapMutex);

  for (ListenerMap::iterator i = m_listenerMap.begin();
       i != m_listenerMap.end();
       ++i) {
    delete i->second;
  }
}
// ============================================================================
// Inherited Service overrides:
// ============================================================================
StatusCode IncidentSvc::initialize()
{
  // initialize the Service Base class
  StatusCode sc = Service::initialize();
  if ( sc.isFailure() ) {
    return sc;
  }

  m_currentIncidentType = 0;

  // set my own (IncidentSvc) properties via the jobOptionService
  sc = setProperties();
  if ( UNLIKELY(sc.isFailure()) )
  {
    error() << "Could not set my properties" << endmsg;
    return sc;
  }

  return StatusCode::SUCCESS;
}
// ============================================================================
StatusCode IncidentSvc::finalize()
{
  DEBMSG << m_timer.outputUserTime( "Incident  timing: Mean(+-rms)/Min/Max:%3%(+-%4%)/%6%/%7%[ms] " , System::milliSec )
         << m_timer.outputUserTime ( "Total:%2%[s]" , System::Sec ) << endmsg ;

  // Finalize this specific service
  StatusCode sc = Service::finalize();
  if ( UNLIKELY(sc.isFailure()) ) { return sc; }

  return StatusCode::SUCCESS;
}
// ============================================================================
// Inherited IIncidentSvc overrides:
// ============================================================================
void IncidentSvc::addListener
( IIncidentListener* lis ,
  const std::string& type ,
  long prio, bool rethrow, bool singleShot)
{

  boost::recursive_mutex::scoped_lock lock(m_listenerMapMutex);

  std::string ltype;
  if( type == "" ) ltype = "ALL";
  else             ltype = type;
  // find if the type already exists
  ListenerMap::iterator itMap = m_listenerMap.find( ltype );
  if( itMap == m_listenerMap.end() ) {
    // if not found, create and insert now a list of listeners
    ListenerList* newlist = new ListenerList();
    std::pair<ListenerMap::iterator, bool> p;
    p = m_listenerMap.insert(ListenerMap::value_type(ltype, newlist));
    if( p.second ) itMap = p.first;
  }
  ListenerList* llist = (*itMap).second;
  // add Listener in the ListenerList according to the priority
  ListenerList::iterator itlist;
  for( itlist = llist->begin(); itlist != llist->end(); itlist++ ) {
    if( (*itlist).priority < prio ) {
      // We insert before the current position
      break;
    }
  }

  DEBMSG << "Adding [" << type << "] listener '" << getListenerName(lis)
         << "' with priority " << prio << endmsg;

  llist->insert(itlist, Listener(lis, prio, rethrow, singleShot));
}
// ============================================================================
void IncidentSvc::removeListener
( IIncidentListener* lis  ,
  const std::string& type )
{

  boost::recursive_mutex::scoped_lock lock(m_listenerMapMutex);

  if( type == "") {
    // remove Listener from all the lists
    ListenerMap::iterator itmap;
    for ( itmap = m_listenerMap.begin(); itmap != m_listenerMap.end();)
    {
      // since the current entry may be eventually deleted
      // we need to keep a memory of the next index before
      // calling recursively this method
      ListenerMap::iterator itmap_old = itmap;
      itmap++;
      removeListener( lis, (*itmap_old).first );
    }
  }
  else {
    ListenerMap::iterator itmap = m_listenerMap.find( type );

    if( itmap == m_listenerMap.end() ) {
      // if not found the incident type then return
      return;
    }
    else {
      ListenerList* llist = (*itmap).second;
      ListenerList::iterator itlist;
      bool justScheduleForRemoval = ( 0!= m_currentIncidentType )
                                    && (type == *m_currentIncidentType);
      // loop over all the entries in the Listener list
      // to remove all of them than matches
      // the listener address. Remember the next index
      // before erasing the current one
      for( itlist = llist->begin(); itlist != llist->end(); ) {
        if( (*itlist).iListener == lis || lis == 0) {
          if (justScheduleForRemoval) {
            (itlist++)->singleShot = true; // remove it as soon as it is safe
          }
          else {
            DEBMSG << "Removing [" << type << "] listener '"
                   << getListenerName(lis) << "'" << endmsg;
            itlist = llist->erase(itlist); // remove from the list now
          }
        }
        else {
          itlist++;
        }
      }
      if( llist->size() == 0) {
        delete llist;
        m_listenerMap.erase(itmap);
      }
    }
  }
}
// ============================================================================
namespace {
  /// @class listenerToBeRemoved
  /// Helper class to identify a Listener that have to be removed from a list.
  struct listenerToBeRemoved{
    inline bool operator() (const IncidentSvc::Listener& l) {
      return l.singleShot;
    }
  };
}
// ============================================================================
void IncidentSvc::i_fireIncident
( const Incident&    incident     ,
  const std::string& listenerType )
{

  boost::recursive_mutex::scoped_lock lock(m_listenerMapMutex);

  // Special case: FailInputFile incident must set the application return code
  if (incident.type() == IncidentType::FailInputFile
      || incident.type() == IncidentType::CorruptedInputFile) {
    SmartIF<IProperty> appmgr(serviceLocator());
    if (incident.type() == IncidentType::FailInputFile)
      // Set the return code to Gaudi::ReturnCode::FailInput (2)
      Gaudi::setAppReturnCode(appmgr, Gaudi::ReturnCode::FailInput).ignore();
    else
      Gaudi::setAppReturnCode(appmgr, Gaudi::ReturnCode::CorruptedInput).ignore();
  }

  ListenerMap::iterator itmap = m_listenerMap.find( listenerType );
  if ( m_listenerMap.end() == itmap ) return;

  // setting this pointer will avoid that a call to removeListener() during
  // the loop triggers a segfault
  m_currentIncidentType = &(incident.type());

  ListenerList* llist = (*itmap).second;
  ListenerList::iterator itlist;
  bool weHaveToCleanUp = false;
  // loop over all registered Listeners

    for( itlist = llist->begin(); itlist != llist->end(); itlist++ )
  {

    VERMSG << "Calling '" << getListenerName((*itlist).iListener)
           << "' for incident [" << incident.type() << "]" << endmsg;

    // handle exceptions if they occur
    try {
      (*itlist).iListener->handle(incident);
    }
    catch( const GaudiException& exc ) {
      error() << "Exception with tag=" << exc.tag() << " is caught"
                 " handling incident" << m_currentIncidentType << endmsg;
      error() <<  exc  << endmsg;
      if ( (*itlist).rethrow ) { throw (exc); }
    }
    catch( const std::exception& exc ) {
     error() << "Standard std::exception is caught"
          " handling incident" << m_currentIncidentType << endmsg;
      error() << exc.what()  << endmsg;
      if ( (*itlist).rethrow ) { throw (exc); }
    }
    catch(...) {
      error() << "UNKNOWN Exception is caught"
          " handling incident" << m_currentIncidentType << endmsg;
      if ( (*itlist).rethrow ) { throw; }
    }
    // check if at least one of the listeners is a one-shot
    weHaveToCleanUp |= itlist->singleShot;
  }
  if (weHaveToCleanUp) {
    // remove all the listeners that need to be removed from the list
    llist->remove_if( listenerToBeRemoved() );
    // if the list is empty, we can remove it
    if( llist->size() == 0) {
      delete llist;
      m_listenerMap.erase(itmap);
    }
  }

  m_currentIncidentType = 0;
}
// ============================================================================
void IncidentSvc::fireIncident( const Incident& incident )
{

  Gaudi::Utils::LockedChrono timer ( m_timer , m_timerLock ) ;

  // Call specific listeners
  i_fireIncident(incident, incident.type());
  // Try listeners registered for ALL incidents
  if ( incident.type() != "ALL" ){ // avoid double calls if somebody fires the incident "ALL"
    i_fireIncident(incident, "ALL");
  }

}
// ============================================================================
// The END
// ============================================================================
