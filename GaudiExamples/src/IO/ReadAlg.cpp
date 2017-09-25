//      ====================================================================
//  ReadAlg.cpp
//      --------------------------------------------------------------------
//
//      Package   : GaudiExamples/Example3
//
//      Author    : Markus Frank
//
//      ====================================================================

// Framework include files
#include "GaudiKernel/DataStoreItem.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/LinkManager.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartDataPtr.h"

// Example related include files
#include "ReadAlg.h"

// Event Model related classes
#include "GaudiExamples/Counter.h"
#include "GaudiExamples/Event.h"
#include "GaudiExamples/MyTrack.h"

#include "GaudiKernel/System.h"

using namespace Gaudi::Examples;

DECLARE_COMPONENT( ReadAlg )

//--------------------------------------------------------------------
// Initialize
//--------------------------------------------------------------------
StatusCode ReadAlg::initialize()
{
  m_recordSvc = service( "FileRecordDataSvc", true );
  if ( !m_recordSvc ) {
    error() << "Unable to retrieve run records service" << endmsg;
    return StatusCode::FAILURE;
  }
  if ( !m_incidentName.empty() ) {
    auto prp = m_recordSvc.as<IProperty>();
    setProperty( "IncidentName", prp->getProperty( "IncidentName" ) );
    m_incidentSvc = service( "IncidentSvc", true );
    if ( !m_incidentSvc ) {
      error() << "Failed to access IncidentSvc." << endmsg;
      return StatusCode::FAILURE;
    }
    m_incidentSvc->addListener( this, m_incidentName );
  }
  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------
// Finalize
//--------------------------------------------------------------------
StatusCode ReadAlg::finalize()
{
  if ( m_incidentSvc ) m_incidentSvc->removeListener( this );
  m_incidentSvc.reset();
  m_recordSvc.reset();
  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------
// IIncidentListener override: Inform that a new incident has occured
//--------------------------------------------------------------------
void ReadAlg::handle( const Incident& incident )
{
  always() << "Got incident: " << incident.type() << " Source:" << incident.source() << endmsg;
  if ( incident.type() == m_incidentName ) {
    std::string n = incident.source();
    always() << "Received incident:" << incident.type() << ": " << n << endmsg;
    SmartDataPtr<Counter> evt_cnt( m_recordSvc.get(), n + "/EvtCount" );
    if ( evt_cnt != 0 )
      always() << "Incident: FileInfo record: " << n << "/EvtCount=" << evt_cnt->value() << endmsg;
    else
      always() << "Incident: NO FileInfo record EvtCounter for " << n << "/EvtCount" << endmsg;

    SmartDataPtr<Counter> sum_cnt( m_recordSvc.get(), n + "/SumCount" );
    if ( sum_cnt != 0 )
      always() << "Incident: FileInfo record: " << n << "/SumCount=" << sum_cnt->value() << endmsg;
    else
      always() << "Incident: NO FileInfo record SumCounter for " << n << "/SumCount" << endmsg;
  }
}

//--------------------------------------------------------------------
// Execute
//--------------------------------------------------------------------
StatusCode ReadAlg::execute()
{
  // This just makes the code below a bit easier to read (and type)
  SmartDataPtr<Event> evt( eventSvc(), "/Event/Header" );

  if ( evt != 0 ) {
    int evt_num = evt->event();
    if ( evt_num <= 10 || evt_num % 100 == 0 ) {
      auto& log = info();
      log << "========= EVENT:" << evt->event() << " RUN:" << evt->run() << " TIME:" << evt->time();
      for ( size_t nc = 0; nc < evt->collisions().size(); ++nc ) {
        if ( evt->collisions()[nc] ) {
          log << " " << evt->collisions()[nc]->collision();
          log << " (" << evt->collisions()[nc].hintID() << ")";
        } else {
          log << " ===";
        }
      }
      log << endmsg;
    }
    SmartDataPtr<MyTrackVector> myTracks( eventSvc(), "/Event/MyTracks" );
    SmartDataPtr<MyVertexVector> myVtx( eventSvc(), "/Event/Collision_0/MyVertices" );
    if ( myTracks != 0 ) {
      IOpaqueAddress* pAddr    = myTracks->registry()->address();
      static std::string fname = "";
      int count                = 0;
      if ( pAddr ) {
        std::string new_fname = pAddr->par()[0];
        if ( fname != new_fname ) {
          fname = new_fname;
          SmartDataPtr<Counter> evt_cnt( m_recordSvc.get(), new_fname + "/EvtCount" );
          if ( evt_cnt != 0 )
            always() << "FileInfo record: " << new_fname << "/EvtCount=" << evt_cnt->value() << endmsg;
          else
            always() << "NO FileInfo record EvtCounter for " << fname << endmsg;

          SmartDataPtr<Counter> sum_cnt( m_recordSvc.get(), new_fname + "/SumCount" );
          if ( sum_cnt != 0 )
            always() << "FileInfo record: " << new_fname << "/SumCount=" << sum_cnt->value() << endmsg;
          else
            always() << "NO FileInfo record SumCounter for " << fname << endmsg;
        }
      }

      for ( MyTrackVector::iterator i = myTracks->begin(); i != myTracks->end() && count++ < 5; i++ ) {
        try {
          if ( evt->event() < 10 || evt->event() % 500 == 0 ) {
            auto& log = debug();
            log << "Evt:";
            if ( ( *i )->event() ) {
              log << ( *i )->event()->event();
            } else {
              log << "Unknown";
            }
            log << " Track:";
            log.width( 12 );
            log << ( *i )->px();
            log.width( 12 );
            log << ( *i )->py();
            log.width( 12 );
            log << ( *i )->pz();
            log << " Org:";
            if ( ( *i )->originVertex() ) {
              log.width( 4 );
              log << ( *i )->originVertex()->index();
              log.width( 10 );
              log << ( *i )->originVertex()->x();
              log.width( 10 );
              log << ( *i )->originVertex()->y();
              log.width( 10 );
              log << ( *i )->originVertex()->z();
            } else {
              log << "No Origin Vertex!";
            }
            log << endmsg << "   Decays:";
            log.width( 4 );
            log << ( *i )->decayVertices().size();
            log << endmsg;
            for ( size_t id = 0; id < ( *i )->decayVertices().size(); ++id ) {
              const MyVertex* v = ( *i )->decayVertices()[id];
              if ( v ) {
                log << "    ";
                log.width( 10 );
                log << v->x();
                log.width( 10 );
                log << v->y();
                log.width( 10 );
                log << v->z() << " Collisions:";
                for ( size_t ic = 0; ic < v->collisions().size(); ++ic ) {
                  log.width( 3 );
                  if ( v->collisions()[ic] ) {
                    log << v->collisions()[ic]->collision();
                    log << " (" << v->collisions()[ic].hintID() << ")";
                  } else {
                    log << "-";
                    log << " (" << v->collisions()[ic].hintID() << ")";
                  }
                }
              } else {
                log << "No decay vertex!";
              }
              log << endmsg;
            }
          }
        } catch ( ... ) {
          error() << "Exception occurred!" << endmsg;
        }
      }
    } else
      warning() << "No tracks found, Event " << evt->event() << endmsg;
    return StatusCode::SUCCESS;
  }
  error() << "Unable to retrieve Event Header object" << endmsg;
  return StatusCode::FAILURE;
}
