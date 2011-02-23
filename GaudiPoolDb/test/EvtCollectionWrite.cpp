// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiPoolDb/test/EvtCollectionWrite.cpp,v 1.4 2007/02/08 10:12:13 hmd Exp $
//	====================================================================
//  EvtCollection.Write.cpp
//	--------------------------------------------------------------------
//
//	Author    : Markus Frank
//
//	====================================================================
#define EVTCOLLECTION_WRITE_CPP

// Framework include files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/SmartDataPtr.h"

// Example related include files
#include "EvtCollectionWrite.h"
#include "Event.h"
#include "MyTrack.h"

#include <cmath>

DECLARE_ALGORITHM_FACTORY(EvtCollectionWrite);

/**  Algorithm parameters which can be set at run time must be declared.
     This should be done in the constructor.
*/
EvtCollectionWrite::EvtCollectionWrite(const std::string& name, ISvcLocator* pSvcLocator)
:	Algorithm(name, pSvcLocator),
  m_evtTupleSvc(0)
{
  declareProperty("NumMcTracks", m_nMCcut=50);
}

// Standard destructor
EvtCollectionWrite::~EvtCollectionWrite()   {
}

StatusCode EvtCollectionWrite::initialize()   {
  StatusCode status = service("NTupleSvc", m_evtTupleSvc, true);
  if ( status.isSuccess() )   {
    // Third: An event collection N tuple
    NTuplePtr nt(m_evtTupleSvc,"/NTUPLES/EvtColl/Dir1/Dir2/Dir3/Collection");
    if ( !nt )    {    // Check if already booked
      nt = m_evtTupleSvc->book ("/NTUPLES/EvtColl/Dir1/Dir2/Dir3/Collection", CLID_ColumnWiseTuple, "Hello World");
      if ( nt )    {
        // Add an index column
        status = nt->addItem ("Ntrack", m_ntrkColl, 0, 5000 );
        status = nt->addItem ("Energy", m_eneColl );
        status = nt->addItem ("Address",m_evtAddrColl);
        status = nt->addItem ("TrkMom", m_ntrkColl, m_trkMom);
      }
      else    {   // did not manage to book the N tuple....
        return StatusCode::FAILURE;
      }
    }
    else  {  // Just reconnect to existing items
      status = nt->item ("Ntrack", m_ntrkColl );
      status = nt->item ("Energy", m_eneColl );
      status = nt->item ("TrkMom", m_trkMom);
      status = nt->item ("Address",m_evtAddrColl );
    }
  }
  return status;
}

// Event callback
StatusCode EvtCollectionWrite::execute() {
	MsgStream log(msgSvc(), name());
  SmartDataPtr<Event> evt(eventSvc(),"/Event");
  if ( evt != 0 )    {
    int evt_num = evt->event();
    SmartDataPtr<MyTrackVector> trkCont(evt, "/MyTracks");
    if ( trkCont != 0 )    {
      // Force an object update since now the original tracks should be
      // present and the local pointers can be updated!
      //m_evtAddrColl = evt->registry()->address();
      m_ntrkColl    = trkCont->size();
      m_eneColl     = 0.0;
      int cnt = 0;
      log << " ->Track:";
      for ( MyTrackVector::iterator i = trkCont->begin(); i != trkCont->end(); i++, cnt++ )   {
        MyTrack* t = (*i);
        m_trkMom[cnt] = float(sqrt(t->px()*t->px() + t->py()*t->py() + t->pz()*t->pz()));
        m_eneColl += m_trkMom[cnt];
      }
      if ( evt_num < 10 || evt_num%500==0 )  {
        log << endmsg;
        log << MSG::INFO << "================ EVENT:" << evt->event() << " RUN:" << evt->run()
            << " ====== N(Track)=" << m_ntrkColl;
        if ( m_ntrkColl < m_nMCcut )   {
          log << " FAILED selection (<" << m_nMCcut << ") ============" << endmsg;
        }
        else    {
          log << " PASSED selection (>=" << m_nMCcut << ") ============" << endmsg;
        }
        if ( m_ntrkColl >= m_nMCcut )   {
          //return m_evtTupleSvc->writeRecord("/NTUPLES/EvtColl/Dir1/Dir2/Dir3/Collection");
        }
      }
      return StatusCode::SUCCESS;
    }
  }
  log << MSG::ERROR << "Unable to retrieve Event object" << endmsg;
  return StatusCode::FAILURE;
}
