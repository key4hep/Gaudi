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
#ifndef JOBOPTIONSVC_PROPERTY_VALUE_H_
#define JOBOPTIONSVC_PROPERTY_VALUE_H_
// ============================================================================
// Includes:
// ============================================================================
// STD & STL:
// ============================================================================
#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>
// ============================================================================
#include "Position.h"
// ============================================================================
namespace Gaudi {
  namespace Parsers {
    // ============================================================================
    class PropertyValue final {
      // ----------------------------------------------------------------------------
    public:
      typedef std::variant<std::string, std::vector<std::string>, std::map<std::string, std::string>> Value;
      typedef std::vector<std::string>                                                                VectorOfStrings;
      typedef std::map<std::string, std::string>                                                      MapOfStrings;

      // ----------------------------------------------------------------------------
      explicit PropertyValue( Value value, bool is_reference = false )
          : value_( std::move( value ) ), is_reference_( is_reference ) {}
      PropertyValue( Value value, const Position& position, bool is_reference = false )
          : value_( std::move( value ) ), position_( position ), is_reference_( is_reference ) {}
      // ----------------------------------------------------------------------------
      const Position& position() const { return position_; }
      // ----------------------------------------------------------------------------
      std::string&       String() { return std::get<std::string>( value_ ); }
      const std::string& String() const { return std::get<std::string>( value_ ); }

      VectorOfStrings&       Vector() { return std::get<VectorOfStrings>( value_ ); }
      const VectorOfStrings& Vector() const { return std::get<VectorOfStrings>( value_ ); }

      MapOfStrings&       Map() { return std::get<MapOfStrings>( value_ ); }
      const MapOfStrings& Map() const { return std::get<MapOfStrings>( value_ ); }
      // ----------------------------------------------------------------------------
      std::string ToString() const;
      bool        HasPosition() const { return position_.Exists(); }
      bool        IsSimple() const;
      bool        IsVector() const;
      bool        IsMap() const;
      bool        IsReference() const { return is_reference_; };
      // ----------------------------------------------------------------------------
      // Operators:
      // ----------------------------------------------------------------------------

      PropertyValue&      operator+=( const PropertyValue& right );
      const PropertyValue operator+( const PropertyValue& right );
      PropertyValue&      operator-=( const PropertyValue& right );
      const PropertyValue operator-( const PropertyValue& right );
      // bool operator == (const PropertyValue& right) const;
    private:
      Value    value_;
      Position position_;
      bool     is_reference_;
      // ----------------------------------------------------------------------------
    }; //  class PropertyValue
    // ============================================================================
    class PropertyValueException : public std::runtime_error {
    public:
      PropertyValueException( const std::string& message ) : std::runtime_error( message ) {}
      static PropertyValueException WrongLValue() {
        return PropertyValueException( "Cannot apply +=/-= operation to left value." );
      }

      static PropertyValueException WrongRValue() {
        return PropertyValueException( "Cannot apply +=/-= operation to right value." );
      }
    };

    class PositionalPropertyValueException : public std::runtime_error {
    public:
      PositionalPropertyValueException( const Position& position, const std::string& message )
          : std::runtime_error( message ), position_( position ) {}
      const Position& position() const { return position_; }

      static PositionalPropertyValueException CouldNotFind( const Position& position, const std::string& name ) {
        return PositionalPropertyValueException( position, "Could not find property " + name + "." );
      }

      static PositionalPropertyValueException CouldNotFindProperty( const Position&    position,
                                                                    const std::string& name ) {
        return PositionalPropertyValueException( position, "Could not find property '" + name + "'." );
      }

      static PositionalPropertyValueException CouldNotFindUnit( const Position& position, const std::string& name ) {
        return PositionalPropertyValueException( position, "Could not find unit '" + name + "'." );
      }

      virtual ~PositionalPropertyValueException() throw() {}

    private:
      Position position_;
    };
    // ============================================================================
  } // namespace Parsers
} // namespace Gaudi
// ============================================================================
#endif // JOBOPTIONSVC_PROPERTY_VALUE_H_
