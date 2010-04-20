#include "GaudiKernel/Auditor.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AudFactory.h"

// ============================================================================
/** @class LoggingAuditor
 *
 *  Simple auditor that prints the event being audited.
 *
 *  @author Marco Clemencic
 *  @date 2008-04-03
 */
namespace GaudiExamples {

class LoggingAuditor: public Auditor {
public:

  virtual void before(StandardEventType evt, INamedInterface* caller) {
    MsgStream log ( msgSvc() , name() );
    log << MSG::INFO << "Auditing before of " << evt;
    if (caller) log << " for " << caller->name();
    log << endmsg;
  }

  virtual void after(StandardEventType evt, INamedInterface* caller, const StatusCode &) {
    MsgStream log ( msgSvc() , name() );
    log << MSG::INFO << "Auditing after of " << evt;
    if (caller) log << " for " << caller->name();
    log << endmsg;
  }

  virtual void before(CustomEventTypeRef evt, INamedInterface* caller) {
    MsgStream log ( msgSvc() , name() );
    log << MSG::INFO << "Auditing before of " << evt;
    if (caller) log << " for " << caller->name();
    log << endmsg;
  }

  virtual void after(CustomEventTypeRef evt, INamedInterface* caller, const StatusCode &) {
    MsgStream log ( msgSvc() , name() );
    log << MSG::INFO << "Auditing after of " << evt;
    if (caller) log << " for " << caller->name();
    log << endmsg;
  }

  virtual void before(StandardEventType evt, const std::string& caller) {
    MsgStream log ( msgSvc() , name() );
    log << MSG::INFO << "Auditing before of " << evt << " for " << caller << endmsg;
  }

  virtual void after(StandardEventType evt, const std::string& caller, const StatusCode &) {
    MsgStream log ( msgSvc() , name() );
    log << MSG::INFO << "Auditing after of " << evt << " for " << caller << endmsg;
  }

  virtual void before(CustomEventTypeRef evt, const std::string& caller) {
    MsgStream log ( msgSvc() , name() );
    log << MSG::INFO << "Auditing before of " << evt << " for " << caller << endmsg;
  }

  virtual void after(CustomEventTypeRef evt, const std::string& caller, const StatusCode &) {
    MsgStream log ( msgSvc() , name() );
    log << MSG::INFO << "Auditing after of " << evt << " for " << caller << endmsg;
  }

  /// constructor
  LoggingAuditor(const std::string& name, ISvcLocator* pSvc):
    Auditor ( name , pSvc ) {}

  /// virtual destructor
  virtual ~LoggingAuditor() {}

};

}

DECLARE_NAMESPACE_AUDITOR_FACTORY(GaudiExamples,LoggingAuditor)
