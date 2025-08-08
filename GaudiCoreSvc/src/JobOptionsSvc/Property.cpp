/***********************************************************************************\
* (c) Copyright 2011-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "Property.h"
#include "PropertyName.h"
#include "PropertyValue.h"
#include "Utils.h"

namespace gp  = Gaudi::Parsers;
namespace gpu = Gaudi::Parsers::Utils;

const gp::Position& gp::Property::DefinedPosition() const { return property_name_.position(); }
const gp::Position& gp::Property::ValuePosition() const { return property_value_.position(); }
const std::string&  gp::Property::ClientName() const { return property_name_.client(); }
const std::string&  gp::Property::NameInClient() const { return property_name_.property(); }
std::string         gp::Property::FullName() const { return property_name_.ToString(); }
std::string gp::Property::ValueAsString() const { return gpu::replaceEnvironments( property_value_.ToString() ); }
std::string gp::Property::ToString() const { return FullName() + " = " + ValueAsString(); }
bool        gp::Property::IsSimple() const { return property_value_.IsSimple(); }
bool        gp::Property::IsVector() const { return property_value_.IsVector(); }
bool        gp::Property::IsMap() const { return property_value_.IsMap(); }
