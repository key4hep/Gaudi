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

#include <Gaudi/Examples/TestVector.h>
#include <GaudiKernel/DataObject.h>
#include <memory>
#include <rust/cxx.h>

namespace Gaudi::Examples::Rust {
  struct Point;
}

std::unique_ptr<DataObject> wrap_point( rust::Box<Gaudi::Examples::Rust::Point> point );

rust::Box<Gaudi::Examples::Rust::Point> const& unwrap_point( DataObject const& obj );

Gaudi::Examples::TestVector const& cast_to_testvector( DataObject const& value );
