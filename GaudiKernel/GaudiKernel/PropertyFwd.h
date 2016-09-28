/// Forward declarations for Property.h.

#ifndef PROPERTY_FWD_H
#define PROPERTY_FWD_H

namespace Gaudi
{
  namespace Details
  {
    class PropertyBase;
    namespace Property
    {
      class NullVerifier;
      class UpdateHandler;
    }
  }
  template <class TYPE, class VERIFIER, class HANDLERS>
  class Property;
}

/// \fixme backward compatibility hack for old Property base class
using Property
    //[[deprecated("use Gaudi::Details::PropertyBase instead")]]
    = Gaudi::Details::PropertyBase;

/// \fixme backward compatibility hack for PropertyWithValue
template <class TYPE, class VERIFIER = Gaudi::Details::Property::NullVerifier,
          class HANDLERS = Gaudi::Details::Property::UpdateHandler>
using PropertyWithValue
    //[[deprecated("use Gaudi::Property instead")]]
    = Gaudi::Property<TYPE, VERIFIER, HANDLERS>;

#endif
