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

#include <GaudiKernel/IAlgorithm.h>
#include <GaudiKernel/IInterface.h>
#include <GaudiKernel/StatusCode.h>
#include <GaudiKernel/StringKey.h>
#include <sstream>

class EventContext;
class IAlgExecStateSvc;

namespace AlgExecState {
  enum State { None = 0, Executing = 1, Done = 2 };
} // namespace AlgExecState

/**
 * wrapper on an Algorithm state.
 *
 * Practically a reference to the IAlgExecStateSvc, an EventContext and a size_t identifying the algorithm involved
 * It forwards all calls to the equivalent API of the IAlgExecStateSvc
 */
class AlgExecStateRef {
public:
  using State = AlgExecState::State;
  enum struct AlgKey : size_t {};
  AlgExecStateRef( IAlgExecStateSvc& svc, const EventContext& ctx, AlgKey algKey )
      : m_svc{ svc }, m_ctx{ ctx }, m_algKey{ algKey } {}

  bool                 filterPassed() const;
  void                 setFilterPassed( bool f = true );
  State                state() const;
  void                 setState( State s );
  void                 setState( State s, const StatusCode& sc );
  const StatusCode&    execStatus() const;
  void                 setExecStatus( const StatusCode& sc = StatusCode::SUCCESS );
  void                 reset();
  friend std::ostream& operator<<( std::ostream& ost, const AlgExecStateRef& s );

private:
  IAlgExecStateSvc&   m_svc;
  const EventContext& m_ctx;
  AlgKey              m_algKey;
};

/**
 * wrapper on an Algorithm state, const version. See AlgExecStateRef for details
 */
class AlgExecStateRefConst {
public:
  AlgExecStateRefConst( const IAlgExecStateSvc& svc, const EventContext& ctx, AlgExecStateRef::AlgKey algKey )
      : m_svc{ svc }, m_ctx{ ctx }, m_algKey{ algKey } {}

  bool                   filterPassed() const;
  AlgExecStateRef::State state() const;
  const StatusCode&      execStatus() const;
  friend std::ostream&   operator<<( std::ostream& ost, const AlgExecStateRefConst& s );

private:
  const IAlgExecStateSvc& m_svc;
  const EventContext&     m_ctx;
  AlgExecStateRef::AlgKey m_algKey;
};

namespace EventStatus {
  enum Status { Invalid = 0, Success = 1, AlgFail = 2, AlgStall = 3, Other = 4 };
  inline std::ostream& operator<<( std::ostream& os, Status s ) {
    static constexpr std::array<const char*, 5> label{ "Invalid", "Success", "AlgFail", "AlgStall", "Other" };
    return os << label.at( s );
  }
} // namespace EventStatus

/**
 *  @brief Abstract interface for a service that manages the Algorithm execution states
 *
 *  This service allows to handle execStates of algorithms.
 *  Algorithms are identified by their AlgKey and the EventContext to be used.
 *  The interface allows to retrieve an algExecState for a given algorithm and reuse
 *  it. It is actually a wrapper on the actual State with the same interface allowing
 *  more efficient subsequent calls for that algorithm (setState, setFilterPassed, ...)
 *
 *  @author Charles Leggett
 *  @date   2016-04-12
 */
class GAUDI_API IAlgExecStateSvc : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( IAlgExecStateSvc, 2, 0 );

  /// get the Algorithm Execution State for a give Algorithm by name
  /// prefer using IAlgorithm::execState, it is more efficient
  virtual AlgExecStateRef algExecState( const std::string& algName, const EventContext& ctx ) = 0;

  /// get the Algorithm Execution State for a give Algorithm and EventContext
  /// prefer using IAlgorithm::execState, it is more efficient
  virtual AlgExecStateRef algExecState( const IAlgorithm* iAlg, const EventContext& ctx ) = 0;

  /// get the Algorithm Execution State for a give Algorithm by name, const version
  /// prefer using IAlgorithm::execState, it is more efficient
  virtual AlgExecStateRefConst algExecState( const std::string& algName, const EventContext& ctx ) const = 0;

  /// get the Algorithm Execution State for a give Algorithm and EventContext, const version
  /// prefer using IAlgorithm::execState, it is more efficient
  virtual AlgExecStateRefConst algExecState( const IAlgorithm* iAlg, const EventContext& ctx ) const = 0;

  /// reset all states for the given EventContext
  virtual void reset( const EventContext& ctx ) = 0;

  /// adds a new algorithm to the service
  virtual AlgExecStateRef::AlgKey addAlg( const Gaudi::StringKey& algName ) = 0;
  AlgExecStateRef::AlgKey         addAlg( const IAlgorithm* iAlg ) { return addAlg( iAlg->nameKey() ); }

  virtual const EventStatus::Status& eventStatus( const EventContext& ctx ) const                             = 0;
  virtual void                       setEventStatus( const EventStatus::Status& sc, const EventContext& ctx ) = 0;
  virtual void                       updateEventStatus( const bool& b, const EventContext& ctx )              = 0;

  virtual unsigned int algErrorCount( const IAlgorithm* iAlg ) const = 0;
  virtual void         resetErrorCount( const IAlgorithm* iAlg )     = 0;
  virtual void         resetErrorCount()                             = 0;
  virtual unsigned int incrementErrorCount( const IAlgorithm* iAlg ) = 0;

  virtual void dump( std::ostream& ost, const EventContext& ctx ) const                  = 0;
  virtual void dump( std::ostream&, const EventContext&, AlgExecStateRef::AlgKey ) const = 0;

private:
  virtual bool                   filterPassed( const EventContext&, AlgExecStateRef::AlgKey ) const                = 0;
  virtual void                   setFilterPassed( const EventContext&, AlgExecStateRef::AlgKey, bool )             = 0;
  virtual AlgExecStateRef::State state( const EventContext&, AlgExecStateRef::AlgKey ) const                       = 0;
  virtual void                   setState( const EventContext&, AlgExecStateRef::AlgKey, AlgExecStateRef::State )  = 0;
  virtual void setState( const EventContext&, AlgExecStateRef::AlgKey, AlgExecStateRef::State, const StatusCode& ) = 0;
  virtual const StatusCode&  execStatus( const EventContext&, AlgExecStateRef::AlgKey ) const                      = 0;
  virtual void               setExecStatus( const EventContext&, AlgExecStateRef::AlgKey, const StatusCode& )      = 0;
  virtual const std::string& algName( AlgExecStateRef::AlgKey ) const                                              = 0;

  friend class AlgExecStateRefConst;
  friend class AlgExecStateRef;
};

inline bool AlgExecStateRef::filterPassed() const { return m_svc.filterPassed( m_ctx, m_algKey ); }
inline void AlgExecStateRef::setFilterPassed( bool f ) { m_svc.setFilterPassed( m_ctx, m_algKey, f ); }
inline AlgExecStateRef::State AlgExecStateRef::state() const { return m_svc.state( m_ctx, m_algKey ); }
inline void AlgExecStateRef::setState( AlgExecStateRef::State s ) { m_svc.setState( m_ctx, m_algKey, s ); }
inline void AlgExecStateRef::setState( AlgExecStateRef::State s, const StatusCode& sc ) {
  m_svc.setState( m_ctx, m_algKey, s, sc );
}
inline const StatusCode& AlgExecStateRef::execStatus() const { return m_svc.execStatus( m_ctx, m_algKey ); }
inline void AlgExecStateRef::setExecStatus( const StatusCode& sc ) { m_svc.setExecStatus( m_ctx, m_algKey, sc ); }
inline void AlgExecStateRef::reset() { m_svc.reset( m_ctx ); }
inline std::ostream& operator<<( std::ostream& ost, const AlgExecStateRef& s ) {
  s.m_svc.dump( ost, s.m_ctx, s.m_algKey );
  return ost;
}

inline bool AlgExecStateRefConst::filterPassed() const { return m_svc.filterPassed( m_ctx, m_algKey ); }
inline AlgExecStateRef::State AlgExecStateRefConst::state() const { return m_svc.state( m_ctx, m_algKey ); }
inline const StatusCode&      AlgExecStateRefConst::execStatus() const { return m_svc.execStatus( m_ctx, m_algKey ); }
inline std::ostream&          operator<<( std::ostream& ost, const AlgExecStateRefConst& s ) {
  s.m_svc.dump( ost, s.m_ctx, s.m_algKey );
  return ost;
}
