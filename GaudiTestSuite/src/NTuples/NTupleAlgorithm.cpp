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
#include "NTupleAlgorithm.h"

#include <GaudiKernel/IHistogramSvc.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/SmartDataPtr.h>

#include <GaudiKernel/INTupleSvc.h>

#include <math.h>

DECLARE_COMPONENT( NTupleAlgorithm )

//------------------------------------------------------------------------------
NTupleAlgorithm::NTupleAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
    : Algorithm( name, pSvcLocator )
//------------------------------------------------------------------------------
{
  m_tuple1 = nullptr;
  m_tuple2 = nullptr;
}

//------------------------------------------------------------------------------
StatusCode NTupleAlgorithm::initialize()
//------------------------------------------------------------------------------
{
  //  Book N-tuple 1
  NTuplePtr nt1( ntupleSvc(), "MyTuples/1" );
  if ( nt1 )
    m_tuple1 = nt1;
  else {
    m_tuple1 = ntupleSvc()->book( "MyTuples/1", CLID_RowWiseTuple, "Row-wise N-Tuple example" );
    if ( m_tuple1 ) {
      m_tuple1->addItem( "Ntrack", m_ntrk ).ignore();
      m_tuple1->addItem( "Energy", m_energy ).ignore();
    } else { // did not manage to book the N tuple....
      error() << "    Cannot book N-tuple:" << long( m_tuple1 ) << endmsg;
      return StatusCode::FAILURE;
    }
  }

  //  Book N-tuple 2
  NTuplePtr nt2( ntupleSvc(), "MyTuples/simple/2" );
  if ( nt2 )
    m_tuple2 = nt2;
  else {
    m_tuple2 = ntupleSvc()->book( "MyTuples/simple/2", CLID_ColumnWiseTuple, "Col-Wise NTuple examples" );
    if ( m_tuple2 ) {
      m_tuple2->addItem( "N", m_n, 0, 100 ).ignore();
      m_tuple2->addItem( "FNumbers", m_n, m_fNumbers ).ignore();
      m_tuple2->addItem( "INumbers", m_n, m_iNumbers ).ignore();
    } else { // did not manage to book the N tuple....
      error() << "    Cannot book N-tuple:" << long( m_tuple2 ) << endmsg;
      return StatusCode::FAILURE;
    }
  }
  info() << "Finished booking NTuples" << endmsg;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode NTupleAlgorithm::execute()
//------------------------------------------------------------------------------
{
  StatusCode status;

  static int n = 0;

  m_ntrk   = long( sin( double( n ) ) * 52. + 50. );
  m_energy = float( sin( double( n ) ) * 52. + 50. );

  m_n = long( abs( ( 12345 * ( n + 1 ) ) % 100 ) );

  for ( int i = 0; i < m_n; i++ ) {
    m_fNumbers[i] = float( cos( double( 2 * n ) ) * 52. + 50. );
    m_iNumbers[i] = long( cos( double( 2 * n ) ) * 52. + 50. );
  }
  n++;

  status = m_tuple1->write();
  if ( status.isFailure() ) {
    error() << "    Cannot fill N-tuple:" << long( m_tuple1 ) << endmsg;
    return StatusCode::FAILURE;
  }
  status = m_tuple2->write();
  if ( status.isFailure() ) {
    error() << "    Cannot fill N-tuple:" << long( m_tuple2 ) << endmsg;
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode NTupleAlgorithm::finalize()
//------------------------------------------------------------------------------
{
  info() << "Finalizing..." << endmsg;
  return StatusCode::SUCCESS;
}
