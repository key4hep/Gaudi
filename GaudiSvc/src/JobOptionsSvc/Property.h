// $Id:$
#ifndef JOBOPTIONSVC_PROPERTY_H_
#define JOBOPTIONSVC_PROPERTY_H_
// ============================================================================
// Includes:
// ============================================================================
// STD & STL:
// ============================================================================
#include <string>
// ============================================================================
// Boost:
// ============================================================================
#include <boost/shared_ptr.hpp>
// ============================================================================
#include "Position.h"
#include "PropertyName.h"
#include "PropertyValue.h"
// ============================================================================
namespace Gaudi { namespace Parsers {
// ============================================================================
class Property {
// ----------------------------------------------------------------------------
 public:
// ----------------------------------------------------------------------------
	struct LessThen : std::binary_function<Property, Property, bool> {
		bool operator()(const Property& first, const Property& second) const {
			return first.FullName() < second.FullName();
		}
	};

 class Equal : std::unary_function<Property, bool>  {
  public:
    Equal(const std::string& short_name): short_name_(short_name){}
    bool operator()(const Property& property) const {
        return short_name_ == property.NameInClient();
    }
  private:
    std::string short_name_;
 };
// ----------------------------------------------------------------------------
  Property(const PropertyName& property_name,
      const PropertyValue& property_value):
    property_name_(property_name), property_value_(property_value) {}
// ----------------------------------------------------------------------------
   const PropertyName& property_name()  const {
      return property_name_;
  }
  PropertyValue& property_value() {
      return property_value_;
  }
// ----------------------------------------------------------------------------
  Property& operator += (const PropertyValue& value){
    property_value_ += value;
    return *this;
  }
  Property& operator -= (const PropertyValue& value) {
    property_value_ -= value;
    return *this;
  }
// ----------------------------------------------------------------------------
  const Position& DefinedPosition() const;
  bool HasDefinedPosition() const { return DefinedPosition().Exists();}
// ----------------------------------------------------------------------------
  const Position& ValuePosition() const;
  bool HasValuePosition() const { return ValuePosition().Exists();}
// ----------------------------------------------------------------------------
  std::string ClientName() const;
  std::string NameInClient() const;
  std::string FullName() const;
  std::string ValueAsString() const;
  std::string ToString() const;
  bool IsSimple() const;
  bool IsVector() const;
  bool IsMap() const;
  bool IsReference() const { return  property_value_.IsReference();}
// ----------------------------------------------------------------------------
 private:
// ----------------------------------------------------------------------------
  PropertyName property_name_;
  PropertyValue property_value_;
};
// ============================================================================
}  /* Gaudi */ }  /* Parsers */
// ============================================================================
#endif  // JOBOPTIONSVC_PROPERTY_H_
