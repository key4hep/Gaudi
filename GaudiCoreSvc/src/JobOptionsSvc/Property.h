/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include "Position.h"
#include "PropertyName.h"
#include "PropertyValue.h"
#include <string>

namespace Gaudi {
  namespace Parsers {
    class Property final {
    public:
      struct LessThen {
        bool operator()( const Property& first, const Property& second ) const {
          return first.FullName() < second.FullName();
        }
      };

      Property( PropertyName property_name, PropertyValue property_value )
          : property_name_( std::move( property_name ) ), property_value_( std::move( property_value ) ) {}

      const PropertyName& property_name() const { return property_name_; }
      PropertyValue&      property_value() { return property_value_; }
      // ----------------------------------------------------------------------------
      Property& operator+=( const PropertyValue& value ) {
        property_value_ += value;
        return *this;
      }
      Property& operator-=( const PropertyValue& value ) {
        property_value_ -= value;
        return *this;
      }

      const Position& DefinedPosition() const;
      bool            HasDefinedPosition() const { return DefinedPosition().Exists(); }

      const Position& ValuePosition() const;
      bool            HasValuePosition() const { return ValuePosition().Exists(); }

      const std::string& ClientName() const;
      const std::string& NameInClient() const;
      std::string        FullName() const;
      std::string        ValueAsString() const;
      std::string        ToString() const;
      bool               IsSimple() const;
      bool               IsVector() const;
      bool               IsMap() const;
      bool               IsReference() const { return property_value_.IsReference(); }

    private:
      PropertyName  property_name_;
      PropertyValue property_value_;
    };
  } // namespace Parsers
} // namespace Gaudi
