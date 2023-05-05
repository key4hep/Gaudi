/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_EVENTCONTEXT_H
#define GAUDIKERNEL_EVENTCONTEXT_H 1

#include "GaudiKernel/EventIDBase.h"
#include <any>
#include <cstddef>
#include <iostream>
#include <limits>

/** @class EventContext EventContext.h GaudiKernel/EventContext.h
 *
 * This class represents an entry point to all the event specific data.
 * It is needed to make the algorithm "aware" of the event it is operating on.
 * This was not needed in the serial version of Gaudi where the assumption
 * of 1-event-at-the-time processing was implicit.
 *
 * This class has nothing to do with the AlgContextSvc or the Context of
 * the EvtSelector!
 *
 * @author Danilo Piparo, Charles Leggett
 * @date 2012
 **/

class EventContext {
public:
  using ContextID_t  = size_t;
  using ContextEvt_t = size_t;

  static constexpr ContextID_t  INVALID_CONTEXT_ID  = std::numeric_limits<ContextID_t>::max();
  static constexpr ContextEvt_t INVALID_CONTEXT_EVT = std::numeric_limits<ContextEvt_t>::max();

  EventContext() = default;

  explicit EventContext( const ContextEvt_t e, const ContextID_t s = INVALID_CONTEXT_ID,
                         const ContextID_t subSlot = INVALID_CONTEXT_ID )
      : m_evt_num( e ), m_evt_slot( s ), m_sub_slot( subSlot ) {
    m_valid = ( e != INVALID_CONTEXT_EVT && s != INVALID_CONTEXT_ID );
  }

  ContextEvt_t       evt() const { return m_evt_num; }
  ContextID_t        slot() const { return m_evt_slot; }
  ContextID_t        subSlot() const { return m_sub_slot; }
  bool               usesSubSlot() const { return m_sub_slot != INVALID_CONTEXT_ID; }
  bool               valid() const { return m_valid; }
  const EventIDBase& eventID() const { return m_eid; }

  void set( const ContextEvt_t e = 0, const ContextID_t s = INVALID_CONTEXT_ID,
            const ContextID_t subSlot = INVALID_CONTEXT_ID ) {
    m_valid    = ( e != INVALID_CONTEXT_EVT && s != INVALID_CONTEXT_ID );
    m_evt_num  = e;
    m_evt_slot = s;
    m_sub_slot = subSlot;
  }

  void setEvt( const ContextEvt_t e ) {
    if ( e == INVALID_CONTEXT_EVT ) setValid( false );
    m_evt_num = e;
  }

  void setSlot( const ContextID_t s ) {
    if ( s == INVALID_CONTEXT_ID ) setValid( false );
    m_evt_slot = s;
  }

  void setSubSlot( const ContextID_t subslot ) { m_sub_slot = subslot; }

  void setValid( const bool b = true ) {
    m_valid = b;
    if ( !m_valid ) {
      m_evt_slot = INVALID_CONTEXT_ID;
      m_evt_num  = INVALID_CONTEXT_EVT;
    }
  }

  void setEventID( const EventIDBase& e ) { m_eid = e; }

  template <typename ValueType, typename... Args>
  auto& emplaceExtension( Args&&... args ) {
    return m_extension.emplace<ValueType>( std::forward<Args>( args )... );
  }

  template <typename T>
  auto& setExtension( T&& t ) {
    m_extension = std::forward<T>( t );
    return getExtension<T>();
  }

  void resetExtension() { m_extension.reset(); }

  std::any detachExtension() { return std::move( m_extension ); }

  template <typename T>
  auto& getExtension() {
    return std::any_cast<std::decay_t<T>&>( m_extension );
  }

  template <typename T>
  const auto& getExtension() const {
    return std::any_cast<std::decay_t<T> const&>( m_extension );
  }

  template <typename T>
  T* tryGetExtension() noexcept {
    return std::any_cast<T>( &m_extension );
  }

  template <typename T>
  const T* tryGetExtension() const noexcept {
    return std::any_cast<T>( &m_extension );
  }

  bool hasExtension() const { return m_extension.has_value(); }

  template <typename T>
  bool hasExtension() const {
    return hasExtension() && m_extension.type() == typeid( std::decay_t<T> );
  }

  const std::type_info& getExtensionType() const { return m_extension.type(); }

private:
  EventIDBase  m_eid{};
  ContextEvt_t m_evt_num{ INVALID_CONTEXT_EVT };
  ContextID_t  m_evt_slot{ INVALID_CONTEXT_ID };
  ContextID_t  m_sub_slot{ INVALID_CONTEXT_ID };
  bool         m_valid{ false };

  std::any m_extension;
};

inline std::ostream& operator<<( std::ostream& os, const EventContext& ctx ) {
  if ( ctx.valid() ) {
    os << "s: " << ctx.slot() << "  e: " << ctx.evt();
    if ( ctx.usesSubSlot() ) os << " sub: " << ctx.subSlot();
    return os;
  } else {
    return os << "INVALID";
  }
}

inline std::ostream& operator<<( std::ostream& os, const EventContext* c ) {
  if ( c ) {
    return os << *c;
  } else {
    return os << "INVALID";
  }
}

#endif // GAUDIKERNEL_EVENTCONTEXT_H
