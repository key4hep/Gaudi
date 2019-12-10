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
// GaudiHistory/AlgorithmHistory.cpp
//
// Contains history information for an Algorithm
//
//
// @author: Charles Leggett
//
///////////////////////////////////////////////////////////////////////////

#include "GaudiKernel/AlgorithmHistory.h"
#include "GaudiKernel/JobHistory.h"
#include <Gaudi/Algorithm.h>
#include <assert.h>
#include <iostream>

using std::endl;
using std::ostream;
using std::type_info;
using std::vector;

//**********************************************************************
// Member functions.
//**********************************************************************

// Constructor.

AlgorithmHistory::AlgorithmHistory( const Gaudi::Algorithm& alg, const JobHistory* job )
    : m_algorithm_type( System::typeinfoName( typeid( alg ) ) )
    , m_algorithm_version( alg.version() )
    , m_algorithm_name( alg.name() )
    , m_algorithm( &alg )
    , m_properties( alg.getProperties() )
    , m_jobHistory( job ) {}

//**********************************************************************

AlgorithmHistory::AlgorithmHistory( std::string algVersion, std::string algName, std::string algType,
                                    const PropertyList& props,
                                    const HistoryList&  subHists )
    : m_algorithm_type( std::move( algType ) ) // FIXME type_info???
    , m_algorithm_version( std::move( algVersion ) )
    , m_algorithm_name( std::move( algName ) )
    , m_algorithm( nullptr )
    , m_properties( props )
    , m_subalgorithm_histories( subHists )
    , m_jobHistory( nullptr ) {}

//**********************************************************************

// Destructor.

AlgorithmHistory::~AlgorithmHistory() {
  for ( auto& i : m_subalgorithm_histories ) delete i;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

const CLID& AlgorithmHistory::classID() {

  static const CLID CLID_AlgorithmHistory = 56809101; // from `clid AlgorithmHistory`
  return CLID_AlgorithmHistory;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

std::ostream& AlgorithmHistory::dump( std::ostream& ost, const bool isXML, int ind ) const {

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

    indent( ost, ind );
    ost << "</COMPONENT>" << std::endl;
  }
  return ost;
}

//**********************************************************************
