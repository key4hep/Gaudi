// $Id:$
#ifndef JOBOPTIONSVC_PROPERTY_NAME_H_
#define JOBOPTIONSVC_PROPERTY_NAME_H_
// ============================================================================
// Includes:
// ============================================================================
// STD & STL:
// ============================================================================
#include <string>
// ============================================================================
// Boost:
// ============================================================================
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
// ============================================================================
#include "Position.h"
// ============================================================================
namespace Gaudi { namespace Parsers {
// ============================================================================
class Position;
// ============================================================================
class PropertyName {
 public:
// ----------------------------------------------------------------------------
  typedef boost::shared_ptr<PropertyName> SharedPtr;
  typedef boost::shared_ptr<const PropertyName> ConstSharedPtr;
  typedef boost::scoped_ptr<PropertyName> ScopedPtr;
  typedef boost::scoped_ptr<const PropertyName> ConstScopedPtr;
// ----------------------------------------------------------------------------
  explicit PropertyName(const std::string& property): client_(""),
      property_(property) {}
  PropertyName(const std::string& property, const Position& pos):
       client_(""), property_(property), position_(pos) {}
  PropertyName(const std::string& client, const std::string& property):
       client_(client), property_(property){}
  PropertyName(const std::string& client, const std::string& property,
           const Position& pos):
               client_(client), property_(property), position_(pos) {}
  const std::string& client() const { return client_;}
  const std::string& property() const { return property_;}
  const Position& position() const { return position_;}
  std::string FullName() const;
  std::string ToString() const;
  bool HasClient() const { return client_.length() > 0;}
  bool HasPosition() const { return position_.Exists();}
 private:
  std::string client_;
  std::string property_;
  Position position_;
};
// ============================================================================
}  /* Gaudi */ }  /* Parsers */
// ============================================================================
#endif  // JOBOPTIONSVC_PROPERTY_NAME_H_
