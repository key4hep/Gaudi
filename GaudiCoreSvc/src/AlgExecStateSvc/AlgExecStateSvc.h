/***********************************************************************************\
* (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDICORESVC_ALGEXECSTATESVC_H
#define GAUDICORESVC_ALGEXECSTATESVC_H 1

#include "GaudiKernel/IAlgExecStateSvc.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/Service.h"

#include <mutex>
#include <unordered_map>
#include <vector>

/** @class AlgExecStateSvc
 * @brief A service that keeps track of the execution state of Algorithm
 *
 */
class AlgExecStateSvc : public extends<Service, IAlgExecStateSvc> {
public:
  using extends::extends;

  typedef IAlgExecStateSvc::AlgStateMap_t AlgStateMap_t;

  using IAlgExecStateSvc::algExecState;
  const AlgExecState&  algExecState( const Gaudi::StringKey& algName, const EventContext& ctx ) const override;
  AlgExecState&        algExecState( IAlgorithm* iAlg, const EventContext& ctx ) override;
  const AlgStateMap_t& algExecStates( const EventContext& ctx ) const override;

  void reset( const EventContext& ctx ) override;

  using IAlgExecStateSvc::addAlg;
  void addAlg( const Gaudi::StringKey& algName ) override;

  const EventStatus::Status& eventStatus( const EventContext& ctx ) const override;

  void setEventStatus( const EventStatus::Status& sc, const EventContext& ctx ) override;

  void updateEventStatus( const bool& b, const EventContext& ctx ) override;

  unsigned int algErrorCount( const IAlgorithm* iAlg ) const override;
  void         resetErrorCount( const IAlgorithm* iAlg ) override;
  unsigned int incrementErrorCount( const IAlgorithm* iAlg ) override;

  void dump( std::ostringstream& ost, const EventContext& ctx ) const override;

private:
  // one vector entry per event slot
  std::vector<AlgStateMap_t>              m_algStates;
  std::vector<std::vector<AlgStateMap_t>> m_algSubSlotStates;

  std::vector<EventStatus::Status> m_eventStatus;
  std::vector<Gaudi::StringKey>    m_preInitAlgs;

  std::unordered_map<Gaudi::StringKey, std::atomic<unsigned int>> m_errorCount;

  void           init();
  void           checkInit() const;
  std::once_flag m_initFlag;
  bool           m_isInit{false};

  std::mutex m_mut;
};

#endif
