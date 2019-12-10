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

std::string HistoryObj::convert_string( const std::string& input_string ) {

  // Conversion of special characteres into xml language

  std::string modified_string;

  for ( const auto& itr : input_string ) {
    if ( itr == '&' )
      modified_string.append( "&amp;" );
    else if ( itr == '<' )
      modified_string.append( "&lt;" );
    else if ( itr == '>' )
      modified_string.append( "&gt;" );
    else if ( itr == '"' )
      modified_string.append( "&quot;" );
    else if ( itr == '\'' )
      modified_string.append( "&apos;" );
    else
      modified_string += itr;
  }

  return modified_string;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void HistoryObj::indent( std::ostream& ost, int i ) const {
  while ( i > 0 ) {
    ost << " ";
    --i;
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

const CLID& HistoryObj::classID() {

  static const CLID CLID_HistoryObj = 86452397;
  return CLID_HistoryObj;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
