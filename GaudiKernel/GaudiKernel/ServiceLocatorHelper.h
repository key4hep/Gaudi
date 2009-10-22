#ifndef GAUDIKERNEL_SERVICELOCATORHELPER_H
#define GAUDIKERNEL_SERVICELOCATORHELPER_H

#include <string>
#include <GaudiKernel/StatusCode.h>
#include <GaudiKernel/SmartIF.h>
#include <GaudiKernel/IService.h>

class ISvcLocator;
class InterfaceID;
class MsgStream;

/** @class ServiceLocatorHelper
 *  @brief an helper to share the implementation of service() among the
 *         various kernel base classes
 * $Id:
 */
class GAUDI_API ServiceLocatorHelper {
public:
  ServiceLocatorHelper(ISvcLocator& svcLoc, const INamedInterface& requester):
    m_svcLoc(svcLoc),
    m_msgLog(SmartIF<IMessageSvc>(&svcLoc), requester.name()), //use requester msg level
    m_requesterName(requester.name()) {
  }
  ServiceLocatorHelper(ISvcLocator& svcLoc,
      const std::string& loggedName,
      const std::string& requesterName):
    m_svcLoc(svcLoc),
    m_msgLog(SmartIF<IMessageSvc>(&svcLoc), loggedName), //use requester msg level
    m_requesterName(requesterName) {
  }
  ServiceLocatorHelper(ISvcLocator& svcLoc, const std::string& requesterName):
    m_svcLoc(svcLoc),
    m_msgLog(SmartIF<IMessageSvc>(&svcLoc), requesterName), //use requester msg level
    m_requesterName(requesterName) {
  }
#if !defined(GAUDI_V22_API) || defined(G22_NEW_SVCLOCATOR)
  ServiceLocatorHelper(ISvcLocator& svcLoc,
		 const MsgStream& log, //use requester msg level
		 const std::string& requesterName):
    m_svcLoc(svcLoc), m_msgLog(log), m_requesterName(requesterName) {}
#endif

  StatusCode getService(const std::string& name,
			bool createIf,
			const InterfaceID& iid,
			void** ppSvc) const {
    return (createIf ?
	    createService(name, iid, ppSvc) :
	    locateService(name, iid, ppSvc, true));
  }

  StatusCode locateService(const std::string& name,
			   const InterfaceID& iid,
			   void** ppSvc,
			   bool quiet=false) const;

  StatusCode createService(const std::string& name,
			   const InterfaceID& iid,
			   void** ppSvc) const;

  StatusCode createService(const std::string& type,
			   const std::string& name,
			   const InterfaceID& iid,
			   void** ppSvc) const;

  SmartIF<IService> service(const std::string& name, const bool quiet=false, const bool createIf=true) const;

private:
  std::string threadName() const;
  std::string threadedName(const std::string& name) const;
  bool isInThread() const;
  ISvcLocator* serviceLocator() const { return &m_svcLoc; }
  MsgStream& log() const { return m_msgLog; }
  const std::string& requesterName() const { return m_requesterName; }
  ISvcLocator& m_svcLoc;
  mutable MsgStream m_msgLog;
  std::string m_requesterName;
};
#endif
