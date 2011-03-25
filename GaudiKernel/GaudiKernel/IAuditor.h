// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/IAuditor.h,v 1.11 2008/10/27 19:22:20 marcocle Exp $
#ifndef GAUDI_IAUDITOR_H
#define GAUDI_IAUDITOR_H

// Include files
#include "GaudiKernel/INamedInterface.h"
#include <string>

/** @class IAuditor IAuditor.h GaudiKernel/IAuditor.h

    The IAuditor is the interface implmented by the AlgAuditor base class.
    Concrete auditors, derived from the AlgAuditor base class are controlled
    via this interface.

    @author Marjorie Shapiro, LBNL
    @author Marco Clemencic <marco.clemencic@cern.ch>
*/
class GAUDI_API IAuditor: virtual public INamedInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IAuditor,3,0);

  /// Defines the standard (= used by the framework) auditable event types.
  enum StandardEventType {
    Initialize,
    ReInitialize,
    Execute,
    BeginRun,
    EndRun,
    Finalize,
    Start,
    Stop,
    ReStart
  };

  /// Type used to allow users to specify a custom event to be audit.
  /// Examples of custom events are callbacks (see
  /// <a href="https://savannah.cern.ch/patch/index.php?1725">patch #1725</a>).
  typedef std::string CustomEventType;
  /// Used in function calls for optimization purposes.
  typedef const CustomEventType& CustomEventTypeRef;

  /// Audit the start of a standard "event".
  virtual void before(StandardEventType, INamedInterface*) = 0;
  /// Audit the start of a standard "event" for callers that do not implement INamedInterface.
  virtual void before(StandardEventType, const std::string&) = 0;

  /// Audit the start of a custom "event".
  virtual void before(CustomEventTypeRef, INamedInterface*) = 0;
  /// Audit the start of a custom "event" for callers that do not implement INamedInterface.
  virtual void before(CustomEventTypeRef, const std::string&) = 0;

  /// Audit the end of a standard "event".
  virtual void after(StandardEventType, INamedInterface*, const StatusCode& sc = StatusCode(StatusCode::SUCCESS,true)) = 0;
  /// Audit the end of a standard "event" for callers that do not implement INamedInterface.
  virtual void after(StandardEventType, const std::string&, const StatusCode& sc = StatusCode(StatusCode::SUCCESS,true)) = 0;

  /// Audit the end of a custom "event".
  virtual void after(CustomEventTypeRef, INamedInterface*, const StatusCode& sc = StatusCode(StatusCode::SUCCESS,true)) = 0;
  /// Audit the end of a custom "event" for callers that do not implement INamedInterface.
  virtual void after(CustomEventTypeRef, const std::string&, const StatusCode& sc = StatusCode(StatusCode::SUCCESS,true)) = 0;

  /// Tell if the auditor is enabled or not.
  virtual bool isEnabled() const = 0;

// ------- Obsolete interface ------
  /// \deprecated use before
  virtual void beforeInitialize(INamedInterface* ) = 0;
  /// \deprecated use after
  virtual void afterInitialize(INamedInterface* ) = 0;

  /// \deprecated use before
  virtual void beforeReinitialize(INamedInterface* ) = 0;
  /// \deprecated use after
  virtual void afterReinitialize(INamedInterface* ) = 0;

  /// \deprecated use before
  virtual void beforeExecute(INamedInterface* ) = 0;
  /// \deprecated use after
  virtual void afterExecute(INamedInterface*, const StatusCode& ) = 0;

  /// \deprecated use before
  virtual void beforeBeginRun(INamedInterface* ) = 0;
  /// \deprecated use after
  virtual void afterBeginRun(INamedInterface* ) = 0;

  /// \deprecated use before
  virtual void beforeEndRun(INamedInterface* ) = 0;
  /// \deprecated use after
  virtual void afterEndRun(INamedInterface* ) = 0;

  /// \deprecated use before
  virtual void beforeFinalize(INamedInterface* ) = 0;
  /// \deprecated use after
  virtual void afterFinalize(INamedInterface* ) = 0;

  /// Used by AuditorSvc.
  virtual StatusCode sysInitialize( ) = 0;

  /// Used by AuditorSvc.
  virtual StatusCode sysFinalize( ) = 0;

};

/// Simple mapping function from IAuditor::StandardEventType to string.
inline const char* toStr(IAuditor::StandardEventType e) {
  switch (e) {
    case IAuditor::Initialize   : return "Initialize";
    case IAuditor::ReInitialize : return "ReInitialize";
    case IAuditor::Execute      : return "Execute";
    case IAuditor::BeginRun     : return "BeginRun";
    case IAuditor::EndRun       : return "EndRun";
    case IAuditor::Finalize     : return "Finalize";
    case IAuditor::Start        : return "Start";
    case IAuditor::Stop         : return "Stop";
    case IAuditor::ReStart      : return "ReStart";
  }
  return NULL; // cannot be reached, but make the compiler happy
}

inline std::ostream & operator << (std::ostream & s, IAuditor::StandardEventType e) {
  return s << toStr(e);
}


#endif // GAUDIKERNEL_IAUDITOR_H

