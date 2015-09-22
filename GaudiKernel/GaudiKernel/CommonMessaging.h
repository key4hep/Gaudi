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

#define generate_has_( method, args )                     \
    template <typename T, typename SFINAE = void>         \
    struct has_ ## method : std::false_type {};           \
    template <typename T>                                 \
    struct has_ ## method<T, is_valid_t<decltype(std::declval<const T&>().method args)>> : std::true_type {};

#define generate_add_(method, ret, args )                 \
    template <typename Base, bool> struct add_ ## method; \
    template <typename Base>                              \
    struct add_ ## method <Base,false> : public Base {    \
        using Base::Base;                                 \
    };                                                    \
    template <typename Base>                              \
    struct add_ ## method <Base, true> : public Base {    \
        using Base::Base;                                 \
        virtual ~add_ ## method () = default;             \
        virtual ret method args const = 0;                \
    }; 

namespace implementation_detail {
    template <typename> struct void_t { typedef void type; };
    template <typename T> using is_valid_t = typename void_t<T>::type;

    generate_has_( name, ()  )
    generate_add_( name, const std::string&, () )
    generate_has_( serviceLocator, () )
    generate_add_( serviceLocator, SmartIF<ISvcLocator>&, () )
}
#undef generate_has_
#undef generate_add_

template <typename Base> using add_name = implementation_detail::add_name< Base, ! implementation_detail::has_name<Base>::value >;
template <typename Base> using add_serviceLocator = implementation_detail::add_serviceLocator< Base, ! implementation_detail::has_serviceLocator<Base>::value >;


template <typename BASE>
class GAUDI_API CommonMessaging: public add_serviceLocator<add_name<BASE>> {
public:
  using base_class = CommonMessaging;

  /// Forward constructor to base class constructor
  using add_serviceLocator<add_name<BASE>>::add_serviceLocator;

  /// Virtual destructor
  ~CommonMessaging() override = default;

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
