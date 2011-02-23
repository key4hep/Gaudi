// $Id: NTupleAlgorithm.cpp,v 1.7 2006/12/15 13:37:36 hmd Exp $


// Include files
#include "NTupleAlgorithm.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IHistogramSvc.h"
#include "GaudiKernel/SmartDataPtr.h"

#include "GaudiKernel/INTupleSvc.h"

#include <math.h>

DECLARE_ALGORITHM_FACTORY(NTupleAlgorithm)

//------------------------------------------------------------------------------
NTupleAlgorithm::NTupleAlgorithm(const std::string& name,
                                 ISvcLocator* pSvcLocator) :
  Algorithm(name, pSvcLocator)
                                //------------------------------------------------------------------------------
{
  m_tuple1 = 0;
  m_tuple2 = 0;
}


//------------------------------------------------------------------------------
StatusCode NTupleAlgorithm::initialize()
//------------------------------------------------------------------------------
{
  MsgStream log( msgSvc(), name() );


  //  Book N-tuple 1
  NTuplePtr nt1(ntupleSvc(), "MyTuples/1");
  if ( nt1 ) m_tuple1 = nt1;
  else {
    m_tuple1 = ntupleSvc()->book ("MyTuples/1", CLID_RowWiseTuple, "Row-wise N-Tuple example");
    if ( m_tuple1 )    {
      m_tuple1->addItem ("Ntrack",   m_ntrk).ignore();
      m_tuple1->addItem ("Energy",   m_energy).ignore();
    }
    else    {   // did not manage to book the N tuple....
      log << MSG::ERROR << "    Cannot book N-tuple:" << long(m_tuple1) << endmsg;
      return StatusCode::FAILURE;
    }
  }

  //  Book N-tuple 2
  NTuplePtr nt2(ntupleSvc(), "MyTuples/simple/2");
  if ( nt2 ) m_tuple2 = nt2;
  else {
    m_tuple2 = ntupleSvc()->book ("MyTuples/simple/2", CLID_ColumnWiseTuple, "Col-Wise NTuple examples");
    if ( m_tuple2 )    {
      m_tuple2->addItem ("N", m_n, 0, 100).ignore();
      m_tuple2->addItem ("FNumbers", m_n, m_fNumbers).ignore();
      m_tuple2->addItem ("INumbers", m_n, m_iNumbers).ignore();
    }
    else    {   // did not manage to book the N tuple....
      log << MSG::ERROR << "    Cannot book N-tuple:" << long(m_tuple2) << endmsg;
      return StatusCode::FAILURE;
    }
  }
  log << MSG::INFO << "Finished booking NTuples" << endmsg;
  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode NTupleAlgorithm::execute()
//------------------------------------------------------------------------------
{
  StatusCode status;
  MsgStream log( msgSvc(), name() );

  static int n = 0;


  m_ntrk   = long(sin(double(n)) * 52. + 50.);
  m_energy = float(sin(double(n)) * 52. + 50.);


  m_n      = abs((1234567*(n+1))%100);

  for( int i = 0; i < m_n; i++ ) {
    m_fNumbers[i] = float(cos(double(2*n)) * 52. + 50.);
    m_iNumbers[i] = long(cos(double(2*n)) * 52. + 50.);
  }
  n++;

  status = m_tuple1->write();
  if( status.isFailure() ){
    log << MSG::ERROR << "    Cannot fill N-tuple:" << long(m_tuple1) << endmsg;
    return StatusCode::FAILURE;
  }
  status = m_tuple2->write();
  if( status.isFailure() ){
    log << MSG::ERROR << "    Cannot fill N-tuple:" << long(m_tuple2) << endmsg;
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode NTupleAlgorithm::finalize()
//------------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "Finalizing..." << endmsg;

  return StatusCode::SUCCESS;
}
