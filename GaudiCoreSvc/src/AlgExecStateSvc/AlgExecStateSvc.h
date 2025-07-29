/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/IAlgExecStateSvc.h>
#include <GaudiKernel/IAlgorithm.h>
#include <GaudiKernel/Service.h>

#include <fmt/format.h>

#include <mutex>
#include <unordered_map>
#include <vector>

/**
 * @brief A service that keeps track of the execution state of Algorithm
 *
 */
class AlgExecStateSvc : public extends<Service, IAlgExecStateSvc> {

  /**
   *  internal state of an algorithm
   */
  class AlgExecStateInternal {
  public:
    bool                   filterPassed() const { return m_filterPassed; }
    void                   setFilterPassed( bool f = true ) { m_filterPassed = f; }
    AlgExecStateRef::State state() const { return m_state; }
    void                   setState( AlgExecStateRef::State s ) { m_state = s; }
    void                   setState( AlgExecStateRef::State s, const StatusCode& sc ) {
      m_state      = s;
      m_execStatus = sc;
    }
    const StatusCode& execStatus() const { return m_execStatus; }
    void              setExecStatus( const StatusCode& sc = StatusCode::SUCCESS ) { m_execStatus = sc; }
    void              reset() { *this = AlgExecStateInternal{}; }

    friend std::ostream& operator<<( std::ostream& ost, const AlgExecStateInternal& s ) {
      ost << "e: ";
      switch ( s.state() ) {
      case AlgExecStateRef::State::None:
        return ost << "n";
      case AlgExecStateRef::State::Executing:
        return ost << "e";
      default:
        return ost << "d f: " << s.filterPassed() << " sc: " << s.execStatus();
      }
    }

  private:
    bool                   m_filterPassed{ true };
    AlgExecStateRef::State m_state{ AlgExecStateRef::State::None };
    StatusCode             m_execStatus{ StatusCode::FAILURE };
  };

  using AlgStates            = std::vector<AlgExecStateInternal>;
  using AlgStatesWithSubSlot = std::unordered_map<AlgExecStateRef::AlgKey, AlgExecStateInternal>;

public:
  using extends::extends;

  AlgExecStateRef algExecState( const IAlgorithm* iAlg, const EventContext& ctx ) override {
    return { *this, ctx, algKey( iAlg->name() ) };
  }
  AlgExecStateRef algExecState( const std::string& algName, const EventContext& ctx ) override {
    return { *this, ctx, algKey( algName ) };
  }
  AlgExecStateRefConst algExecState( const IAlgorithm* iAlg, const EventContext& ctx ) const override {
    return { *this, ctx, algKey( iAlg->name() ) };
  }
  AlgExecStateRefConst algExecState( const std::string& algName, const EventContext& ctx ) const override {
    return { *this, ctx, algKey( algName ) };
  }
  void reset( const EventContext& ctx ) override;

  using IAlgExecStateSvc::addAlg;
  AlgExecStateRef::AlgKey addAlg( const Gaudi::StringKey& ) override;

  const EventStatus::Status& eventStatus( const EventContext& ctx ) const override;
  void                       setEventStatus( const EventStatus::Status& sc, const EventContext& ctx ) override;
  void                       updateEventStatus( const bool& b, const EventContext& ctx ) override;

  unsigned int algErrorCount( const IAlgorithm* iAlg ) const override;
  void         resetErrorCount() override;
  void         resetErrorCount( const IAlgorithm* iAlg ) override;
  unsigned int incrementErrorCount( const IAlgorithm* iAlg ) override;

  void dump( std::ostream& ost, const EventContext& ctx ) const override;
  void dump( std::ostream&, const EventContext&, AlgExecStateRef::AlgKey ) const override;

private:
  bool                   filterPassed( const EventContext&, AlgExecStateRef::AlgKey ) const override;
  void                   setFilterPassed( const EventContext&, AlgExecStateRef::AlgKey, bool ) override;
  AlgExecStateRef::State state( const EventContext&, AlgExecStateRef::AlgKey ) const override;
  void                   setState( const EventContext&, AlgExecStateRef::AlgKey, AlgExecStateRef::State ) override;
  void setState( const EventContext&, AlgExecStateRef::AlgKey, AlgExecStateRef::State, const StatusCode& ) override;
  const StatusCode&  execStatus( const EventContext&, AlgExecStateRef::AlgKey ) const override;
  void               setExecStatus( const EventContext&, AlgExecStateRef::AlgKey, const StatusCode& ) override;
  const std::string& algName( AlgExecStateRef::AlgKey ) const override;

  AlgExecStateInternal&       getInternalState( const EventContext& ctx, AlgExecStateRef::AlgKey k );
  AlgExecStateInternal const& getInternalState( const EventContext& ctx, AlgExecStateRef::AlgKey k ) const;
  AlgExecStateRef::AlgKey     algKey( const std::string& algName ) const { return m_algNameToIndex.at( algName ); }

  // one vector entry per event slot
  std::vector<AlgStates> m_algStates;
  // one map entry per event slot and subslot, with AlgKey as a key, for sparse storage
  std::vector<std::vector<AlgStatesWithSubSlot>> m_algSubSlotStates;
  // algorithm name to slot (with heterogeneous lookup), needed to regognize double addition
  std::unordered_map<Gaudi::StringKey, AlgExecStateRef::AlgKey, Gaudi::StringKeyHash, std::equal_to<>> m_algNameToIndex;

  std::vector<EventStatus::Status> m_eventStatus;
  std::vector<Gaudi::StringKey>    m_preInitAlgs;
  // keep the names of the algorithms by AlgKey for logging purposes
  std::vector<std::string> m_algNames;

  // Cannot be a vector as atomics are not copy/move constructible
  std::unordered_map<AlgExecStateRef::AlgKey, std::atomic<unsigned int>> m_errorCount;

  void           init();
  void           checkInit() const;
  std::once_flag m_initFlag;
  bool           m_isInit{ false };

  mutable std::mutex m_mut;
};
