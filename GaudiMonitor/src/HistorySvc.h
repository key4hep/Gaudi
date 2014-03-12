// $Id: HistorySvc.h,v 1.8 2008/06/04 12:35:15 marcocle Exp $

#ifndef GAUDISVC_FASTHISTORYSVC_H
#define GAUDISVC_FASTHISTORYSVC_H

#include "GaudiKernel/IHistorySvc.h"

#ifndef GAUDIKERNEL_SERVICE_H
 #include "GaudiKernel/Service.h"
#endif
#ifndef KERNEL_STATUSCODES_H
 #include "GaudiKernel/StatusCode.h"
#endif

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IVersHistoryObj.h"

#include <vector>
#include <string>
#include <set>
#include <map>
#include <fstream>
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/MsgStream.h"

class StoreGateSvc;
class IAlgContextSvc;
class IIncidentSvc;
class HistoryObj;
class DataHistory;
class AlgorithmHistory;
class ServiceHistory;
struct DHH;

/** @class HistorySvc HistorySvc.h
 *
 *  HistorySvc class definition
 *
 */

class HistorySvc: public extends2<Service, IHistorySvc, IIncidentListener> {
public:

  virtual StatusCode initialize();
  virtual StatusCode reinitialize();
  virtual StatusCode finalize();
  virtual StatusCode stop();

  virtual StatusCode captureState();

  virtual StatusCode registerJob();
  virtual StatusCode listProperties() const;
  virtual JobHistory* getJobHistory() const;

  virtual StatusCode registerSvc(const IService&);
  virtual StatusCode listProperties(const IService&) const;
  virtual ServiceHistory* getServiceHistory(const IService&) const;
  virtual void getServiceHistory(std::set<ServiceHistory*>&) const;

  virtual StatusCode registerAlg(const Algorithm&);
  virtual StatusCode listProperties(const Algorithm&) const;
  virtual AlgorithmHistory* getAlgHistory(const Algorithm&) const;
  virtual void getAlgHistory(std::set<AlgorithmHistory*>&) const;

  virtual StatusCode registerAlgTool(const IAlgTool&);
  virtual StatusCode listProperties(const IAlgTool&) const;
  virtual AlgToolHistory* getAlgToolHistory(const IAlgTool&) const;
  virtual void getAlgToolHistory(std::set<AlgToolHistory*>&) const;

  virtual DataHistory* createDataHistoryObj(const CLID& id,
					    const std::string& key,
					    const std::string& store);
  virtual StatusCode registerDataHistory(const CLID& id,
					 const std::string& key,
					 const std::string& store);
  virtual DataHistory* getDataHistory(const CLID& id, const std::string& key,
				      const std::string& store) const;
  virtual int getDataHistory(const CLID& id, const std::string& key,
			     const std::string& storeName,
			     std::list<DataHistory*>& dhlist) const;

  HistorySvc( const std::string& name, ISvcLocator* svc );

  virtual void handle(const Incident& inc);

  // Destructor.
  virtual ~HistorySvc();

private:

//   typedef std::map<const CLID, std::map<const std::string, DataHistory*> >
//           DataHistMap;

  typedef IVersHistoryObj::PropertyList PropertyList;

  typedef std::multimap<DHH,DataHistory*> DataHistMap;
  typedef DataHistMap::iterator DHMitr;
  typedef DataHistMap::const_iterator DHMCitr;

  void clearState();

  bool m_isInitialized;
  bool m_dump;
  bool m_activate;

  IAlgContextSvc *p_algCtxSvc;

  std::set<const Algorithm*> m_algs;
  std::map<const Algorithm*, AlgorithmHistory*> m_algmap;

  std::set<const IAlgTool*> m_ialgtools;
  std::set<const AlgTool*> m_algtools;
  std::map<const AlgTool*, AlgToolHistory*> m_algtoolmap;

  std::set<const IService*> m_svcs;
  std::map<const IService*, ServiceHistory*> m_svcmap;

  std::multimap<DHH, DataHistory*> m_datMap;

//   std::set<DataHistory*, DataHistory::DataHistoryOrder> m_datMap;

  JobHistory *m_jobHistory;

  std::string m_outputFile;

  void dumpProperties(std::ofstream&) const;
  void dumpProperties(const IService&,  std::ofstream&) const;
  void dumpProperties(const Algorithm&, std::ofstream&) const;
  void dumpProperties(const IAlgTool&,  std::ofstream&) const;

  void dumpState(std::ofstream&) const;
  void dumpState(const INamedInterface*, std::ofstream&) const;

  std::string dumpProp(const Property*, const bool isXML=false,
		       int indent=0) const;

  IAlgorithm* getCurrentIAlg() const;

  IIncidentSvc *m_incidentSvc;
  SmartIF<IToolSvc> m_toolSvc;

  mutable MsgStream m_log;
  bool m_outputFileTypeXML;

};

#endif
