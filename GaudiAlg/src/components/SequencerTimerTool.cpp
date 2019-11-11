// Include files

// From ROOT
#include "TH1D.h"

// from Gaudi
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiUtils/Aida2ROOT.h"

// local
#include "SequencerTimerTool.h"

//-----------------------------------------------------------------------------
// Implementation file for class : SequencerTimerTool
//
// 2004-05-19 : Olivier Callot
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_COMPONENT( SequencerTimerTool )

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
SequencerTimerTool::SequencerTimerTool( const std::string& type, const std::string& name, const IInterface* parent )
    : GaudiHistoTool( type, name, parent ) {
  declareInterface<ISequencerTimerTool>( this );

  // Histograms are disabled by default in this tool.
  setProperty( "HistoProduce", false ).ignore();
}
//=========================================================================
//
//=========================================================================
StatusCode SequencerTimerTool::initialize() {
  const StatusCode sc = GaudiHistoTool::initialize();
  if ( sc.isFailure() ) return sc;
  double            sum = 0;
  TimerForSequencer norm( "normalize", m_headerSize, m_normFactor );
  norm.start();
  IRndmGenSvc* rsvc = svc<IRndmGenSvc>( "RndmGenSvc", true );
  { // Use dummy loop suggested by Vanya Belyaev:
    Rndm::Numbers gauss;
    gauss.initialize( rsvc, Rndm::Gauss( 0., 1.0 ) ).ignore();
    unsigned int shots = m_shots;
    while ( 0 < --shots ) { sum += gauss() * sum; }
  }
  norm.stop();
  double time  = norm.lastCpu();
  m_speedRatio = 1. / time;
  info() << "This machine has a speed about " << format( "%6.2f", 1000. * m_speedRatio )
         << " times the speed of a 2.8 GHz Xeon." << endmsg;
  if ( m_normalised ) m_normFactor = m_speedRatio;
  return sc;
}

//=========================================================================
//  Finalize : Report timers
//=========================================================================
StatusCode SequencerTimerTool::finalize() {

  if ( !m_timerList.empty() ) {
    std::string line( m_headerSize + 68, '-' );
    info() << line << endmsg << "This machine has a speed about " << format( "%6.2f", 1000. * m_speedRatio )
           << " times the speed of a 2.8 GHz Xeon.";
    if ( m_normalised ) info() << " *** All times are renormalized ***";
    info() << endmsg << TimerForSequencer::header( m_headerSize ) << endmsg << line << endmsg;

    std::string lastName;
    for ( const auto& timr : m_timerList ) {
      if ( lastName == timr.name() ) continue; // suppress duplicate
      lastName = timr.name();
      info() << timr << endmsg;
    }
    info() << line << endmsg;
  }
  return GaudiHistoTool::finalize();
}

//=========================================================================
//  Return the index of a specified name. Trailing and leading spaces ignored
//=========================================================================
int SequencerTimerTool::indexByName( const std::string& name ) {
  auto beg  = name.find_first_not_of( " \t" );
  auto end  = name.find_last_not_of( " \t" );
  auto temp = name.substr( beg, end - beg + 1 );
  auto i    = std::find_if( std::begin( m_timerList ), std::end( m_timerList ), [&]( const TimerForSequencer& timer ) {
    beg = timer.name().find_first_not_of( " \t" );
    end = timer.name().find_last_not_of( " \t" );
    return timer.name().compare( beg, end - beg + 1, temp ) == 0;
  } );
  return i != std::end( m_timerList ) ? std::distance( std::begin( m_timerList ), i ) : -1;
}

//=========================================================================
//  Build and save the histograms
//=========================================================================
void SequencerTimerTool::saveHistograms() {
  if ( produceHistos() ) {
    info() << "Saving Timing histograms" << endmsg;
    const size_t        bins       = m_timerList.size();
    AIDA::IHistogram1D* histoTime  = book( "ElapsedTime", 0, bins, bins );
    AIDA::IHistogram1D* histoCPU   = book( "CPUTime", 0, bins, bins );
    AIDA::IHistogram1D* histoCount = book( "Count", 0, bins, bins );
    TH1D*               tHtime     = Gaudi::Utils::Aida2ROOT::aida2root( histoTime );
    TH1D*               tHCPU      = Gaudi::Utils::Aida2ROOT::aida2root( histoCPU );
    TH1D*               tHCount    = Gaudi::Utils::Aida2ROOT::aida2root( histoCount );
    for ( const auto& tfsq : m_timerList ) {
      tHtime->Fill( tfsq.name().c_str(), tfsq.elapsedTotal() );
      tHCPU->Fill( tfsq.name().c_str(), tfsq.cpuTotal() );
      tHCount->Fill( tfsq.name().c_str(), tfsq.count() );
    }
  }
}

//=============================================================================
// Add a timer
//=============================================================================
int SequencerTimerTool::addTimer( const std::string& name ) {
  std::string myName( 2 * m_indent, ' ' );
  myName += name;
  if ( myName.size() < m_headerSize ) { myName += std::string( m_headerSize - myName.size(), ' ' ); }
  m_timerList.emplace_back( std::move( myName ), m_headerSize, m_normFactor );
  return m_timerList.size() - 1;
}

//=============================================================================
