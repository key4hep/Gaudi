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
#include <vector>
// ============================================================================
#include <boost/variant.hpp>
// ============================================================================
#include "Position.h"
// ============================================================================
namespace Gaudi
{
  namespace Parsers
  {
    // ============================================================================
    class PropertyValue final
    {
      // ----------------------------------------------------------------------------
    public:
      typedef boost::variant<std::string, std::vector<std::string>, std::map<std::string, std::string>> Value;
      typedef std::vector<std::string> VectorOfStrings;
      typedef std::map<std::string, std::string> MapOfStrings;

      // ----------------------------------------------------------------------------
      explicit PropertyValue( Value value, bool is_reference = false )
          : value_( std::move( value ) ), is_reference_( is_reference )
      {
      }
      PropertyValue( Value value, const Position& position, bool is_reference = false )
          : value_( std::move( value ) ), position_( position ), is_reference_( is_reference )
      {
      }
      // ----------------------------------------------------------------------------
      const Position& position() const { return position_; }
      // ----------------------------------------------------------------------------
      std::string&       String() { return boost::get<std::string>( value_ ); }
      const std::string& String() const { return boost::get<std::string>( value_ ); }

      VectorOfStrings&       Vector() { return boost::get<VectorOfStrings>( value_ ); }
      const VectorOfStrings& Vector() const { return boost::get<VectorOfStrings>( value_ ); }

      MapOfStrings&       Map() { return boost::get<MapOfStrings>( value_ ); }
      const MapOfStrings& Map() const { return boost::get<MapOfStrings>( value_ ); }
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

      PropertyValue& operator+=( const PropertyValue& right );
      const PropertyValue operator+( const PropertyValue& right );
      PropertyValue& operator-=( const PropertyValue& right );
      const PropertyValue operator-( const PropertyValue& right );
      // bool operator == (const PropertyValue& right) const;
    private:
      Value    value_;
      Position position_;
      bool     is_reference_;
      // ----------------------------------------------------------------------------
    }; //  class PropertyValue
    // ============================================================================
    class PropertyValueException : public std::runtime_error
    {
    public:
      PropertyValueException( const std::string& message ) : std::runtime_error( message ) {}
      static PropertyValueException              WrongLValue()
      {
        return PropertyValueException( "Cannot apply +=/-= operation to left value." );
      }

      static PropertyValueException WrongRValue()
      {
        return PropertyValueException( "Cannot apply +=/-= operation to right value." );
      }
    };

    class PositionalPropertyValueException : public std::runtime_error
    {
    public:
      PositionalPropertyValueException( const Position& position, const std::string& message )
          : std::runtime_error( message ), position_( position )
      {
      }
      const Position& position() const { return position_; }

      static PositionalPropertyValueException CouldNotFind( const Position& position, const std::string& name )
      {
        return PositionalPropertyValueException( position, "Could not find property " + name + "." );
      }

      static PositionalPropertyValueException CouldNotFindProperty( const Position& position, const std::string& name )
      {
        return PositionalPropertyValueException( position, "Could not find property '" + name + "'." );
      }

      static PositionalPropertyValueException CouldNotFindUnit( const Position& position, const std::string& name )
      {
        return PositionalPropertyValueException( position, "Could not find unit '" + name + "'." );
      }

      virtual ~PositionalPropertyValueException() throw() {}

    private:
      Position position_;
    };
    // ============================================================================
  } /* Gaudi */
} /* Parsers */
// ============================================================================
#endif // JOBOPTIONSVC_PROPERTY_VALUE_H_
