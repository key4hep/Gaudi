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
// GaudiHistory/ServiceHistory.cpp
//
// Contains history information for a Service
//
//
// @author: Charles Leggett
//
///////////////////////////////////////////////////////////////////////////

#include "GaudiKernel/ServiceHistory.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/JobHistory.h"
#include "GaudiKernel/Service.h"
#include <iostream>

using namespace std;
using Gaudi::Details::PropertyBase;

//
///////////////////////////////////////////////////////////////////////////
//

ServiceHistory::ServiceHistory()
    : //  HistoryObj(),
    m_pService( nullptr )
    , m_name( "none" )
    , m_type( "none" )
    , m_version( "none" ) {}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
ServiceHistory::ServiceHistory( const IService* isv, const JobHistory* job )
    : //  HistoryObj(),
    m_pService( isv )
    , m_jobHistory( job )
    , m_name( isv->name() )
    , m_version( "none" ) {

  const Service* svc = dynamic_cast<const Service*>( isv );
  if ( !svc ) throw GaudiException( "Cannot dynamic cast to Service class", name(), StatusCode::FAILURE );
  m_type       = System::typeinfoName( typeid( *svc ) );
  m_properties = svc->getProperties();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

ServiceHistory::ServiceHistory( const IService& isv, const JobHistory* job )
    : //  HistoryObj(),
    m_pService( &isv )
    , m_jobHistory( job )
    , m_name( isv.name() )
    , m_version( "none" ) {

  const Service* svc = dynamic_cast<const Service*>( &isv );
  if ( svc ) {
    m_type       = System::typeinfoName( typeid( *svc ) );
    m_properties = svc->getProperties();
  } else {
    throw GaudiException( "Cannot dynamic cast to Service class", name(), StatusCode::FAILURE );
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

const CLID& ServiceHistory::classID() {

  static const CLID CLID_ServiceHistory = 187225489; // from `clid ServiceHistory`
  return CLID_ServiceHistory;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

std::ostream& ServiceHistory::dump( std::ostream& ost, const bool isXML, int ind ) const {

  if ( !isXML ) {

    ost << "Name: " << name() << endl;
    ost << "Type: " << type() << endl;
    ost << "Version: " << version() << endl;

    // Properties
    ost << "Properties: [" << endl;

    for ( const auto& ipprop : properties() ) {
      const PropertyBase& prop = *ipprop;
      prop.fillStream( ost );
      ost << endl;
    }
    ost << "]" << endl;

  } else {

    ind += 2;
    indent( ost, ind );
    ost << "<COMPONENT name=\"" << name() << "\" class=\"" << convert_string( type() ) << "\" version=\""
        << convert_string( version() ) << "\">" << endl;

    for ( const auto& ipprop : properties() ) {
      const PropertyBase& prop = *ipprop;

      indent( ost, ind + 2 );
      ost << "<PROPERTY name=\"" << prop.name() << "\" value=\"" << convert_string( prop.toString() )
          << "\" documentation=\"" << convert_string( prop.documentation() ) << "\">" << endl;
    }

    indent( ost, ind );
    ost << "</COMPONENT>" << endl;
  }
  return ost;
}
