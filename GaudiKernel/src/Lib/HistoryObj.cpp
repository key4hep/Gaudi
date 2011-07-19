// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/src/Lib/HistoryObj.cpp,v 1.1 2004/07/14 18:13:58 leggett Exp $
//====================================================================
//      History.cpp
//--------------------------------------------------------------------
//
//  Description: Implementation of HistoryObj base class
//
//  Author     : Charles Leggett
//====================================================================

#define GAUDIKERNEL_HISTORYOBJ_CPP

#include "GaudiKernel/HistoryObj.h"
#include "GaudiKernel/Property.h"

using namespace std;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

HistoryObj::HistoryObj()
{

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

std::string 
HistoryObj::convert_string(const std::string& input_string){

  //Conversion of special characteres into xml language

  std::string::const_iterator itr_string;
  std::string modified_string;

  for(itr_string=input_string.begin(); itr_string!=input_string.end(); itr_string++) {
    if (*itr_string == '&') modified_string.append("&amp;");
    else if (*itr_string == '<') modified_string.append("&lt;");
    else if (*itr_string == '>') modified_string.append("&gt;");
    else if (*itr_string == '"') modified_string.append("&quot;");
    else if (*itr_string == '\'') modified_string.append("&apos;");
    else if (*itr_string == '\"') modified_string.append("&quot;");
    else modified_string+=*itr_string;
  }

  return modified_string;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void 
HistoryObj::indent(std::ostream& ost, int i) const {
  while (i > 0) {
    ost << " ";
    i--;
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

const CLID& 
HistoryObj::classID() { 

  static CLID CLID_HistoryObj = 86452397;
  return CLID_HistoryObj; 

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Output stream.

std::ostream& operator<<(std::ostream& lhs, const HistoryObj& rhs) {

  rhs.dump(lhs,false);

  return lhs;
}
