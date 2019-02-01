#ifndef GAUDIKERNEL_IINTERFACE_H
#define GAUDIKERNEL_IINTERFACE_H

// Include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/System.h"
#include <ostream>
#include <type_traits>
#include <typeinfo>

/// Macro to declare the interface ID when using the new mechanism of extending and implementing interfaces.
#define DeclareInterfaceID( iface, major, minor )                                                                      \
  static const InterfaceID& interfaceID() { return iid::interfaceID(); }                                               \
  using iid      = Gaudi::InterfaceId<iface, major, minor>;                                                            \
  using ext_iids = iid::iids

/**
 * @class InterfaceID Kernel.h GaudiKernel/Kernel.h
 *
 * Interface ID class. It consists of several fields: interface number,
 * major version number and minor version number. Interface number should be
 * allocated globally. The major and minor version numbers are used for
 * checking compatibility between interface versions.
 *
 * @author Pere Mato
 * @author Sebastien Ponce
 */
class GAUDI_API InterfaceID final {
public:
#if defined( GAUDI_V20_COMPAT ) && !defined( G21_NEW_INTERFACES )
  /// constructor from a pack long
  constexpr InterfaceID( unsigned long lid )
      : m_id( lid & 0xFFFF ), m_major_ver( ( lid & 0xFF000000 ) >> 24 ), m_minor_ver( ( lid & 0xFF0000 ) >> 16 ) {}
#endif
  /// constructor from components
  constexpr InterfaceID( unsigned long id, unsigned long major, unsigned long minor = 0 )
      : m_id( id ), m_major_ver( major ), m_minor_ver( minor ) {}
  /// constructor from components
  InterfaceID( const char* name, unsigned long major, unsigned long minor = 0 )
      : m_id( hash32( name ) ), m_major_ver( major ), m_minor_ver( minor ) {}
#if defined( GAUDI_V20_COMPAT ) && !defined( G21_NEW_INTERFACES )
  /// conversion to unsigned long
  constexpr operator unsigned long() const { return ( m_major_ver << 24 ) + ( m_minor_ver << 16 ) + m_id; }
#endif
  /// get the interface identifier
  constexpr unsigned long id() const { return m_id; }
  /// get the major version of the interface
  constexpr unsigned long majorVersion() const { return m_major_ver; }
  /// get the minor version of the interface
  constexpr unsigned long minorVersion() const { return m_minor_ver; }
  /** check compatibility. The major version is check and the minor one
   *  should be bigger or equal
   */
  constexpr bool versionMatch( const InterfaceID& iid ) const {
    return ( id() == iid.id() && majorVersion() == iid.majorVersion() && minorVersion() >= iid.minorVersion() );
  }
  /// check full compatibility.
  constexpr bool fullMatch( const InterfaceID& iid ) const {
    return ( id() == iid.id() && majorVersion() == iid.majorVersion() && minorVersion() == iid.minorVersion() );
  }
  /// compare operator
  constexpr bool operator==( const InterfaceID& iid ) const { return fullMatch( iid ); }
  /// Jenkins one-at-time hash function -- see https://en.wikipedia.org/wiki/Jenkins_hash_function
  static unsigned int hash32( const char* key ) {
    unsigned int hash = 0;
    for ( const char* k = key; *k; ++k ) {
      hash += *k;
      hash += ( hash << 10 );
      hash ^= ( hash >> 6 );
    }
    hash += ( hash << 3 );
    hash ^= ( hash >> 11 );
    hash += ( hash << 15 );
    return hash;
  }

private:
  unsigned long m_id;
  unsigned long m_major_ver;
  unsigned long m_minor_ver;
};

namespace Gaudi {

  template <typename... I>
  struct interface_list {};

  namespace meta {
    // identity T -> type = T; note that id_<T> is a complete type, even if T is not
    template <typename T>
    struct id_ {
      using type = T;
    };

    namespace detail {
      template <typename... Is>
      struct inherit_from : Is... {};

      template <typename List, typename I>
      struct append1 {};

      // interpose an id_<I> as id_<I> is a complete type, even if I is not... and we need complete
      // types to inherit from
      template <typename... Is, typename I>
      struct append1<interface_list<Is...>, I>
          : id_<typename std::conditional<std::is_base_of<id_<I>, inherit_from<id_<Is>...>>::value,
                                          interface_list<Is...>, interface_list<Is..., I>>::type> {};

      template <typename, typename>
      struct appendN {};

      template <typename State>
      struct appendN<interface_list<>, State> : id_<State> {};

      template <typename... Is, typename I, typename List>
      struct appendN<interface_list<I, Is...>, List> : appendN<interface_list<Is...>, typename append1<List, I>::type> {
      };
    } // namespace detail

    template <typename... Is>
    using unique_interface_list = detail::appendN<interface_list<Is...>, interface_list<>>;
  } // namespace meta

  // interface_list concatenation
  template <typename... I>
  struct interface_list_cat;

  // identity
  template <typename... I>
  struct interface_list_cat<interface_list<I...>> : meta::unique_interface_list<I...> {};

  // binary op
  template <typename... I1, typename... I2>
  struct interface_list_cat<interface_list<I1...>, interface_list<I2...>> : meta::unique_interface_list<I1..., I2...> {
  };

  // induction of binary op
  template <typename... I1, typename... I2, typename... Others>
  struct interface_list_cat<interface_list<I1...>, interface_list<I2...>, Others...>
      : interface_list_cat<interface_list<I1..., I2...>, Others...> {};

  // append is a special case of concatenation...
  template <typename... I>
  struct interface_list_append;

  template <typename... Is, typename I>
  struct interface_list_append<interface_list<Is...>, I>
      : interface_list_cat<interface_list<Is...>, interface_list<I>> {};

  // helpers for implementation of interface cast
  namespace iid_cast_details {
    template <typename I>
    constexpr void* void_cast( const I* i ) {
      return const_cast<I*>( i );
    }

    template <typename... Is>
    struct iid_cast_t;

    template <>
    struct iid_cast_t<> {
      template <typename P>
      constexpr void* operator()( const InterfaceID&, P* ) const {
        return nullptr;
      }
    };

    template <typename I, typename... Is>
    struct iid_cast_t<I, Is...> {
      template <typename P>
      inline void* operator()( const InterfaceID& tid, P* ptr ) const {
        return tid.versionMatch( I::interfaceID() ) ? void_cast<typename I::interface_type>( ptr )
                                                    : iid_cast_t<Is...>{}( tid, ptr );
      }
    };
  } // namespace iid_cast_details

  template <typename... Is>
  std::vector<std::string> getInterfaceNames( Gaudi::interface_list<Is...> ) {
    return {Is::name()...};
  }

  template <typename... Is, typename P>
  inline void* iid_cast( const InterfaceID& tid, Gaudi::interface_list<Is...>, P* ptr ) {
    constexpr auto iid_cast_ = iid_cast_details::iid_cast_t<Is...>{};
    return iid_cast_( tid, ptr );
  }

  /// Class to handle automatically the versioning of the interfaces when they
  /// are inheriting from other interfaces.
  /// @author Marco Clemencic
  template <typename INTERFACE, unsigned long majVers, unsigned long minVers>
  struct InterfaceId final {
    /// interface type
    using interface_type = INTERFACE;
    /// List of interfaces
    using iids = typename Gaudi::interface_list_append<typename interface_type::ext_iids, InterfaceId>::type;

    static inline std::string      name() { return System::typeinfoName( typeid( INTERFACE ) ); }
    static constexpr unsigned long majorVersion() { return majVers; }
    static constexpr unsigned long minorVersion() { return minVers; }

    static inline const std::type_info& TypeInfo() { return typeid( typename iids::type ); }

    static inline const InterfaceID& interfaceID() {
      static const InterfaceID s_iid( name().c_str(), majVers, minVers );
      return s_iid;
    }
  };

  constexpr struct fullMatch_t {
    template <typename IFACE1, typename IFACE2, unsigned long major1, unsigned long major2, unsigned long minor1,
              unsigned long minor2>
    constexpr bool operator()( InterfaceId<IFACE1, major1, minor1>, InterfaceId<IFACE2, major2, minor2> ) const {
      return false;
    }
    template <typename IFACE, unsigned long major, unsigned long minor>
    constexpr bool operator()( InterfaceId<IFACE, major, minor>, InterfaceId<IFACE, major, minor> ) const {
      return true;
    }
  } fullMatch{};

  constexpr struct majorMatch_t {
    template <typename IFACE1, typename IFACE2, unsigned long major1, unsigned long major2, unsigned long minor1,
              unsigned long minor2>
    constexpr bool operator()( InterfaceId<IFACE1, major1, minor1>, InterfaceId<IFACE2, major2, minor2> ) const {
      return false;
    }
    template <typename IFACE, unsigned long major, unsigned long minor1, unsigned long minor2>
    constexpr bool operator()( InterfaceId<IFACE, major, minor1>, InterfaceId<IFACE, major, minor2> ) const {
      return true;
    }
  } majorMatch{};
} // namespace Gaudi

/** @class IInterface IInterface.h GaudiKernel/IInterface.h
    Definition of the basic interface

    @author Pere Mato
    @author Markus Frank
    @author Marco Clemencic
*/
class GAUDI_API IInterface {
public:
  /// Interface ID
  using iid = Gaudi::InterfaceId<IInterface, 0, 0>;

  /// Extra interfaces
  using ext_iids = Gaudi::interface_list<iid>;

  /// Return an instance of InterfaceID identifying the interface.
  static inline const InterfaceID& interfaceID() { return iid::interfaceID(); }

  /// main cast function
  virtual void* i_cast( const InterfaceID& ) const
#if defined( GAUDI_V20_COMPAT ) && !defined( G21_NEW_INTERFACES )
  {
    return nullptr;
  }
#else
      = 0;
#endif

  /// Returns a vector of strings containing the names of all the implemented interfaces.
  virtual std::vector<std::string> getInterfaceNames() const
#if defined( GAUDI_V20_COMPAT ) && !defined( G21_NEW_INTERFACES )
  {
    return {};
  }
#else
      = 0;
#endif

  /// Increment the reference count of Interface instance
  virtual unsigned long addRef() = 0;

  /// Release Interface instance
  virtual unsigned long release() = 0;

  /// Current reference count
  virtual unsigned long refCount() const
#if defined( GAUDI_V20_COMPAT ) && !defined( G21_NEW_INTERFACES )
  {
    IInterface* ths = const_cast<IInterface*>( this );
    ths->addRef();
    return ths->release();
  } // new method, so we need a default implementation for v20 compatibility
#else
      = 0;
#endif

  /// Set the void** to the pointer to the requested interface of the instance.
  virtual StatusCode queryInterface( const InterfaceID& ti, void** pp ) = 0;

  /// Return status
  enum class Status : StatusCode::code_t {
    /// Failure
    FAILURE = 0,
    /// Normal successful completion
    SUCCESS = 1,
    /// Requested interface is not available
    NO_INTERFACE,
    /// Requested interface version is incompatible
    VERSMISMATCH,
    /// Last error
    LAST_ERROR
  };

  /// Virtual destructor
  virtual ~IInterface() = default;
};

STATUSCODE_ENUM_DECL( IInterface::Status )

namespace Gaudi {
  /// Cast a IInterface pointer to an IInterface specialization (TARGET).
  template <typename TARGET>
  TARGET* Cast( IInterface* i ) {
    return reinterpret_cast<TARGET*>( i->i_cast( TARGET::interfaceID() ) );
  }
  /// Cast a IInterface pointer to an IInterface specialization (TARGET).
  /// const version
  template <typename TARGET>
  const TARGET* Cast( const IInterface* i ) {
    return reinterpret_cast<const TARGET*>( i->i_cast( TARGET::interfaceID() ) );
  }
} // namespace Gaudi

/** Templated function that throws an exception if the version if the interface
    implemented by the object behind the interface is incompatible. This is the
    case if the major version differs or that the minor version of the client is
    bigger than of the implementer.

    @note A call to this function implies an increment of the reference count in
    case of success.

    @author Pere Mato
*/
template <class I>
bool isValidInterface( I* i ) {
  void* ii = nullptr;
  ;
  StatusCode sc = i->queryInterface( I::interfaceID(), &ii );
  return sc.isSuccess();
}

//#ifdef GAUDI_V20_COMPAT
/// ostream operator for InterfaceID. Needed by PluginSvc
inline std::ostream& operator<<( std::ostream& s, const InterfaceID& id ) {
  s << "IID_" << id.id();
  return s;
}
//#endif

/// Small function to be used instead of the construct (void**)&pointer, which
/// produces, on gcc 4.1 optimized, the warning
/// <code>warning: dereferencing type-punned pointer will break strict-aliasing rules</code>
/// The assempler code produced is equivalent to the one with the direct cast.
/// The function can be used als to replace the construct (IInterface *&)m_MySvc.
/// Replacement rules:
/// <table>
/// <tr><td><tt>(void**)&pointer</tt></td><td><tt>pp_cast<void>(&pointer)</tt></td></tr>
/// <tr><td><tt>(IInterface *&)m_mySvc</tt></td><td><tt>*pp_cast<IInterface>(&m_mySvc)</tt></td></tr>
/// </table>
template <class DEST, class SRC>
inline DEST** pp_cast( SRC** ptr ) {
  return reinterpret_cast<DEST**>( ptr );
}

#include "GaudiKernel/extend_interfaces.h"
#include "GaudiKernel/extends.h"
#include "GaudiKernel/implements.h"

#endif // GAUDIKERNEL_IINTERFACE_H
