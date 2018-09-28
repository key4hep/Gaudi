#ifdef _WIN32
#pragma warning( disable : 4786 )
#endif
// ============================================================================
// STD & STL
// ============================================================================
#include <algorithm>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/ChronoEntity.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Stat.h"
#include "GaudiKernel/StatEntity.h"
#include "GaudiKernel/StatusCode.h"
// ============================================================================
/// local
// ============================================================================
#include "ChronoStatSvc.h"
// ============================================================================
/// Instantiation of a static factory class used by clients to create
/// instances of this service
// ============================================================================
DECLARE_COMPONENT( ChronoStatSvc )
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
constexpr struct CompareFirstOfPointerPair_t {
  template <typename S, typename T>
  inline bool operator()( const std::pair<S*, T*>& p1, const std::pair<S*, T*>& p2 ) const
  {
    auto e1 = p1.first;
    auto e2 = p2.first;
    return ( !e1 || !e2 ) || *e1 < *e2;
  }
} CompareFirstOfPointerPair{};
// ============================================================================
// Compound assignment operator.
// ============================================================================
void ChronoStatSvc::merge( const ChronoStatSvc& css )
{

  // Add the content of the maps, leave the rest unchanged

  // Merge Chronomaps
  for ( auto& item : css.m_chronoEntities ) {
    const IChronoStatSvc::ChronoTag& key = item.first;
    const ChronoEntity&              val = item.second;
    if ( m_chronoEntities.count( key ) )
      m_chronoEntities[key] += val;
    else
      m_chronoEntities.insert( std::pair<IChronoStatSvc::ChronoTag, ChronoEntity>( key, val ) );
  }

  // Merge StatMaps
  for ( auto& item : css.m_statEntities ) {
    const IChronoStatSvc::StatTag& key = item.first;
    const StatEntity&              val = item.second;
    if ( m_statEntities.count( key ) )
      m_statEntities[key] += val;
    else
      m_statEntities.emplace( key, val );
  }
}
// ============================================================================
// Implementation of IService::initialize()
// ============================================================================
StatusCode ChronoStatSvc::initialize()
{
  StatusCode sc = Service::initialize();
  if ( sc.isFailure() ) return sc;
  ///
  // Set my own properties
  sc = setProperties();

  if ( sc.isFailure() ) {
    error() << "setting my properties" << endmsg;
    return StatusCode::FAILURE;
  }

  // only add an EndEvent listener if per-event output requested
  if ( !m_perEventFile.empty() ) {
    m_ofd.open( m_perEventFile );
    if ( !m_ofd.is_open() ) {
      error() << "unable to open per-event output file \"" << m_perEventFile << "\"" << endmsg;
      return StatusCode::FAILURE;
    } else {
      auto ii = serviceLocator()->service<IIncidentSvc>( "IncidentSvc" );
      if ( !ii ) {
        error() << "Unable to find IncidentSvc" << endmsg;
        return StatusCode::FAILURE;
      }
      ii->addListener( this, IncidentType::EndEvent );
    }
  }

  if ( m_chronoTableFlag && !m_printUserTime && !m_printSystemTime && !m_printEllapsedTime ) {
    m_printUserTime = true;
  }
  ///
  if ( m_printUserTime || m_printSystemTime || m_printEllapsedTime ) {
    m_chronoTableFlag = true;
  }
  ///
  /// start its own chrono
  chronoStart( name() );
  ///
  return StatusCode::SUCCESS;
}
// ============================================================================
// Implementation of IService::finalize()
// ============================================================================
StatusCode ChronoStatSvc::finalize()
{
  std::string local = name() + ".finalize()";
  ///
  MsgStream main_log( msgSvc(), local );
  ///
  /// stop its own chrono
  chronoStop( name() );

  if ( m_ofd.is_open() ) {
    debug() << "writing per-event timing data to '" << m_perEventFile << "'" << endmsg;
    for ( const auto& itr : m_perEvtTime ) {
      m_ofd << itr.first.substr( 0, itr.first.length() - 8 ) << " ";
      for ( const auto& itt : itr.second ) {
        m_ofd << " " << (long int)( itt );
      }
      m_ofd << std::endl;
    }

    m_ofd.close();
  }

  ///
  /// Is the final chrono table to be printed?
  if ( m_chronoTableFlag && !m_chronoEntities.empty() && ( m_printUserTime || m_printSystemTime ) ) {
    /// decoration
    MsgStream         log( msgSvc(), "*****Chrono*****" );
    const std::string stars( ( m_chronoCoutFlag ) ? 126 : 100, '*' );
    if ( m_chronoCoutFlag ) {
      std::cout << stars << std::endl;
      std::cout << local << " The Final CPU consumption (Chrono) Table "
                << ( m_chronoOrderFlag ? "(ordered)" : "(not ordered)" ) << std::endl;
      std::cout << stars << std::endl;
    } else {
      log << (MSG::Level)m_chronoPrintLevel << stars << endmsg;
      log << (MSG::Level)m_chronoPrintLevel << " The Final CPU consumption ( Chrono ) Table "
          << ( m_chronoOrderFlag ? "(ordered)" : "(not ordered)" ) << endmsg;
      log << (MSG::Level)m_chronoPrintLevel << stars << endmsg;
    }
    ///
    { // prepare container for printing
      std::vector<std::pair<ChronoEntity*, const ChronoTag*>> tmpCont;
      tmpCont.reserve( m_chronoEntities.size() );
      for ( auto& it : m_chronoEntities ) {
        tmpCont.emplace_back( &it.second, &it.first );
      }
      // sort it
      if ( m_chronoOrderFlag ) std::sort( tmpCont.begin(), tmpCont.end(), CompareFirstOfPointerPair );
      // print User Time statistics
      if ( m_printUserTime ) {
        for ( auto iter = tmpCont.begin(); tmpCont.end() != iter; ++iter ) {
          //
          ChronoEntity* entity = iter->first;
          if ( !entity ) {
            continue;
          } /// CONTINUE
          const ChronoTag* tag = iter->second;
          if ( !tag ) {
            continue;
          } /// CONTINUE
          ///
          entity->stop(); /// stop chrono (if it is still in RUNNING status)
          ///
          if ( m_chronoCoutFlag ) /// print to std::cout
          {
            std::cout << *tag << "\t" << entity->outputUserTime() << std::endl;
          } else /// print to MsgStream
          {
            MsgStream( msgSvc(), *tag ) << m_chronoPrintLevel << entity->outputUserTime() << endmsg;
          }
          //
        }
      }
      ///
      if ( m_printSystemTime ) {
        ///
        /// decoration
        if ( m_printUserTime && m_chronoCoutFlag ) {
          std::cout << stars << std::endl;
        } else if ( m_printUserTime && !m_chronoCoutFlag ) {
          log << (MSG::Level)m_chronoPrintLevel << stars << endmsg;
        }
        ///
        for ( auto iter = tmpCont.begin(); tmpCont.end() != iter; ++iter ) {
          ///
          ChronoEntity* entity = iter->first;
          if ( !entity ) {
            continue;
          } /// CONTINUE
          const ChronoTag* tag = iter->second;
          if ( !tag ) {
            continue;
          } /// CONTINUE
          ///
          entity->stop(); /// stop chrono (if it is still in RUNNING status)
          ///
          if ( m_chronoCoutFlag ) /// print to std::cout
          {
            std::cout << *tag << "\t" << entity->outputSystemTime() << std::endl;
          } else /// print to MsgStream
          {
            MsgStream( msgSvc(), *tag ) << m_chronoPrintLevel << entity->outputSystemTime() << endmsg;
          }
          //
        }
      }
      ///
      if ( m_printEllapsedTime ) {
        ///
        /// decoration
        if ( ( m_printUserTime || m_printSystemTime ) && m_chronoCoutFlag ) {
          std::cout << stars << std::endl;
        } else if ( ( m_printUserTime || m_printSystemTime ) && !m_chronoCoutFlag ) {
          log << (MSG::Level)m_chronoPrintLevel << stars << endmsg;
        }
        ///
        for ( const auto& i : tmpCont ) {
          ///
          ChronoEntity* entity = i.first;
          if ( !entity ) {
            continue;
          } /// CONTINUE
          const ChronoTag* tag = i.second;
          if ( !tag ) {
            continue;
          } /// CONTINUE
          ///
          entity->stop(); /// stop chrono (if it is still in RUNNING status)
          ///
          if ( m_chronoCoutFlag ) /// print to std::cout
          {
            std::cout << *tag << "\t" << entity->outputElapsedTime() << std::endl;
          } else /// print to MsgStream
          {
            MsgStream( msgSvc(), *tag ) << m_chronoPrintLevel << entity->outputElapsedTime() << endmsg;
          }
          //
        }
      }
      ///
      tmpCont.clear(); /// clear the temporary container
    }
    /// the final decoration
    if ( m_chronoCoutFlag ) {
      std::cout << stars << std::endl;
    } else {
      log << m_chronoPrintLevel << stars << endmsg;
    }
  }

  ///

  /// Is the final stat table to be printed?
  if ( m_statTableFlag ) {
    printStats();
  }

  if ( !m_statsOutFileName.value().empty() ) {
    saveStats();
  }

  main_log << MSG::INFO << " Service finalized successfully " << endmsg;

  return Service::finalize(); ///< finalize the base class
}
// ============================================================================
// Implementation of IChronoStatSvc::chronoStart
// ============================================================================
ChronoEntity* ChronoStatSvc::chronoStart( const ChronoTag& chronoTag )
{
  ChronoEntity& entity = m_chronoEntities[chronoTag];
  entity.start();
  return &entity;
}
// ============================================================================
// Implementation of IChronoStatSvc::chronoStop
// ============================================================================
const ChronoEntity* ChronoStatSvc::chronoStop( const IChronoStatSvc::ChronoTag& chronoTag )
{
  ChronoEntity& entity = m_chronoEntities[chronoTag];
  entity.stop();
  return &entity;
}
// ============================================================================
// Implementation of IChronoStatSvc::chronoDelta
// ============================================================================
IChronoStatSvc::ChronoTime ChronoStatSvc::chronoDelta( const IChronoStatSvc::ChronoTag& chronoTag,
                                                       IChronoStatSvc::ChronoType       theType )
{
  return m_chronoEntities[chronoTag].delta( theType );
}
// ============================================================================
// Implementation of IChronoStatSvc::chronoPrint
// ============================================================================
void ChronoStatSvc::chronoPrint( const IChronoStatSvc::ChronoTag& chronoTag )
{
  MsgStream log( msgSvc(), chronoTag );
  if ( m_printUserTime ) {
    log << (MSG::Level)m_chronoPrintLevel << m_chronoEntities[chronoTag].outputUserTime() << endmsg;
  }
  if ( m_printSystemTime ) {
    log << (MSG::Level)m_chronoPrintLevel << m_chronoEntities[chronoTag].outputSystemTime() << endmsg;
  }
}
// ============================================================================
// Implementation of IChronoSvc::chronoStatus
// ============================================================================
IChronoStatSvc::ChronoStatus ChronoStatSvc::chronoStatus( const IChronoStatSvc::ChronoTag& chronoTag )
{
  return m_chronoEntities[chronoTag].status();
}
// ============================================================================
// Implementation of IChronoStatSvc::stat
// ============================================================================
void ChronoStatSvc::stat( const IChronoStatSvc::StatTag& statTag, const IChronoStatSvc::StatFlag& statFlag )
{
  auto theIter = m_statEntities.find( statTag );

  StatEntity* theStat = nullptr;
  // if new entity, specify the number of events to be skipped
  if ( theIter == m_statEntities.end() ) {
    // new stat entity
    StatEntity& theSe = m_statEntities[statTag];
    theStat           = &theSe;
  } else {
    // existing stat entity
    theStat = &theIter->second;
  }

  theStat->addFlag( statFlag );
}
// ============================================================================
// Implementation of IChronoStatSvc::statPrint
// ============================================================================
void ChronoStatSvc::statPrint( const IChronoStatSvc::StatTag& statTag )
{
  MsgStream log( msgSvc(), statTag );
  log << (MSG::Level)m_statPrintLevel << m_statEntities[statTag] << endmsg;
}
// ============================================================================
/*  extract the chrono entity for the given tag (name)
 *  @see IChronoStatSvc
 *  @param t chrono tag(name)
 *  @return pointer to chrono entity
 */
// ============================================================================
const ChronoEntity* ChronoStatSvc::chrono( const IChronoStatSvc::ChronoTag& t ) const
{
  auto it = m_chronoEntities.find( t );
  return m_chronoEntities.end() != it ? &( it->second ) : nullptr;
}
// ============================================================================
/*  extract the stat   entity for the given tag (name)
 *  @see IChronoStatSvc
 *  @param t stat   tag(name)
 *  @return pointer to stat   entity
 */
// ============================================================================
StatEntity* ChronoStatSvc::stat( const IChronoStatSvc::StatTag& t )
{
  auto it = m_statEntities.find( t );
  return m_statEntities.end() != it ? &( it->second ) : nullptr;
}
// ============================================================================
// dump all the statistics into an ASCII file
// ============================================================================
void ChronoStatSvc::saveStats()
{
  std::ofstream out( m_statsOutFileName.value(), std::ios_base::out | std::ios_base::trunc );
  if ( !out.good() ) {
    info() << "Could not open the output file for writing chrono statistics [" << m_statsOutFileName.value() << "]"
           << endmsg;
    return;
  } else {
    // format it our way
    out << std::scientific << std::setprecision( 8 );
  }

  // ChronoEntity
  std::vector<std::pair<const ChronoEntity*, const ChronoTag*>> chronos;
  chronos.reserve( m_chronoEntities.size() );
  std::transform( std::begin( m_chronoEntities ), std::end( m_chronoEntities ), std::back_inserter( chronos ),
                  []( ChronoMap::const_reference i ) { return std::make_pair( &i.second, &i.first ); } );

  // sort it
  std::sort( std::begin( chronos ), std::end( chronos ), CompareFirstOfPointerPair );

  // print User Time statistics
  for ( const auto& iter : chronos ) {
    //
    const ChronoEntity* entity = iter.first;
    if ( !entity ) {
      continue;
    } /// CONTINUE

    const ChronoTag* tag = iter.second;
    if ( !tag ) {
      continue;
    } /// CONTINUE

    // create an entry in the .INI-like table
    out << "\n[" << *tag << "]\n";

    // user
    out << "cpu_user_total = " << entity->uTotalTime() << "\n";
    out << "cpu_user_min = " << entity->uMinimalTime() << "\n";
    out << "cpu_user_mean = " << entity->uMeanTime() << "\n";
    out << "cpu_user_RMS = " << entity->uRMSTime() << "\n";
    out << "cpu_user_max = " << entity->uMaximalTime() << "\n";
    out << "cpu_user_nbr = " << entity->nOfMeasurements() << "\n";

    // system
    out << "\n"; // just for clarity
    out << "cpu_system_total = " << entity->kTotalTime() << "\n";
    out << "cpu_system_min = " << entity->kMinimalTime() << "\n";
    out << "cpu_system_mean = " << entity->kMeanTime() << "\n";
    out << "cpu_system_RMS = " << entity->kRMSTime() << "\n";
    out << "cpu_system_max = " << entity->kMaximalTime() << "\n";
    out << "cpu_system_nbr = " << entity->nOfMeasurements() << "\n";

    // real
    out << "\n"; // just for clarity
    out << "cpu_real_total = " << entity->eTotalTime() << "\n";
    out << "cpu_real_min = " << entity->eMinimalTime() << "\n";
    out << "cpu_real_mean = " << entity->eMeanTime() << "\n";
    out << "cpu_real_RMS = " << entity->eRMSTime() << "\n";
    out << "cpu_real_max = " << entity->eMaximalTime() << "\n";
    out << "cpu_real_nbr = " << entity->nOfMeasurements() << "\n";
  }

  out << std::endl;
}
// ============================================================================
// print the "Stat" part of the ChronoStatSvc
// ============================================================================
void ChronoStatSvc::printStats()
{
  /// nothing to print?
  if ( m_statEntities.empty() ) {
    return;
  }

  MsgStream log( msgSvc(), "******Stat******" );
  ///
  const std::string stars( ( m_statCoutFlag ) ? 126 : 100, '*' );
  ///
  if ( m_statCoutFlag ) {
    std::cout << stars << std::endl;
    std::cout << " The Final stat Table " << ( m_statOrderFlag ? "(ordered)" : "(not ordered)" ) << std::endl;
    std::cout << stars << std::endl;
  } else {
    log << m_statPrintLevel << stars << endmsg;
    log << m_statPrintLevel << " The Final stat Table " << ( m_statOrderFlag ? "(ordered)" : "(not ordered)" )
        << endmsg;
    log << m_statPrintLevel << stars << endmsg;
  }

  {
    // prepare container for printing
    typedef std::pair<const StatEntity*, const StatTag*> SPair;
    typedef std::vector<SPair> SCont;
    SCont                      tmpCont;
    std::transform( std::begin( m_statEntities ), std::end( m_statEntities ), std::back_inserter( tmpCont ),
                    []( StatMap::const_reference i ) { return std::make_pair( &i.second, &i.first ); } );
    // sort it
    if ( m_statOrderFlag ) std::sort( tmpCont.begin(), tmpCont.end(), CompareFirstOfPointerPair );
    // print the table header
    if ( m_statCoutFlag ) {
      std::cout << m_header.value() << std::endl;
    } else {
      log << m_statPrintLevel << m_header.value() << endmsg;
    }

    // loop over counters and print them:
    for ( const auto& iter : tmpCont ) {
      ///
      const StatEntity* entity = iter.first;
      if ( !entity ) {
        continue;
      } /// CONTINUE
      const StatTag* tag = iter.second;
      if ( !tag ) {
        continue;
      } /// CONTINUE
      ///
      if ( m_statCoutFlag ) {
        entity->print( std::cout, true, *tag, m_useEffFormat, "%|-15.15s|%|17t|" );
      } else {
        std::ostringstream ost;
        entity->print( ost, true, *tag, m_useEffFormat, "%|-15.15s|%|17t|" );
        log << m_statPrintLevel << ost.str() << endmsg;
      }
    }
    tmpCont.clear();
  }
  /// decoration
  if ( m_statCoutFlag ) {
    std::cout << stars << std::endl;
  } else {
    log << m_statPrintLevel << stars << endmsg;
  }
}

// ============================================================================

void ChronoStatSvc::handle( const Incident& /* inc */ )
{

  if ( !m_ofd.is_open() ) return;

  for ( const auto& itr : m_chronoEntities ) {
    if ( itr.first.find( ":Execute" ) == std::string::npos ) continue;

    auto itm = m_perEvtTime.find( itr.first );
    if ( itm == m_perEvtTime.end() ) {
      m_perEvtTime[itr.first] = {itr.second.delta( IChronoSvc::ELAPSED )};
    } else {
      itm->second.push_back( itr.second.delta( IChronoSvc::ELAPSED ) );
    }
  }
}

// ============================================================================
// The END
// ============================================================================
