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

#include <Gaudi/Concepts.h>
#include <GaudiKernel/Kernel.h>
#include <GaudiKernel/StatusCode.h>
#include <GaudiKernel/System.h>
#include <ostream>
#include <type_traits>
#include <typeinfo>

/// Macro to declare the interface ID when using the new mechanism of extending and implementing interfaces.
#define DeclareInterfaceID( iface, major, minor )                                                                      \
  static const InterfaceID& interfaceID() { return iid::interfaceID(); }                                               \
  using iid      = Gaudi::InterfaceId<iface, major, minor>;                                                            \
  using ext_iids = typename iid::iids

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
  /// constructor from components
  constexpr InterfaceID( unsigned long id, unsigned long major, unsigned long minor = 0 )
      : m_id( id ), m_major_ver( major ), m_minor_ver( minor ) {}
  /// constructor from components
  InterfaceID( const char* name, unsigned long major, unsigned long minor = 0 )
      : m_id( hash32( name ) ), m_major_ver( major ), m_minor_ver( minor ) {}
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

  /// ostream operator for InterfaceID. Needed by PluginSvc
  friend std::ostream& operator<<( std::ostream& s, const InterfaceID& id ) {
    s << "IID_" << id.id();
    return s;
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
          : id_<std::conditional_t<std::is_base_of_v<id_<I>, inherit_from<id_<Is>...>>, interface_list<Is...>,
                                   interface_list<Is..., I>>> {};

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

  template <typename... Is>
  std::vector<std::string> getInterfaceNames( Gaudi::interface_list<Is...> ) {
    return { Is::name()... };
  }

  template <typename... Is, typename P>
  void* iid_cast( const InterfaceID& tid, Gaudi::interface_list<Is...>, P* ptr ) {
    const void* target = nullptr;
    ( ( tid.versionMatch( Is::interfaceID() ) &&
        ( target = static_cast<typename Is::interface_type const*>( ptr ), true ) ) ||
      ... );
    return const_cast<void*>( target );
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

  template <Gaudi::IsInterface TARGET>
  TARGET* cast() {
    if ( auto output = i_cast( TARGET::interfaceID() ) ) { return reinterpret_cast<TARGET*>( output ); }
    if constexpr ( Gaudi::IsInterface<TARGET> ) {
      void* tgt = nullptr;
      queryInterface( TARGET::interfaceID(), &tgt ).ignore();
      if ( tgt ) {
        // queryInterface bumps the reference count of the target object, but we should not
        auto* target = reinterpret_cast<TARGET*>( tgt );
        // we cannot use release() because we may be called with an object with initial reference count of 0 and that
        // would delete the object
        static_cast<const IInterface*>( target )->decRef();
        return target;
      }
    }
    return nullptr;
  }

  template <Gaudi::IsInterface TARGET>
  TARGET const* cast() const {
    if ( auto output = i_cast( TARGET::interfaceID() ) ) { return reinterpret_cast<TARGET const*>( output ); }
    if constexpr ( Gaudi::IsInterface<TARGET> ) {
      void* tgt = nullptr;
      const_cast<IInterface*>( this )->queryInterface( TARGET::interfaceID(), &tgt ).ignore();
      if ( tgt ) {
        // queryInterface bumps the reference count of the target object, but we should not
        auto* target = reinterpret_cast<const TARGET*>( tgt );
        // we cannot use release() because we may be called with an object with initial reference count of 0 and that
        // would delete the object
        static_cast<const IInterface*>( target )->decRef();
        return target;
      }
    }
    return nullptr;
  }

  template <typename TARGET>
    requires( !Gaudi::IsInterface<TARGET> )
  TARGET* cast() {
    return dynamic_cast<TARGET*>( this );
  }

  template <typename TARGET>
    requires( !Gaudi::IsInterface<TARGET> )
  TARGET const* cast() const {
    return dynamic_cast<TARGET const*>( this );
  }

  /// Returns a vector of strings containing the names of all the implemented interfaces.
  virtual std::vector<std::string> getInterfaceNames() const = 0;

  /// Increment the reference count of Interface instance
  virtual unsigned long addRef() const = 0;

  /// Release Interface instance
  virtual unsigned long release() const = 0;

  /// Current reference count
  virtual unsigned long refCount() const = 0;

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

  // Note: these cannot be protected methods because they may be needed by specializations
  // that delegate to other instances (like queryInterface does).
  virtual void const* i_cast( const InterfaceID& ) const = 0;

  void* i_cast( const InterfaceID& iid ) {
    return const_cast<void*>( const_cast<const IInterface*>( this )->i_cast( iid ) );
  }

protected:
  /// Decrement reference count and return the new reference count
  virtual unsigned long decRef() const = 0;
};

STATUSCODE_ENUM_DECL( IInterface::Status )

namespace Gaudi {
  template <typename TARGET>
  TARGET* Cast( IInterface* i ) {
    return i ? i->cast<TARGET>() : nullptr;
  }

  template <typename TARGET>
  const TARGET* Cast( const IInterface* i ) {
    return i ? i->cast<const TARGET>() : nullptr;
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
template <class IFace>
bool isValidInterface( IFace* i ) {
  void* ii = nullptr;
  return i->queryInterface( IFace::interfaceID(), &ii ).isSuccess();
}

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

#include <GaudiKernel/extend_interfaces.h>
#include <GaudiKernel/extends.h>
#include <GaudiKernel/implements.h>
