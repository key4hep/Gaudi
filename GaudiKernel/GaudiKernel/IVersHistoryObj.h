#ifndef GAUDIKERNEL_IVERSHISTORYOBJ_H
#define GAUDIKERNEL_IVERSHISTORYOBJ_H

#include "GaudiKernel/Kernel.h" // GAUDI_API
#include "GaudiKernel/PropertyFwd.h"
#include <string>
#include <vector>

/** @class IVersHistoryObj IVersHistoryObj.h GaudiKernel/IVersHistoryObj.h

    Interface for Versioned History Objects

    @author Charles Leggett
    @date   Feb 1 2011

*/

class GAUDI_API IVersHistoryObj {
public:
  typedef std::vector<Gaudi::Details::PropertyBase*> PropertyList;

public:
  virtual ~IVersHistoryObj() = default;

  virtual const std::string& name() const    = 0;
  virtual const std::string& type() const    = 0;
  virtual const std::string& version() const = 0;

  virtual const PropertyList& properties() const = 0;
};

#endif
