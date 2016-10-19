#ifndef GAUDICORESVC_ALGEXECSTATESVC_H
#define GAUDICORESVC_ALGEXECSTATESVC_H 1


#include "GaudiKernel/Service.h"
#include "GaudiKernel/IAlgExecStateSvc.h"
#include "GaudiKernel/IAlgorithm.h"


#include <vector>
#include <map>
#include <mutex>

/** @class AlgExecStateSvc
  * @brief A service that keeps track of the execution state of Algorithm
  *  *
  */
class AlgExecStateSvc: public extends<Service,
                                 IAlgExecStateSvc> {
public:
  /// Constructor
  AlgExecStateSvc( const std::string& name, ISvcLocator* svc );

  /// Destructor
  ~AlgExecStateSvc();

  virtual StatusCode initialize() override final;
  virtual StatusCode finalize() override final;

  typedef IAlgExecStateSvc::AlgStateMap_t AlgStateMap_t;

  const AlgExecState& algExecState(const Gaudi::StringKey& algName, 
                                   const EventContext& ctx) const;
  const AlgExecState& algExecState(IAlgorithm* iAlg,     
                                   const EventContext& ctx) const;
  AlgExecState& algExecState(IAlgorithm* iAlg,     
                             const EventContext& ctx);
  const AlgStateMap_t& algExecStates(const EventContext& ctx) const;

  
  const AlgExecState& algExecState(const Gaudi::StringKey& algName) const;
  const AlgExecState& algExecState(IAlgorithm* iAlg) const;
  AlgExecState& algExecState(IAlgorithm* iAlg);
  const AlgStateMap_t& algExecStates() const;

  
  void reset(const EventContext& ctx);
  void reset();

  void addAlg(IAlgorithm* iAlg);
  void addAlg(const Gaudi::StringKey& algName);

  const EventStatus::Status& eventStatus() const;
  const EventStatus::Status& eventStatus(const EventContext& ctx) const;

  void setEventStatus(const EventStatus::Status& sc);
  void setEventStatus(const EventStatus::Status& sc, const EventContext& ctx);

  void updateEventStatus(const bool& b);
  void updateEventStatus(const bool& b, const EventContext& ctx);

  void dump(std::ostringstream& ost) const;
  void dump(std::ostringstream& ost, const EventContext& ctx) const;

private:

  std::string trans(const EventStatus::Status& sc) const;

  // one vector entry per event slot
  //  typedef std::map<Gaudi::StringKey, AlgExecState> AlgStateMap_t;
  typedef std::vector< AlgStateMap_t > AlgStates_t;
  AlgStates_t m_algStates;
  
  std::vector< EventStatus::Status > m_eventStatus;
  std::vector< Gaudi::StringKey > m_preInitAlgs;

  bool m_isMT;

  void init();
  void checkInit() const;
  std::once_flag m_initFlag;
  bool m_isInit;
  
  std::mutex m_mut;
};

#endif
