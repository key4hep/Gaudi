// $Id: ChronoEntity.cpp,v 1.4 2008/06/06 13:20:13 marcocle Exp $ 
// ============================================================================
#define GAUDIKERNEL_CHRONOENTITY_CPP 1 
// ============================================================================
// incldue files 
// ============================================================================
// STD & STL 
// ============================================================================
#include <iostream> 
#include <iomanip> 
#include <cmath> 
#include <algorithm>
#include <cstdio>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Kernel.h" 
#include "GaudiKernel/System.h" 
#include "GaudiKernel/ChronoEntity.h" 
// ============================================================================
// Boost 
// ============================================================================
#include "boost/format.hpp"
// ============================================================================
/** @file 
 *  implementation fiel for class ChronoEntity 
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date:   December 1, 1999 
 */
// ============================================================================
namespace 
{
  /// unit here is microsecond
  const System::TimeType MyUnit = System::microSec ; // unit here is microsecond 
  /// unit here is microsecond
  const double microsecond  =    1                 ; // unit here is microsecond 
  const double millisecond  = 1000  * microsecond ;
  const double second       = 1000  * millisecond ; 
  const double minute       =   60  *      second ; 
  const double hour         =   60  *      minute ; 
  const double day          =   24  *        hour ; 
  const double week         =    7  *         day ; 
  const double month        =   30  *         day ;
  const double year         =  365  *         day ;
  const double nanosecond   = 0.001 * microsecond ;
}
// ============================================================================
// Constructor 
// ============================================================================
ChronoEntity::ChronoEntity() 
  /// current status of this chrono object; 
  : m_status        ( IChronoSvc::UNKNOWN )  // current status
  /// delta user time
  , m_delta_user    ( -1 ) // delta user time
  /// delta kernel time
  , m_delta_kernel  ( -1 ) // delta kernel time
  /// delta elapsed time
  , m_delta_elapsed ( -1 ) // delta elapsed time
  /// start stamp for current measurement of user time
  , m_start_user    (  0 ) // start for user 
  /// start stamp for  current measurement of Kernel time
  , m_start_kernel  (  0 ) // start for kernel 
  /// start stamp for current measurement of Elapsed time
  , m_start_elapsed (  0 ) // start for elapsed  
{}
// ============================================================================
// start the chrono
// ============================================================================
IChronoSvc::ChronoStatus  ChronoEntity::start()
{
  if ( IChronoSvc::RUNNING == m_status ) { return m_status ; }
  //
  // following lines could be platform dependent!
  //
  // Store in object the measured times
  m_start_user    = static_cast<IChronoSvc::ChronoTime>(System::userTime     ( MyUnit ));
  m_start_kernel  = static_cast<IChronoSvc::ChronoTime>(System::kernelTime   ( MyUnit ));
  m_start_elapsed = static_cast<IChronoSvc::ChronoTime>(System::ellapsedTime ( MyUnit ));
  ///
  m_status = IChronoSvc::RUNNING;
  ///
  return m_status ;
}
// ============================================================================
// stop the chrono
// ============================================================================
IChronoSvc::ChronoStatus  ChronoEntity::stop()
{
  if ( IChronoSvc::RUNNING != m_status ) { return m_status ; }
  
  // following lines could be platform dependent!
  
  m_delta_user    = System::userTime     ( MyUnit ) - m_start_user    ;
  m_delta_kernel  = System::kernelTime   ( MyUnit ) - m_start_kernel  ;
  m_delta_elapsed = System::ellapsedTime ( MyUnit ) - m_start_elapsed ;
  
  // update the counters:
  
  m_user     += m_delta_user    ;
  m_kernel   += m_delta_kernel  ;
  m_elapsed  += m_delta_elapsed ;
  
  // set new status
  
  m_status = IChronoSvc::STOPPED;  
  
  return m_status ;
}
// ============================================================================
// print user time
// ============================================================================
std::string ChronoEntity::outputUserTime      () const
{
  std::string res ("Time User   : ") ;
  return res += format 
    ( uTotalTime     () ,
      uMinimalTime   () ,
      uMeanTime      () ,
      uRMSTime       () ,
      uMaximalTime   () ,
      nOfMeasurements() );
}
// ============================================================================
// print system time
// ============================================================================
std::string ChronoEntity::outputSystemTime      () const
{
  std::string res ("Time System : ") ;
  return res += format
    ( kTotalTime     () ,
      kMinimalTime   () ,
			kMeanTime      () ,
			kRMSTime       () ,
			kMaximalTime   () ,
			nOfMeasurements() );
}
// ============================================================================
// print time
std::string ChronoEntity::outputElapsedTime      () const
{
  std::string res ("TimeElapsed: ") ;
  return res += format
    ( eTotalTime     () , 
      eMinimalTime   () , 
      eMeanTime      () , 
      eRMSTime       () , 
      eMaximalTime   () , 
      nOfMeasurements() );  
}
// ============================================================================
// print the chrono  
// ============================================================================
std::string ChronoEntity::format
( const double        total     , 
  const double        minimal   ,
  const double        mean      , 
  const double        rms       , 
  const double        maximal   , 
  const unsigned long number    ) const
{
  ///  
  const int buffer_size         = 256 ;
  char      buffer[buffer_size]       ;
  int       index               = 0   ;
  ///
  long double  unit = 1.0 ;
  std::string cunit = ""  ; 
  ///  
  if     ( total / microsecond  <  500 ) 
  { unit = microsecond ; cunit = " [us]"    ; } 
  else if( total / millisecond  <  500 )
  { unit = millisecond ; cunit = " [ms]"    ; } 
  else if( total /      second  <  500 ) 
  { unit =      second ; cunit = "  [s]"     ; }
  else if( total /      minute  <  500 )
  { unit =      minute ; cunit = "[min]"   ; }
  else if( total /        hour  <  500 ) 
    { unit =        hour ; cunit = "  [h]"     ; }
  else if( total /         day  <   10 )
  { unit =         day ; cunit = "[day]"   ; }
  else if( total /        week  <    5 ) 
  { unit =        week ; cunit = "  [w]"  ; }
  else if( total /       month  <   20 ) 
  { unit =       month ; cunit = "[mon]" ; }
  else                                   
  { unit =        year ; cunit = "  [y]"  ; } 
  ///
  index += sprintf( buffer + index , "Tot=%5.3g" , (double) (total / unit) ) ;
  index += sprintf( buffer + index , "%s" , cunit.c_str()          ) ;
  ///
  if( 1 < number )
  {
    
    if     ( mean / microsecond  <  500 ) 
    { unit = microsecond ; cunit = " [us]"    ; } 
    else if( mean / millisecond  <  500 )
    { unit = millisecond ; cunit = " [ms]"    ; } 
    else if( mean /      second  <  500 ) 
    { unit =      second ; cunit = "  [s]"     ; }
    else if( mean /      minute  <  500 )
    { unit =      minute ; cunit = "[min]"   ; }
    else if( mean /        hour  <  500 ) 
    { unit =        hour ; cunit = "  [h]"     ; }
    else if( mean /         day  <   10 )
    { unit =         day ; cunit = "[day]"   ; }
    else if( mean /        week  <    5 ) 
    { unit =        week ; cunit = "  [w]"  ; }
    else if( mean /       month  <   20 ) 
    { unit =       month ; cunit = "[mon]" ; }
    else                                   
    { unit =        year ; cunit = "[year]"  ; } 
    ///      
    index += 
      sprintf( buffer + index , 
               " Ave/Min/Max=%5.3g" , (double) ( mean / unit ) ) ;
    index += 
      sprintf( buffer + index , 
               "(+-%5.3g)"          , (double) ( rms  / unit ) ) ;
    index += 
      sprintf( buffer + index , 
               "/%5.3g"             , (double) ( minimal  / unit ) ) ;
    index += 
      sprintf( buffer + index , 
               "/%5.3g"             , (double) ( maximal  / unit ) ) ;
    ///
    index += sprintf( buffer + index , "%s" , cunit.c_str() );
    ///
  }
  else
  { index += sprintf( buffer + index , "%44s" , " " ); }    
  ///
  index += sprintf( buffer + index , " #=%3lu" , number );
  ///
  return std::string( buffer, 0 , index );
  ///
}
// ============================================================================
// comparison operator 
// ============================================================================
bool ChronoEntity::operator<( const ChronoEntity& e ) const
{ 
  return 
    ( &e           == this          ) ? false : 
    ( totalTime () < e.totalTime () ) ? true  :
    ( totalTime () > e.totalTime () ) ? false :
    ( m_user       < e.m_user       ) ? true  :
    ( e.m_user     <   m_user       ) ? false : 
    ( m_kernel     < e.m_kernel     ) ? true  :
    ( e.m_kernel   <   m_kernel     ) ? false : 
    ( m_elapsed    < e.m_elapsed    ) ? true  :
    ( e.m_elapsed  <   m_elapsed    ) ? false : false ;
}
// ============================================================================
/*  print the chrono according the format and units 
 *  @param typ  the chrono type 
 *  @param fmt  the format string 
 *  @param unit the unit
 *  @return the string representations 
 *  @see boost::format
 */
// ============================================================================
std::string ChronoEntity::outputTime 
( IChronoSvc::ChronoType typ  , 
  const std::string&     fmt  , 
  System::TimeType       unit ) const 
{
  boost::format _fmt ( fmt ) ;
  // allow various number of arguments 
  using namespace boost::io ;  
  _fmt.exceptions ( all_error_bits ^ ( too_many_args_bit | too_few_args_bit ) ) ;
  //
  double _unit = microsecond ;
  switch ( unit ) 
  {
  case System::Year     : _unit = year        ; break ;
  case System::Month    : _unit = month       ; break ;
  case System::Day      : _unit = day         ; break ;
  case System::Hour     : _unit = hour        ; break ;
  case System::Min      : _unit = minute      ; break ;
  case System::Sec      : _unit = second      ; break ;
  case System::milliSec : _unit = millisecond ; break ;
  case System::microSec : _unit = microsecond ; break ;
  case System::nanoSec  : _unit = nanosecond  ; break ;
  default               : _unit = microsecond ; break ;
  }
  //
  const StatEntity* stat = &m_user;
  switch ( typ ) 
  {
  case IChronoSvc::USER    : stat = &m_user    ; break ;
  case IChronoSvc::KERNEL  : stat = &m_kernel  ; break ;
  case IChronoSvc::ELAPSED : stat = &m_elapsed ; break ;
  default                  : stat = &m_user    ; break ;
  }
  //  
  _fmt 
    %   ( stat -> nEntries    ()         )   // %1 : #entries 
    %   ( stat -> flag        () / _unit )   // %2 : total time 
    %   ( stat -> flagMean    () / _unit )   // %3 : mean time 
    %   ( stat -> flagRMS     () / _unit )   // %4 : RMS  time 
    %   ( stat -> flagMeanErr () / _unit )   // %5 : error in mean time 
    %   ( stat -> flagMin     () / _unit )   // %6 : minimal time 
    %   ( stat -> flagMax     () / _unit ) ; // %7 : maximal time 
  //
  return _fmt.str() ;
}
// ==========================================================================

// ============================================================================
// The END 
// ============================================================================
