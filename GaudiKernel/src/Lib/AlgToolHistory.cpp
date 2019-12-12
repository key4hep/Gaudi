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
///////////////////////////////////////////////////////////////////////////
//
// GaudiHistory/AlgToolHistory.cpp
//
// Contains history information for an AlgTool
//
//
// @author: Charles Leggett
//
///////////////////////////////////////////////////////////////////////////

#include "GaudiKernel/AlgToolHistory.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/JobHistory.h"

#include <iostream>

using std::endl;
using std::ostream;

AlgToolHistory::AlgToolHistory( const AlgTool& alg, const JobHistory* job )
    : m_type( alg.type() )
    , m_version( "UNKNOWN" )
    , m_name( alg.name() )
    , m_tool( &alg )
    , m_properties( alg.getProperties() )
    , m_jobHistory( job ) {}

//**********************************************************************

AlgToolHistory::AlgToolHistory( const std::string& algVersion, const std::string& algName, const std::string& algType,
                                const AlgTool* tool, const PropertyList& props,
                                const JobHistory* job )
    : m_type( algType )
    , // FIXME type_info???
    m_version( algVersion )
    , m_name( algName )
    , m_tool( tool )
    , m_properties( props )
    , m_jobHistory( job ) {}

//**********************************************************************

const CLID& AlgToolHistory::classID() {

  static const CLID CLID_AlgToolHistory = 171959758; // from `clid AlgToolHistory`
  return CLID_AlgToolHistory;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void AlgToolHistory::dump( std::ostream& ost, const bool isXML, int ind ) const {

  if ( !isXML ) {

    ost << "Name: " << algtool_name() << endl;
    ost << "Type: " << algtool_type() << endl;
    ost << "Version: " << algtool_version() << endl;
    ost << "Parent: " << algtool_instance()->name() << endl;

    // Properties
    ost << "Properties: [" << endl;

    for ( const auto& iprop : properties() ) {
      iprop->fillStream( ost );
      ost << endl;
    }
    ost << "]" << endl;

  } else {

    ind += 2;
    indent( ost, ind );
    ost << "<COMPONENT name=\"" << algtool_name() << "\" class=\"" << convert_string( algtool_type() )
        << "\" version=\"" << convert_string( algtool_version() ) << "\" parent=\""
        << convert_string( algtool_instance()->name() ) << "\">" << endl;

    for ( const auto& iprop : properties() ) {

      indent( ost, ind + 2 );
      ost << "<PROPERTY name=\"" << iprop->name() << "\" value=\"" << convert_string( iprop->toString() )
          << "\" documentation=\"" << convert_string( iprop->documentation() ) << "\">" << endl;
    }

    indent( ost, ind );
    ost << "</COMPONENT>" << endl;
  }
}

//**********************************************************************

ostream& operator<<( ostream& lhs, const AlgToolHistory& rhs ) {

  rhs.dump( lhs, false );

  return lhs;
}
