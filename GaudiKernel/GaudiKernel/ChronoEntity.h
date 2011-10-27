#ifndef GAUDIKERNEL_CHRONOENTITY_H
#define GAUDIKERNEL_CHRONOENTITY_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IChronoSvc.h"
#include "GaudiKernel/StatEntity.h"
#include "GaudiKernel/Timing.h"
// ============================================================================
/** @class ChronoEntity ChronoEntity.h GaudiKernel/ChronoEntity.h
 *
 *  a small helper class for
 *  implementation of ChronoStatSvc service,
 *  It also could be used as some local timer
 *
 *  @author   Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date December 1, 1999
 */
class GAUDI_API ChronoEntity
{
public:
  /// default constructor
  ChronoEntity();
  /// destructor
  ~ChronoEntity(){}
  ///
public:
  // ==========================================================================
  // The basic Chrono Operations
  // ==========================================================================
  /// start the current chrono
  IChronoSvc::ChronoStatus        start() ;
  /// stop the chrono
  IChronoSvc::ChronoStatus        stop () ;
  // ==========================================================================
  /// return the last delta-time of type "type"
  inline IChronoSvc::ChronoTime   delta ( IChronoSvc::ChronoType type ) const ;
  /// return the status of chrono
  inline IChronoSvc::ChronoStatus status() const ;
  // ==========================================================================
public:
  // ==========================================================
  // Access to Chrono Statistics
  // ==========================================================
  /// number of chrono measurements
  inline unsigned long nOfMeasurements   () const ;
  // ==========================================================
  /// minimal measurement for user time
  inline double uMinimalTime   () const ;
  /// minimal measurement for kernel time
  inline double kMinimalTime   () const ;
  /// minimal measurement for elapsed time
  inline double eMinimalTime   () const ;
  /// maximal measurement for user time
  inline double uMaximalTime   () const ;
  /// maximal measurement for kernel time
  inline double kMaximalTime   () const ;
  /// maximal measurement for elapsed time
  inline double eMaximalTime   () const ;
  /// total user time
  inline double uTotalTime     () const ;
  /// total Kernel time
  inline double kTotalTime     () const ;
  /// total Elapsed time
  inline double eTotalTime     () const ;
  /// total time
  inline double totalTime      () const ;
  /// average Kernel Time
  inline double kMeanTime      () const ;
  /// average User   Time
  inline double uMeanTime      () const ;
  /// average Elapsed   Time
  inline double eMeanTime      () const ;
  /// r.m.s Kernel Time
  inline double kRMSTime       () const ;
  /// r.m.s User Time
  inline double uRMSTime       () const ;
  /// r.m.s Elapsed Time
  inline double eRMSTime       () const ;
  /// error in mean Kernel time
  inline double kMeanErrorTime () const ;
  /// error in mean User   time
  inline double uMeanErrorTime () const ;
  /// error in mean Elapsed   time
  inline double eMeanErrorTime () const ;
  // ==========================================================================
public:
  // ==========================================================================
  /// comparison operator
  bool operator<( const ChronoEntity& entity ) const;
  // ==========================================================================
public:
  // ==========================================================================
  /// print the chrono ;
  std::string outputUserTime     () const ;
  /// print the chrono ;
  std::string outputSystemTime   () const ;
  /// print the chrono ;
  std::string outputElapsedTime  () const ;
  // ==========================================================================
public:
  // ==========================================================================
  /** print the chrono according the format and units
   *  @param fmt  the format string
   *  @param unit the unit
   *  @return the string representations
   *  @see boost::format
   */
  inline std::string outputUserTime
  ( const std::string& fmt  ,
    System::TimeType   unit ) const ;
  /** print the chrono according the format and units
   *  @param fmt  the format string
   *  @param unit the unit
   *  @return the string representations
   *  @see boost::format
   */
  inline std::string outputSystemTime
  ( const std::string& fmt  ,
    System::TimeType   unit ) const ;
  /** print the chrono according the format and units
   *  @param fmt  the format string
   *  @param unit the unit
   *  @return the string representations
   *  @see boost::format
   */
  inline std::string outputElapsedTime
  ( const std::string& fmt  ,
    System::TimeType   unit ) const ;
  // ==========================================================================
  /** print the chrono according the format and units
   *
   *  The format fields are:
   *
   *   -# number of entries
   *   -# total  time        (in the specified units)
   *   -# mean   time        (in the specified units)
   *   -# r.m.s. time        (in the specified units)
   *   -# error in mean time (in the specified units)
   *   -# minimal time       (in the specified units)
   *   -# maximal time       (in the specified units)
   *
   *  @param typ  the chrono type
   *  @param fmt  the format string
   *  @param unit the unit
   *  @return the string representations
   *  @see boost::format
   */
  std::string outputTime
  ( IChronoSvc::ChronoType typ  ,
    const std::string&     fmt  ,
    System::TimeType       unit ) const ;
  // ==========================================================================
protected:
  // ==========================================================================
  /// format
  std::string format
  ( const double        total     ,
    const double        minimal   ,
    const double        mean      ,
    const double        rms       ,
    const double        maximal   ,
    const unsigned long number    ) const ;
  // ==========================================================================
private:
  // ==========================================================================
  /// current status of this chrono object;
  IChronoSvc::ChronoStatus m_status ;
  /// delta process times
  System::ProcessTime      m_delta;
  /// start stamp for current measurement of process times
  System::ProcessTime      m_start;
  /// the actual storage of "user" time
  StatEntity m_user    ; // the actual storage of "user" time
  /// the actual storage of "kernel" time
  StatEntity m_kernel  ; // the actual storage of "kernel" time
  /// the actual storage of "elapsed" time
  StatEntity m_elapsed ; // the actual storage of "elapsed" time
  /// internal unit used for the system time conversion (microseconds)
  static const System::TimeType TimeUnit = System::microSec;
  // ==========================================================================
};
// ============================================================================
// return the status of chrono
// ============================================================================
inline IChronoSvc::ChronoStatus
ChronoEntity::status() const { return m_status; }
// ============================================================================
// number of chrono measurements
// ============================================================================
inline unsigned long
ChronoEntity::nOfMeasurements   () const { return m_user   . nEntries() ; }
// ============================================================================
// minimal measurement for user time
// ============================================================================
inline double ChronoEntity::uMinimalTime      () const
{ return m_user   . flagMin () ; }
// ============================================================================
// minimal measurement for kernel time
// ============================================================================
inline double
ChronoEntity::kMinimalTime      () const { return m_kernel . flagMin () ; }
// ============================================================================
// minimal measurement for elapsed time
// ============================================================================
inline double
ChronoEntity::eMinimalTime      () const { return m_elapsed. flagMin () ; }
// ============================================================================
// maximal measurement for user time
// ============================================================================
inline double
ChronoEntity::uMaximalTime      () const { return m_user   . flagMax () ; }
// ============================================================================
// maximal measurement for kernel time
// ============================================================================
inline double
ChronoEntity::kMaximalTime      () const { return m_kernel . flagMax () ; }
// ============================================================================
// maximal measurement for ellapsed time
// ============================================================================
inline double
ChronoEntity::eMaximalTime      () const { return m_elapsed . flagMax () ; }
// ============================================================================
// total user time
// ============================================================================
inline double
ChronoEntity::uTotalTime        () const { return m_user    . flag () ; }
// ============================================================================
// total Kernel time
// ============================================================================
inline double
ChronoEntity::kTotalTime        () const { return m_kernel  . flag () ; }
// ============================================================================
// total Elapsed time
// ============================================================================
inline double
ChronoEntity::eTotalTime        () const { return m_elapsed . flag () ; }
// ============================================================================
// total time
// ============================================================================
inline double
ChronoEntity::totalTime         () const
{ return uTotalTime() + kTotalTime()  ; }
// ============================================================================
// average Kernel Time
// ============================================================================
inline double
ChronoEntity::kMeanTime      () const { return m_kernel  . flagMean () ; }
// ============================================================================
// average User   Time
// ============================================================================
inline double
ChronoEntity::uMeanTime      () const { return m_user    . flagMean () ; }
// ============================================================================
// average Elapsed   Time
// ============================================================================
inline double
ChronoEntity::eMeanTime      () const { return m_elapsed . flagMean () ; }
// ============================================================================
// r.m.s Kernel Time
// ============================================================================
inline double
ChronoEntity::kRMSTime       () const { return m_kernel  . flagRMS  () ; }
// ============================================================================
// r.m.s User Time
// ============================================================================
inline double
ChronoEntity::uRMSTime       () const { return m_user    . flagRMS  () ; }
// ============================================================================
// r.m.s Elapsed Time
// ============================================================================
inline double
ChronoEntity::eRMSTime       () const { return m_elapsed . flagRMS () ; }
// ============================================================================
// error in mean Kernel time
// ============================================================================
inline double
ChronoEntity::kMeanErrorTime () const { return m_kernel  . flagMeanErr () ; }
// ============================================================================
// error in mean User   time
// ============================================================================
inline double
ChronoEntity::uMeanErrorTime () const { return m_user    . flagMeanErr () ; }
// ============================================================================
// error in mean Elapsed   time
// ============================================================================
inline double
ChronoEntity::eMeanErrorTime () const { return m_elapsed . flagMeanErr () ; }
// ============================================================================
// return the last delta-time of type "type"
// ============================================================================
inline IChronoSvc::ChronoTime
ChronoEntity::delta ( IChronoSvc::ChronoType type ) const
{
  const IChronoSvc::ChronoTime result = -1 ;
  switch ( type )
  {
  case IChronoSvc::USER    : return m_delta.userTime<TimeUnit>();
  case IChronoSvc::KERNEL  : return m_delta.kernelTime<TimeUnit>();
  case IChronoSvc::ELAPSED : return m_delta.elapsedTime<TimeUnit>();
  default                  : return result;
  }
  // cannot reach this point
}
// ============================================================================
/*  print the chrono according the format and units
 *  @param fmt  the format string
 *  @param unit the unit
 *  @return the string representations
 *  @see boost::format
 */
// ============================================================================
inline std::string
ChronoEntity::outputUserTime
( const std::string& fmt  ,
  System::TimeType   unit ) const
{
  return outputTime ( IChronoSvc::USER , fmt , unit ) ;
}
// ============================================================================
/*  print the chrono according the format and units
 *  @param fmt  the format string
 *  @param unit the unit
 *  @return the string representations
 *  @see boost::format
 */
// ============================================================================
inline std::string
ChronoEntity::outputSystemTime
( const std::string& fmt  ,
  System::TimeType   unit ) const
{
  return outputTime ( IChronoSvc::KERNEL , fmt , unit ) ;
}
// ============================================================================
/*  print the chrono according the format and units
 *  @param fmt  the format string
 *  @param unit the unit
 *  @return the string representations
 *  @see boost::format
 */
// ============================================================================
inline std::string
ChronoEntity::outputElapsedTime
( const std::string& fmt  ,
  System::TimeType   unit ) const
{
  return outputTime ( IChronoSvc::ELAPSED , fmt , unit ) ;
}
// ============================================================================
// The END
// ============================================================================
#endif  //  GAUDIKERNEL_CHRONOENTITY_H
// ============================================================================










