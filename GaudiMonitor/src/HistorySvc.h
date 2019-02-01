#ifndef GAUDISVC_FASTHISTORYSVC_H
#define GAUDISVC_FASTHISTORYSVC_H

#include "GaudiKernel/IHistorySvc.h"

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IVersHistoryObj.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/StatusCode.h"
#include <Gaudi/Algorithm.h>

#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/MsgStream.h"
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>

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

class HistorySvc : public extends<Service, IHistorySvc, IIncidentListener> {
public:
  using extends::extends;

  StatusCode initialize() override;
  StatusCode reinitialize() override;
  StatusCode finalize() override;
  StatusCode stop() override;

  virtual StatusCode captureState();

  StatusCode  registerJob() override;
  StatusCode  listProperties() const override;
  JobHistory* getJobHistory() const override;

  StatusCode      registerSvc( const IService& ) override;
  StatusCode      listProperties( const IService& ) const override;
  ServiceHistory* getServiceHistory( const IService& ) const override;
  void            getServiceHistory( std::set<ServiceHistory*>& ) const override;

  StatusCode        registerAlg( const Gaudi::Algorithm& ) override;
  StatusCode        listProperties( const Gaudi::Algorithm& ) const override;
  AlgorithmHistory* getAlgHistory( const Gaudi::Algorithm& ) const override;
  void              getAlgHistory( std::set<AlgorithmHistory*>& ) const override;

  StatusCode      registerAlgTool( const IAlgTool& ) override;
  StatusCode      listProperties( const IAlgTool& ) const override;
  AlgToolHistory* getAlgToolHistory( const IAlgTool& ) const override;
  void            getAlgToolHistory( std::set<AlgToolHistory*>& ) const override;

  DataHistory*       createDataHistoryObj( const CLID& id, const std::string& key, const std::string& store ) override;
  virtual StatusCode registerDataHistory( const CLID& id, const std::string& key, const std::string& store );
  DataHistory*       getDataHistory( const CLID& id, const std::string& key, const std::string& store ) const override;
  virtual int        getDataHistory( const CLID& id, const std::string& key, const std::string& storeName,
                                     std::list<DataHistory*>& dhlist ) const;

  void handle( const Incident& inc ) override;

private:
  //   typedef std::map<const CLID, std::map<const std::string, DataHistory*> >
  //           DataHistMap;

  typedef IVersHistoryObj::PropertyList PropertyList;

  typedef std::multimap<DHH, DataHistory*> DataHistMap;
  typedef DataHistMap::iterator            DHMitr;
  typedef DataHistMap::const_iterator      DHMCitr;

  Gaudi::Property<bool>        m_dump{this, "Dump", false};
  Gaudi::Property<bool>        m_activate{this, "Activate", true};
  Gaudi::Property<std::string> m_outputFile{this, "OutputFile"};

  void clearState();

  bool m_isInitialized = false;

  IAlgContextSvc* p_algCtxSvc = nullptr;

  std::map<const Gaudi::Algorithm*, AlgorithmHistory*> m_algmap;

  std::set<const IAlgTool*>                  m_ialgtools;
  std::map<const AlgTool*, AlgToolHistory*>  m_algtoolmap;
  std::map<const IService*, ServiceHistory*> m_svcmap;

  std::multimap<DHH, DataHistory*> m_datMap;

  std::unique_ptr<JobHistory> m_jobHistory;

  void dumpProperties( std::ofstream& ) const;
  void dumpProperties( const IService&, std::ofstream& ) const;
  void dumpProperties( const Gaudi::Algorithm&, std::ofstream& ) const;
  void dumpProperties( const IAlgTool&, std::ofstream& ) const;

  void dumpState( std::ofstream& ) const;
  void dumpState( const INamedInterface*, std::ofstream& ) const;

  std::string dumpProp( const Gaudi::Details::PropertyBase*, const bool isXML = false, int indent = 0 ) const;

  IAlgorithm* getCurrentIAlg() const;

  IIncidentSvc*     m_incidentSvc = nullptr;
  SmartIF<IToolSvc> m_toolSvc;

  bool m_outputFileTypeXML = false;
};

#endif
