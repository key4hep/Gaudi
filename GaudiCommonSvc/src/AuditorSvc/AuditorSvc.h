// $Id: $
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

// Typedefs
  typedef std::list<IAuditor*>     ListAudits;
  typedef std::vector<std::string> VectorName;

// Inherited Service overrides:
//
  // Initialize the service.
  virtual StatusCode initialize();

  // Finalize the service.
  virtual StatusCode finalize();

// IAuditorSvc interfaces overwrite
//
  /// The following methods are meant to be implemented by the child class...

  virtual void before(StandardEventType, INamedInterface*);
  virtual void before(StandardEventType, const std::string&);

  virtual void before(CustomEventTypeRef, INamedInterface*);
  virtual void before(CustomEventTypeRef, const std::string&);

  virtual void after(StandardEventType, INamedInterface*, const StatusCode&);
  virtual void after(StandardEventType, const std::string&, const StatusCode&);

  virtual void after(CustomEventTypeRef, INamedInterface*, const StatusCode&);
  virtual void after(CustomEventTypeRef, const std::string&, const StatusCode&);

  // inform Auditors that the initialize() is about to be called
  virtual void beforeInitialize(INamedInterface* ini);
  // inform Auditors that the initialize() has been called
  virtual void afterInitialize(INamedInterface* ini);

  // inform Auditors that the reinitialize() is about to be called
  virtual void beforeReinitialize(INamedInterface* ini);
  // inform Auditors that the reinitialize() has been called
  virtual void afterReinitialize(INamedInterface* ini);

  // inform Auditors that the execute() is about to be called
  virtual void beforeExecute(INamedInterface* ini);
  // inform Auditors that the execute() has been called
  virtual void afterExecute(INamedInterface* ini, const StatusCode& );

  // inform Auditors that the beginRun() is about to be called
  virtual void beforeBeginRun(INamedInterface* ini);
  // inform Auditors that the beginRun() has been called
  virtual void afterBeginRun(INamedInterface* ini);

  // inform Auditors that the endRun() is about to be called
  virtual void beforeEndRun(INamedInterface* ini);
  // inform Auditors that the endRun() has been called
  virtual void afterEndRun(INamedInterface* ini);

  // inform Auditors that the finalize() is about to be called
  virtual void beforeFinalize(INamedInterface* ini);
  // inform Auditors that the finalize() has been called
  virtual void afterFinalize(INamedInterface* ini);

  virtual bool isEnabled() const ;

  virtual StatusCode sysInitialize();
  virtual StatusCode sysFinalize();

  // management functionality: retrieve an Auditor
  virtual IAuditor* getAuditor( const std::string& name );

  // Standard Constructor.
  //   Input:  name   String with service name
  //   Input:  svc    Pointer to service locator interface
  AuditorSvc( const std::string& name, ISvcLocator* svc );

  // Destructor.
  virtual ~AuditorSvc();

private:
  // management helper
  IAuditor* newAuditor_( MsgStream&, const std::string& );
  IAuditor* findAuditor_( const std::string& );
  StatusCode syncAuditors_();

  // List of auditor names
  VectorName   m_audNameList;

  // Manager list of Auditors
  ListAudits   m_pAudList;

  // To disable alltogether the auditors
  bool m_isEnabled;
};

#endif
