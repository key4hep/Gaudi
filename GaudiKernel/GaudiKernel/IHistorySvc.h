// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/IHistorySvc.h,v 1.1 2004/07/14 18:13:57 leggett Exp $

#ifndef GAUDIKERNEL_IHISTORYSVC_H
#define GAUDIKERNEL_IHISTORYSVC_H


#include "GaudiKernel/IService.h"
#include "GaudiKernel/ClassID.h"

#include <string>
#include <set>

class JobHistory;

class ServiceHistory;
class IService;

class Algorithm;
class AlgorithmHistory;

class IAlgTool;
class AlgToolHistory;

class DataHistory;
class HistoryObj;

/** @class IHistorySvc IHistorySvc.h GaudiKernel/IHistorySvc.h

    Definition of the IHistorySvc interface class

    @author Charles Leggett
*/

class GAUDI_API IHistorySvc : virtual public IService {
public:
  /// InterfaceID
  DeclareInterfaceID(IHistorySvc,2,0);

  // Job level objects
  virtual StatusCode registerJob() = 0;
  virtual StatusCode listProperties() const = 0;
  virtual JobHistory* getJobHistory() const = 0;

  // Service level objects
  virtual StatusCode registerSvc(const IService&) = 0;
  virtual StatusCode listProperties(const IService&) const = 0;
  virtual ServiceHistory* getServiceHistory(const IService&) const = 0;
  virtual void getServiceHistory(std::set<ServiceHistory*>&) const = 0;

  // Algorithm level objects
  virtual StatusCode registerAlg(const Algorithm&) = 0;
  virtual StatusCode listProperties(const Algorithm&) const = 0;
  virtual AlgorithmHistory* getAlgHistory(const Algorithm&) const = 0;
  virtual void getAlgHistory(std::set<AlgorithmHistory*>&) const = 0;

  // AlgTool level objects
  virtual StatusCode registerAlgTool(const IAlgTool&) = 0;
  virtual StatusCode listProperties(const IAlgTool&) const = 0;
  virtual AlgToolHistory* getAlgToolHistory(const IAlgTool&) const = 0;
  virtual void getAlgToolHistory(std::set<AlgToolHistory*>&) const = 0;

  // DataObj level objects
  virtual DataHistory* createDataHistoryObj(const CLID& id,
					    const std::string& key,
					    const std::string& storeName) = 0;
  virtual DataHistory* getDataHistory(const CLID& id, const std::string& key,
				      const std::string& storeName) const = 0;

};

#endif // GAUDIKERNEL_IHISTORYSVC_H
