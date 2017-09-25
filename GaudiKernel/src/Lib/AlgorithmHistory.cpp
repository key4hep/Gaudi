///////////////////////////////////////////////////////////////////////////
//
// GaudiHistory/AlgorithmHistory.cpp
//
// Contains history information for an Algorithm
//
//
// @author: Charles Leggett
//
///////////////////////////////////////////////////////////////////////////

#include "GaudiKernel/AlgorithmHistory.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/JobHistory.h"
#include <assert.h>
#include <iostream>

using std::ostream;
using std::endl;
using std::type_info;
using std::vector;

//**********************************************************************
// Member functions.
//**********************************************************************

// Constructor.

AlgorithmHistory::AlgorithmHistory( const Algorithm& alg, const JobHistory* job )
    : m_algorithm_type( System::typeinfoName( typeid( alg ) ) )
    , m_algorithm_version( alg.version() )
    , m_algorithm_name( alg.name() )
    , m_algorithm( &alg )
    , m_properties( alg.getProperties() )
    , m_jobHistory( job )
{
  assert( alg.subAlgorithms() != 0 );

  for ( const auto& ialg : *alg.subAlgorithms() ) {
    m_subalgorithm_histories.push_back( new AlgorithmHistory( *ialg, job ) );
  }
}

//**********************************************************************

AlgorithmHistory::AlgorithmHistory( const std::string& algVersion, const std::string& algName,
                                    const std::string& algType, const PropertyList& props,
                                    const HistoryList& subHists )
    : m_algorithm_type( algType )
    , // FIXME type_info???
    m_algorithm_version( algVersion )
    , m_algorithm_name( algName )
    , m_algorithm( nullptr )
    , m_properties( props )
    , m_subalgorithm_histories( subHists )
    , m_jobHistory( nullptr )
{
}

//**********************************************************************

// Destructor.

AlgorithmHistory::~AlgorithmHistory()
{
  for ( auto& i : m_subalgorithm_histories ) delete i;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

const CLID& AlgorithmHistory::classID()
{

  static const CLID CLID_AlgorithmHistory = 56809101; // from `clid AlgorithmHistory`
  return CLID_AlgorithmHistory;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void AlgorithmHistory::dump( std::ostream& ost, const bool isXML, int ind ) const
{

  if ( !isXML ) {
    ost << "Type: " << algorithm_type() << endl;
    ost << "Name: " << algorithm_name() << endl;
    ost << "Version: " << algorithm_version() << endl;
    // Properties.
    ost << "Properties: [" << endl;
    ;
    for ( const auto& iprop : properties() ) {
      iprop->fillStream( ost );
      ost << endl;
    }
    ost << "]" << endl;
    // Subalgorithms.
    if ( subalgorithm_histories().size() == 0 ) {
      ost << "No subalgorithms.";
    } else {
      ost << "Subalgorithms: {" << endl;
      for ( auto iphist = subalgorithm_histories().begin(); iphist != subalgorithm_histories().end(); ++iphist ) {
        if ( iphist == subalgorithm_histories().begin() ) {
          ost << "----------" << endl;
        }
        ost << **iphist << endl;
        ost << "----------" << endl;
      }
      ost << "}";
    }
  } else {
    ind += 2;
    indent( ost, ind );
    ost << "<COMPONENT name=\"" << algorithm_name() << "\" class=\"" << convert_string( algorithm_type() )
        << "\" version=\"" << algorithm_version() << "\">" << std::endl;

    for ( const auto& iprop : properties() ) {
      indent( ost, ind + 2 );
      ost << "<PROPERTY name=\"" << iprop->name() << "\" value=\"" << convert_string( iprop->toString() )
          << "\" documentation=\"" << convert_string( iprop->documentation() ) << "\">" << std::endl;
    }

    // Subalgs
    for ( const auto& iphist : subalgorithm_histories() ) {
      iphist->dump( ost, isXML, ind + 2 );
    }

    indent( ost, ind );
    ost << "</COMPONENT>" << std::endl;
  }
}

//**********************************************************************
// Free functions.
//**********************************************************************

// Output stream.

ostream& operator<<( ostream& lhs, const AlgorithmHistory& rhs )
{
  rhs.dump( lhs, false );
  return lhs;
}

//**********************************************************************
