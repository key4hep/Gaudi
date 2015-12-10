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
#include <utility>

/** Templated class to add the standard messaging functionalities
 *
 */

#define generate_( method, ret, args )               \
                                                     \
    template <typename T, typename SFINAE = void>    \
    struct has_ ## method : std::false_type {};      \
    template <typename T>                            \
    struct has_ ## method<T, is_valid_t<decltype(std::declval<const T&>().method args)>> : std::true_type {}; \
                                                     \
    template <typename Base>                         \
    struct add_ ## method : public Base {            \
        using Base::Base;                            \
        virtual ~add_ ## method () = default;        \
        virtual ret method args const = 0;           \
    };

namespace implementation_detail {
    template <typename> struct void_t { using type = void; };
    template <typename T> using is_valid_t = typename void_t<T>::type;

    generate_( name, const std::string&, () )
    generate_( serviceLocator, SmartIF<ISvcLocator>&, () )
}
#undef generate_

template <typename Base> using add_name = typename std::conditional<implementation_detail::has_name<Base>::value,
                                                                    Base,
                                                                    implementation_detail::add_name< Base>>::type;
template <typename Base> using add_serviceLocator = typename std::conditional<implementation_detail::has_serviceLocator<Base>::value,
                                                                              Base,
                                                                              implementation_detail::add_serviceLocator< Base>>::type;


template <typename Base> class CommonMessaging;

class CommonMessagingBase {
public:
  /// Virtual destructor
  virtual ~CommonMessagingBase() = default;
  /// cold functionality
  virtual void create_msgSvc() const = 0;
  virtual void create_msgStream() const = 0;



  /** The standard message service.
   *  Returns a pointer to the standard message service.
   */
  inline SmartIF<IMessageSvc>& msgSvc() const {
    if (UNLIKELY(!m_msgsvc)) create_msgSvc();
    return m_msgsvc;
  }

  /// Return an uninitialized MsgStream.
  inline MsgStream& msgStream() const {
    if (UNLIKELY((m_createMsgStream))) create_msgStream();
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
    if (UNLIKELY(!m_msgStream)) create_msgStream();
    return m_level;
  }

  /// Backward compatibility function for getting the output level
  inline MSG::Level outputLevel() const __attribute__ ((deprecated)) { return msgLevel(); }

  /// get the output level from the embedded MsgStream
  inline bool msgLevel(MSG::Level lvl) const { return UNLIKELY(msgLevel() <= lvl); }

private:
  template <typename Base> friend class CommonMessaging;

  /// The predefined message stream
  mutable std::unique_ptr<MsgStream> m_msgStream;

  mutable MSG::Level m_level = MSG::NIL;
  /// Flag to trigger a new MsgStream
  mutable bool m_createMsgStream = true;

  /// Pointer to the message service;
  mutable SmartIF<IMessageSvc> m_msgsvc;


};

template <typename BASE>
class GAUDI_API CommonMessaging: public add_serviceLocator<add_name<BASE>>, public CommonMessagingBase {
public:
  using base_class = CommonMessaging;

  /// Forward constructor to base class constructor
  using add_serviceLocator<add_name<BASE>>::add_serviceLocator;

private:
  // out-of-line 'cold' functions -- put here so as to not blow up the inline 'hot' functions
  void create_msgSvc() const override final {
      // Get default implementation of the message service.
      m_msgsvc = this->serviceLocator();
  }
  void create_msgStream() const override final {
      auto& ms = msgSvc();
      m_msgStream.reset(new MsgStream(ms, this->name()));
      m_createMsgStream = (!ms.isValid() || !m_msgStream);
      m_level = m_msgStream ? m_msgStream->level() : MSG::NIL;
  }

protected:
  /// Update the output level of the cached MsgStream.
  /// This function is meant to be called by the update handler of the OutputLevel property.
  void updateMsgStreamOutputLevel(int level) {
    if (level != MSG::NIL && level != m_level) {
      if (msgSvc()) {
        msgSvc()->setOutputLevel(this->name(), level);
      }
      if (m_msgStream) m_msgStream->setLevel(level);
      if (UNLIKELY(MSG::Level(level) <= MSG::DEBUG))
        debug() << "Property update for OutputLevel : new value = " << level << endmsg;
      m_level = MSG::Level(level);
    }
  }

};


#endif /* COMMONMESSAGING_H_ */
