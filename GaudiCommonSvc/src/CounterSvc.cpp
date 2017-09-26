// ============================================================================
// STD & SLT
// ============================================================================
#include <algorithm>
#include <functional>
#include <numeric>
#include <utility>
#include <vector>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/HashMap.h"
#include "GaudiKernel/ICounterSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/Stat.h"
// ============================================================================
/** @class CounterSvc
 *  Simple implementation of the abstract interface ICounterSvc
 *  @author Markus FRANK
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-05-25
 */
class CounterSvc : public extends<Service, ICounterSvc>
{
public:
  // inherit constructor
  using extends::extends;
  /// Standard destructor
  ~CounterSvc() override { remove().ignore(); }
  /// Finalization
  StatusCode finalize() override
  {
    if ( msgLevel( MSG::DEBUG ) || m_print ) {
      print();
    }
    remove().ignore();
    // finalize the base class
    return Service::finalize(); ///< finalize the base class
  }
  // ==========================================================================
  /** Access an existing counter object.
   *
   * @param  group         [IN]     Hint for smart printing
   * @param  name          [IN]     Counter name
   *
   * @return Pointer to existing counter object (NULL if non existing).
   */
  Counter* get( const std::string& group, const std::string& name ) const override;
  /// get all counters form the given group:
  ICounterSvc::Counters get( const std::string& group ) const override;
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
  StatusCode create( const std::string& group, const std::string& name, longlong initial_value,
                     Counter*& refpCounter ) override;
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
  CountObject create( const std::string& group, const std::string& name, longlong initial_value = 0 ) override;
  /** Remove a counter object. If the counter object does not exists,
   * the return code is COUNTER_NOT_PRESENT. The counter may not
   * be used anymore after this call.
   * @param  group         [IN]     Hint for smart printing
   * @param  name          [IN]     Counter name
   * @param  initial_value [IN]     Initial counter value
   * @param  refpCounter   [OUT]    Reference to store pointer to counter.
   * @return StatusCode indicating failure or success.
   */
  StatusCode remove( const std::string& group, const std::string& name ) override;
  /** Remove all counters of a given group. If no such counter exists
   * the return code is COUNTER_NOT_PRESENT
   * @param  group         [IN]     Hint for smart printing
   * @return StatusCode indicating failure or success.
   */
  StatusCode remove( const std::string& group ) override;
  /// Remove all known counter objects
  virtual StatusCode remove();
  /** Print counter value
   * @param  group         [IN]     Hint for smart printing
   * @param  name          [IN]     Counter name
   * @param printer        [IN]     Print actor
   *
   * @return StatusCode indicating failure or success.
   */
  StatusCode print( const std::string& group, const std::string& name, Printout& printer ) const override;
  /** If no such counter exists the return code is COUNTER_NOT_PRESENT
   * Note: This call is not direct access.
   * @param  group         [IN]     Hint for smart printing
   * @param printer        [IN]     Print actor
   *
   * @return StatusCode indicating failure or success.
   */
  StatusCode print( const std::string& group, Printout& printer ) const override;
  /** Print counter value
   * @param pCounter       [IN]     Pointer to Counter object
   * @param printer        [IN]     Print actor
   *
   * @return StatusCode indicating failure or success.
   */
  StatusCode print( const Counter* pCounter, Printout& printer ) const override;
  /** Print counter value
   * @param refCounter     [IN]     Reference to CountObject object
   * @param printer        [IN]     Print actor
   *
   * @return StatusCode indicating failure or success.
   */
  StatusCode print( const CountObject& pCounter, Printout& printer ) const override;
  /** @param printer        [IN]     Print actor
   *  @return StatusCode indicating failure or success.
   */
  StatusCode print( Printout& printer ) const override;
  /// Default Printout for counters
  StatusCode defaultPrintout( MsgStream& log, const Counter* pCounter ) const override;

private:
  // find group/name for the counter:
  inline std::pair<std::string, std::string> _find( const Counter* c ) const
  {
    if ( c ) {
      for ( const auto& i : m_counts ) {
        auto j = std::find_if( i.second.begin(), i.second.end(),
                               [&]( const NameMap::value_type& k ) { return k.second == c; } );
        if ( j != i.second.end() ) return {i.first, j->first};
      }
    }
    return {};
  }
  // get the overall number of counters
  inline size_t num() const
  {
    return std::accumulate( m_counts.begin(), m_counts.end(), size_t{0}, []( size_t r, const CountMap::value_type& i ) {
      return r + std::count_if( i.second.begin(), i.second.end(),
                                []( const NameMap::value_type& j ) -> bool { return j.second; } );
    } );
  }

public:
  /// "standard" printout a'la GaudiCommon
  void print() const;

private:
  typedef GaudiUtils::HashMap<std::string, Counter*> NameMap;
  typedef GaudiUtils::HashMap<std::string, NameMap> CountMap;
  // the actual map of counters
  CountMap m_counts; ///< the actual map of counters

  Gaudi::Property<bool> m_print{this, "PrintStat", true, "print statistics"};
  Gaudi::Property<std::string> m_header{this, "StatTableHeader",
                                        "       Counter :: Group         |     #     |    sum     | "
                                        "mean/eff^* | rms/err^*  |     min     |     max     |",
                                        "the header row for the output Stat-table"};
  Gaudi::Property<std::string> m_format1{
      this, "RegularRowFormat",
      " %|15.15s|%|-15.15s|%|32t||%|10d| |%|11.7g| |%|#11.5g| |%|#11.5g| |%|#12.5g| |%|#12.5g| |",
      "the format for the regular row in the output Stat-table"};
  Gaudi::Property<std::string> m_format2{
      this, "EfficiencyRowFormat",
      "*%|15.15s|%|-15.15s|%|32t||%|10d| |%|11.5g| |(%|#9.7g| +- %|-#9.7g|)%%|   -------   |   -------   |",
      "the format for the regular row in the outptu Stat-table"};
  Gaudi::Property<bool> m_useEffFormat{this, "UseEfficiencyRowFormat", true,
                                       "use the special format for printout of efficiency counters"};
};
// ===========================================================================
// Instantiation of a static factory class used by clients
// ===========================================================================
DECLARE_COMPONENT( CounterSvc )
// ===========================================================================
// Access a counter object by name and group
// ===========================================================================
CounterSvc::Counter* CounterSvc::get( const std::string& grp, const std::string& nam ) const
{
  auto i = m_counts.find( grp );
  if ( m_counts.end() == i ) {
    return nullptr;
  } // RETURN
  auto j = i->second.find( nam );
  if ( i->second.end() == j ) {
    return nullptr;
  }                 // RETURN
  return j->second; // RETURN
}
// ===========================================================================
// get all counters form the given group:
// ===========================================================================
ICounterSvc::Counters CounterSvc::get( const std::string& group ) const
{
  ICounterSvc::Counters result;
  auto i = m_counts.find( group );
  if ( i != m_counts.end() ) {
    std::transform( i->second.begin(), i->second.end(), std::back_inserter( result ),
                    [&]( const NameMap::value_type& j ) {
                      return CountObject{j.second, i->first, j.first};
                    } );
  }
  return result;
}
// ===========================================================================
// Create/get a counter object.
// ===========================================================================
#ifdef __ICC
// disable icc remark #2259: non-pointer conversion from "longlong={long long}" to "double" may lose significant bits
#pragma warning( push )
#pragma warning( disable : 2259 )
#endif
StatusCode CounterSvc::create( const std::string& grp, const std::string& nam, longlong initial_value,
                               Counter*& refpCounter )
{
  // try to find existing counter:
  refpCounter = get( grp, nam );
  if ( refpCounter ) {
    return COUNTER_EXISTS;
  } // RETURN
  // create the new counter
  auto newc   = new Counter();
  refpCounter = newc;
  if ( 0 != initial_value ) {
    refpCounter->addFlag( static_cast<double>( initial_value ) ); // icc remark #2259
  }
  // find a proper group
  auto i = m_counts.find( grp );
  // (create a group if needed)
  if ( m_counts.end() == i ) {
    i = m_counts.emplace( grp, NameMap() ).first;
  }
  // insert new counter with proper name into proper group:
  i->second.emplace( nam, newc );
  return StatusCode::SUCCESS; // RETURN
}
#ifdef __ICC
// re-enable icc remark #2259
#pragma warning( pop )
#endif
// ===========================================================================
// Create a new counter object. If the counter object exists already,
// ===========================================================================
CounterSvc::CountObject CounterSvc::create( const std::string& group, const std::string& name, longlong initial_value )
{
  Counter* p    = nullptr;
  StatusCode sc = create( group, name, initial_value, p );
  if ( sc.isSuccess() && p ) {
    return CountObject( p, group, name );
  }
  throw std::runtime_error( "CounterSvc::Counter('" + group + "::" + name + "') exists already!" );
}
// ===========================================================================
// Remove a counter object. The tuple (group,name) identifies the counter uniquely
// ===========================================================================
StatusCode CounterSvc::remove( const std::string& grp, const std::string& nam )
{
  auto i = m_counts.find( grp );
  if ( m_counts.end() == i ) {
    return COUNTER_NOT_PRESENT;
  } // RETURN
  auto j = i->second.find( nam );
  if ( i->second.end() == j ) {
    return COUNTER_NOT_PRESENT;
  } // RETURN
  delete j->second;
  i->second.erase( j );
  return StatusCode::SUCCESS;
}
// ===========================================================================
// Remove a group of counter objects.
// ===========================================================================
StatusCode CounterSvc::remove( const std::string& grp )
{
  auto i = m_counts.find( grp );
  if ( m_counts.end() == i ) {
    return COUNTER_NOT_PRESENT;
  } // RETURN
  for ( auto& j : i->second ) delete j.second;
  i->second.clear();
  return StatusCode::SUCCESS;
}
// ===========================================================================
// Remove all known counter objects
// ===========================================================================
StatusCode CounterSvc::remove()
{
  // remove group by group
  for ( auto& i : m_counts ) remove( i.first ).ignore();
  m_counts.clear();
  return StatusCode::SUCCESS;
}
// ===========================================================================
// Print counter value
// ===========================================================================
StatusCode CounterSvc::print( const std::string& grp, const std::string& nam, Printout& printer ) const
{
  const Counter* c = get( grp, nam );
  if ( !c ) {
    return COUNTER_NOT_PRESENT;
  } // RETURN
  // create the stream and use it!
  return printer( msgStream(), c );
}

namespace
{
  /// Small helper class to add a check on the counter to the Printout class.
  class conditionalPrint
  {
  private:
    CounterSvc::Printout* printer;
    MsgStream* log;

  public:
    conditionalPrint( CounterSvc::Printout& _p, MsgStream& _l ) : printer( &_p ), log( &_l ) {}
    template <class Pair>
    void operator()( const Pair& p )
    {
      if ( p.second ) {
        ( *printer )( *log, p.second ).ignore();
      }
    }
  };
}
// ===========================================================================
// Print the counter value for the whole group of counters
// ===========================================================================
StatusCode CounterSvc::print( const std::string& grp, Printout& printer ) const
{
  auto i = m_counts.find( grp );
  if ( m_counts.end() == i ) {
    return COUNTER_NOT_PRESENT;
  }

  // Force printing in alphabetical order
  std::map<std::string, Counter*> sorted_map( i->second.begin(), i->second.end() );
  std::for_each( sorted_map.begin(), sorted_map.end(), conditionalPrint( printer, msgStream() ) );
  return StatusCode::SUCCESS; // RETURN
}
// ===========================================================================
// Print counter value
// ===========================================================================
StatusCode CounterSvc::print( const Counter* pCounter, Printout& printer ) const
{
  return printer( msgStream(), pCounter );
}
// ===========================================================================
// Print counter value
// ===========================================================================
StatusCode CounterSvc::print( const CountObject& refCounter, Printout& printer ) const
{
  return print( refCounter.counter(), printer );
}
// ===========================================================================
// Print all known counters
// ===========================================================================
StatusCode CounterSvc::print( Printout& printer ) const
{
  // Force printing in alphabetical order
  std::map<std::pair<std::string, std::string>, Counter*> sorted_map;
  for ( const auto& i : m_counts )
    for ( const auto& j : i.second ) sorted_map[{i.first, j.first}] = j.second;
  std::for_each( sorted_map.begin(), sorted_map.end(), conditionalPrint( printer, msgStream() ) );
  return StatusCode::SUCCESS;
}
// ===========================================================================
// Print counter value
// ===========================================================================
StatusCode CounterSvc::defaultPrintout( MsgStream& log, const Counter* c ) const
{
  if ( !c ) {
    return StatusCode::FAILURE;
  }
  auto p = _find( c );

  log << MSG::ALWAYS << CountObject( const_cast<Counter*>( c ), p.first, p.second ) << endmsg;

  return StatusCode::SUCCESS;
}
// ===========================================================================
// "standard" printout a'la GaudiCommon
// ===========================================================================
void CounterSvc::print() const
{
  // number of counters
  const auto _num = num();
  if ( 0 != _num ) {
    always() << "Number of counters : " << _num << endmsg << m_header.value() << endmsg;
  }
  // Force printing in alphabetical order
  std::map<std::pair<std::string, std::string>, Counter*> sorted_map;
  for ( const auto& i : m_counts ) {
    for ( const auto& j : i.second ) {
      if ( j.second ) sorted_map[{i.first, j.first}] = j.second;
    }
  }
  for ( const auto& i : sorted_map ) {
    always() << Gaudi::Utils::formatAsTableRow( i.first.second, i.first.first, *i.second, m_useEffFormat, m_format1,
                                                m_format2 )
             << endmsg;
  }
}
// ============================================================================

// ============================================================================
// The END
// ============================================================================
