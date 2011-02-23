// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/MinimalEventLoopMgr.h,v 1.4 2008/07/15 12:48:18 marcocle Exp $
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
class GAUDI_API MinimalEventLoopMgr: public extends1<Service, IEventProcessor>
{
public:
  /// Creator friend class
  typedef std::list<SmartIF<IAlgorithm> >  ListAlg;
  typedef std::list<IAlgorithm*>  ListAlgPtrs;
  typedef std::list<std::string>   ListName;
  typedef std::vector<std::string> VectorName;

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
  /// State of the object
  State               m_state;
  /// Scheduled stop of event processing
  bool                m_scheduledStop;
  /// Instance of the incident listener waiting for AbortEvent.
  SmartIF<IIncidentListener>  m_abortEventListener;
  /// Flag signalling that the event being processedhas to be aborted
  /// (skip all following top algs).
  bool                m_abortEvent;
  /// Source of the AbortEvent incident.
  std::string         m_abortEventSource;

public:
  /// Standard Constructor
  MinimalEventLoopMgr(const std::string& nam, ISvcLocator* svcLoc);
  /// Standard Destructor
  virtual ~MinimalEventLoopMgr();

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
  virtual StatusCode initialize();
  /// implementation of IService::start
  virtual StatusCode start();
  /// implementation of IService::stop
  virtual StatusCode stop();
  /// implementation of IService::finalize
  virtual StatusCode finalize();
  /// implementation of IService::reinitialize
  virtual StatusCode reinitialize();
  /// implementation of IService::restart
  virtual StatusCode restart();

  /// implementation of IEventProcessor::nextEvent
  virtual StatusCode nextEvent(int maxevt);
  /// implementation of IEventProcessor::executeEvent(void* par)
  virtual StatusCode executeEvent(void* par );
  /// implementation of IEventProcessor::executeRun( )
  virtual StatusCode executeRun(int maxevt);
  /// implementation of IEventProcessor::stopRun( )
  virtual StatusCode stopRun();

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
