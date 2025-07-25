/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
// Include files
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/PathResolver.h>

#include "PartPropSvc.h"

#include <HepPDT/HeavyIonUnknownID.hh>

#include <fstream>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/tokenizer.hpp>

//*************************************************************************//
StatusCode PartPropSvc::initialize() {

  StatusCode status = Service::initialize();

  if ( status.isFailure() ) {
    error() << "Could not initialize main svc" << endmsg;
    return StatusCode::FAILURE;
  }

  std::string key = m_pdtFiles.value();

  typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
  boost::char_separator<char>                           sep( ", " );
  boost::char_separator<char>                           sep_eq( "=" );

  tokenizer tokens( key, sep );
  for ( auto it = tokens.begin(); it != tokens.end(); ++it ) {

    tokenizer tok2( *it, sep_eq );
    int       nToks( std::distance( tok2.begin(), tok2.end() ) );

    auto              it2   = tok2.begin();
    const std::string fname = *it2;
    std::string       fmt;
    if ( nToks == 1 ) {
      info() << "No table format type specified for \"" << fname << "\". Assuming PDG" << endmsg;
      fmt = "PDG";
    } else {
      ++it2;
      fmt = *it2;
    }

    // see if input file exists in $DATAPATH
    std::string rfile = System::PathResolver::find_file( fname, "DATAPATH" );
    if ( rfile.empty() ) {
      error() << "Could not find PDT file: \"" << fname << "\" in $DATAPATH" << endmsg;
      return StatusCode::FAILURE;
    }

    // is the file readable?
    std::ifstream pdfile{ rfile };
    if ( !pdfile ) {
      error() << "Could not open PDT file: \"" << rfile << "\"" << endmsg;
      return StatusCode::FAILURE;
    }

    std::string FMT = boost::algorithm::to_upper_copy( fmt );

    inputFunPtr pF = nullptr;
    try {
      pF = parseTableType( FMT );
    } catch ( ... ) {
      error() << "Could not determine Particle Property table type: \"" << FMT << "\" for file \"" << fname << "\""
              << endmsg;
      return StatusCode::FAILURE;
    }

    debug() << "Adding PDT file \"" << rfile << "\" type " << FMT << endmsg;

    m_inputs.emplace_back( rfile, pF );
  }

  return status;
}
//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode PartPropSvc::finalize() {

  m_pdt.reset();

  if ( m_upid_local && m_upid ) {
    m_upid_local = false;
    // This will cause a memory leak, but we can't delete it as the
    // destructor of HepPDT::processUnknownID is protected.
    // We need this though to call reinitialize successfully.
    m_upid = nullptr;
  }

  StatusCode status = Service::finalize();

  if ( status.isSuccess() ) { debug() << "Service finalised successfully" << endmsg; }

  return status;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

PartPropSvc::inputFunPtr PartPropSvc::parseTableType( const std::string& typ ) {
  static const auto table = {
      std::make_pair( "PDG", &HepPDT::addPDGParticles ),       std::make_pair( "PYTHIA", &HepPDT::addPythiaParticles ),
      std::make_pair( "EVTGEN", &HepPDT::addEvtGenParticles ), std::make_pair( "HERWIG", &HepPDT::addHerwigParticles ),
      std::make_pair( "ISAJET", &HepPDT::addIsajetParticles ), std::make_pair( "QQ", &HepPDT::addQQParticles ) };
  auto i = std::find_if( std::begin( table ), std::end( table ),
                         [&]( const std::pair<const char*, inputFunPtr>& p ) { return typ == p.first; } );
  if ( i == std::end( table ) ) {
    error() << "Unknown Particle Data file type: \"" << typ << "\"" << endmsg;
    throw std::runtime_error( "error parsing particle table type" );
  }
  return i->second;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode PartPropSvc::createTable() {

  // use a handler for unknown heavy ions
  if ( !m_upid ) {
    setUnknownParticleHandler( new HepPDT::HeavyIonUnknownID, "Default Heavy Ion Handler" );
    m_upid_local = true;
  }

  m_pdt.emplace( m_upid_name, m_upid );

  HepPDT::TableBuilder tb( *m_pdt );

  for ( const auto& itr : m_inputs ) {
    const auto& f  = itr.first;
    const auto& pF = itr.second;

    debug() << "Reading PDT file \"" << f << "\"" << endmsg;

    std::ifstream pdfile{ f };
    // build a table from the file
    if ( !pF( pdfile, tb ) ) {
      error() << "Error reading PDT file: \"" << f << "\"" << endmsg;
      return StatusCode::FAILURE;
    }
  }

  return StatusCode::SUCCESS;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

HepPDT::ParticleDataTable* PartPropSvc::PDT() {

  if ( !m_pdt ) {
    debug() << "creating ParticleDataTable" << endmsg;
    if ( createTable().isFailure() ) {
      fatal() << "Could not create ParticleDataTable" << endmsg;
      m_pdt.reset();
    }
  }

  return m_pdt ? &m_pdt.value() : nullptr;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void PartPropSvc::setUnknownParticleHandler( HepPDT::ProcessUnknownID* puid, const std::string& n ) {
  if ( m_pdt ) {
    error() << "not setting Unknown Particle Handler \"" << n << "\" as ParticleDataTable already instantiated"
            << endmsg;
    return;
  }

  debug() << "setting Unknown Particle Handler \"" << n << "\" at " << puid << endmsg;

  if ( m_upid ) {
    warning() << "overriding previously selected Unknown Particle Handler \"" << m_upid_name << "\" with \"" << n
              << "\"" << endmsg;
  }

  m_upid      = puid;
  m_upid_name = n;
}

// Instantiation of a static factory class used by clients to create
//  instances of this service
DECLARE_COMPONENT( PartPropSvc )
//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//
