// $Id: IInterface.h,v 1.14 2008/01/29 08:28:14 marcocle Exp $
#ifndef GAUDIKERNEL_IINTERFACE_H
#define GAUDIKERNEL_IINTERFACE_H

// Include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/System.h"
#include "GaudiKernel/StatusCode.h"
#include <ostream>

#ifndef __GCCXML__
// Meta Programming Library (MPL) headers
#include <boost/mpl/set.hpp>
#include <boost/mpl/insert.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/for_each.hpp>
namespace mpl = boost::mpl;
#endif

#include <typeinfo>

#ifndef __GCCXML__
/// Macro to declare the interface ID when using the new mechanism of extending and implementing interfaces.
#define DeclareInterfaceID(name, major, minor) \
  static const InterfaceID &interfaceID(){ return iid::interfaceID(); } \
  typedef Gaudi::InterfaceId< name , major , minor > iid; \
  typedef iid::iids::type ext_iids
#else
// GCCXML work-around
#define DeclareInterfaceID(name, major, minor) \
  static const InterfaceID &interfaceID(){ static InterfaceID xx(0UL,0UL,0UL); return xx; }
#endif

#ifndef __GCCXML__
/// Macro to declare the interface ID when using the new mechanism of extending and implementing interfaces.
#define DeclareInterfaceIDMultiBase(name, major, minor) \
  static const InterfaceID &interfaceID(){ return iid::interfaceID(); } \
  typedef Gaudi::InterfaceId< name , major , minor > iid
#else
// GCCXML work-around
#define DeclareInterfaceIDMultiBase(name, major, minor) \
  static const InterfaceID &interfaceID(){ static InterfaceID xx(0UL,0UL,0UL); return xx; }
#endif

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
class GAUDI_API InterfaceID {
public:
#if defined(GAUDI_V20_COMPAT) && !defined(G21_NEW_INTERFACES)
  /// constructor from a pack long
  InterfaceID( unsigned long lid ) : m_id( lid & 0xFFFF ),
                                     m_major_ver( (lid & 0xFF000000)>>24 ),
                                     m_minor_ver( (lid & 0xFF0000)>> 16 ) { }
#endif
  /// constructor from components
  InterfaceID( unsigned long id, unsigned long major, unsigned long minor = 0)
    : m_id( id ), m_major_ver( major ), m_minor_ver( minor ) { }
  /// constructor from components
  InterfaceID( const char* name, unsigned long major, unsigned long minor = 0)
    : m_id( hash32(name) ), m_major_ver( major ), m_minor_ver( minor ) { }
  /// destructor
  ~InterfaceID() { }
#if defined(GAUDI_V20_COMPAT) && !defined(G21_NEW_INTERFACES)
  /// conversion to unsigned long
  operator unsigned long() const {
    return (m_major_ver << 24) + (m_minor_ver << 16) + m_id;
  }
#endif
  /// get the interface identifier
  unsigned long id() const { return m_id; }
  /// get the major version of the interface
  unsigned long majorVersion() const { return m_major_ver; }
  /// get the minor version of the interface
  unsigned long minorVersion() const { return m_minor_ver; }
  /** check compatibility. The major version is check and the minor one
   *  should be bigger or equal
   */
  bool versionMatch( const InterfaceID& iid ) const {
    return ( id() == iid.id() &&
             majorVersion() == iid.majorVersion() &&
             minorVersion() >= iid.minorVersion() );
  }
  /// check full compatibility.
  bool fullMatch( const InterfaceID& iid ) const {
    return ( id() == iid.id() &&
             majorVersion() == iid.majorVersion() &&
             minorVersion() == iid.minorVersion() );
  }
  /// compare operator
  bool operator == (const InterfaceID& iid ) const { return fullMatch(iid); }
  /// one-at-time hash function
  static unsigned int hash32(const char* key) {
    unsigned int hash;
    const char* k;
    for (hash = 0, k = key; *k; k++) {
      hash += *k; hash += (hash << 10); hash ^= (hash >> 6);
    }
    hash += (hash << 3); hash ^= (hash >> 11); hash += (hash << 15);
    return hash;
  }

private:
  unsigned long m_id;
  unsigned long m_major_ver;
  unsigned long m_minor_ver;
};

namespace Gaudi {
  /// Class to handle automatically the versioning of the interfaces when they
  /// are inheriting from other interfaces.
  /// @author Marco Clemencic
  template <typename INTERFACE, unsigned long majVers, unsigned long minVers>
  class InterfaceId {
  public:
    /// Interface type
    typedef INTERFACE iface_type;

#ifndef __GCCXML__
    /// List of interfaces
    typedef mpl::insert<typename iface_type::ext_iids, InterfaceId> iids;
#endif

    static inline std::string name() { return System::typeinfoName(typeid(INTERFACE)); }

    static inline unsigned long majorVersion(){return majVers;}
    static inline unsigned long minorVersion(){return minVers;}

    static inline const std::type_info &TypeInfo() {
#ifndef __GCCXML__
      return typeid(typename iids::type);
#else
      return typeid(INTERFACE); // avoid compilation errors
#endif
    }

    static const InterfaceID& interfaceID()
    {
      static InterfaceID s_iid(name().c_str(),majVers,minVers);
      return s_iid;
    }

  };
}

/** @class IInterface IInterface.h GaudiKernel/IInterface.h
    Definition of the basic interface

    @author Pere Mato
    @author Markus Frank
    @author Marco Clemencic
*/
class GAUDI_API IInterface {
public:
#ifndef __GCCXML__
  /// Interface ID
  typedef Gaudi::InterfaceId<IInterface,0,0> iid;

  /// Extra interfaces
  typedef mpl::set1<iid> ext_iids;
#endif

  /// Return an instance of InterfaceID identifying the interface.
  static inline const InterfaceID &interfaceID(){
#ifndef __GCCXML__
    return iid::interfaceID();
#else
    // GCCXML work-around
    static InterfaceID xx(0UL,0UL,0UL);
    return xx;
#endif
  }

  /// main cast function
  virtual void *i_cast(const InterfaceID &) const
#if defined(GAUDI_V20_COMPAT) && !defined(G21_NEW_INTERFACES)
  {return 0;}
#else
  = 0;
#endif

  /// Returns a vector of strings containing the names of all the implemented interfaces.
  virtual std::vector<std::string> getInterfaceNames() const
#if defined(GAUDI_V20_COMPAT) && !defined(G21_NEW_INTERFACES)
  {return std::vector<std::string>();}
#else
  = 0;
#endif

  /// Increment the reference count of Interface instance
  virtual unsigned long addRef() = 0;

  /// Release Interface instance
  virtual unsigned long release() = 0;

  /// Current reference count
  virtual unsigned long refCount() const
#if defined(GAUDI_V20_COMPAT) && !defined(G21_NEW_INTERFACES)
  { IInterface* ths = const_cast<IInterface*>(this);
    ths->addRef();
    return ths->release(); } // new method, so we need a default implementation for v20 compatibility
#else
  = 0;
#endif

  /// Set the void** to the pointer to the requested interface of the instance.
  virtual StatusCode queryInterface(const InterfaceID &ti, void** pp) = 0;

  /// Return status
  enum Status {
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
  virtual ~IInterface() {}
};

namespace Gaudi {
  /// Cast a IInterface pointer to an IInterface specialization (TARGET).
  template <typename TARGET>
  TARGET* Cast(IInterface *i){
    return reinterpret_cast<TARGET*>(i->i_cast(TARGET::interfaceID()));
  }
  /// Cast a IInterface pointer to an IInterface specialization (TARGET).
  /// const version
  template <typename TARGET>
  const TARGET* Cast(const IInterface *i){
    return reinterpret_cast<const TARGET*>(i->i_cast(TARGET::interfaceID()));
  }
}

/** Templated function that throws an exception if the version if the interface
    implemented by the object behind the interface is incompatible. This is the
    case if the major version differs or that the minor version of the client is
    bigger than of the implementer.

    @note A call to this function implies an increment of the reference count in
    case of success.

    @author Pere Mato
*/
template <class I>
bool isValidInterface( I* i) {
  void* ii;
  StatusCode sc =  i->queryInterface( I::interfaceID(), &ii);
  return sc.isSuccess();
}

//#ifdef GAUDI_V20_COMPAT
/// ostream operator for InterfaceID. Needed by PluginSvc
inline std::ostream& operator << ( std::ostream& s, const InterfaceID& id) {
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
template <class DEST,class SRC>
inline DEST** pp_cast(SRC** ptr){
  return reinterpret_cast<DEST**>(ptr);
}

#include "GaudiKernel/extend_interfaces.h"
#include "GaudiKernel/implements.h"
#include "GaudiKernel/extends.h"

#endif  // GAUDIKERNEL_IINTERFACE_H
