// $Id: ChronoStatSvc.cpp,v 1.18 2008/05/13 12:37:19 marcocle Exp $
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.18 $
// ============================================================================
#ifdef _WIN32
#pragma warning( disable : 4786 )
#endif
// ============================================================================
// STD & STL
// ============================================================================
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <functional>
#include <fstream>
#include <iomanip>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ChronoEntity.h"
#include "GaudiKernel/StatEntity.h"
#include "GaudiKernel/Stat.h"
// ============================================================================
/// local
// ============================================================================
#include "ChronoStatSvc.h"
// ============================================================================
/// Instantiation of a static factory class used by clients to create
/// instances of this service
// ============================================================================
DECLARE_SERVICE_FACTORY(ChronoStatSvc)
// ============================================================================
/** @file
 *   Implementation of class ChronoStatSvc
 *
 *   @author:  Vanya BELYAEV Ivan.Belyaev@itep.ru
 *   @date:    December 1, 1999
 */
// ============================================================================
// ============================================================================
//  comparison functor
// ============================================================================
class ComparePairOfChronoEntityAndChronoTag
  : public std::binary_function<
  const std::pair<ChronoEntity*,const IChronoStatSvc::ChronoTag*> ,
  const std::pair<ChronoEntity*,const IChronoStatSvc::ChronoTag*> , bool >
{
public:
  inline bool operator()
    ( const  std::pair<ChronoEntity*,
      const IChronoStatSvc::ChronoTag*>& p1 ,
      const  std::pair< ChronoEntity*,
      const IChronoStatSvc::ChronoTag*>& p2 ) const
  {
    const ChronoEntity* e1 = p1.first;
    const ChronoEntity* e2 = p2.first;
    return ( ( 0 == e1 || 0 == e2 ) ? true : (*e1)<(*e2) ) ;
  }
};
// ============================================================================
//  comparison functor
// ============================================================================
class ComparePairOfStatEntityAndStatTag
  : public std::binary_function<
  const std::pair<const StatEntity*,const IChronoStatSvc::StatTag*> ,
  const std::pair<const StatEntity*,const IChronoStatSvc::StatTag*> , bool >
{
public:
  ///
  inline bool operator()
    ( const std::pair<const StatEntity*,
      const IChronoStatSvc::StatTag*>& p1,
      const std::pair<const StatEntity*,
      const IChronoStatSvc::StatTag*>& p2 ) const
  {
    const StatEntity* se1 = p1.first;
    const StatEntity* se2 = p2.first;
    return ( 0 == se1 || 0 == se2 ) ? true : (*se1)<(*se2) ;
  }
};
// ============================================================================
// Constructor
// ============================================================================
ChronoStatSvc::ChronoStatSvc
( const std::string& name, ISvcLocator* svcloc )
  : base_class( name , svcloc )
  , m_chronoEntities   ()
  , m_chronoPrintLevel ( MSG::INFO )
  , m_statEntities     ()
  , m_statPrintLevel   ( MSG::INFO )
  //
  // the header row
  , m_header  ( "     Counter     |     #     |    sum     | mean/eff^* | rms/err^*  |     min     |     max     |")
  // format for regular statistical printout rows
  , m_format1 ( " %|-15.15s|%|17t||%|10d| |%|11.7g| |%|#11.5g| |%|#11.5g| |%|#12.5g| |%|#12.5g| |" )
  // format for "efficiency" statistical printout rows
  , m_format2 ( "*%|-15.15s|%|17t||%|10d| |%|11.5g| |(%|#9.7g| +- %|-#9.7g|)%%|   -------   |   -------   |" )
  // flag to use the special "efficiency" format
  , m_useEffFormat ( true )
{
  /// decide if the final printout should be performed
  declareProperty ( "ChronoPrintOutTable"    ,
                    m_chronoTableFlag      = true      );
  /// define the destination of the table to be printed
  declareProperty ( "ChronoDestinationCout"  ,
                    m_chronoCoutFlag       = false     );
  /// print level
  declareProperty ( "ChronoPrintLevel"       ,
                    m_intChronoPrintLevel  = MSG::INFO );
  /// if printout is to be performed,
  /// should one take care about some ordering?
  declareProperty ( "ChronoTableToBeOrdered" ,
                    m_chronoOrderFlag      = true      );
  /// if User     Time information to be printed?
  declareProperty ( "PrintUserTime"          ,
                    m_printUserTime        = true      );
  /// if System   Time information to be printed?
  declareProperty ( "PrintSystemTime"        ,
                    m_printSystemTime      = false     );
  /// if Ellapsed Time information to be printed?
  declareProperty ( "PrintEllapsedTime"      ,
                    m_printEllapsedTime    = false     );
  ///
  /// deside if the final printout should be performed
  declareProperty ( "StatPrintOutTable"      ,
                    m_statTableFlag        = true      );
  /// define the destination of the table to be printed
  declareProperty ( "StatDestinationCout"    ,
                    m_statCoutFlag         = false     );
  /// print level
  declareProperty ( "StatPrintLevel"         ,
                    m_intStatPrintLevel    = MSG::INFO );
  /// if printout is to be performed,
  /// should one take care about some ordering?
  declareProperty ( "StatTableToBeOrdered"   ,
                    m_statOrderFlag        = true      );

  // specify the number of events to be skipped by the memory auditor
  // in order to better spot memory leak
  declareProperty ( "NumberOfSkippedEventsForMemStat"      ,
                    m_numberOfSkippedEventsForMemStat = -1 ) ;

  declareProperty( "AsciiStatsOutputFile",
		   m_statsOutFileName = "",
		   "Name of the output file storing the stats. If empty, no"
		   " statistics will be saved (default)" );

  declareProperty
    ( "StatTableHeader"        , m_header                          ,
      "The header row for the output Stat-table"                   ) ;
  declareProperty
    ( "RegularRowFormat"       , m_format1                         ,
      "The format for the regular row in the output Stat-table"    ) ;
  declareProperty
    ( "EfficiencyRowFormat"    , m_format2                         ,
      "The format for the regular row in the outptu Stat-table"    ) ;
  declareProperty
    ( "UseEfficiencyRowFormat" , m_useEffFormat                    ,
      "Use the special format for printout of efficiency counters" ) ;
}
// ============================================================================
// Destructor.
// ============================================================================
ChronoStatSvc::~ChronoStatSvc()
{
  // clear the container of chrono entities
  m_chronoEntities.clear();
  // clear the container of stat   entities
  m_statEntities.clear();
}
// ============================================================================
// Implementation of IService::initialize()
// ============================================================================
StatusCode ChronoStatSvc::initialize()
{
  StatusCode sc = Service::initialize();
  if ( sc.isFailure() ) return sc;
  ///
  MsgStream log( msgSvc() , this->name() );

  // Set my own properties
  sc = setProperties();

  if (sc.isFailure()) {
    log << MSG::ERROR << "setting my properties" << endmsg;
    return StatusCode::FAILURE;
  }

  log << MSG::INFO << " Number of skipped events for MemStat"
      << m_numberOfSkippedEventsForMemStat << endmsg ;

  ///
  /// redefine some "properties"
  ///
  m_statPrintLevel =
    ( MSG::FATAL < m_intStatPrintLevel   ) ? MSG::FATAL :
    ( MSG::NIL   > m_intStatPrintLevel   ) ? MSG::NIL   :
    ( MSG::Level ) m_intStatPrintLevel  ;
  ///
  m_chronoPrintLevel =
    ( MSG::FATAL < m_intChronoPrintLevel ) ? MSG::FATAL :
    ( MSG::NIL   > m_intChronoPrintLevel ) ? MSG::NIL   :
    ( MSG::Level ) m_intChronoPrintLevel ;
  ///
  if( m_chronoTableFlag  &&
      !m_printUserTime   &&
      !m_printSystemTime &&
      !m_printEllapsedTime ) { m_printUserTime   = true ; }
  ///
  if(  m_printUserTime    ||
       m_printSystemTime  ||
       m_printEllapsedTime ) { m_chronoTableFlag = true ; }
  ///
  /// start its own chrono
  chronoStart( name() ) ;
  ///
  return StatusCode::SUCCESS;
}
// ============================================================================
// Implementation of IService::finalize()
// ============================================================================
StatusCode ChronoStatSvc::finalize()
{
  std::string local = name()+".finalize()";
  ///
  MsgStream main_log( msgSvc() , local );
  ///
  /// stop its own chrono
  chronoStop( name() ) ;

  ///
  /// Is the final chrono table to be printed?
  if ( m_chronoTableFlag &&
       !m_chronoEntities.empty() &&
       ( m_printUserTime || m_printSystemTime ) )
    {
      /// decoration
      MsgStream log( msgSvc() , "*****Chrono*****" );
      const std::string stars( ( m_chronoCoutFlag ) ? 126 : 100  , '*' );
      if( m_chronoCoutFlag  )
	{
	  std::cout << stars << std::endl;
	  std::cout << local <<  " The Final CPU consumption (Chrono) Table "
		    << ( m_chronoOrderFlag ? "(ordered)" : "(not ordered)" ) << std::endl;
	  std::cout << stars << std::endl;
	}
      else
	{
	  log << (MSG::Level) m_chronoPrintLevel
        << stars << endmsg;
	  log << (MSG::Level) m_chronoPrintLevel
        << " The Final CPU consumption ( Chrono ) Table "
	      << ( m_chronoOrderFlag ? "(ordered)" : "(not ordered)" ) << endmsg;
	  log << (MSG::Level) m_chronoPrintLevel << stars << endmsg;
	}
      ///
      {  // prepare container for printing
	typedef std::pair<ChronoEntity*,const ChronoTag*>        MPair;
	typedef std::vector<MPair>                               MCont;
	MCont tmpCont;
	for( ChronoMap::iterator it = m_chronoEntities.begin() ;
       m_chronoEntities.end() != it ;  ++it )
	  { tmpCont.push_back( MPair( &(it->second) , &(it->first) ) ) ; }
	// sort it
	if( m_chronoOrderFlag )
  { std::sort( tmpCont.begin() ,
               tmpCont.end()   ,
               ComparePairOfChronoEntityAndChronoTag() ); }
	// print User Time statistics
        if( m_printUserTime   )
	  {
	    for( MCont::iterator iter = tmpCont.begin() ;  tmpCont.end() != iter ; ++iter )
	      {
		//
		ChronoEntity*    entity = iter->first  ; if( 0 == entity ) { continue ; } /// CONTINUE
		const ChronoTag* tag    = iter->second ; if( 0 == tag    ) { continue ; } /// CONTINUE
		///
		entity->stop();              /// stop chrono (if it is still in RUNNING status)
		///
		if     ( m_chronoCoutFlag )  /// print to std::cout
		  { std::cout << *tag << "\t"  << entity->outputUserTime  () << std::endl ; }
		else                         /// print to MsgStream
		  {
		    MsgStream(msgSvc(), *tag)
          << m_chronoPrintLevel << entity->outputUserTime  () << endmsg ;
		  }
		//
	      }
	  }
	///
        if( m_printSystemTime )
	  {
	    ///
	    /// decoration
	    if      ( m_printUserTime && m_chronoCoutFlag   )
	      { std::cout << stars << std::endl;                           }
	    else if ( m_printUserTime && !m_chronoCoutFlag  )
	      { log << (MSG::Level) m_chronoPrintLevel << stars << endmsg; }
	    ///
	    for( MCont::iterator iter = tmpCont.begin() ;  tmpCont.end() != iter ; ++iter )
	      {
		///
		ChronoEntity*    entity = iter->first  ; if( 0 == entity ) { continue ; } /// CONTINUE
		const ChronoTag* tag    = iter->second ; if( 0 == tag    ) { continue ; } /// CONTINUE
		///
		entity->stop();              /// stop chrono (if it is still in RUNNING status)
		///
		if     ( m_chronoCoutFlag )  /// print to std::cout
		  { std::cout << *tag << "\t"  << entity->outputSystemTime() << std::endl ; }
		else                         /// print to MsgStream
		  {
		    MsgStream(msgSvc(), *tag)
          << m_chronoPrintLevel << entity->outputSystemTime() << endmsg ;
		  }
		//
	      }
	  }
	///
        if( m_printEllapsedTime )
	  {
	    ///
	    /// decoration
	    if      ( ( m_printUserTime || m_printSystemTime ) &&  m_chronoCoutFlag  )
	      { std::cout << stars << std::endl;                           }
	    else if ( ( m_printUserTime || m_printSystemTime ) && !m_chronoCoutFlag  )
	      { log << (MSG::Level) m_chronoPrintLevel << stars << endmsg; }
	    ///
	    for( MCont::iterator iter = tmpCont.begin() ;  tmpCont.end() != iter ; ++iter )
	      {
		///
		ChronoEntity*    entity = iter->first  ; if( 0 == entity ) { continue ; } /// CONTINUE
		const ChronoTag* tag    = iter->second ; if( 0 == tag    ) { continue ; } /// CONTINUE
		///
		entity->stop();              /// stop chrono (if it is still in RUNNING status)
		///
		if     ( m_chronoCoutFlag )  /// print to std::cout
		  { std::cout << *tag << "\t"  << entity->outputElapsedTime() << std::endl ; }
		else                         /// print to MsgStream
		  {
		    MsgStream(msgSvc(), *tag)
          << m_chronoPrintLevel << entity->outputElapsedTime() << endmsg ;
		  }
		//
	      }
	  }
	///
	tmpCont.clear();   /// clear the temporary container
      }
      /// the final decoration
      if( m_chronoCoutFlag  ) { std::cout << stars << std::endl;              }
      else                    { log << m_chronoPrintLevel << stars << endmsg; }
    }

  ///

  /// Is the final stat table to be printed?
  if ( m_statTableFlag ) { printStats () ; }

  if ( !m_statsOutFileName.value().empty() ) {
    saveStats();
  }

  main_log << MSG::INFO << " Service finalized successfully " << endmsg;

  return Service::finalize(); ///< finalize the base class
}
// ============================================================================
// Implementation of IChronoStatSvc::chronoStart
// ============================================================================
ChronoEntity*
ChronoStatSvc::chronoStart
( const ChronoTag& chronoTag )
{
  ChronoEntity& entity = m_chronoEntities [ chronoTag ] ;
  entity.start() ;
  return &entity ;
}
// ============================================================================
// Implementation of IChronoStatSvc::chronoStop
// ============================================================================
const ChronoEntity*
ChronoStatSvc::chronoStop
( const IChronoStatSvc::ChronoTag& chronoTag )
{
  ChronoEntity& entity = m_chronoEntities [ chronoTag ] ;
  entity.stop() ;
  return &entity ;
}
// ============================================================================
// Implementation of IChronoStatSvc::chronoDelta
// ============================================================================
IChronoStatSvc::ChronoTime
ChronoStatSvc::chronoDelta
( const IChronoStatSvc::ChronoTag& chronoTag,
  IChronoStatSvc::ChronoType theType )
{
  return m_chronoEntities[ chronoTag ].delta( theType );
}
// ============================================================================
// Implementation of IChronoStatSvc::chronoPrint
// ============================================================================
void    ChronoStatSvc::chronoPrint
( const IChronoStatSvc::ChronoTag& chronoTag )
{
  MsgStream log ( msgSvc() , chronoTag );
  if( m_printUserTime   ) {
    log << (MSG::Level) m_chronoPrintLevel
        << m_chronoEntities[ chronoTag ].outputUserTime  ()
	<< endmsg;
  }
  if( m_printSystemTime ) {
    log << (MSG::Level) m_chronoPrintLevel
        << m_chronoEntities[ chronoTag ].outputSystemTime()
	<< endmsg;
  }
}
// ============================================================================
// Implementation of IChronoSvc::chronoStatus
// ============================================================================
IChronoStatSvc::ChronoStatus
ChronoStatSvc::chronoStatus
( const IChronoStatSvc::ChronoTag& chronoTag )
{ return m_chronoEntities[ chronoTag ].status(); }
// ============================================================================
// Implementation of IChronoStatSvc::stat
// ============================================================================
void    ChronoStatSvc::stat
( const IChronoStatSvc::StatTag    & statTag    ,
  const IChronoStatSvc::StatFlag   & statFlag   )
{
  StatMap::iterator theIter=m_statEntities.find(statTag);

  StatEntity * theStat=0 ;
  // if new entity, specify the neumber of events to be skipped
  if (theIter==m_statEntities.end()){
    // new stat entity
    StatEntity& theSe = m_statEntities[ statTag ];
    theStat=& theSe;
    theStat->setnEntriesBeforeReset(m_numberOfSkippedEventsForMemStat);
  }
  else
  {
    //existing stat entity
    theStat=&((*theIter).second);
  }

  theStat->addFlag ( statFlag ) ;
}
// ============================================================================
// Implementation of IChronoStatSvc::statPrint
// ============================================================================
void    ChronoStatSvc::statPrint
( const IChronoStatSvc::StatTag& statTag )
{
  MsgStream log ( msgSvc() , statTag ) ;
  log << (MSG::Level) m_statPrintLevel << m_statEntities[ statTag ] << endmsg;
}
// ============================================================================
/*  extract the chrono entity for the given tag (name)
 *  @see IChronoStatSvc
 *  @param t chrono tag(name)
 *  @return pointer to chrono entity
 */
// ============================================================================
const ChronoEntity* ChronoStatSvc::chrono
( const IChronoStatSvc::ChronoTag& t ) const
{
  ChronoMap::const_iterator it = m_chronoEntities.find ( t ) ;
  if ( m_chronoEntities.end() != it ) { return &(it->second) ; }
  return 0 ;
}
// ============================================================================
/*  extract the stat   entity for the given tag (name)
 *  @see IChronoStatSvc
 *  @param t stat   tag(name)
 *  @return pointer to stat   entity
 */
// ============================================================================
const StatEntity*   ChronoStatSvc::stat
( const IChronoStatSvc::StatTag&   t ) const
{
  StatMap::const_iterator it = m_statEntities.find ( t ) ;
  if ( m_statEntities.end() != it ) { return &(it->second) ; }
  return 0 ;
}
// ============================================================================
// dump all the statistics into an ASCII file
// ============================================================================
void ChronoStatSvc::saveStats()
{
  std::ofstream out( m_statsOutFileName.value().c_str(),
		     std::ios_base::out | std::ios_base::trunc );
  if ( !out.good() ) {
    MsgStream msg( msgSvc() , name() );
    msg << MSG::INFO
	<< "Could not open the output file for writing chrono statistics ["
	<< m_statsOutFileName.value() << "]"
	<< endmsg;
    return;
  } else {
    // format it our way
    out << std::scientific << std::setprecision(8) ;
  }

  // ChronoEntity
  typedef std::pair<ChronoEntity*, const ChronoTag*> MPair;
  typedef std::vector<MPair>                         MCont;
  MCont chronos;

  for( ChronoMap::iterator it = m_chronoEntities.begin() ;
       m_chronoEntities.end() != it ;  ++it ) {
    chronos.push_back( MPair( &(it->second) , &(it->first) ) ) ;
  }

  // sort it
  std::sort( chronos.begin() ,
	     chronos.end()   ,
	     ComparePairOfChronoEntityAndChronoTag() );

  // print User Time statistics
  for( MCont::iterator iter = chronos.begin() ;
       chronos.end() != iter;
       ++iter ) {
    //
    const ChronoEntity*    entity = iter->first;
    if( 0 == entity ) { continue ; } /// CONTINUE

    const ChronoTag* tag    = iter->second ;
    if( 0 == tag    ) { continue ; } /// CONTINUE

    // create an entry in the .INI-like table
    out << "\n[" << *tag << "]\n";

    // user
    out << "cpu_user_total = " << entity->uTotalTime() << "\n";
    out << "cpu_user_min = "   << entity->uMinimalTime() << "\n";
    out << "cpu_user_mean = "  << entity->uMeanTime() << "\n";
    out << "cpu_user_RMS = "   << entity->uRMSTime() << "\n";
    out << "cpu_user_max = "   << entity->uMaximalTime() << "\n";
    out << "cpu_user_nbr = "   << entity->nOfMeasurements() << "\n";

    // system
    out << "\n"; // just for clarity
    out << "cpu_system_total = " << entity->kTotalTime() << "\n";
    out << "cpu_system_min = "   << entity->kMinimalTime() << "\n";
    out << "cpu_system_mean = "  << entity->kMeanTime() << "\n";
    out << "cpu_system_RMS = "   << entity->kRMSTime() << "\n";
    out << "cpu_system_max = "   << entity->kMaximalTime() << "\n";
    out << "cpu_system_nbr = "   << entity->nOfMeasurements() << "\n";

    // real
    out << "\n"; // just for clarity
    out << "cpu_real_total = " << entity->eTotalTime() << "\n";
    out << "cpu_real_min = "   << entity->eMinimalTime() << "\n";
    out << "cpu_real_mean = "  << entity->eMeanTime() << "\n";
    out << "cpu_real_RMS = "   << entity->eRMSTime() << "\n";
    out << "cpu_real_max = "   << entity->eMaximalTime() << "\n";
    out << "cpu_real_nbr = "   << entity->nOfMeasurements() << "\n";

  }

  out << std::endl;
}
// ============================================================================
// print the "Stat" part of the ChronoStatSvc
// ============================================================================
void ChronoStatSvc::printStats()
{
  /// nothing to print?
  if ( m_statEntities.empty() ) { return ; }

  MsgStream log ( msgSvc() , "******Stat******" ) ;
  ///
  const std::string stars( ( m_statCoutFlag ) ? 126 : 100  , '*' ) ;
  ///
  if ( m_statCoutFlag  )
	{
	  std::cout << stars << std::endl;
	  std::cout <<  " The Final stat Table "
              << ( m_statOrderFlag ? "(ordered)" : "(not ordered)" ) << std::endl;
	  std::cout << stars << std::endl;
	}
  else
  {
	  log << m_statPrintLevel << stars << endmsg;
	  log <<  m_statPrintLevel << " The Final stat Table "
	      << ( m_statOrderFlag ? "(ordered)" : "(not ordered)" ) << endmsg;
	  log << m_statPrintLevel << stars << endmsg;
	}

  {
    // prepare container for printing
    typedef std::pair<const StatEntity*,const StatTag*>  SPair;
    typedef std::vector<SPair>                           SCont;
    SCont tmpCont;
    for( StatMap::const_iterator it = m_statEntities.begin();
         it != m_statEntities.end(); it++ )
    { tmpCont.push_back( SPair( &(it->second) , &(it->first) ) ) ; }
    // sort it
    if ( m_statOrderFlag )
    { std::sort( tmpCont.begin() ,
                 tmpCont.end()   ,
                 ComparePairOfStatEntityAndStatTag() ); }


    {
      // print the table header
      if ( m_statCoutFlag ) { std::cout <<                     m_header << std::endl ; }
      else                  { log       << m_statPrintLevel << m_header << endmsg    ; }
    }
    // loop over counters and print them:
    for ( SCont::iterator iter = tmpCont.begin() ; tmpCont.end() != iter ; ++iter )
	  {
	    ///
      const StatEntity* entity = iter->first  ;
      if ( 0 == entity ) { continue ; } /// CONTINUE
      const StatTag*    tag    = iter->second ;
      if ( 0 == tag    ) { continue ; } /// CONTINUE
	    ///
	    if ( m_statCoutFlag )
      {
        std::cout
          << Gaudi::Utils::formatAsTableRow
          ( *tag , *entity , m_useEffFormat , m_format1 , m_format2 )
          << std::endl;
      }
	    else
      {
        log
          << m_statPrintLevel
          << Gaudi::Utils::formatAsTableRow
          ( *tag , *entity , m_useEffFormat , m_format1 , m_format2 )
          << endmsg ;
      }
	  }
    tmpCont.clear();
  }
  /// decoration
  if ( m_statCoutFlag  ) { std::cout << stars << std::endl;            }
  else                   { log << m_statPrintLevel << stars << endmsg; }
}

// ============================================================================
// The END
// ============================================================================
