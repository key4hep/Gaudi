#ifndef      GAUDISVC_CHRONOSTATSVC_H__
#define      GAUDISVC_CHRONOSTATSVC_H__
// ============================================================================
// include files
// ============================================================================
// STD & STL
// ============================================================================
#include <string>
#include <map>
#include <functional>
#include <fstream>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/IIncidentListener.h"
// ============================================================================
/// forward declarations
// ============================================================================
class ISvcLocator  ;
class IMessageSvc  ;
// ============================================================================
/**  @class ChronoStatSvc  ChronoStatSvc.h
 *  The  Chrono & Stat Sservice:
 *   service implements the IChronoStatSvc  interface and  provides the
 *   basic chronometry and some statistical counts needed by all applications
 *
 *   @author:  Vanya BELYAEV Ivan.Belyaev@itep.ru
 *   @daate:   December 1, 1999
 */
class ChronoStatSvc: public extends<Service,
                                    IChronoStatSvc,
                                    IIncidentListener> {
public:
  // ============================================================================
  /// some useful typedefs
  typedef std::map<IChronoStatSvc::ChronoTag,ChronoEntity>  ChronoMap ;
  typedef std::map<IChronoStatSvc::StatTag,StatEntity>      StatMap   ;
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
  virtual       ChronoEntity* chronoStart
  ( const IChronoStatSvc::ChronoTag& chronoTag ) override;
  // ============================================================================
  /** Implementation of IChronoStatSvc::chronoStop
   *  @see IChronoStatSvc
   */
  virtual const ChronoEntity* chronoStop
  ( const IChronoStatSvc::ChronoTag& chronoTag ) override;
  // ============================================================================
  /** Implementation of IchronoStatSvc::chronoDelta
   *  @see IChronoStatSvc
   */
  virtual IChronoStatSvc::ChronoTime chronoDelta
  ( const IChronoStatSvc::ChronoTag& chronoTag,
    IChronoStatSvc::ChronoType theType ) override;
  // ============================================================================
  /** Implementation of IChronoStatSvc::chronoPrint
   *  @see IChronoStatSvc
   */
  virtual void    chronoPrint
  ( const IChronoStatSvc::ChronoTag& chronoTag ) override;
  // ============================================================================
  /** Implementation of IChronoStatSvc::chronoStatus
   *  @see IChronoStatSvc
   */
  virtual ChronoStatus  chronoStatus
  ( const IChronoStatSvc::ChronoTag& chronoTag ) override;
  // ============================================================================
  /** Implementation of IChronoStatSvc::stat
   *  add statistical information to the entity , tagged by its name
   *  @see IChronoStatSvc
   */
  virtual void     stat
  ( const IChronoStatSvc::StatTag    &  statTag    ,
    const IChronoStatSvc::StatFlag   &  statFlag   )  override;
  // ============================================================================
  /** prints (using message service)  info about
   *  statistical entity, tagged by its name
   *  @see IChronoStatSvc
   */
  virtual void     statPrint
  (  const IChronoStatSvc::ChronoTag& statTag)  override;
  // ============================================================================
  /** extract the chrono entity for the given tag (name)
   *  @see IChronoStatSvc
   *  @param t chrono tag(name)
   *  @return pointer to chrono entity
   */
  virtual const ChronoEntity* chrono
  ( const IChronoStatSvc::ChronoTag& t ) const   override;
  // ============================================================================
  /** extract the stat   entity for the given tag (name)
   *  @see IChronoStatSvc
   *  @param t stat   tag(name)
   *  @return pointer to stat   entity
   */
  virtual const StatEntity*   stat
  ( const IChronoStatSvc::StatTag&   t ) const  override;
  // ============================================================================
  /**  Default constructor.
   *   @param name service instance name
   *   @param svcloc pointer to servcie locator
   */
  ChronoStatSvc ( const std::string& name, ISvcLocator* svcloc );
  /// Compound assignment operator
  void merge ( const ChronoStatSvc& css);
  /// Destructor.
  ~ChronoStatSvc() override = default;
  // ============================================================================

public:
  void handle(const Incident& incident) override;


protected:
  // ============================================================================
  // print the "Stat" part of the ChronoStatSvc
  void printStats() ;
  // ============================================================================
private:
  // ============================================================================
  // default/copy constructor and assignment are disabled
  ChronoStatSvc() = delete;
  ChronoStatSvc( const  ChronoStatSvc& ) = delete;
  ChronoStatSvc& operator=( const  ChronoStatSvc& ) = delete;
  // ============================================================================
  /// dump the statistics into an ASCII file for offline processing
  void saveStats();
  // ============================================================================
private:
  // ============================================================================
  /// chrono part
  ChronoMap      m_chronoEntities;
  /// level of info printing
  int            m_intChronoPrintLevel ;
  MSG::Level     m_chronoPrintLevel    = MSG::INFO ;
  /// flag for printing the final table
  bool           m_chronoTableFlag     ;
  /// flag for destination of the the final table
  bool           m_chronoCoutFlag      ;
  /// flag for formattion the final statistic table
  bool           m_chronoOrderFlag     ;
  /// flag for printing User   quantities
  bool           m_printUserTime       ;
  /// flag for printing System quantities
  bool           m_printSystemTime     ;
  /// flag for printing Ellapsed quantities
  bool           m_printEllapsedTime   ;

  // property
  long m_numberOfSkippedEventsForMemStat ;

  /// stat part
  StatMap        m_statEntities;

  /// level of info printing
  int            m_intStatPrintLevel ;
  MSG::Level     m_statPrintLevel     = MSG::INFO;
  /// flag for printing the final table
  bool           m_statTableFlag     ;
  /// flag for destination of the t he final table
  bool           m_statCoutFlag      ;
  /// flag for formattion the final statistic table
  bool           m_statOrderFlag     ;
  ///

  /// Name of the output file where we'll dump the stats
  StringProperty m_statsOutFileName;

  // the header row
  std::string    m_header  ; ///< the header row
  // format for regular statistical printout rows
  std::string    m_format1 ; ///< format for regular statistical printout rows
  // format for "efficiency" statistical printout rows
  std::string    m_format2 ; ///< format for "efficiency" statistical printout rows
  // flag to use the special "efficiency" format
  bool           m_useEffFormat = true ; ///< flag to use the special "efficiency" format

  typedef std::map<ChronoTag, std::vector<IChronoSvc::ChronoTime> > TimeMap;
  TimeMap m_perEvtTime;
  std::string   m_perEventFile;
  std::ofstream m_ofd;

  // ============================================================================
};
// ============================================================================
// The END
// ============================================================================
#endif   //  GAUDISVC_CHRONOSTATSVC_H
// ============================================================================


