#ifndef GAUDIKERNEL_IALGEXECSTATESVC_H
#define GAUDIKERNEL_IALGEXECSTATESVC_H 1

#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/StringKey.h"
#include <map>
#include <sstream>
#include <string>

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

class AlgExecState
{
public:
  enum State { None = 0, Executing = 1, Done = 2 };

  bool              filterPassed() const { return m_filterPassed; }
  State             state() const { return m_state; }
  const StatusCode& execStatus() const { return m_execStatus; }

  void setFilterPassed( bool f = true ) { m_filterPassed = f; }
  void setState( State s ) { m_state = s; }
  void setState( State s, const StatusCode& sc )
  {
    m_state      = s;
    m_execStatus = sc;
  }
  void setExecStatus( const StatusCode& sc = StatusCode::SUCCESS ) { m_execStatus = sc; }
  void                                  reset() { *this = AlgExecState{}; }

private:
  bool       m_filterPassed{true};
  State      m_state{State::None};
  StatusCode m_execStatus{StatusCode( StatusCode::FAILURE, true )};
};

inline std::ostream& operator<<( std::ostream& ost, const AlgExecState& s )
{
  ost << "e: ";
  if ( s.state() == AlgExecState::State::None ) {
    ost << "n";
  } else if ( s.state() == AlgExecState::State::Executing ) {
    ost << "e";
  } else {
    ost << "d f: " << s.filterPassed() << " sc: " << s.execStatus();
  }
  return ost;
}

namespace EventStatus
{
  enum Status { Invalid = 0, Success = 1, AlgFail = 2, AlgStall = 3, Other = 4 };
  inline std::ostream& operator<<( std::ostream& os, Status s )
  {
    static const std::array<const char*, 5> label{"Invalid", "Success", "AlgFail", "AlgStall", "Other"};
    return os << label.at( s );
  }
}

class GAUDI_API IAlgExecStateSvc : virtual public IInterface
{
public:
  /// InterfaceID
  DeclareInterfaceID( IAlgExecStateSvc, 1, 0 );

  typedef std::map<Gaudi::StringKey, AlgExecState> AlgStateMap_t;

  // get the Algorithm Execution State for a give Algorithm and EventContext
  virtual const AlgExecState& algExecState( const Gaudi::StringKey& algName, const EventContext& ctx ) const = 0;
  virtual const AlgExecState& algExecState( IAlgorithm* iAlg, const EventContext& ctx ) const                = 0;
  virtual AlgExecState& algExecState( IAlgorithm* iAlg, const EventContext& ctx )                            = 0;

  // get all the Algorithm Execution States for a given EventContext
  virtual const AlgStateMap_t& algExecStates( const EventContext& ctx ) const = 0;

  virtual void reset( const EventContext& ctx ) = 0;

  virtual void addAlg( IAlgorithm* iAlg )                = 0;
  virtual void addAlg( const Gaudi::StringKey& algName ) = 0;

  virtual const EventStatus::Status& eventStatus( const EventContext& ctx ) const = 0;

  virtual void setEventStatus( const EventStatus::Status& sc, const EventContext& ctx ) = 0;

  virtual void updateEventStatus( const bool& b, const EventContext& ctx ) = 0;

  virtual unsigned int algErrorCount( const IAlgorithm* iAlg ) const = 0;
  virtual void resetErrorCount( const IAlgorithm* iAlg )             = 0;
  virtual unsigned int incrementErrorCount( const IAlgorithm* iAlg ) = 0;

  virtual void dump( std::ostringstream& ost, const EventContext& ctx ) const = 0;
};

#endif
