/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
/// Forward declarations for Property.h.

#ifndef PROPERTY_FWD_H
#define PROPERTY_FWD_H

/// macro to help implementing backward compatible changed
/// in derived projects
#define GAUDI_PROPERTY_v2

namespace Gaudi {
  namespace Details {
    class PropertyBase;
    namespace Property {
      struct NullVerifier;
      struct UpdateHandler;
    } // namespace Property
  }   // namespace Details
  template <class TYPE, class VERIFIER, class HANDLERS>
  class Property;
} // namespace Gaudi

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
