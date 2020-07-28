/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "COPYING".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once
/// Forward declarations for Gaudi/Property.h

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
