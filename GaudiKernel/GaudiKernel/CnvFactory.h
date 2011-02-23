// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/CnvFactory.h,v 1.12 2006/12/06 17:18:03 mato Exp $
#ifndef GAUDIKERNEL_CNVFACTORY_H
#define GAUDIKERNEL_CNVFACTORY_H

#include "GaudiKernel/Kernel.h"
#include "Reflex/PluginService.h"
#include "GaudiKernel/ClassID.h"
#include "RVersion.h"


// The following is needed to be backward compatible with the old factories of Gaudi. Otherwise the components
// having the constructor/destructor protected will not be working

class IConverter;
class ISvcLocator;

template <typename T> class CnvFactory {
public:
  static IConverter* create(ISvcLocator *svcloc) {
    return new T(svcloc );
  }
};

namespace {
  template < typename P > class Factory<P, IConverter*(ISvcLocator*)> {
  public:
#if ROOT_VERSION_CODE < ROOT_VERSION(5,21,6)
    static void* Func( void*, const std::vector<void*>& arg, void*) {
      return CnvFactory<P>::create((ISvcLocator*)(arg[0]));
    }
#else
    static void Func( void *retaddr, void*, const std::vector<void*>& arg, void*) {
      *(IConverter**) retaddr = CnvFactory<P>::create((ISvcLocator*)(arg[0]));
    }
#endif
  };
}

// Identified class for converters

class GAUDI_API ConverterID {
  public:
   ConverterID( long stype, CLID clid ) : m_stype(stype), m_clid(clid) {}
   ~ConverterID() {}
   bool operator ==(const ConverterID& id) const { return m_stype == id.m_stype && m_clid == id.m_clid; }
  private:
   friend std::ostream& operator << ( std::ostream&, const ConverterID&);
   long m_stype;
   CLID m_clid;
};

inline std::ostream& operator << ( std::ostream& s, const ConverterID& id) {
    s << "CNV_" << id.m_stype << "_" << id.m_clid;
    return s;
}

#define DECLARE_GENERIC_CONVERTER(x) /* dummy */
#define DECLARE_NAMESPACE_GENERIC_CONVERTER(n,x) /* dummy */
#define DECLARE_CONVERTER(x) /*dummy*/
#define DECLARE_NAMESPACE_CONVERTER(n,x)   /*dummy */

// Macros to declare component factories
#define DECLARE_CONVERTER_FACTORY(x) \
  PLUGINSVC_FACTORY_WITH_ID(x,ConverterID(x::storageType(),x::classID()),IConverter*(ISvcLocator*))

// Macros to declare component factories in a C++ namespace
#define DECLARE_NAMESPACE_CONVERTER_FACTORY(n,x) \
using n::x; PLUGINSVC_FACTORY_WITH_ID(x,ConverterID(x::storageType(),x::classID()),IConverter*(ISvcLocator*))

#endif // GAUDIKERNEL_CNVFACTORY_H
