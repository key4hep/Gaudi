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
/*
 * CommonMessaging.h
 *
 *  Created on: Feb 20, 2009
 *      Author: Marco Clemencic
 */

#ifndef COMMONMESSAGING_H_
#define COMMONMESSAGING_H_
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/detected.h"
#include <boost/thread/tss.hpp>

#include <memory>
#include <utility>

/** Templated class to add the standard messaging functionalities
 *
 */

#define generate_( method, ret, args )                                                                                 \
                                                                                                                       \
  template <typename T>                                                                                                \
  using _has_##method = decltype( std::declval<const T&>().method args );                                              \
                                                                                                                       \
  template <typename T>                                                                                                \
  using has_##method = Gaudi::cpp17::is_detected<_has_##method, T>;                                                    \
                                                                                                                       \
  template <typename Base>                                                                                             \
  struct add_##method : public Base {                                                                                  \
    using Base::Base;                                                                                                  \
    virtual ~add_##method()       = default;                                                                           \
    virtual ret method args const = 0;                                                                                 \
  };

namespace implementation_detail {
  generate_( name, const std::string&, () )

      generate_( serviceLocator, SmartIF<ISvcLocator>&, () )
}
#undef generate_

template <typename Base>
using add_name =
    std::conditional_t<implementation_detail::has_name<Base>::value, Base, implementation_detail::add_name<Base>>;
template <typename Base>
using add_serviceLocator = std::conditional_t<implementation_detail::has_serviceLocator<Base>::value, Base,
                                              implementation_detail::add_serviceLocator<Base>>;

template <typename Base>
class CommonMessaging;

class CommonMessagingBase {
public:
  /// Virtual destructor
  virtual ~CommonMessagingBase() = default;
  /// cold functionality
  virtual void create_msgStream() const = 0;

  /** The standard message service.
   *  Returns a pointer to the standard message service.
   */
  const SmartIF<IMessageSvc>& msgSvc() const { return m_msgsvc; }

  /// Return an uninitialized MsgStream.
  MsgStream& msgStream() const {
    if ( !m_msgStream.get() ) create_msgStream();
    return *m_msgStream;
  }

  /** Predefined configurable message stream for the efficient printouts
   *
   *  @code
   *
   *  if ( a < 0 ) { msgStream( MSG::ERROR ) << "a = " << endmsg ; }
   *
   *  @endcode
   *
   *  @return Reference to the predefined stream
   */
  MsgStream& msgStream( const MSG::Level level ) const { return msgStream() << level; }

  /// shortcut for the method msgStream(MSG::ALWAYS)
  MsgStream& always() const { return msgStream( MSG::ALWAYS ); }

  /// shortcut for the method msgStream(MSG::FATAL)
  MsgStream& fatal() const { return msgStream( MSG::FATAL ); }

  /// shortcut for the method msgStream(MSG::ERROR)
  MsgStream& err() const { return msgStream( MSG::ERROR ); }

  /// shortcut for the method msgStream(MSG::ERROR)
  MsgStream& error() const { return msgStream( MSG::ERROR ); }

  /// shortcut for the method msgStream(MSG::WARNING)
  MsgStream& warning() const { return msgStream( MSG::WARNING ); }

  /// shortcut for the method msgStream(MSG::INFO)
  MsgStream& info() const { return msgStream( MSG::INFO ); }

  /// shortcut for the method msgStream(MSG::DEBUG)
  MsgStream& debug() const { return msgStream( MSG::DEBUG ); }

  /// shortcut for the method msgStream(MSG::VERBOSE)
  MsgStream& verbose() const { return msgStream( MSG::VERBOSE ); }

  /// shortcut for the method msgStream(MSG::INFO)
  MsgStream& msg() const { return msgStream( MSG::INFO ); }

private:
  template <typename Base>
  friend class CommonMessaging;

  mutable bool       m_commonMessagingReady = false;
  mutable MSG::Level m_level                = MSG::NIL;

  /// The predefined message stream
  mutable boost::thread_specific_ptr<MsgStream> m_msgStream;

  /// Pointer to the message service;
  mutable SmartIF<IMessageSvc> m_msgsvc;
};

template <typename BASE>
class GAUDI_API CommonMessaging : public add_serviceLocator<add_name<BASE>>, public CommonMessagingBase {
public:
  using base_class = CommonMessaging;

  /// Forward constructor to base class constructor
  using add_serviceLocator<add_name<BASE>>::add_serviceLocator;

  /// get the cached level (originally extracted from the embedded MsgStream)
  MSG::Level msgLevel() const {
    if ( m_commonMessagingReady ) return m_level;
    return setUpMessaging();
  }

  /// get the output level from the embedded MsgStream
  bool msgLevel( MSG::Level lvl ) const { return msgLevel() <= lvl; }

private:
  // out-of-line 'cold' functions -- put here so as to not blow up the inline 'hot' functions
  void create_msgStream() const override final { m_msgStream.reset( new MsgStream( msgSvc(), this->name() ) ); }

  /// Initialise the messaging objects
  void initMessaging() const {
    if ( !m_msgsvc ) {
      // Get default implementation of the message service.
      m_msgsvc = this->serviceLocator();
    }
    create_msgStream();
    m_level = MSG::Level( m_msgStream.get() ? m_msgStream->level() : MSG::NIL );
    // if we could not get a MessageSvc, we should try again the initial set up
    m_commonMessagingReady = m_msgsvc;
  }

protected:
  /// Set up local caches
  MSG::Level setUpMessaging() const {
    if ( !m_commonMessagingReady ) { initMessaging(); }
    return m_level;
  }
  /// Reinitialize internal states.
  MSG::Level resetMessaging() {
    m_commonMessagingReady = false;
    return setUpMessaging();
  }
  /// Update the output level of the cached MsgStream.
  /// This function is meant to be called by the update handler of the OutputLevel property.
  void updateMsgStreamOutputLevel( int level ) {
    setUpMessaging();
    if ( level != MSG::NIL && level != m_level ) {
      if ( msgSvc() ) msgSvc()->setOutputLevel( this->name(), level );
      if ( m_msgStream.get() ) m_msgStream->setLevel( level );
      if ( MSG::Level( level ) <= MSG::DEBUG )
        debug() << "Property update for OutputLevel : new value = " << level << endmsg;
      m_level = MSG::Level( level );
    }
  }
};

#endif /* COMMONMESSAGING_H_ */
