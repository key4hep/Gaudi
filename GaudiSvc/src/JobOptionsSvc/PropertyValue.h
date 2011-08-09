// $Id:$
#ifndef JOBOPTIONSVC_PROPERTY_VALUE_H_
#define JOBOPTIONSVC_PROPERTY_VALUE_H_
// ============================================================================
// Includes:
// ============================================================================
// STD & STL:
// ============================================================================
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
// ============================================================================
#include <boost/variant.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
// ============================================================================
#include "Position.h"
// ============================================================================
namespace Gaudi { namespace Parsers {
// ============================================================================
class PropertyValue {
// ----------------------------------------------------------------------------
 public:
  typedef boost::shared_ptr<PropertyValue> SharedPtr;
  typedef boost::shared_ptr<const PropertyValue> ConstSharedPtr;
  typedef boost::scoped_ptr<PropertyValue> ScopedPtr;
  typedef boost::scoped_ptr<const PropertyValue> ConstScopedPtr;

  typedef boost::variant<std::string, std::vector<std::string>,
           std::map<std::string, std::string> > Value;
  typedef std::vector<std::string> VectorOfStrings;
  typedef std::map<std::string, std::string> MapOfStrings;

// ----------------------------------------------------------------------------
  explicit PropertyValue(const Value& value,
      bool is_reference=false): value_(value), is_reference_(is_reference) {}
  PropertyValue(const Value& value, const Position& position,
      bool is_reference=false): value_(value), position_(position),
      is_reference_(is_reference) {}
// ----------------------------------------------------------------------------
  const Position& position() const { return position_;}
// ----------------------------------------------------------------------------
  std::string& String() { return boost::get<std::string>(value_);}
  const std::string& String() const { return boost::get<std::string>(value_);}

  VectorOfStrings& Vector() { return boost::get<VectorOfStrings>(value_);}
  const VectorOfStrings& Vector() const {
    return boost::get<VectorOfStrings>(value_);}

  MapOfStrings& Map() { return boost::get<MapOfStrings>(value_);}
  const MapOfStrings& Map() const { return boost::get<MapOfStrings>(value_);}
// ----------------------------------------------------------------------------
  std::string ToString() const;
  bool HasPosition() const {return position_.Exists();}
  bool IsSimple() const;
  bool IsVector() const;
  bool IsMap() const;
  bool IsReference() const { return is_reference_;};
  // ----------------------------------------------------------------------------
  // Operators:
  // ----------------------------------------------------------------------------

  PropertyValue& operator += (const PropertyValue& right);
  const PropertyValue operator + (const PropertyValue& right);
  PropertyValue& operator -= (const PropertyValue& right);
  const PropertyValue operator - (const PropertyValue& right);
  //bool operator == (const PropertyValue& right) const;
 private:
  Value value_;
  Position position_;
  bool is_reference_;
// ----------------------------------------------------------------------------
};  //  class PropertyValue
// ============================================================================
class PropertyValueException : public std::runtime_error {
 public:
  PropertyValueException(const std::string& message):
    std::runtime_error(message){}
  static PropertyValueException WrongLValue() {
    return PropertyValueException("Cannot apply +=/-= operation to left value.");
  }

  static PropertyValueException WrongRValue() {
    return PropertyValueException("Cannot apply +=/-= operation to right value.");
  }

};

class PositionalPropertyValueException : public std::runtime_error {
 public:
  PositionalPropertyValueException(const Position& position,
      const std::string& message): std::runtime_error(message),
      position_(position){}
  const Position& position() const { return position_;}

  static PositionalPropertyValueException CouldNotFind(const Position& position,
      const std::string& name) {
    return PositionalPropertyValueException(position,
        "Could not find property "+name+".");
  }

  static PositionalPropertyValueException CouldNotFindProperty(
      const Position& position,
        const std::string& name) {
      return PositionalPropertyValueException(position,
          "Could not find property '"+name+"'.");
  }

  static PositionalPropertyValueException CouldNotFindUnit(
        const Position& position,
          const std::string& name) {
        return PositionalPropertyValueException(position,
            "Could not find unit '"+name+"'.");
    }

  virtual ~PositionalPropertyValueException() throw() {}
 private:
  Position position_;
};
// ============================================================================
}  /* Gaudi */ }  /* Parsers */
// ============================================================================
#endif  // JOBOPTIONSVC_PROPERTY_VALUE_H_
