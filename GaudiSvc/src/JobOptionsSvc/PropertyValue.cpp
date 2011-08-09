// $Id:$
// ============================================================================
// Include files
// ============================================================================
#include "PropertyValue.h"
// ============================================================================
// STD & STL:
// ============================================================================
// ============================================================================
// Boost:
// ============================================================================
#include <boost/format.hpp>
#include <boost/foreach.hpp>
// ============================================================================
namespace gp = Gaudi::Parsers;
// ============================================================================
bool gp::PropertyValue::IsSimple() const {
  return boost::get<std::string>(&value_) != NULL;
}
// ============================================================================
bool gp::PropertyValue::IsVector() const {
  return boost::get<std::vector<std::string> >(&value_) != NULL;
}
// ============================================================================
bool gp::PropertyValue::IsMap() const {
  return boost::get<std::map<std::string, std::string> >(&value_) != NULL;
}
// ============================================================================
gp::PropertyValue&
gp::PropertyValue::operator += (const PropertyValue& right) {

  if (IsSimple() || IsReference()) {
    throw PropertyValueException::WrongLValue();
  }

  if (IsVector()) {
    if (right.IsSimple()) {
      boost::get<VectorOfStrings>(value_).push_back(
          boost::get<std::string>(right.value_));
      return *this;
    }
    if (right.IsVector()){
      VectorOfStrings& vec = boost::get<VectorOfStrings>(value_);
      BOOST_FOREACH(const std::string& item,
          boost::get<VectorOfStrings>(right.value_)) {
        vec.push_back(item);
      }
      return *this;
    }
    throw PropertyValueException::WrongRValue();
  }

  if (IsMap()) {
    if (!right.IsMap()) {
      throw PropertyValueException::WrongRValue();
    }
    MapOfStrings& map  = boost::get<MapOfStrings>(value_);
    const MapOfStrings& rmap = boost::get<MapOfStrings>(right.value_);
    BOOST_FOREACH(const MapOfStrings::value_type& item, rmap) {
      map.insert(item);
    }
    return *this;
  }
  return *this;
}

const gp::PropertyValue
gp::PropertyValue::operator+(const PropertyValue& right) {
  return PropertyValue(*this) += right;

}

gp::PropertyValue&
gp::PropertyValue::operator-=(const PropertyValue& right) {
  if (IsSimple() || IsReference()) {
    throw PropertyValueException::WrongLValue();
  }

  if (IsVector()) {
    VectorOfStrings& vec = Vector();
    if (right.IsSimple()) {
      vec.erase(std::find(vec.begin(), vec.end(), right.String()));
      return *this;
    }

    if (right.IsVector()) {
      const VectorOfStrings& rvec = right.Vector();
      BOOST_FOREACH(const std::string& item, rvec) {
        vec.erase(std::find(vec.begin(), vec.end(), item));
      }
      return *this;
    }
    throw PropertyValueException::WrongRValue();
  }

  if (IsMap()) {
    MapOfStrings& map  = Map();
    if (right.IsSimple()) {
      map.erase(right.String());
      return *this;
    }

    if (right.IsVector()) {
      const VectorOfStrings& rvec = right.Vector();
      BOOST_FOREACH(const std::string& item, rvec) {
        map.erase(item);
      }
      return *this;
    }
    throw PropertyValueException::WrongRValue();
  }
  throw PropertyValueException::WrongLValue();
}

const gp::PropertyValue
gp::PropertyValue::operator-(const PropertyValue& right) {
  return PropertyValue(*this) -= right;
}
// ============================================================================
std::string gp::PropertyValue::ToString() const {
  if  (IsReference()) {
    const std::vector<std::string>*
       value = boost::get<std::vector<std::string> >(&value_);
    assert(value != NULL);
    if (value->at(0) != "") {
      return "@"+value->at(0)+"."+value->at(1);
    } else {
      return "@"+value->at(0);
    }
  }
  if (const std::string* value = boost::get<std::string>(&value_)) {
    return *value;
  } else if (const std::vector<std::string>*
      value = boost::get<std::vector<std::string> >(&value_)) {
    std::string result = "[";
    std::string delim = "";
    BOOST_FOREACH(const std::string& in, *value) {
      result += delim + in;
      delim = ", ";
    }
    return result+"]";
  } else if (const std::map<std::string, std::string>*
      value = boost::get<std::map<std::string, std::string> >(&value_)) {
    std::string result = "{";
    std::string delim = "";
    typedef std::pair<std::string, std::string> pair_t;
    BOOST_FOREACH(const pair_t& in, *value) {
      result += delim + in.first + ":" + in.second;
      delim = ", ";
    }
    return result+"}";
  }
  assert(false);
  // @todo Check the validity of this logic
  return std::string(); // avoid compilation warning
}

