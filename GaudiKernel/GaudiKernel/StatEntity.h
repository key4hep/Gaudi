// $Id: StatEntity.h,v 1.7 2007/08/06 08:39:40 marcocle Exp $
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.7 $
// ============================================================================
#ifndef   GAUDIKERNEL_STATENTITY_H
#define   GAUDIKERNEL_STATENTITY_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <string>
#include <iostream>
// ============================================================================
// Gaudi
// ============================================================================
#include "GaudiKernel/Kernel.h"
// ============================================================================
/** @class StatEntity StatEntity.h GaudiKernel/StatEntity.h
 *
 *  The basic counter used for Monitoring purposes.
 *
 *  It is used as a small helper class for implementation of
 *  class ChronoStatSvc but it also could be used in stand-alone
 *  mode to perform trivial statistical evaluations. e.g.
 *
 *  Essentially the generic counter could be considered as
 *  the trivial 1-bin
 *
 *  @code
 *
 *   // get all tracks
 *   const Tracks* tracks = ... ;
 *   // create the counter
 *   StatEntity chi2 ;
 *   // make a loop over all tracks:
 *   for ( Tracks::const_iterator itrack = tracks->begin() ;
 *         tracks->end() != itrack ; ++itrack )
 *    {
 *        const Track* track = *itrack ;
 *
 *        // use the counters to accumulate information:
 *        chi2 += track -> chi2  () ;
 *    } ;
 *
 *   // Extract the information from the counter:
 *
 *   // get number of entries (== number of tracks)
 *   int nEntries = chi2.nEntries() ;
 *
 *   // get the minimal value of chi2
 *   double chi2Min = chi2.flagMin() ;
 *
 *   // get the average value of chi2:
 *   double meanChi2 = chi2.flagMean() ;
 *
 *   // get R.M.S. for chi2-distribution:
 *   double rmsChi2   = chi2.flagRMS() ;
 *
 *   .. etc...
 *
 *  @endcode
 *
 *  @author  Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
 *  @date    26/11/1999
 *  @date    2005-08-02
 */
class GAUDI_API StatEntity
{
public:
  // ==========================================================================
  /// the default constructor
  StatEntity  () { reset() ; }
  /* The full constructor from all important values:
   * @attention it need to be coherent with
   *            the actual structure of the class
   *            and the format description
   *  @see StatEntity::format
   *  @param entries number of entries
   *  @param flag  accumulated flag
   *  @param flag2 accumulated statistics: flag**2
   *  @param minFlag the minimum value for flag
   *  @param maxFlag the maximum value for flag
   */
  StatEntity ( const unsigned long entries ,
               const double        flag    ,
               const double        flag2   ,
               const double        minFlag ,
               const double        maxFlag ) ;
  /// destructor
  ~StatEntity () {}
  // ==========================================================================
public: // the basic quantities
  // ==========================================================================
  /// getters
  const unsigned long& nEntries () const { return m_se_nEntries         ; }
  /// accumulated value
  const double&        sum      () const { return m_se_accumulatedFlag  ; }
  /// accumulated value**2
  const double&        sum2     () const { return m_se_accumulatedFlag2 ; }
  /// mean value of counter
  double               mean     () const ;
  /// r.m.s of value
  double               rms      () const ;
  /// error in mean value of counter
  double               meanErr  () const ;
  /// minimal value
  const double&        min      () const { return m_se_minimalFlag ; }
  /// maximal value
  const double&        max      () const { return m_se_maximalFlag ; }
  // ==========================================================================
public:
  // ==========================================================================
  /** Interpret the counter as some measure of efficiency
   *  The efficiency is calculated as the ratio of the weight
   *  over the number of entries
   *  One gets the correct interpretation in the case of
   *  filling the counters only with 0 and 1.
   *  Some checks are performed:
   *   - number of counts must be positive
   *   - "flag" must be non-negative
   *   - "flag" does not exceed the overall number of counts
   *
   *  If these conditions are not satisfied the method returns -1,
   *  otherwise it returns the value of "flagMean"
   *
   *  @code
   *
   *  StatEntity& stat = ... ;
   *
   *  for ( TRACKS::iterator itrack = tracks.begin() ;
   *        tracks.end() != itrack ; itrack )
   *   {
   *     const bool good = PT( *itrack ) > 1 * Gaudi::Units::GeV ;
   *     stat += good ;
   *   }
   *
   *  std::cout << " Efficiency of PT-cut is "
   *            << stat.efficiency() << std::endl ;
   *
   *  @endcode
   *  @see StatEntity::flagMean
   *  @see StatEntity::eff
   *  @see StatEntity::efficiencyErr
   */
  double efficiency () const ;
  /** Interpret the counter as some measure of efficiency and evaluate the
   *  uncertainty of this efficiency using <b>binomial</b> estimate.
   *  The efficiency is calculated as the ratio of the weight
   *  over the number of entries
   *  One gets the correct interpretation in the case of
   *  filling the counters only with 0 and 1.
   *  Some checks are performed:
   *   - number of counts must be positive
   *   - "flag" must be non-negative
   *   - "flag" does not exceed the overall number of counts
   *
   *  If these conditions are not satisfied the method returns -1.
   *
   *  @attention The action of this method is <b>DIFFERENT</b> from the action of
   *             the method StatEntity::flagMeanErr
   *
   *  @code
   *
   *  StatEntity& stat = ... ;
   *
   *  for ( TRACKS::iterator itrack = tracks.begin() ;
   *        tracks.end() != itrack ; itrack )
   *   {
   *     const bool good = PT( *itrack ) > 1 * Gaudi::Units::GeV ;
   *     stat += good ;
   *   }
   *
   *  std::cout << " Efficiency of PT-cut is "
   *            << stat.efficiency    () << "+-"
   *            << stat.efficiencyErr () << std::endl ;
   *
   *  @endcode
   *  @see StatEntity::efficiency
   *  @see StatEntity::effErr
   *  @see StatEntity::flagMeanErr
   */
  double efficiencyErr () const ;
  /// shortcut, @see StatEntity::efficiency
  double eff           () const { return efficiency    () ; }
  /// shortcut, @see StatEntity::efficiencyErr
  double effErr        () const { return efficiencyErr () ; }
  // ==========================================================================
public: // operators
  // ==========================================================================
  /** General increment operator for the flag
   *  Could be used for easy manipulation with StatEntity object:
   *
   *  @code
   *
   *   StatEntity stat ;
   *
   *   for ( int i =    ... )
   *    {
   *       double eTotal = .... ;
   *
   *       // increment the counter
   *       stat += eTotal ;
   *    }
   *
   *  @endcode
   *
   *  @param f counter increment
   */
  StatEntity& operator+= ( const double f )
  {
    addFlag ( f ) ;
    return *this ;
  }
  /**  Pre-increment operator for the flag
   *   Could be used for easy manipulation with StatEntity object:
   *
   *  @code
   *
   *   StatEntity stat ;
   *
   *   for ( int i =    ... )
   *    {
   *       double eTotal = .... ;
   *
   *       // increment the counter
   *       if ( eTotal > 1 * TeV ) { ++stat ; } ;
   *    }
   *
   *  @endcode
   */
  StatEntity& operator++ ()    { return   (*this)+= 1  ; }
  /** Post-increment operator for the flag.
   *  Actually it is the same as pre-increment.
   *  Could be used for easy manipulation with StatEntity object:
   *
   *  @code
   *
   *   StatEntity stat ;
   *
   *   for ( int i =    ... )
   *    {
   *       double eTotal = .... ;
   *
   *       // increment the counter
   *       if ( eTotal > 1 * TeV ) { stat++ ; } ;
   *    }
   *
   *  @endcode
   */
  StatEntity& operator++ (int) { return ++(*this)      ; }
  /** General decrement operator for the flag
   *  Could be used for easy manipulation with StatEntity object:
   *
   *  @code
   *
   *   StatEntity stat ;
   *
   *   for ( int i =    ... )
   *    {
   *       double eTotal = .... ;
   *
   *       // decrement the counter
   *       stat -= eTotal ;
   *    }
   *
   *  @endcode
   *
   *  @param f counter increment
   */
  StatEntity& operator-= ( const double   f )
  {
    addFlag ( -f ) ;
    return *this ;
  }
  /** Pre-decrement operator for the flag
   *   Could be used for easy manipulation with StatEntity object:
   *
   *  @code
   *
   *   StatEntity stat ;
   *
   *   for ( int i =    ... )
   *    {
   *       double eTotal = .... ;
   *
   *       // increment the counter
   *       if ( eTotal > 1 * TeV ) { --stat ; } ;
   *    }
   *
   *  @endcode
   */
  StatEntity& operator-- () { return (*this)-=1  ; }
  /** Post-decrement operator for the flag
   *  Could be used for easy manipulation with StatEntity object:
   *
   *  @code
   *
   *   StatEntity stat ;
   *
   *   for ( int i =    ... )
   *    {
   *       double eTotal = .... ;
   *
   *       // increment the counter
   *       if ( eTotal > 1 * TeV ) { stat-- ; } ;
   *    }
   *
   *  @endcode
   */
  StatEntity& operator-- (int) { return --(*this) ; }
  /** Assignment from the flag
   *  The action: reset and the general increment
   *  Such case could be useful for statistical monitoring
   *
   *  @code
   *
   *  const long numberOfHits = ... ;
   *
   *  StatEntity& stat = ... ;
   *
   *  stat = numberOfHits ;
   *
   *  @endcode
   *
   *  @param f new value of the counter
   *  @return self-reference
   */
  StatEntity& operator= ( const double   f )
  {
    // reset the statistics
    reset() ;            ///< reset the statistics
    // use the regular increment
    return ((*this)+=f); ///< use the regular increment
  }
  /** increment with other counter (useful for Monitoring/Adder )
   *
   *  @code
   *
   *   const StatEntity second = ... ;
   *
   *   StatEntity first = ... ;
   *
   *   first += second ;
   *
   *  @endcode
   *
   *  @param other counter to be added
   *  @return self-reference
   */
  StatEntity& operator+= ( const StatEntity& other ) ;
  // ==========================================================================
public:
  // ==========================================================================
  /// comparison
  bool operator<( const StatEntity& se ) const ;
  /** add a value
   *  @param Flag value to be added
   *  @return number of entries
   */
  unsigned long add ( const double Flag ) ;
  /// reset the counters
  void reset () ;
  /// DR specify number of entry before reset
  void setnEntriesBeforeReset ( unsigned long nEntriesBeforeReset );
  /// representation as string
  std::string   toString () const;
  /** printout  to std::ostream
   *  @param s the reference to the output stream
   */
  std::ostream& print      ( std::ostream& o = std::cout ) const ;
  /** printout  to std::ostream
   *  @param s the reference to the output stream
   */
  std::ostream& fillStream ( std::ostream& o ) const { return print ( o ) ; }
  // ==========================================================================
public: // aliases (a'la ROOT)
  // ==========================================================================
  /// get sum
  inline double Sum     () const { return sum     () ; } // get sum
  /// get mean
  inline double Mean    () const { return mean    () ; } // get mean
  /// get error in mean
  inline double MeanErr () const { return meanErr () ; } // get error in mean
  /// get rms
  inline double Rms     () const { return rms     () ; } // get rms
  /// get rms
  inline double RMS     () const { return rms     () ; } // get rms
  /// get efficiency
  inline double Eff     () const { return eff     () ; } // get efficiency
  /// get minimal value
  inline double Min     () const { return min     () ; } // get minimal value
  /// get maximal value
  inline double Max     () const { return max     () ; } // get maximal value
  // ==========================================================================
public: // some legacy methods, to be removed ...
  // ==========================================================================
  /// accumulated "flag"
  inline double   flag               () const { return sum     () ; }
  /// accumulated "flag squared"
  inline double   flag2              () const { return sum2    () ; }
  /// mean value of flag
  inline double   flagMean           () const { return mean    () ; }
  /// r.m.s of flag
  inline double   flagRMS            () const { return rms     () ; }
  /// error in mean value of flag
  inline double   flagMeanErr        () const { return meanErr () ; }
  /// minimal flag
  inline double   flagMin            () const { return min     () ; }
  /// maximal flag
  inline double   flagMax            () const { return max     () ; }
  /** add a flag
   *  @param Flag value to be added
   *  @return number of entries
   */
  inline unsigned long addFlag ( const double Flag ) { return add ( Flag ) ; }
  // ==========================================================================
public:
  // ==========================================================================
  /** the internal format description
   *  @attention: it needs to be coherent with the actual
   *              class structure!
   *  It is useful for DIM publishing
   *  @see StatEntity::size
   */
  static const std::string& format () ;
  /** the actual size of published data
   *  @attention: it needs to be coherent with the actual
   *              class structure!
   *  It is useful for DIM publishing
   *  @see StatEntity::format
   */
  static int                size  () ;
  // ==========================================================================
private:
  // ==========================================================================
  /// number of calls
  unsigned long                m_se_nEntries          ;
  /// accumulated flag
  double                       m_se_accumulatedFlag   ;
  double                       m_se_accumulatedFlag2  ;
  double                       m_se_minimalFlag       ;
  double                       m_se_maximalFlag       ;
  // DR number of calls before reset
  long                         m_se_nEntriesBeforeReset ;
  // ==========================================================================
};
// ============================================================================
/// external operator for addition of StatEntity and a number
GAUDI_API StatEntity operator+( const StatEntity& entity , const double      value  ) ;
/// external operator for addition of StatEntity and a number
GAUDI_API StatEntity operator+( const double      value  , const StatEntity& entity ) ;
/// external operator for addition of StatEntity and a number
GAUDI_API StatEntity operator+( const StatEntity& entity , const StatEntity& value  ) ;
/// external operator for subtraction of StatEntity and a number
GAUDI_API StatEntity operator-( const StatEntity& entity , const double      value  ) ;
/// external printout operator to std::ostream
GAUDI_API std::ostream& operator<<( std::ostream& stream , const StatEntity& entity ) ;
// ============================================================================
namespace Gaudi
{
  // ==========================================================================
  namespace Utils
  {
    // ========================================================================
    /** print the counter in a form of the table row
     *
     *  @code
     *
     *    // loop over counters:
     *    for ( ... )
     *     {
     *      const StatEntity&  counter = ... ;
     *      info() << formatAsTableRow( counter , true ) << endmsg ;
     *     }
     *
     *  @endcode
     *
     *  The regular counter is printed as 6 fields
     *    - number of entries      (long)
     *    - overal sum             (double)
     *    - mean value             (double)
     *    - RMS                    (double)
     *    - minimal value          (double)
     *    - maximal value          (double)
     *
     *  The printout is performed according to the format, described
     *  in optional "format1" parameter using Boost Format Library.
     *  The incredible flexibility of Boost Format Library allows
     *  to customize the overall layout of the table form
     *  minor change the format of individual columns to the overall
     *  reshuffling of the reformatted columns
     *
     *  If the parameter "flag" is set to <c>true</c>, <b>AND</b>
     *  the content of the counter allows the interpretation of
     *  the counter as binomial efficiency counter,  the printout
     *  is performed in a form of 4 numbers:
     *     - number of entries                           (long)
     *     - overall sum                                 (double)
     *     - binomial efficiency [perCent]               (double)
     *     - uncertainty in binomial efficiency [perCent] (double)
     *
     *  The printout is performed according to format,
     *  specified by parameter "format2"
     *
     *  @see http://boost.org/libs/format
     *
     *  @param name    the name associated with the counter
     *  @param counter counter to be printed
     *  @param flag    use the special format for "efficiency" rows
     *  @param format1 row format for the regular rows
     *  @param format2 special row format for the "efficiency" rows
     *  @return formatted row in the table
     */
    GAUDI_API std::string formatAsTableRow
    ( const StatEntity&  counter ,
      const bool         flag    ,
      const std::string& format1 =
      " |%|7d| |%|11.7g| |%|#11.5g| |%|#10.5g| |%|#10.5g| |%|#10.5g| |"        ,
      const std::string& format2 =
      "*|%|7d| |%|11.5g| |(%|#9.7g| +- %|-#8.6g|)%%|   -----   |   -----   |" );
    // ========================================================================
    /** print the counter in a form of the table row
     *
     *  @code
     *
     *  std::string header = "     Counter     |     #     |    sum     |" ;
     *  header += " mean/eff^* | rms/err^*  |     min     |     max     |" ;
     *
     *   info() << header << endmsg ;
     *
     *    // loop over counters:
     *    for ( ... )
     *     {
     *      const std::string& name    = ... ;
     *      const StatEntity&  counter = ... ;
     *      info() << formatAsTableRow( name , counter ) << endmsg ;
     *     }
     *
     *  @endcode
     *
     *  The table row for the regular counter is printed as 7 fields
     *    - counter name           (string)
     *    - number of entries      (long)
     *    - overal sum             (double)
     *    - mean value             (double)
     *    - RMS                    (double)
     *    - minimal value          (double)
     *    - maximal value          (double)
     *
     *  The printout is performed according to the format, described
     *  in optional "format1" parameter using Boost Format Library.
     *  The incredible flexibility of Boost Format Library allows
     *  to customize the overall layout of the table form
     *  minor change the format of individual columns to the overall
     *  reshuffling of the reformatted columns
     *
     *  If the parameter "flag" is set to <c>true</c> <b>AND</b>
     *  the counter name, converted to the lowercase constant one
     *   of the substrings: "eff","acc","fltr","filt","pass"
     *  <b>AND</b> (of course) the content of the counter allows the
     *  interpretation of the counter as the binomial efficiency counter,
     *  the printout is performed using 5 fields
     *   ("binomial efficiency format")
     *     - counter name                                (string)
     *     - number of entries                           (long)
     *     - overall sum                                 (double)
     *     - binomial efficiency [perCent]               (double)
     *     - uncertainty in binomial efficiency [perCent] (double)
     *
     *  The printout is performed according to format,
     *  specified by parameter "format2"
     *
     *  @see http://boost.org/libs/format
     *
     *  @param name    the name associated with the counter
     *  @param counter counter to be printed
     *  @param flag    use the special format for "efficiency" rows
     *  @param format1 row format for the regular rows
     *  @param format2 special row format for the "efficiency" rows
     *  @return formatted row in the table
     */
    GAUDI_API std::string formatAsTableRow
    ( const std::string& name           ,
      const StatEntity&  counter        ,
      const bool         flag    = true ,
      const std::string& format1 =
      " %|-15.15s|%|17t||%|7d| |%|11.7g| |%|#11.5g| |%|#10.5g| |%|#10.5g| |%|#10.5g| |"       ,
      const std::string& format2 =
      "*%|-15.15s|%|17t||%|7d| |%|11.5g| |(%|#9.7g| +- %|-#8.6g|)%%|   -----   |   -----   |" );
    // ========================================================================
    /** print the counter in a form of the table row
     *
     *  @code
     *
     *   std::string header = "       Counter :: Group         |     #     |";
     *   header += "    sum     | mean/eff^* | rms/err^*  |     min     |";
     *   header += "     max     |") ;
     *
     *   info() << header << endmsg ;
     *
     *    // loop over counters:
     *    for ( ... )
     *     {
     *      const std::string& name    = ... ;
     *      const std::string& group   = .. ;
     *      const StatEntity&  counter = ... ;
     *      info() << formatAsTableRow ( name , group , counter ) << endmsg ;
     *     }
     *
     *  @endcode
     *
     *
     *  The table row for the regular counter is printed as 8 fields
     *    - counter name           (string)
     *    - counter group          (string)
     *    - number of entries      (long)
     *    - overall sum            (double)
     *    - mean value             (double)
     *    - RMS                    (double)
     *    - minimal value          (double)
     *    - maximal value          (double)
     *
     *  The printout is performed according to the format, described
     *  in optional "format1" parameter using Boost Format Library.
     *  The incredible flexibility of Boost Format Library allows
     *  to customize the overall layout of the table form
     *  minor change the format of individual columns to the overall
     *  reshuffling of the reformatted columns
     *
     *  If the parameter "flag" is set to <c>true</c> <b>AND</b>
     *  either the counter name or counter group, converted to
     *  the lowercase contains one of the substrings:
     *  "eff","acc","fltr","filt","pass"
     *  <b>AND</b> (of course) the content of the counter allows the
     *  interpretation of the counter as the binomial efficiency counter,
     *  the printout is performed using 6 fields
     *   ("binomial efficiency format")
     *     - counter name                                (string)
     *     - counter group                               (string)
     *     - number of entries                           (long)
     *     - overall sum                                 (double)
     *     - binomial efficiency [perCent]               (double)
     *     - uncertainty in binomial efficiency [perCent] (double)
     *
     *  The printout is performed according to format,
     *  specified by parameter "format2"
     *
     *  @see http://boost.org/libs/format
     *
     *  @param name the name associated with the counter
     *  @param group the group associated with the counter
     *  @param counter counter to be printed
     *  @param flag    use the special format for "efficiency" rows
     *  @param format1 row format for the regular rows
     *  @param format2 the special row format for the "efficiency" rows
     *  @return formatted row in the table
     */
    GAUDI_API std::string formatAsTableRow
    ( const std::string& name           ,
      const std::string& group          ,
      const StatEntity&  entity         ,
      const bool         flag    = true ,
      const std::string& format1 =
      " %|15.15s|%|-15.15s|%|32t||%|7d| |%|11.7g| |%|#11.5g| |%|#10.5g| |%|#10.5g| |%|#10.5g| |"         ,
      const std::string& format2 =
      "*%|15.15s|%|-15.15s|%|32t||%|7d| |%|11.5g| |(%|#9.7g| +- %|-#8.6g|)%%|    -----    |   -----   |" ) ;
    // ========================================================================
  } //                                            end of namespace Gaudi::Utils
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIKERNEL_STATENTITY_H
// ============================================================================






