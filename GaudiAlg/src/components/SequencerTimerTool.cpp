// $Id: SequencerTimerTool.cpp,v 1.12 2007/01/10 16:33:32 hmd Exp $
// Include files

// From ROOT
#include "TH1D.h"

// from Gaudi
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiUtils/Aida2ROOT.h"

// local
#include "SequencerTimerTool.h"

//-----------------------------------------------------------------------------
// Implementation file for class : SequencerTimerTool
//
// 2004-05-19 : Olivier Callot
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY(SequencerTimerTool)

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
  SequencerTimerTool::SequencerTimerTool( const std::string& type,
                                          const std::string& name,
                                          const IInterface* parent )
    : GaudiHistoTool ( type, name , parent )
    , m_indent( 0 )
    , m_normFactor( 0.001 )
    , m_speedRatio(0)
{
  declareInterface<ISequencerTimerTool>(this);

  m_shots = 3500000 ; // 1s on 2.8GHz Xeon, gcc 3.2, -o2
  declareProperty( "shots"        , m_shots );
  declareProperty( "Normalised"   , m_normalised = false );
  declareProperty( "GlobalTiming" , m_globalTiming = false );
  declareProperty( "NameSize"     , m_headerSize = 30,
                   "Number of characters to be used in algorithm name column" );
  // Histograms are disabled by default in this tool.
  setProperty("HistoProduce", false).ignore();
}
//=============================================================================
// Destructor
//=============================================================================
SequencerTimerTool::~SequencerTimerTool() {}


//=========================================================================
//
//=========================================================================
StatusCode SequencerTimerTool::initialize ( )
{
  const StatusCode sc = GaudiHistoTool::initialize();
  if ( sc.isFailure() ) return sc;
  double sum = 0;
  TimerForSequencer norm( "normalize", m_headerSize, m_normFactor );
  norm.start();
  IRndmGenSvc* rsvc = svc<IRndmGenSvc>( "RndmGenSvc", true );
  { // Use dummy loop suggested by Vanya Belyaev:
    Rndm::Numbers gauss;
    gauss.initialize( rsvc , Rndm::Gauss(0.,1.0) ).ignore();
    unsigned int shots = m_shots;
    while( 0 < --shots ) { sum += gauss() * sum ; }
  }
  norm.stop();
  double time = norm.lastCpu();
  m_speedRatio = 1./time;
  info() << "This machine has a speed about "
         << format( "%6.2f", 1000.*m_speedRatio)
         << " times the speed of a 2.8 GHz Xeon." << endmsg ;
  if ( m_normalised ) {
    m_normFactor = m_speedRatio;
  }
  return sc;
}

//=========================================================================
//  Finalize : Report timers
//=========================================================================
StatusCode SequencerTimerTool::finalize ( )
{

  std::string line(m_headerSize + 68, '-');
  info() << line << endmsg
         << "This machine has a speed about "
         << format( "%6.2f", 1000.*m_speedRatio)
         << " times the speed of a 2.8 GHz Xeon.";
  if ( m_normalised ) info() <<" *** All times are renormalized ***";
  info() << endmsg << TimerForSequencer::header( m_headerSize ) << endmsg
         << line << endmsg;

  std::string lastName = "";
  for ( unsigned int kk=0 ; m_timerList.size() > kk ; kk++ )
  {
    if ( lastName == m_timerList[kk].name() ) continue; // suppress duplicate
    lastName = m_timerList[kk].name();
    info() << m_timerList[kk] << endmsg;
  }
  info() << line << endmsg;

  return GaudiHistoTool::finalize();
}

//=========================================================================
//  Return the index of a specified name. Trailing and leading spaces ignored
//=========================================================================
int SequencerTimerTool::indexByName ( const std::string& name )
{
  std::string::size_type beg = name.find_first_not_of(" \t");
  std::string::size_type end = name.find_last_not_of(" \t");
  std::string temp = name.substr( beg, end-beg+1 );
  for ( unsigned int kk=0 ; m_timerList.size() > kk ; kk++ ) {
    beg =  m_timerList[kk].name().find_first_not_of(" \t");
    end =  m_timerList[kk].name().find_last_not_of(" \t");
    if ( m_timerList[kk].name().substr(beg,end-beg+1) == temp ) return kk;
  }
  return -1;
}

//=========================================================================
//  Build and save the histograms
//=========================================================================
void SequencerTimerTool::saveHistograms()
{
  if ( produceHistos() )
  {
    info() << "Saving Timing histograms" << endmsg;
    const size_t bins = m_timerList.size();
    AIDA::IHistogram1D* histoTime = book("ElapsedTime", 0, bins, bins);
    AIDA::IHistogram1D* histoCPU  = book("CPUTime", 0, bins, bins);
    AIDA::IHistogram1D* histoCount  = book("Count", 0, bins, bins);
    TH1D* tHtime  = Gaudi::Utils::Aida2ROOT::aida2root(histoTime);
    TH1D* tHCPU   = Gaudi::Utils::Aida2ROOT::aida2root(histoCPU);
    TH1D* tHCount = Gaudi::Utils::Aida2ROOT::aida2root(histoCount);
    for ( size_t kk = 0 ; bins > kk ; kk++ )
    {
      TimerForSequencer &tfsq = m_timerList[kk];
      tHtime->Fill(tfsq.name().c_str(), tfsq.elapsedTotal());
      tHCPU->Fill(tfsq.name().c_str(), tfsq.cpuTotal());
      tHCount->Fill(tfsq.name().c_str(), tfsq.count());
    }
  }
}

//=============================================================================
// Add a timer
//=============================================================================
int SequencerTimerTool::addTimer( const std::string& name )
{
  std::string myName;
  if ( 0 < m_indent )
  {
    const std::string prefix( m_indent, ' ' );
    myName += prefix;
  }
  myName += name;
  if ( myName.size() < m_headerSize )
  {
    const std::string space( m_headerSize - myName.size(), ' ' );
    myName += space ;
  }

  //myName = myName.substr( 0, m_headerSize );

  m_timerList.push_back( TimerForSequencer( myName,
                                            m_headerSize,
                                            m_normFactor ) );

  return m_timerList.size() - 1;
}

//=============================================================================
