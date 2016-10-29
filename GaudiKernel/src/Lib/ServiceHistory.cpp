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
    , m_version( "none" )
{
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
ServiceHistory::ServiceHistory( const IService* isv, const JobHistory* job )
    : //  HistoryObj(),
    m_pService( isv )
    , m_jobHistory( job )
    , m_name( isv->name() )
    , m_version( "none" )
{

  const Service* svc = dynamic_cast<const Service*>( isv );
  m_type             = System::typeinfoName( typeid( *svc ) );
  m_properties       = svc->getProperties();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

ServiceHistory::ServiceHistory( const IService& isv, const JobHistory* job )
    : //  HistoryObj(),
    m_pService( &isv )
    , m_jobHistory( job )
    , m_name( isv.name() )
    , m_version( "none" )
{

  const Service* svc = dynamic_cast<const Service*>( &isv );
  m_type             = System::typeinfoName( typeid( *svc ) );
  m_properties       = svc->getProperties();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

const CLID& ServiceHistory::classID()
{

  static const CLID CLID_ServiceHistory = 187225489; // from `clid ServiceHistory`
  return CLID_ServiceHistory;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void ServiceHistory::dump( std::ostream& ost, const bool isXML, int ind ) const
{

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
}

//**********************************************************************
// Free functions.
//**********************************************************************

// Output stream.

ostream& operator<<( ostream& lhs, const ServiceHistory& rhs )
{

  rhs.dump( lhs, false );

  return lhs;
}
