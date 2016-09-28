#ifndef PROPERTY_FWD_H
#define PROPERTY_FWD_H

namespace Gaudi
{
  namespace Details
  {
    class PropertyBase;
  }
}
/// \fixme backward compatibility hack for PropertyBase
using Property = Gaudi::Details::PropertyBase;

#endif
