#ifndef GAUDISVC_CHRONOSTATSVC_H__
#define GAUDISVC_CHRONOSTATSVC_H__
// ============================================================================
// include files
// ============================================================================
// STD & STL
// ============================================================================
#include <atomic>
#include <fstream>
#include <functional>
#include <map>
#include <mutex>
#include <string>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ThreadLocalContext.h"
// ============================================================================
/// forward declarations
// ============================================================================
class ISvcLocator;
class IMessageSvc;
// ============================================================================
/**  @class ChronoStatSvc  ChronoStatSvc.h
 *  The  Chrono & Stat Sservice:
 *   service implements the IChronoStatSvc  interface and  provides the
 *   basic chronometry and some statistical counts needed by all applications
 *
 *   @author:  Vanya BELYAEV Ivan.Belyaev@itep.ru
 *   @daate:   December 1, 1999
 */
class ChronoStatSvc : public extends<Service, IChronoStatSvc, IIncidentListener> {
public:
  // ============================================================================
  /// some useful typedefs
  typedef std::map<IChronoStatSvc::ChronoTag, ChronoEntity> ChronoMap;
  typedef std::map<IChronoStatSvc::StatTag, StatEntity>     StatMap;
  // ============================================================================
public:
  // ============================================================================
  /// Implementation of IService::initialize()
  StatusCode initialize() override;
  /// Implementation of IService::finalize()
  StatusCode finalize() override;
  // ============================================================================
public:
  // ============================================================================
  /** Implementation of IChronoStatSvc::chronoStart
   *  @see IChronoStatSvc
   */
  ChronoEntity* chronoStart( const IChronoStatSvc::ChronoTag& chronoTag ) override;
  // ============================================================================
  /** Implementation of IChronoStatSvc::chronoStop
   *  @see IChronoStatSvc
   */
  const ChronoEntity* chronoStop( const IChronoStatSvc::ChronoTag& chronoTag ) override;
  // ============================================================================
  /** Implementation of IchronoStatSvc::chronoDelta
   *  @see IChronoStatSvc
   */
  IChronoStatSvc::ChronoTime chronoDelta( const IChronoStatSvc::ChronoTag& chronoTag,
                                          IChronoStatSvc::ChronoType       theType ) override;
  // ============================================================================
  /** Implementation of IChronoStatSvc::chronoPrint
   *  @see IChronoStatSvc
   */
  void chronoPrint( const IChronoStatSvc::ChronoTag& chronoTag ) override;
  // ============================================================================
  /** Implementation of IChronoStatSvc::chronoStatus
   *  @see IChronoStatSvc
   */
  ChronoStatus chronoStatus( const IChronoStatSvc::ChronoTag& chronoTag ) override;
  // ============================================================================
  /** Implementation of IChronoStatSvc::stat
   *  add statistical information to the entity , tagged by its name
   *  @see IChronoStatSvc
   */
  void stat( const IChronoStatSvc::StatTag& statTag, const IChronoStatSvc::StatFlag& statFlag ) override;
  // ============================================================================
  /** prints (using message service)  info about
   *  statistical entity, tagged by its name
   *  @see IChronoStatSvc
   */
  void statPrint( const IChronoStatSvc::ChronoTag& statTag ) override;
  // ============================================================================
  /** extract the chrono entity for the given tag (name)
   *  @see IChronoStatSvc
   *  @param t chrono tag(name)
   *  @return pointer to chrono entity
   */
  const ChronoEntity* chrono( const IChronoStatSvc::ChronoTag& t ) const override;
  // ============================================================================
  /** extract the stat   entity for the given tag (name)
   *  @see IChronoStatSvc
   *  @param t stat   tag(name)
   *  @return pointer to stat   entity
   */
  StatEntity* stat( const IChronoStatSvc::StatTag& t ) override;
  // ============================================================================
  /**  Default constructor.
   *   @param name service instance name
   *   @param svcloc pointer to service locator
   */
  ChronoStatSvc( const std::string& name, ISvcLocator* svcloc ) : base_class( name, svcloc ) {}

  /// Compound assignment operator
  void merge( const ChronoStatSvc& css );
  // ============================================================================

public:
  void handle( const Incident& incident ) override;

protected:
  // ============================================================================
  // print the "Stat" part of the ChronoStatSvc
  void printStats();
  // ============================================================================
private:
  // ============================================================================
  // default/copy constructor and assignment are disabled
  ChronoStatSvc()                       = delete;
  ChronoStatSvc( const ChronoStatSvc& ) = delete;
  ChronoStatSvc& operator=( const ChronoStatSvc& ) = delete;
  // ============================================================================
  /// dump the statistics into an ASCII file for offline processing
  void saveStats();
  // ============================================================================
private:
  bool isMT() const;

  ChronoEntity& getEntity( const ChronoTag& chronoTag ) {
    lock_t lock( m_mutex );
    return m_chronoEntities[chronoTag];
  }

  // basically limit the integer to MSG::Level range
  static MSG::Level int2level( int l ) {
    return static_cast<MSG::Level>(
        std::max( std::min( l, static_cast<int>( MSG::FATAL ) ), static_cast<int>( MSG::NIL ) ) );
  };
  // ============================================================================
  /// chrono part
  ChronoMap m_chronoEntities;
  /// Mutex protecting m_chronoEntities.
  mutable std::mutex                  m_mutex;
  typedef std::lock_guard<std::mutex> lock_t;

  /// level of info printing
  MSG::Level m_chronoPrintLevel = MSG::INFO;

  /// stat part
  StatMap m_statEntities;
  /// level of info printing
  MSG::Level m_statPrintLevel = MSG::INFO;

  Gaudi::Property<bool> m_chronoTableFlag{this, "ChronoPrintOutTable", true,
                                          "decide if the final printout should be performed"};
  Gaudi::Property<bool> m_chronoCoutFlag{this, "ChronoDestinationCout", false,
                                         "define the destination of the table to be printed"};
  Gaudi::Property<int>  m_intChronoPrintLevel{
      this, "ChronoPrintLevel", MSG::INFO, [this]( auto& ) { m_chronoPrintLevel = int2level( m_intChronoPrintLevel ); },
      "print level"};
  Gaudi::Property<bool> m_chronoOrderFlag{this, "ChronoTableToBeOrdered", true, "should the printout be ordered"};
  Gaudi::Property<bool> m_printUserTime{this, "PrintUserTime", true};
  Gaudi::Property<bool> m_printSystemTime{this, "PrintSystemTime", false};
  Gaudi::Property<bool> m_printEllapsedTime{this, "PrintEllapsedTime", false};
  Gaudi::Property<bool> m_statTableFlag{this, "StatPrintOutTable", true,
                                        "decide if the final printout should be performed"};
  Gaudi::Property<bool> m_statCoutFlag{this, "StatDestinationCout", false,
                                       "define the destination of the table to be printed"};
  Gaudi::Property<int>  m_intStatPrintLevel{this, "StatPrintLevel", MSG::INFO,
                                           [this]( auto& ) { m_statPrintLevel = int2level( m_intStatPrintLevel ); },
                                           "print level"};
  Gaudi::Property<bool> m_statOrderFlag{this, "StatTableToBeOrdered", true, "should the printout be ordered"};

  Gaudi::Property<std::string> m_statsOutFileName{
      this, "AsciiStatsOutputFile", "",
      "Name of the output file storing the stats. If empty, no statistics will be saved (default)"};

  Gaudi::Property<std::string> m_header{
      this, "StatTableHeader",
      " |    Counter     |     #     |    sum     | mean/eff^* | rms/err^*  |     min     |     max     |",
      "The header row for the output Stat-table"};
  Gaudi::Property<bool> m_useEffFormat{this, "UseEfficiencyRowFormat", true,
                                       "Use the special format for printout of efficiency counters"};

  Gaudi::Property<std::string> m_perEventFile{this, "PerEventFile", "", "File name for per-event deltas"};

  ServiceHandle<IInterface> m_hiveWhiteBoardSvc{this, "HiveWhiteBoardSvc", "EventDataSvc"};

  typedef std::map<ChronoTag, std::vector<IChronoSvc::ChronoTime>> TimeMap;
  TimeMap                                                          m_perEvtTime;
  std::ofstream                                                    m_ofd;

  // ============================================================================
};
// ============================================================================
// The END
// ============================================================================
#endif //  GAUDISVC_CHRONOSTATSVC_H
