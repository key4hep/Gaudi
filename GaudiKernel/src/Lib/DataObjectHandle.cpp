#include "GaudiKernel/DataObjectHandle.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>
#include <csignal>
#include <algorithm>

//const char ADDR_SEP = '&';
//const char FIELD_SEP = '|';
//const char ITEM_SEP = '#';

// const std::string DataObjectHandle::NULL_ = "NULL";

// std::ostream& operator<<( std::ostream&   stream ,
//                           const DataObjectHandle& di   )
// { return stream << di.toString() ; }

// const std::string DataObjectHandle::toString() const {
//   std::stringstream str;

//   str << m_key << FIELD_SEP;
//   str << boost::lexical_cast<std::string>(m_mode);

//   return str.str();
// }

// const std::string DataObjectHandle::pythonRepr() const {
//   return "DataObjectHandle(\"" + toString() + "\")";
// }

// void DataObjectHandle::fromString(const std::string& s) {

//   if(s == "") //nothing to do
//     return;

//   std::vector<std::string> items;

//   boost::split(items, s, boost::is_any_of(boost::lexical_cast<std::string>(FIELD_SEP)), boost::token_compress_on);

//   setKey(items[0]);

//   setMode(Gaudi::DataHandle::Mode(boost::lexical_cast<int>( items[1] )));

// }







