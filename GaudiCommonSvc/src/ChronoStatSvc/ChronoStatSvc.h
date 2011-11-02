// $Id: ChronoStatSvc.h,v 1.13 2008/05/13 12:37:19 marcocle Exp $
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.13 $
// ============================================================================
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
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IChronoStatSvc.h"
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
class ChronoStatSvc: public extends1<Service, IChronoStatSvc> {
public:
  // ============================================================================
  /// some useful typedefs
  typedef std::map<IChronoStatSvc::ChronoTag,ChronoEntity>  ChronoMap ;
  typedef std::map<IChronoStatSvc::StatTag,StatEntity>      StatMap   ;
  // ============================================================================
public:
  // ============================================================================
  /// Implementation of IService::initialize()
  virtual StatusCode initialize();
  /// Implementation of IService::finalize()
  virtual StatusCode finalize();
  // ============================================================================
public:
  // ============================================================================
  /** Implementation of IChronoStatSvc::chronoStart
   *  @see IChronoStatSvc
   */
  virtual       ChronoEntity* chronoStart
  ( const IChronoStatSvc::ChronoTag& chronoTag );
  // ============================================================================
  /** Implementation of IChronoStatSvc::chronoStop
   *  @see IChronoStatSvc
   */
  virtual const ChronoEntity* chronoStop
  ( const IChronoStatSvc::ChronoTag& chronoTag );
  // ============================================================================
  /** Implementation of IchronoStatSvc::chronoDelta
   *  @see IChronoStatSvc
   */
  virtual IChronoStatSvc::ChronoTime chronoDelta
  ( const IChronoStatSvc::ChronoTag& chronoTag,
    IChronoStatSvc::ChronoType theType );
  // ============================================================================
  /** Implementation of IChronoStatSvc::chronoPrint
   *  @see IChronoStatSvc
   */
  virtual void    chronoPrint
  ( const IChronoStatSvc::ChronoTag& chronoTag );
  // ============================================================================
  /** Implementation of IChronoStatSvc::chronoStatus
   *  @see IChronoStatSvc
   */
  virtual ChronoStatus  chronoStatus
  ( const IChronoStatSvc::ChronoTag& chronoTag );
  // ============================================================================
  /** Implementation of IChronoStatSvc::stat
   *  add statistical information to the entity , tagged by its name
   *  @see IChronoStatSvc
   */
  virtual void     stat
  ( const IChronoStatSvc::StatTag    &  statTag    ,
    const IChronoStatSvc::StatFlag   &  statFlag   ) ;
  // ============================================================================
  /** prints (using message service)  info about
   *  statistical entity, tagged by its name
   *  @see IChronoStatSvc
   */
  virtual void     statPrint
  (  const IChronoStatSvc::ChronoTag& statTag) ;
  // ============================================================================
  /** extract the chrono entity for the given tag (name)
   *  @see IChronoStatSvc
   *  @param t chrono tag(name)
   *  @return pointer to chrono entity
   */
  virtual const ChronoEntity* chrono
  ( const IChronoStatSvc::ChronoTag& t ) const  ;
  // ============================================================================
  /** extract the stat   entity for the given tag (name)
   *  @see IChronoStatSvc
   *  @param t stat   tag(name)
   *  @return pointer to stat   entity
   */
  virtual const StatEntity*   stat
  ( const IChronoStatSvc::StatTag&   t ) const ;
  // ============================================================================
  /**  Default constructor.
   *   @param name service instance name
   *   @param svcloc pointer to servcie locator
   */
  ChronoStatSvc ( const std::string& name, ISvcLocator* svcloc );
  /// Destructor.
  virtual ~ChronoStatSvc();
  // ============================================================================
protected:
  // ============================================================================
  // print the "Stat" part of the ChronoStatSvc
  void printStats() ;
  // ============================================================================
private:
  // ============================================================================
  // default constructor is disabled
  ChronoStatSvc() ;
  // copy constructor is disabled
  ChronoStatSvc( const  ChronoStatSvc& ) ;
  // assignement operator constructor is disabled
  ChronoStatSvc& operator=( const  ChronoStatSvc& ) ;
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
  MSG::Level     m_chronoPrintLevel    ;
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
  MSG::Level     m_statPrintLevel    ;
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
  bool           m_useEffFormat ; ///< flag to use the special "efficiency" format
  // ============================================================================
};
// ============================================================================
// The END
// ============================================================================
#endif   //  GAUDISVC_CHRONOSTATSVC_H
// ============================================================================


