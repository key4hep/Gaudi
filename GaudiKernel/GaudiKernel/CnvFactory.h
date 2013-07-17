#ifndef GAUDIKERNEL_CNVFACTORY_H
#define GAUDIKERNEL_CNVFACTORY_H

#include "GaudiKernel/Kernel.h"
#include <Gaudi/PluginService.h>
#include "GaudiKernel/ClassID.h"
#include <sstream>

class IConverter;
class ISvcLocator;

typedef Gaudi::PluginService::Factory1<IConverter*,
                                       ISvcLocator*> CnvFactory;

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

// Macros to declare component factories
#define DECLARE_CONVERTER_FACTORY(x) \
    DECLARE_FACTORY_WITH_ID(x, ConverterID(x::storageType(), x::classID()), CnvFactory)

// Macros to declare component factories in a C++ namespace
#define DECLARE_NAMESPACE_CONVERTER_FACTORY(n, x) \
using n::x; DECLARE_FACTORY_WITH_ID(x, ConverterID(x::storageType(), x::classID()), CnvFactory)

#endif // GAUDIKERNEL_CNVFACTORY_H
