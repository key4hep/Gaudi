#ifndef GAUDIKERNEL_IALGEXECSTATESVC_H
#define GAUDIKERNEL_IALGEXECSTATESVC_H 1

#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/StringKey.h"
#include <string>
#include <sstream>
#include <map>

class IAlgorithm;
class EventContext;

//-----------------------------------------------------------------------------

/** @class IAlgExecStateSvc GaudiKernel/IAlgExecStateSvc.h
 *
 *  @brief Abstract interface for a service that manages the Algorithm execution
 *         states
 *
 *  @author Charles Leggett
 *  @date   2016-04-12
 */

//-----------------------------------------------------------------------------

class AlgExecState {
public:

  AlgExecState() : m_filterPassed(true), m_isExecuted(false), 
                   m_execStatus(StatusCode(StatusCode::FAILURE,true)) {}

  bool filterPassed() const { return m_filterPassed; }
  bool isExecuted() const { return m_isExecuted; }
  const StatusCode& execStatus() const { return m_execStatus; }

  void setFilterPassed(bool f = true) { m_filterPassed = f; }
  void setExecuted(bool e = true) { m_isExecuted = e; }
  void setExecStatus(const StatusCode& sc = StatusCode::SUCCESS) { 
    m_execStatus = sc; }

  void reset() { 
    m_filterPassed = true; 
    m_isExecuted = false; 
    m_execStatus = StatusCode(StatusCode::FAILURE,true); 
  }

private:

  bool m_filterPassed;
  bool m_isExecuted;
  StatusCode m_execStatus;  

};

inline std::ostream& operator<< (std::ostream&  ost, const AlgExecState& s) {
  return ost << "e: " << s.isExecuted() << " f: " << s.filterPassed()
             << " sc: " << s.execStatus();
}

namespace EventStatus {
  enum Status {
    Invalid = 0,
    Success = 1,
    AlgFail = 2,
    AlgStall = 3,
    Other = 4
  };
}

class GAUDI_API  IAlgExecStateSvc : virtual public IInterface {
 public:

  /// InterfaceID
  DeclareInterfaceID(IAlgExecStateSvc,1,0);

  typedef std::map<Gaudi::StringKey, AlgExecState> AlgStateMap_t;

  // get the Algorithm Execution State for a give Algorithm and EventContext
  virtual const AlgExecState& algExecState(const Gaudi::StringKey& algName, 
                                           const EventContext& ctx) const = 0;
  virtual const AlgExecState& algExecState(IAlgorithm* iAlg,     
                                           const EventContext& ctx) const = 0;
  virtual AlgExecState& algExecState(IAlgorithm* iAlg, 
                                     const EventContext& ctx) = 0;

  // get all the Algorithm Execution States for a given EventContext
  virtual const AlgStateMap_t& algExecStates(const EventContext& ctx) const = 0;  

  virtual void reset(const EventContext& ctx) = 0;

  virtual void addAlg(IAlgorithm* iAlg) = 0;
  virtual void addAlg(const Gaudi::StringKey& algName) = 0;

  virtual const EventStatus::Status& eventStatus(const EventContext& ctx) const = 0;

  virtual void setEventStatus(const EventStatus::Status& sc, const EventContext& ctx) = 0;

  virtual void updateEventStatus(const bool& b, const EventContext& ctx) = 0;

  virtual unsigned int algErrorCount(const IAlgorithm* iAlg) const = 0;
  virtual void resetErrorCount(const IAlgorithm* iAlg) = 0;
  virtual unsigned int incrementErrorCount(const IAlgorithm* iAlg) = 0;

  virtual void dump(std::ostringstream& ost, const EventContext& ctx) const = 0;

};

#endif
