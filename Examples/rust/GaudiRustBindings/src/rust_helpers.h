/***********************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "COPYING".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <Gaudi/Rust/AlgWrapper.h>
#include <rust/cxx.h>

namespace Gaudi::Rust::helpers {
  inline rust::String getPropertyValueHelper( AlgWrapper const& alg, std::string const& name ) {
    return alg.getPropertyValue( name );
  }

  template <typename T, typename = std::enable_if_t<!std::is_base_of_v<DataObject, T>>>
  struct DataObjectConverter {
    inline std::unique_ptr<DataObject> into_dataobject_up( T&& value ) {
      return std::make_unique<AnyDataWrapper<T>>( std::move( value ) );
    }
    inline T const& from_dataobject( DataObject const& obj ) {
      return static_cast<AnyDataWrapper<T> const&>( obj ).getData();
    }
  };

#define dobj_conversion( name, type )                                                                                  \
  inline std::unique_ptr<DataObject> dobj_from_##name( type value ) {                                                  \
    return DataObjectConverter<type>().into_dataobject_up( std::move( value ) );                                       \
  }                                                                                                                    \
  inline type const& name##_from_dobj( DataObject const& obj ) {                                                       \
    return DataObjectConverter<type>().from_dataobject( obj );                                                         \
  }

  dobj_conversion( i32, std::int32_t );
  dobj_conversion( i64, std::int64_t );
  dobj_conversion( u32, std::uint32_t );
  dobj_conversion( u64, std::uint64_t );
  dobj_conversion( f32, float );
  dobj_conversion( f64, double );
} // namespace Gaudi::Rust::helpers
