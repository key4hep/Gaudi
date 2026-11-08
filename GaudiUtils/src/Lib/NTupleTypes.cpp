/***********************************************************************************\
* (c) Copyright 2026 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/details/NTupleTypes.h>
#include <TClass.h>
#include <TClassEdit.h>
#include <TDataType.h>
#include <TROOT.h>
#include <TVirtualCollectionProxy.h>
#include <format>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace {
  std::optional<std::pair<std::type_index, std::string_view>> fundamentalType( EDataType code ) {
    switch ( code ) {
    case kChar_t:
      return { { typeid( char ), "char" } };
    case kUChar_t:
      return { { typeid( unsigned char ), "unsigned char" } };
    case kShort_t:
      return { { typeid( short ), "short" } };
    case kUShort_t:
      return { { typeid( unsigned short ), "unsigned short" } };
    case kInt_t:
      return { { typeid( int ), "int" } };
    case kUInt_t:
      return { { typeid( unsigned int ), "unsigned int" } };
    case kLong_t:
      return { { typeid( long ), "long" } };
    case kULong_t:
      return { { typeid( unsigned long ), "unsigned long" } };
    case kLong64_t:
      return { { typeid( long long ), "long long" } };
    case kULong64_t:
      return { { typeid( unsigned long long ), "unsigned long long" } };
    case kFloat_t:
      return { { typeid( float ), "float" } };
    case kDouble_t:
      return { { typeid( double ), "double" } };
    case kBool_t:
      return { { typeid( bool ), "bool" } };
    default:
      return std::nullopt;
    }
  }
} // namespace

namespace Gaudi::details {

  std::type_index typeIndexOf( const std::string& typeName ) {
    if ( auto* cls = TClass::GetClass( typeName.c_str() ) ) {
      if ( auto* info = cls->GetTypeInfo() ) return *info;
      throw std::invalid_argument( std::format( "No compiled dictionary for {}.", typeName ) );
    }
    if ( auto* dataType = gROOT->GetType( typeName.c_str() ) ) {
      if ( auto fundamental = fundamentalType( static_cast<EDataType>( dataType->GetType() ) ) ) {
        return fundamental->first;
      }
    }
    throw std::invalid_argument( std::format( "unknown type '{}', provide a dictionary please", typeName ) );
  }

  VectorElement vectorElementOf( const std::string& vectorType ) {
    auto* cls   = TClass::GetClass( vectorType.c_str() );
    auto* proxy = cls ? cls->GetCollectionProxy() : nullptr;
    if ( !proxy || proxy->GetCollectionType() != ROOT::kSTLvector ) {
      throw std::invalid_argument( std::format( "type '{}' is not a std::vector", vectorType ) );
    }
    if ( TClassEdit::TSplitType split( cls->GetName() ); split.fElements.size() != 3 ) {
      throw std::invalid_argument( std::format( "{}: only the default allocator is supported", vectorType ) );
    }
    if ( proxy->HasPointers() ) {
      throw std::invalid_argument( std::format( "Vectors of pointers are not supported: {}", vectorType ) );
    }
    const auto increment = proxy->GetIncrement();
    if ( auto* valueClass = proxy->GetValueClass() ) return { valueClass->GetName(), increment };
    // std::vector<bool> is packed, there is no element to point at
    auto fundamental = fundamentalType( proxy->GetType() );
    if ( !fundamental || proxy->GetType() == kBool_t ) {
      throw std::invalid_argument( std::format( "cannot write the elements of {} one by one", vectorType ) );
    }
    return { std::string{ fundamental->second }, increment };
  }

} // namespace Gaudi::details
