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
#include "helpers.h"
#include <GaudiKernel/AnyDataWrapper.h>
#include <memory>
#include <rust/cxx.h>

using Point_t = rust::Box<Gaudi::Examples::Rust::Point>;

std::unique_ptr<DataObject> wrap_point( Point_t point ) {
  return std::make_unique<AnyDataWrapper<Point_t>>( std::move( point ) );
}

Point_t const& unwrap_point( DataObject const& obj ) {
  return static_cast<AnyDataWrapper<Point_t> const&>( obj ).getData();
}
