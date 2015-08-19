/*
 * CommonMessaging.h
 *
 *  Created on: Feb 20, 2009
 *      Author: Marco Clemencic
 */

#ifndef COMMONMESSAGING_H_
#define COMMONMESSAGING_H_
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/GaudiException.h"

#include <memory>

/** Templated class to add the standard messaging functionalities
 *
 */
template <class BASE>
class GAUDI_API CommonMessaging: public BASE {
public:
  typedef CommonMessaging base_class;

  /// Forward to base class constructor
  template <typename ... Args> CommonMessaging(Args&&... args) : 
  BASE(std::forward<Args>(args)...) {}

  /// Virtual destructor
  ~CommonMessaging() override = default;

  /// Needed to locate the message service
  virtual SmartIF<ISvcLocator>& serviceLocator() const = 0;

  /// This is needed to avoid ambiguous calls to name()
  virtual const std::string&  name() const = 0;

  /** The standard message service.
   *  Returns a pointer to the standard message service.
   */
  inline SmartIF<IMessageSvc>& msgSvc() const {
    if (!m_msgsvc) {
      // Get default implementation of the message service.
      m_msgsvc = this->serviceLocator();
    }
    return m_msgsvc;
  }

#if defined(GAUDI_V20_COMPAT) && !defined(G21_NO_DEPRECATED)
  /** The standard message service.
   *  Returns a pointer to the standard message service.
   *  (Alias to msgSvc())
   */
  inline SmartIF<IMessageSvc>& messageService() const {
    return msgSvc();
  }
#endif

  /// Return an uninitialized MsgStream.
  inline MsgStream& msgStream() const {
    if (UNLIKELY((!m_msgStream) || (!m_streamWithService))) {
      auto& ms = msgSvc();
      m_msgStream.reset(new MsgStream(ms, this->name()));
      m_streamWithService = ms.isValid();
    }
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
  inline MsgStream& msgStream(const MSG::Level level) const {
    return msgStream() << level;
  }

  /// shortcut for the method msgStream(MSG::ALWAYS)
  inline MsgStream&  always() const { return msgStream(MSG::ALWAYS); }

  /// shortcut for the method msgStream(MSG::FATAL)
  inline MsgStream&   fatal() const { return msgStream(MSG::FATAL); }

  /// shortcut for the method msgStream(MSG::ERROR)
  inline MsgStream&     err() const { return msgStream(MSG::ERROR); }

  /// shortcut for the method msgStream(MSG::ERROR)
  inline MsgStream&   error() const { return msgStream(MSG::ERROR); }

  /// shortcut for the method msgStream(MSG::WARNING)
  inline MsgStream& warning() const { return msgStream(MSG::WARNING); }

  /// shortcut for the method msgStream(MSG::INFO)
  inline MsgStream&    info() const { return msgStream(MSG::INFO); }

  /// shortcut for the method msgStream(MSG::DEBUG)
  inline MsgStream&   debug() const { return msgStream(MSG::DEBUG); }

  /// shortcut for the method msgStream(MSG::VERBOSE)
  inline MsgStream& verbose() const { return msgStream(MSG::VERBOSE); }

  /// shortcut for the method msgStream(MSG::INFO)
  inline MsgStream&     msg() const { return msgStream(MSG::INFO); }

  /// get the output level from the embedded MsgStream
  inline MSG::Level msgLevel() const {
    return msgStream().level();
  }

  /// get the output level from the embedded MsgStream
  inline bool msgLevel(MSG::Level lvl) const {
    return UNLIKELY(msgLevel() <= lvl);
  }

protected:
  /// Pointer to the message service;
  mutable SmartIF<IMessageSvc> m_msgsvc;

  /// The predefined message stream
  mutable std::unique_ptr<MsgStream> m_msgStream;

  /// Flag to create a new MsgStream if it was created without the message service
  mutable bool m_streamWithService = false;

  /// Update the output level of the cached MsgStream.
  /// This function is meant to be called by the update handler of the OutputLevel property.
  void updateMsgStreamOutputLevel(int level) {
    if (m_msgStream) m_msgStream->setLevel(level);
  }

};


#endif /* COMMONMESSAGING_H_ */
