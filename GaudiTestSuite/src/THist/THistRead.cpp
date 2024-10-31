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
#include "THistRead.h"

#include <GaudiKernel/ITHistSvc.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/RndmGenerators.h>
#include <math.h>

#include <TDirectory.h>
#include <TError.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>
#include <TKey.h>
#include <TTree.h>

DECLARE_COMPONENT( THistRead )

//------------------------------------------------------------------------------
THistRead::THistRead( const std::string& name, ISvcLocator* pSvcLocator )
    : Algorithm( name, pSvcLocator )
//------------------------------------------------------------------------------
{}

//------------------------------------------------------------------------------
StatusCode THistRead::initialize()
//------------------------------------------------------------------------------
{
  if ( m_ths.retrieve().isFailure() ) {
    error() << "Couldn't get THistSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  // stream read1, 1D in "/xxx"
  StatusCode sc1a = m_ths->regHist( "/read1/xxx/1Dgauss" );
  TH1*       h1( nullptr );
  StatusCode sc1b = m_ths->getHist( "/read1/xxx/1Dgauss", h1 );
  if ( sc1a.isFailure() || sc1b.isFailure() || h1 == nullptr ) {
    error() << "Couldn't read gauss1d" << endmsg;
  } else {
    info() << h1->GetName() << ": " << h1->GetEntries() << endmsg;
  }

  // stream read2, 2D tree in "/"
  StatusCode sc2a = m_ths->regHist( "/read2/2Dgauss" );
  TH2*       h2( nullptr );
  StatusCode sc2b = m_ths->getHist( "/read2/2Dgauss", h2 );
  if ( sc2a.isFailure() || sc2b.isFailure() || h2 == nullptr ) {
    error() << "Couldn't read 2Dgauss" << endmsg;
  } else {
    info() << h2->GetName() << ": " << h2->GetEntries() << endmsg;
  }

  // 3D tree in "/"
  StatusCode sc3a = m_ths->regHist( "/read2/3Dgauss" );
  TH3*       h3( nullptr );
  StatusCode sc3b = m_ths->getHist( "/read2/3Dgauss", h3 );
  if ( sc3a.isFailure() || sc3b.isFailure() || h3 == nullptr ) {
    error() << "Couldn't read 3Dgauss" << endmsg;
  } else {
    info() << h3->GetName() << ": " << h3->GetEntries() << endmsg;
  }

  // Profile in "/"
  StatusCode sc4a = m_ths->regHist( "/read2/profile" );
  TH1*       tp( nullptr );
  StatusCode sc4b = m_ths->getHist( "/read2/profile", tp );
  if ( sc4a.isFailure() || sc4b.isFailure() || tp == nullptr ) {
    error() << "Couldn't read profile" << endmsg;
  } else {
    info() << tp->GetName() << ": " << tp->GetEntries() << endmsg;
  }

  // Tree with branches in "/trees/stuff"
  StatusCode sc5a = m_ths->regTree( "/read2/trees/stuff/treename" );
  TTree*     tr( nullptr );
  StatusCode sc5b = m_ths->getTree( "/read2/trees/stuff/treename", tr );
  if ( sc5a.isFailure() || sc5b.isFailure() || tr == nullptr ) {
    error() << "Couldn't read tree" << endmsg;
  } else {
    info() << tr->GetName() << ": " << tr->GetEntries() << endmsg;
  }
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode THistRead::execute()
//------------------------------------------------------------------------------
{
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode THistRead::finalize()
//------------------------------------------------------------------------------
{
  debug() << "Finalizing..." << endmsg;
  return StatusCode::SUCCESS;
}
