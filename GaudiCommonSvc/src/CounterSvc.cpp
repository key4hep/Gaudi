// $Id: CounterSvc.cpp,v 1.6 2007/09/24 19:15:57 marcocle Exp $
// ============================================================================
// CVS tag $Name:  $, verison $Revision: 1.6 $
// ============================================================================
// STD & SLT
// ============================================================================
#include <utility>
#include <vector>
#include <algorithm>
#include <functional>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/ICounterSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/HashMap.h"
// ============================================================================
/** @class CounterSvc
 *  Simple implementation of the abstract interface ICounterSvc
 *  @author Markus FRANK
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-05-25
 */
class CounterSvc: public extends1<Service, ICounterSvc> {
public:
  /// Standard Constructor
  CounterSvc ( const std::string& name   ,
               ISvcLocator*       svcLoc )
    : base_class(name, svcLoc)
    , m_counts ()
    , m_print  ( true )
    //
    // the header row
    , m_header  ( "       Counter :: Group         |     #     |    sum     | mean/eff^* | rms/err^*  |     min     |     max     |")
    // format for regular statistical printout rows
    , m_format1 ( " %|15.15s|%|-15.15s|%|32t||%|10d| |%|11.7g| |%|#11.5g| |%|#11.5g| |%|#12.5g| |%|#12.5g| |"         )
    // format for "efficiency" statistical printout rows
    , m_format2 ( "*%|15.15s|%|-15.15s|%|32t||%|10d| |%|11.5g| |(%|#9.7g| +- %|-#9.7g|)%%|   -------   |   -------   |" )
    // flag to use the special "efficiency" format
    , m_useEffFormat ( true )
    //
  {
    declareProperty ("PrintStat" , m_print ) ;
    //
    declareProperty
      ( "StatTableHeader"        , m_header                          ,
        "The header row for the output Stat-table"                   ) ;
    //
    declareProperty
      ( "RegularRowFormat"       , m_format1                         ,
        "The format for the regular row in the output Stat-table"    ) ;
    //
    declareProperty
      ( "EfficiencyRowFormat"    , m_format2                         ,
        "The format for the regular row in the outptu Stat-table"    ) ;
    //
    declareProperty
      ( "UseEfficiencyRowFormat" , m_useEffFormat                    ,
        "Use the special format for printout of efficiency counters" ) ;
  }
  /// Standard destructor
  virtual ~CounterSvc() { remove().ignore() ; }
  /// Finalization
  virtual StatusCode finalize()
  {
    if ( outputLevel() <= MSG::DEBUG || m_print ) { print () ; }
    remove().ignore() ;
    // finalize the base class
    return Service::finalize() ; ///< finalize the base class
  }
  // ==========================================================================
  /** Access an existing counter object.
   *
   * @param  group         [IN]     Hint for smart printing
   * @param  name          [IN]     Counter name
   *
   * @return Pointer to existing counter object (NULL if non existing).
   */
  virtual Counter* get
  ( const std::string& group ,
    const std::string& name  ) const;
  /// get all counters form the given group:
  virtual ICounterSvc::Counters get ( const std::string& group ) const ;
  /** Create a new counter object. If the counter object exists already
   * the existing object is returned. In this event the return code is
   * COUNTER_EXISTS. The ownership of the actual counter stays with the
   * service.
   *
   * @param  group         [IN]     Hint for smart printing
   * @param  name          [IN]     Counter name
   * @param  initial_value [IN]     Initial counter value
   * @param  refpCounter   [OUT]    Reference to store pointer to counter.
   *
   * @return StatusCode indicating failure or success.
   */
  virtual StatusCode create
  ( const std::string& group ,
    const std::string& name  ,
    longlong initial_value   ,
    Counter*& refpCounter    ) ;
  /** Create a new counter object. If the counter object exists already,
   * a std::runtime_error exception is thrown. The ownership of the
   * actual counter stays with the service.
   *
   * @param  group         [IN]     Hint for smart printing
   * @param  name          [IN]     Counter name
   * @param  initial_value [IN]     Initial counter value
   * @param  refpCounter   [OUT]    Reference to store pointer to counter.
   *
   * @return Fully initialized CountObject.
   */
  virtual CountObject create
  ( const std::string& group    ,
    const std::string& name     ,
    longlong initial_value = 0  ) ;
  /** Remove a counter object. If the counter object does not exists,
   * the return code is COUNTER_NOT_PRESENT. The counter may not
   * be used anymore after this call.
   * @param  group         [IN]     Hint for smart printing
   * @param  name          [IN]     Counter name
   * @param  initial_value [IN]     Initial counter value
   * @param  refpCounter   [OUT]    Reference to store pointer to counter.
   * @return StatusCode indicating failure or success.
   */
  virtual StatusCode remove
  ( const std::string& group ,
    const std::string& name  ) ;
  /** Remove all counters of a given group. If no such counter exists
   * the return code is COUNTER_NOT_PRESENT
   * @param  group         [IN]     Hint for smart printing
   * @return StatusCode indicating failure or success.
   */
  virtual StatusCode remove
  ( const std::string& group ) ;
  /// Remove all known counter objects
  virtual StatusCode remove();
  /** Print counter value
   * @param  group         [IN]     Hint for smart printing
   * @param  name          [IN]     Counter name
   * @param printer        [IN]     Print actor
   *
   * @return StatusCode indicating failure or success.
   */
  virtual StatusCode print
  ( const std::string& group,
    const std::string& name,
    Printout& printer) const;
  /** If no such counter exists the return code is COUNTER_NOT_PRESENT
   * Note: This call is not direct access.
   * @param  group         [IN]     Hint for smart printing
   * @param printer        [IN]     Print actor
   *
   * @return StatusCode indicating failure or success.
   */
  virtual StatusCode print
  (const std::string& group,
   Printout& printer) const;
  /** Print counter value
   * @param pCounter       [IN]     Pointer to Counter object
   * @param printer        [IN]     Print actor
   *
   * @return StatusCode indicating failure or success.
   */
  virtual StatusCode print
  ( const Counter* pCounter,
    Printout& printer) const;
  /** Print counter value
   * @param refCounter     [IN]     Reference to CountObject object
   * @param printer        [IN]     Print actor
   *
   * @return StatusCode indicating failure or success.
   */
  virtual StatusCode print
  ( const CountObject& pCounter,
    Printout& printer) const;
  /** @param printer        [IN]     Print actor
   *  @return StatusCode indicating failure or success.
   */
  virtual StatusCode print(Printout& printer) const;
  /// Default Printout for counters
  virtual StatusCode defaultPrintout
  ( MsgStream& log,
    const Counter* pCounter) const ;
private:
  // find group/name for the counter:
  inline std::pair<std::string,std::string> _find ( const Counter* c ) const
  {
    if ( 0 == c ) { return std::pair<std::string,std::string>() ; }
    for ( CountMap::const_iterator i = m_counts.begin() ; m_counts.end() != i ; ++i )
    {
      for ( NameMap::const_iterator j = i->second.begin() ; i->second.end() != j ; ++j )
      { if ( j->second == c ) { return std::make_pair( i->first , j->first )  ; } }
    }
    return std::pair<std::string,std::string>() ;
  }
  // get the overall number of counters
  inline size_t num () const
  {
    size_t result = 0 ;
    {
      for ( CountMap::const_iterator i = m_counts.begin(); i != m_counts.end(); ++i )
      {
        for ( NameMap::const_iterator j = i->second.begin() ; i->second.end() != j ; ++j )
        { if ( 0 != j->second ) { ++result ; } ; }
      }
    }
    return result ;
  }
public:
  /// "standard" printout a'la GaudiCommon
  void print () const ;
private:
  typedef GaudiUtils::HashMap<std::string,Counter*> NameMap  ;
  typedef GaudiUtils::HashMap<std::string,NameMap>  CountMap ;
  // the actual map of counters
  CountMap m_counts ; ///< the actual map of counters
  // boolean flag to print statistics
  bool     m_print  ; ///< boolean flag to print statistics
  // the header row
  std::string    m_header  ; ///< the header row
  // format for regular statistical printout rows
  std::string    m_format1 ; ///< format for regular statistical printout rows
  // format for "efficiency" statistical printout rows
  std::string    m_format2 ; ///< format for "efficiency" statistical printout rows
  // flag to use the special "efficiency" format
  bool           m_useEffFormat ; ///< flag to use the special "efficiency" format
} ;
// ===========================================================================
// Instantiation of a static factory class used by clients
// ===========================================================================
DECLARE_SERVICE_FACTORY(CounterSvc)
// ===========================================================================
// Access a counter object by name and group
// ===========================================================================
CounterSvc::Counter* CounterSvc::get
( const std::string& grp  ,
  const std::string& nam  ) const
{
  CountMap::const_iterator i = m_counts.find  ( grp ) ;
  if (  m_counts.end() == i ) { return 0 ; }                    // RETURN
  NameMap::const_iterator  j = i->second.find ( nam ) ;
  if ( i->second.end() == j ) { return 0 ; }                    // RETURN
  return j->second ;                                            // RETURN
}
// ===========================================================================
// get all counters form the given group:
// ===========================================================================
ICounterSvc::Counters CounterSvc::get ( const std::string& group ) const
{
  ICounterSvc::Counters result ;
  CountMap::const_iterator i = m_counts.find  ( group ) ;
  if (  m_counts.end() == i ) { return result ; } // RETURN
  for ( NameMap::const_iterator j = i->second.begin() ; i->second.end() != j ; ++j )
  { result.push_back( CountObject ( j->second, i->first , j->first ) ) ; }
  return result ;
}
// ===========================================================================
// Create/get a counter object.
// ===========================================================================
#ifdef __ICC
// disable icc remark #2259: non-pointer conversion from "longlong={long long}" to "double" may lose significant bits
#pragma warning(push)
#pragma warning(disable:2259)
#endif
StatusCode CounterSvc::create
( const std::string& grp ,
  const std::string& nam ,
  longlong initial_value ,
  Counter*& refpCounter  )
{
  // try to find existing counter:
  refpCounter = get ( grp , nam ) ;
  if ( 0 != refpCounter ) { return COUNTER_EXISTS ; }                // RETURN
  // create the new counter
  Counter* newc = new Counter() ;
  refpCounter = newc ;
  if ( 0 != initial_value ) {
    refpCounter->addFlag ( static_cast<double>(initial_value) ) ; // icc remark #2259
  }
  // find a proper group
  CountMap::iterator i = m_counts.find  ( grp ) ;
  // (create a group if needed)
  if (  m_counts.end() == i )
  { i = m_counts.insert ( std::make_pair ( grp , NameMap() ) ).first ; }
  // insert new counter with proper name into proper group:
  i->second.insert( std::make_pair( nam , newc ) );
  return StatusCode::SUCCESS ;                                     // RETURN
}
#ifdef __ICC
// re-enable icc remark #2259
#pragma warning(pop)
#endif
// ===========================================================================
// Create a new counter object. If the counter object exists already,
// ===========================================================================
CounterSvc::CountObject CounterSvc::create
( const std::string& group ,
  const std::string& name  ,
  longlong initial_value   )
{
  Counter* p = 0;
  StatusCode sc = create ( group, name, initial_value, p ) ;
  if ( sc.isSuccess() && 0 != p ) { return CountObject ( p , group , name ) ; }
  throw std::runtime_error("CounterSvc::Counter('"+group+"::"+name+"') exists already!");
}
// ===========================================================================
// Remove a counter object. The tuple (group,name) identifies the counter uniquely
// ===========================================================================
StatusCode CounterSvc::remove
( const std::string& grp ,
  const std::string& nam )
{
  CountMap::iterator i = m_counts.find  ( grp ) ;
  if (  m_counts.end() == i ) { return COUNTER_NOT_PRESENT ; }  // RETURN
  NameMap::iterator  j = i->second.find ( nam ) ;
  if ( i->second.end() == j ) { return COUNTER_NOT_PRESENT ; }  // RETURN
  delete j->second ;
  i->second.erase ( j ) ;
  return StatusCode::SUCCESS ;
}
// ===========================================================================
// Remove a group of counter objects.
// ===========================================================================
StatusCode CounterSvc::remove ( const std::string& grp )
{
  CountMap::iterator i = m_counts.find ( grp  ) ;
  if (  m_counts.end() == i ) { return COUNTER_NOT_PRESENT ; }  // RETURN
  for ( NameMap::iterator j = i->second.begin() ; i->second.end() != j ; ++j )
  { delete j->second ; }
  i->second.clear() ;
  return StatusCode::SUCCESS ;
}
// ===========================================================================
// Remove all known counter objects
// ===========================================================================
StatusCode CounterSvc::remove()
{
  // remove group by group
  for ( CountMap::iterator i = m_counts.begin() ; m_counts.end() != i ; ++i )
  { remove ( i->first ).ignore () ; }
  m_counts.clear() ;
  return StatusCode::SUCCESS;
}
// ===========================================================================
// Print counter value
// ===========================================================================
StatusCode CounterSvc::print
( const std::string& grp,
  const std::string& nam,
  Printout& printer) const
{
  const Counter* c = get( grp , nam ) ;
  if ( 0 == c ) { return COUNTER_NOT_PRESENT ; }                  // RETURN
  // create the stream and use it!
  MsgStream log ( msgSvc() , name() ) ;
  return printer ( log , c ) ;
}

namespace {
  /// Small helper class to add a check on the counter to the Printout class.
  class conditionalPrint {
  private:
    CounterSvc::Printout *printer;
    MsgStream *log;
  public:
    conditionalPrint(CounterSvc::Printout &_p, MsgStream &_l): printer(&_p), log(&_l) {}
    template<class Pair>
    void operator() (const Pair &p) {
      if (p.second) {
        (*printer)(*log, p.second).ignore();
      }
    }
  };
}
// ===========================================================================
// Print the counter value for the whole group of counters
// ===========================================================================
StatusCode CounterSvc::print
( const std::string& grp ,
  Printout& printer      ) const
{
  CountMap::const_iterator i = m_counts.find ( grp ) ;
  if ( m_counts.end() == i ) { return COUNTER_NOT_PRESENT ; }

  MsgStream log(msgSvc(), name());
  // Force printing in alphabetical order
  typedef std::map<std::string, Counter*> sorted_map_t;
  sorted_map_t sorted_map(i->second.begin(), i->second.end());
  std::for_each(sorted_map.begin(), sorted_map.end(),
                conditionalPrint(printer, log));
  return StatusCode::SUCCESS;   // RETURN
}
// ===========================================================================
// Print counter value
// ===========================================================================
StatusCode CounterSvc::print
( const Counter* pCounter,
  Printout& printer ) const
{
  MsgStream log(msgSvc(), name() ) ;
  return printer ( log , pCounter ) ;
}
// ===========================================================================
// Print counter value
// ===========================================================================
StatusCode CounterSvc::print
( const CountObject& refCounter,
  Printout& printer) const
{ return print( refCounter.counter() , printer ) ; }
// ===========================================================================
// Print all known counters
// ===========================================================================
StatusCode CounterSvc::print( Printout& printer ) const
{
  MsgStream log ( msgSvc() , name() ) ;
  // Force printing in alphabetical order
  typedef std::map<std::pair<std::string,std::string>, Counter*> sorted_map_t;
  sorted_map_t sorted_map;
  for ( CountMap::const_iterator i = m_counts.begin(); i != m_counts.end(); ++i )
  {
    for ( NameMap::const_iterator j = i->second.begin() ; i->second.end() != j ; ++j )
    {
      sorted_map[std::make_pair(i->first, j->first)] = j->second;
    }
  }
  std::for_each(sorted_map.begin(), sorted_map.end(),
                conditionalPrint(printer, log));
  return StatusCode::SUCCESS;
}
// ===========================================================================
// Print counter value
// ===========================================================================
StatusCode CounterSvc::defaultPrintout
( MsgStream& log,
  const Counter* c ) const
{
  if ( 0 == c ) { return StatusCode::FAILURE ; }
  std::pair<std::string,std::string> p = _find ( c ) ;

  log << MSG::ALWAYS
      << CountObject( const_cast<Counter*>(c) , p.first , p.second )
      << endmsg ;

  return StatusCode::SUCCESS;
}
// ===========================================================================
// "standard" printout a'la GaudiCommon
// ===========================================================================
void CounterSvc::print () const
{
  MsgStream log ( msgSvc() , name() ) ;
  // number of counters
  const size_t _num = num() ;
  if ( 0 != _num )
  {
    log << MSG::ALWAYS
        << "Number of counters : "  << _num << endmsg
        << m_header << endmsg ;
  }
  {
    // Force printing in alphabetical order
    typedef std::map<std::pair<std::string,std::string>, Counter*> sorted_map_t;
    sorted_map_t sorted_map;
    for ( CountMap::const_iterator i = m_counts.begin(); i != m_counts.end(); ++i )
    {
      for ( NameMap::const_iterator j = i->second.begin() ; i->second.end() != j ; ++j )
      {
        Counter* c = j->second ;
        if ( 0 == c ) { continue ; }
        sorted_map[std::make_pair(i->first, j->first)] = c;
      }
    }
    for (sorted_map_t::const_iterator i = sorted_map.begin(); i != sorted_map.end(); ++i )
      log << Gaudi::Utils::formatAsTableRow( i->first.second
                                           , i->first.first
                                           , *i->second
                                           , m_useEffFormat
                                           , m_format1
                                           , m_format2 )
          << endmsg ;
  }
}
// ============================================================================

// ============================================================================
// The END
// ============================================================================
