#ifndef AuditorSvc_AuditorSvc_H
#define AuditorSvc_AuditorSvc_H

// Include Files
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IAuditorSvc.h"
#include <list>
#include <vector>

//------------------------------------------------------------------
//
// ClassName:   AuditorSvc
//
// Description: This service manages Auditors.
//              Auditors can be inserted in the system to audit the Algorithms is
//              in their functions.
//------------------------------------------------------------------
class AuditorSvc : public extends1<Service, IAuditorSvc> {

public:


// Inherited Service overrides:
//
  // Initialize the service.
  StatusCode initialize() override;

  // Finalize the service.
  StatusCode finalize() override;

// IAuditorSvc interfaces overwrite
//
  /// The following methods are meant to be implemented by the child class...

  void before(StandardEventType, INamedInterface*) override;
  void before(StandardEventType, const std::string&) override;

  void before(CustomEventTypeRef, INamedInterface*) override;
  void before(CustomEventTypeRef, const std::string&) override;

  void after(StandardEventType, INamedInterface*, const StatusCode&) override;
  void after(StandardEventType, const std::string&, const StatusCode&) override;

  void after(CustomEventTypeRef, INamedInterface*, const StatusCode&) override;
  void after(CustomEventTypeRef, const std::string&, const StatusCode&) override;

  // inform Auditors that the initialize() is about to be called
  void beforeInitialize(INamedInterface* ini) override;
  // inform Auditors that the initialize() has been called
  void afterInitialize(INamedInterface* ini) override;

  // inform Auditors that the reinitialize() is about to be called
  void beforeReinitialize(INamedInterface* ini) override;
  // inform Auditors that the reinitialize() has been called
  void afterReinitialize(INamedInterface* ini) override;

  // inform Auditors that the execute() is about to be called
  void beforeExecute(INamedInterface* ini) override;
  // inform Auditors that the execute() has been called
  void afterExecute(INamedInterface* ini, const StatusCode& ) override;

  // inform Auditors that the beginRun() is about to be called
  void beforeBeginRun(INamedInterface* ini) override;
  // inform Auditors that the beginRun() has been called
  void afterBeginRun(INamedInterface* ini) override;

  // inform Auditors that the endRun() is about to be called
  void beforeEndRun(INamedInterface* ini) override;
  // inform Auditors that the endRun() has been called
  void afterEndRun(INamedInterface* ini) override;

  // inform Auditors that the finalize() is about to be called
  void beforeFinalize(INamedInterface* ini) override;
  // inform Auditors that the finalize() has been called
  void afterFinalize(INamedInterface* ini) override;

  bool isEnabled() const  override;

  StatusCode sysInitialize() override;
  StatusCode sysFinalize() override;

  // management functionality: retrieve an Auditor
  IAuditor* getAuditor( const std::string& name ) override;

  // Standard Constructor.
  //   Input:  name   String with service name
  //   Input:  svc    Pointer to service locator interface
  AuditorSvc( const std::string& name, ISvcLocator* svc );

  // Destructor.
  ~AuditorSvc() override = default;

private:
  // management helper
  IAuditor* newAuditor_( MsgStream&, const std::string& );
  IAuditor* findAuditor_( const std::string& );
  StatusCode syncAuditors_();

  // List of auditor names
  std::vector<std::string> m_audNameList;

  // Manager list of Auditors
  std::vector<IAuditor*>     m_pAudList;

  // To disable alltogether the auditors
  bool m_isEnabled = true;
};

#endif
