#ifndef GAUDISVC_FASTHISTORYSVC_H
#define GAUDISVC_FASTHISTORYSVC_H

#include "GaudiKernel/IHistorySvc.h"

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IVersHistoryObj.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/StatusCode.h"

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

class HistorySvc : public extends<Service, IHistorySvc, IIncidentListener>
{
public:
  virtual StatusCode initialize();
  virtual StatusCode reinitialize();
  virtual StatusCode finalize();
  virtual StatusCode stop();

  virtual StatusCode captureState();

  virtual StatusCode registerJob();
  virtual StatusCode listProperties() const;
  virtual JobHistory* getJobHistory() const;

  virtual StatusCode registerSvc( const IService& );
  virtual StatusCode listProperties( const IService& ) const;
  virtual ServiceHistory* getServiceHistory( const IService& ) const;
  virtual void getServiceHistory( std::set<ServiceHistory*>& ) const;

  virtual StatusCode registerAlg( const Algorithm& );
  virtual StatusCode listProperties( const Algorithm& ) const;
  virtual AlgorithmHistory* getAlgHistory( const Algorithm& ) const;
  virtual void getAlgHistory( std::set<AlgorithmHistory*>& ) const;

  virtual StatusCode registerAlgTool( const IAlgTool& );
  virtual StatusCode listProperties( const IAlgTool& ) const;
  virtual AlgToolHistory* getAlgToolHistory( const IAlgTool& ) const;
  virtual void getAlgToolHistory( std::set<AlgToolHistory*>& ) const;

  virtual DataHistory* createDataHistoryObj( const CLID& id, const std::string& key, const std::string& store );
  virtual StatusCode registerDataHistory( const CLID& id, const std::string& key, const std::string& store );
  virtual DataHistory* getDataHistory( const CLID& id, const std::string& key, const std::string& store ) const;
  virtual int getDataHistory( const CLID& id, const std::string& key, const std::string& storeName,
                              std::list<DataHistory*>& dhlist ) const;

  using extends::extends;

  virtual void handle( const Incident& inc );

  // Destructor.
  virtual ~HistorySvc() = default;

private:
  //   typedef std::map<const CLID, std::map<const std::string, DataHistory*> >
  //           DataHistMap;

  typedef IVersHistoryObj::PropertyList PropertyList;

  typedef std::multimap<DHH, DataHistory*> DataHistMap;
  typedef DataHistMap::iterator DHMitr;
  typedef DataHistMap::const_iterator DHMCitr;

  BooleanProperty m_dump{this, "Dump", false};
  BooleanProperty m_activate{this, "Activate", true};
  StringProperty m_outputFile{this, "OutputFile"};

  void clearState();

  bool m_isInitialized = false;

  IAlgContextSvc* p_algCtxSvc = nullptr;

  std::map<const Algorithm*, AlgorithmHistory*> m_algmap;

  std::set<const IAlgTool*> m_ialgtools;
  std::map<const AlgTool*, AlgToolHistory*> m_algtoolmap;
  std::map<const IService*, ServiceHistory*> m_svcmap;

  std::multimap<DHH, DataHistory*> m_datMap;

  std::unique_ptr<JobHistory> m_jobHistory;

  void dumpProperties( std::ofstream& ) const;
  void dumpProperties( const IService&, std::ofstream& ) const;
  void dumpProperties( const Algorithm&, std::ofstream& ) const;
  void dumpProperties( const IAlgTool&, std::ofstream& ) const;

  void dumpState( std::ofstream& ) const;
  void dumpState( const INamedInterface*, std::ofstream& ) const;

  std::string dumpProp( const Gaudi::Details::PropertyBase*, const bool isXML = false, int indent = 0 ) const;

  IAlgorithm* getCurrentIAlg() const;

  IIncidentSvc* m_incidentSvc = nullptr;
  SmartIF<IToolSvc> m_toolSvc;

  bool m_outputFileTypeXML = false;
};

#endif
