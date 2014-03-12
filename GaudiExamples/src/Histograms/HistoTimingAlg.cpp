// $Id: HistoTimingAlg.cpp,v 1.5 2008/10/09 09:59:14 marcocle Exp $
// Include files

#include "boost/assign/list_of.hpp"

// local
#include "HistoTimingAlg.h"

// only needed for old style histogram for speed comparison
#include "AIDA/IHistogram1D.h"

//-----------------------------------------------------------------------------
// Implementation file for class : HistoTimingAlg
//
// 2005-08-12 : Chris Jones
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_COMPONENT(HistoTimingAlg)

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
  HistoTimingAlg::HistoTimingAlg( const std::string& name,
                                  ISvcLocator* pSvcLocator)
    : GaudiHistoAlg ( name , pSvcLocator )
{
  declareProperty( "UseLookup", m_useGaudiAlg = false );
  declareProperty( "NumHistos", m_nHistos = 20 );
  declareProperty( "NumTracks", m_nTracks = 30 );
}

//=============================================================================
// Destructor
//=============================================================================
HistoTimingAlg::~HistoTimingAlg() {}

//=============================================================================
// Initialization
//=============================================================================
StatusCode HistoTimingAlg::initialize()
{
  // must be called first
  const StatusCode sc = GaudiHistoAlg::initialize();
  if ( sc.isFailure() ) return sc;

  // random number generator
  IRndmGenSvc * randSvc = svc<IRndmGenSvc>( "RndmGenSvc", true );
  if ( !randSvc || !m_rand.initialize( randSvc, Rndm::Flat(0.,1.) ) )
  {
    return Error( "Unable to create Random generator" );
  }

  // book histos
  for ( unsigned int iH = 0; iH < m_nHistos; ++iH )
  {
    std::ostringstream title;
    title << "Histogram Number " << iH;
    m_histos[ book1D(title.str(),0, 1,100) ] = title.str();
  }

  return sc;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode HistoTimingAlg::execute()
{

  for ( unsigned int iT = 0; iT < m_nTracks; ++iT )
  {

    // fill histos
    for ( HistoMap::iterator iH = m_histos.begin(); iH != m_histos.end(); ++iH )
    {
      if ( m_useGaudiAlg )
      {
        plot1D( m_rand(), iH->second, 0,1,100 );
      }
      else
      {
        iH->first->fill( m_rand() );
      }
    }

  }

  return StatusCode::SUCCESS;
}

//=============================================================================
