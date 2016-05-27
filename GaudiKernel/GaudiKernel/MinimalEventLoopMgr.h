#ifndef GAUDIKERNEL_MINIMALEVENTLOOPMGR_H
#define GAUDIKERNEL_MINIMALEVENTLOOPMGR_H 1

// Framework include files
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IIncidentListener.h"

// STL include files
#include <list>
#include <vector>

/** @class MinimalEventLoopMgr
 *  This is the default processing manager of the application manager.
 *  This object handles the minimal requirements needed by the application manager.
 *  It also is capable of handling a bunch of algorithms and output streams.
 *  However, they list may as well be empty.
 *
 *  @author Markus Frank
 *  @version 1.0
 */
class GAUDI_API MinimalEventLoopMgr: public extends<Service,
                                                    IEventProcessor>
{
public:
  typedef std::vector<SmartIF<IAlgorithm> >  ListAlg;

protected:
  // enums
  enum State { OFFLINE, CONFIGURED, FINALIZED, INITIALIZED };
  /// Reference to the IAppMgrUI interface of the application manager
  SmartIF<IAppMgrUI> m_appMgrUI;
  /// Reference to the incident service
  SmartIF<IIncidentSvc> m_incidentSvc;
  /// List of top level algorithms
  ListAlg             m_topAlgList;
  /// List of output streams
  ListAlg             m_outStreamList;
  /// Out Stream type
  std::string         m_outStreamType;
  /// List of top level algorithms names
  StringArrayProperty m_topAlgNames;
  /// List of output stream names
  StringArrayProperty m_outStreamNames;
  ///
  BooleanProperty     m_printCFExp{"PrintControlFlowExpression", false};
  /// State of the object
  State               m_state = OFFLINE;
  /// Scheduled stop of event processing
  bool                m_scheduledStop = false;
  /// Instance of the incident listener waiting for AbortEvent.
  SmartIF<IIncidentListener>  m_abortEventListener;
  /// Flag signalling that the event being processedhas to be aborted
  /// (skip all following top algs).
  bool                m_abortEvent = false;
  /// Source of the AbortEvent incident.
  std::string         m_abortEventSource;

public:
  /// Standard Constructor
  MinimalEventLoopMgr(const std::string& nam, ISvcLocator* svcLoc);
  /// Standard Destructor
  ~MinimalEventLoopMgr() override = default;

#if defined(GAUDI_V20_COMPAT) && !defined(G21_NO_DEPRECATED)
protected:
  /// Helper to release interface pointer
  template<class T> T* releaseInterface(T* iface)   {
    if ( 0 != iface ) iface->release();
    return 0;
  }
public:
#endif

  /// implementation of IService::initialize
  StatusCode initialize() override;
  /// implementation of IService::start
  StatusCode start() override;
  /// implementation of IService::stop
  StatusCode stop() override;
  /// implementation of IService::finalize
  StatusCode finalize() override;
  /// implementation of IService::reinitialize
  StatusCode reinitialize() override;
  /// implementation of IService::restart
  StatusCode restart() override;

  /// implementation of IEventProcessor::nextEvent
  StatusCode nextEvent(int maxevt) override;
  /// implementation of IEventProcessor::executeEvent(void* par)
  StatusCode executeEvent(void* par ) override;
  /// implementation of IEventProcessor::executeRun( )
  StatusCode executeRun(int maxevt) override;
  /// implementation of IEventProcessor::stopRun( )
  StatusCode stopRun() override;

  /// Top algorithm List handler
  void topAlgHandler( Property& p);
  /// decodeTopAlgNameList & topAlgNameListHandler
  StatusCode decodeTopAlgs();
  /// Output stream List handler
  void outStreamHandler( Property& p);
  /// decodeOutStreamNameList & outStreamNameListHandler
  StatusCode decodeOutStreams();

private:
  /// Fake copy constructor (never implemented).
  MinimalEventLoopMgr(const MinimalEventLoopMgr&);
  /// Fake assignment operator (never implemented).
  MinimalEventLoopMgr& operator= (const MinimalEventLoopMgr&);

};
#endif // GAUDIKERNEL_MINIMALEVENTLOOPMGR_H
