// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiSvc/src/AuditorSvc/AuditorSvc.cpp,v 1.19 2008/10/27 19:22:21 marcocle Exp $

// Include Files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IAuditor.h"
#include "GaudiKernel/INamedInterface.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/TypeNameString.h"
#include "GaudiKernel/GaudiException.h"
#include "AuditorSvc.h"

// Instantiation of a static factory class used by clients to create
//  instances of this service
DECLARE_SERVICE_FACTORY(AuditorSvc)

using ROOT::Reflex::PluginService;
//
// ClassName:   AuditorSvc
//
// Description: This service manages Auditors.
//------------------------------------------------------------------

//- private helpers ---
IAuditor* AuditorSvc::newAuditor_( MsgStream& log, const std::string& name ) {
  // locate the auditor factory, instantiate a new auditor, initialize it
  IAuditor* aud = 0;
  StatusCode sc;
  Gaudi::Utils::TypeNameString item(name) ;
  aud = PluginService::Create<IAuditor*>( item.type(), item.name(), serviceLocator().get() );
  if ( aud ) {
    aud->addRef();
    if ( m_targetState >= Gaudi::StateMachine::INITIALIZED ) {
      sc = aud->sysInitialize();
      if ( sc.isFailure() ) {
        log << MSG::WARNING << "Failed to initialize Auditor " << name << endmsg;
        aud->release();
        aud = 0;
      }
    }
  }
  else {
    log << MSG::WARNING << "Unable to retrieve factory for Auditor " << name << endmsg;
  }

  return aud;
}

IAuditor* AuditorSvc::findAuditor_( const std::string& name ) {
  // find an auditor by name, return 0 on error
  IAuditor* aud = 0;
  const std::string item_name = Gaudi::Utils::TypeNameString(name).name();
  for ( ListAudits::iterator it = m_pAudList.begin() ; it != m_pAudList.end(); ++it ) {
    if ( (*it)->name() == item_name ) {
      (*it)->addRef();
      aud = *it;
      break;
    }
  }

  return aud;
}

StatusCode AuditorSvc::syncAuditors_() {
  if ( m_audNameList.size() == m_pAudList.size() )
    return StatusCode::SUCCESS;

  MsgStream log( msgSvc(), name() );
  StatusCode sc;

//   if ( sc.isFailure() ) {
//     log << MSG::ERROR << "Unable to locate ObjectManager Service" << endmsg;
//     return sc;
//   }

  // create all declared Auditors that do not yet exist
  for ( VectorName::iterator it = m_audNameList.begin(); it != m_audNameList.end(); it++ ) {

    // this is clumsy, but the PropertyMgr won't tell us when my property changes right
    // under my nose, so I'll have to figure this out the hard way
    if ( !findAuditor_( *it ) ) { // if auditor does not yet exist
      IAuditor* aud = newAuditor_( log, *it );

      if ( aud != 0 ) {
        m_pAudList.push_back( aud );
      }
      else {
        log << MSG::ERROR << "Error constructing Auditor " << *it << endmsg;
        sc = StatusCode::FAILURE;
      }
    }
  }
  return sc;
}

// Standard Constructor.
//   Input:  name   String with service name
//   Input:  svc    Pointer to service locator interface
AuditorSvc::AuditorSvc( const std::string& name, ISvcLocator* svc )
: base_class(name, svc) {
  declareProperty("Auditors", m_audNameList );
  declareProperty("Enable", m_isEnabled = true);
  m_pAudList.clear();
}

// Destructor.
AuditorSvc::~AuditorSvc() {
}

// Inherited Service overrides:
//
  // Initialize the service.
StatusCode AuditorSvc::initialize() {
  StatusCode sc = Service::initialize();
  if ( sc.isFailure() )
    return sc;

  // create auditor objects for all named auditors
  sc = syncAuditors_();

  return sc;
}

  // Finalise the service.
StatusCode AuditorSvc::finalize() {

  for (ListAudits::iterator it = m_pAudList.begin() ; it != m_pAudList.end(); it++) {
    if((*it)->isEnabled()) {
       (*it)->sysFinalize().ignore();
    }
    (*it)->release();
  }
  m_pAudList.clear();

  // Finalize this specific service
  return Service::finalize();
}

// --------- "Before" methods ---------
void AuditorSvc::before(StandardEventType evt, INamedInterface* obj) {
  if (!isEnabled()) return;
  for (ListAudits::iterator it = m_pAudList.begin() ; it != m_pAudList.end(); it++) {
    if((*it)->isEnabled()) {
      (*it)->before(evt,obj);
    }
  }
}

void AuditorSvc::before(StandardEventType evt, const std::string &name) {
  if (!isEnabled()) return;
  for (ListAudits::iterator it = m_pAudList.begin() ; it != m_pAudList.end(); it++) {
    if((*it)->isEnabled()) {
      (*it)->before(evt,name);
    }
  }
}

void AuditorSvc::before(CustomEventTypeRef evt, INamedInterface* obj) {
  if (!isEnabled()) return;
  for (ListAudits::iterator it = m_pAudList.begin() ; it != m_pAudList.end(); it++) {
    if((*it)->isEnabled()) {
      (*it)->before(evt,obj);
    }
  }
}

void AuditorSvc::before(CustomEventTypeRef evt, const std::string &name) {
  if (!isEnabled()) return;
  for (ListAudits::iterator it = m_pAudList.begin() ; it != m_pAudList.end(); it++) {
    if((*it)->isEnabled()) {
      (*it)->before(evt,name);
    }
  }
}

// --------- "After" methods ---------
void AuditorSvc::after(StandardEventType evt, INamedInterface* obj, const StatusCode& sc) {
  if (!isEnabled()) return;
  for (ListAudits::iterator it = m_pAudList.begin() ; it != m_pAudList.end(); it++) {
    if((*it)->isEnabled()) {
      (*it)->after(evt,obj,sc);
    }
  }
}

void AuditorSvc::after(StandardEventType evt, const std::string &name, const StatusCode& sc) {
  if (!isEnabled()) return;
  for (ListAudits::iterator it = m_pAudList.begin() ; it != m_pAudList.end(); it++) {
    if((*it)->isEnabled()) {
      (*it)->after(evt,name,sc);
    }
  }
}

void AuditorSvc::after(CustomEventTypeRef evt, INamedInterface* obj, const StatusCode& sc) {
  if (!isEnabled()) return;
  for (ListAudits::iterator it = m_pAudList.begin() ; it != m_pAudList.end(); it++) {
    if((*it)->isEnabled()) {
      (*it)->after(evt,obj,sc);
    }
  }
}

void AuditorSvc::after(CustomEventTypeRef evt, const std::string &name, const StatusCode& sc) {
  if (!isEnabled()) return;
  for (ListAudits::iterator it = m_pAudList.begin() ; it != m_pAudList.end(); it++) {
    if((*it)->isEnabled()) {
      (*it)->after(evt,name,sc);
    }
  }
}

// --------- obsolete methods ---------
#define OBSOLETION(name) \
  void AuditorSvc::name(INamedInterface*) { \
    throw GaudiException("The method IAuditor::" #name " is obsolete do not call it.", \
                         "AuditorSvc::" #name , StatusCode::FAILURE); \
  }

OBSOLETION(beforeInitialize)
OBSOLETION(afterInitialize)

OBSOLETION(beforeReinitialize)
OBSOLETION(afterReinitialize)

OBSOLETION(beforeExecute)
void AuditorSvc::afterExecute(INamedInterface*,const StatusCode&) {
  throw GaudiException("The method afterExecute is obsolete do not call it.",
                       "AuditorSvc::afterExecute" , StatusCode::FAILURE);
}

OBSOLETION(beforeBeginRun)
OBSOLETION(afterBeginRun)

OBSOLETION(beforeEndRun)
OBSOLETION(afterEndRun)

OBSOLETION(beforeFinalize)
OBSOLETION(afterFinalize)


bool AuditorSvc::isEnabled( ) const {
  return m_isEnabled;
}

StatusCode AuditorSvc::sysInitialize(){
  return Service::sysInitialize();
}
StatusCode AuditorSvc::sysFinalize(){
  return Service::sysFinalize();
}


IAuditor* AuditorSvc::getAuditor( const std::string& name ) {
  // by interactively setting properties, auditors might be out of sync
  if ( !syncAuditors_().isSuccess() ) {
    // as we didn't manage to sync auditors, the safest bet is to assume the
    // worse...
    // So don't let clients play with an AuditorSvc in an inconsistent state
    return 0;
  }

  // search available auditors, returns 0 on error
  return findAuditor_( name );
}
