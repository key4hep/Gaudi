// $Id:$
// Copyright 2011 alexander.mazurov@gmail.com
// ============================================================================
// Include files
// ============================================================================
#include "Property.h"

#include "PropertyName.h"
#include "PropertyValue.h"
#include "Utils.h"
// ============================================================================
// STD & STL:
// ============================================================================
// ============================================================================
// Boost:
// ============================================================================
#include <boost/format.hpp>
// ============================================================================
// Namespace aliases:
namespace gp = Gaudi::Parsers;
namespace gpu = Gaudi::Parsers::Utils;
// ============================================================================
const gp::Position& gp::Property::DefinedPosition() const {
    return property_name_.position();
}
// ============================================================================
const gp::Position& gp::Property::ValuePosition() const {
    return property_value_.position();
}
// ============================================================================
std::string gp::Property::ClientName() const {
    return property_name_.client();
}
// ============================================================================
std::string gp::Property::NameInClient() const {
    return property_name_.property();
}
// ============================================================================
std::string gp::Property::FullName() const {
    return property_name_.ToString();
}
// ============================================================================
std::string gp::Property::ValueAsString() const {
    return gpu::replaceEnvironments(property_value_.ToString());
}
// ============================================================================
std::string gp::Property::ToString() const {
    return FullName()+" = "+ValueAsString();
}
// ============================================================================
bool gp::Property::IsSimple() const {
    return property_value_.IsSimple();
}
// ============================================================================
bool gp::Property::IsVector() const {
    return property_value_.IsVector();
}
// ============================================================================
bool gp::Property::IsMap() const {
    return property_value_.IsMap();
}
// ============================================================================
