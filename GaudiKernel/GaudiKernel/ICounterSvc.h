// $Id: ICounterSvc.h,v 1.4 2007/08/06 08:39:40 marcocle Exp $
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.4 $
// ============================================================================
#ifndef GAUDIKERNEL_ICOUNTERSVC_H
#define GAUDIKERNEL_ICOUNTERSVC_H
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <string>
#include <vector>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/StatEntity.h"
#include "GaudiKernel/Stat.h"
// ============================================================================
// Forward declarations
// ============================================================================
class MsgStream   ;
class StatEntity  ;
class ICounterSvc ;
// ============================================================================
/** @class ICounterSvc ICounterSvc.h GaudiKernel/ICounterSvc.h
 *
 * Create / access multi purpose counters.
 *
 *  Counter Creation:
 *  -----------------
 * @code
 *
 *   // counter creation:
 *   ICounterSvc::CountObject obj = svc->create("AlgName","NumExecuted",0);
 *
 *    obj++;
 *    obj += 10;
 *
 *    ICounterSvc::Counter*  m_counter = obj.counter();
 *    // ICounterSvc::Counter*  m_counter = obj ; ///< the same
 *
 *    ICounterSvc::Counter* m_counter = 0;
 *    if ( svc->create("AlgName","NumExecuted",0, m_counter).isSuccess() )
 *    {
 *       CountObject obj(m_counter);
 *       obj++;
 *    }
 *
 * @endcode
 *
 *  Counter access:
 *  ---------------
 *
 * @code
 *
 *   ICounterSvc::Counter* m_counter = svc->get("AlgName","NumExecuted");
 *   if ( m_counter )  {....}
 *
 *   // or:
 *
 *   try {
 *     ICounterSvc::CountObject obj(svc->get("AlgName","NumExecuted"));
 *     obj++;
 *   }
 *   catch(std::invalid_argument e)  {
 *      ...handle exception ...
 *   }
 *
 *  @endcode
 *
 *
 *  @author  Markus Frank
 *  @author modified by Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @version 3.0
 */
class GAUDI_API ICounterSvc: virtual public IInterface
{
public:
  /// InterfaceID
  DeclareInterfaceID(ICounterSvc,4,0);
  /// the actual type of counter @see StatEntity
  typedef StatEntity Counter ;
  /** Ease the manipulation of counters in a way, that they
   *  behave like objects:
   *  Avoid:    Counter* cnt = ...;
   *            ++(*cnt);
   *
   *  Instead:  Counter* cnt = ...;
   *            CountObject obj(cnt);
   *            ++cnt;
   *
   *  Note: No inheritance!
   *        This class has no virtual destructor
   *
   *  @author  Markus Frank
   *  @version 1.0
   */
  typedef Stat       CountObject ;
  /// the actual type of vectors of initialized counters
  typedef std::vector<CountObject> Counters ;
  /** @class Printout ICounterSvc.h GaudiKernel/ICounterSvc.h
   *
   * Print counters for each element in the range [first, last)
   *  e.g.  for_each(start, end, PrintCube) ;
   *
   *  @author  Markus Frank
   *  @version 1.0
   */
  class Printout
  {
  public:
    /// Standard initializing constructor
    Printout  ( ICounterSvc* svc ) ;
    /// destructor
    ~Printout () ;
    /// Callback for printout with Counter pointers
    StatusCode operator()( MsgStream& log , const Counter* cnt )  const ;
  private:
    // no defauld constructor
    Printout () ; ///< no defauld constructor
    // no copy
    Printout ( const Printout& ) ; //< no coy constructor
    // no assignement
    Printout& operator=( const Printout& ) ; ///< no assigment is allowed
  private:
    /// Reference to counter service
    ICounterSvc* m_svc;
  } ;
  /** Access an existing counter object.
   *
   * @param  group         [IN] Hint for smart printing
   * @param  name          [IN] Counter name
   * @param  createIf      [IN] flag to indicate the creation if not counter found
   *
   * @return Pointer to existing counter object (NULL if non existing).
   */
  virtual Counter* get
  ( const std::string& group ,
    const std::string& name  ) const = 0;
  /** get all counters form the given group:
   *  @code
   *
   *    ICounterSvc::Counters cnts = svc->get("Efficiency") ;
   *
   *    MsgStream& stream = ... ;
   *    for ( ICounterSvc::Counters::const_iterator ic = cnts.begin()  ;
   *          cnts.end() != ic ; ++ic )
   *       {
   *          stream << (*ic) << endmsg ;
   *       }
   *
   *
   *  @endcode
   *  @see ICounterSvc::Counters
   *  @param gorup the gorup name
   *  @return vector of the properly initialized counters
   */
  virtual Counters get ( const std::string& group ) const = 0 ;
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
  ( const std::string& group         ,
    const std::string& name          ,
    longlong           initial_value ,
    Counter*&          refpCounter   ) = 0;
  /** If the counter object exists already,
   * a std::runtime_error exception is thrown. The ownership of the
   * actual counter stays with the service.
   * @param  group         [IN]     Hint for smart printing
   * @param  name          [IN]     Counter name
   * @param  initial_value [IN]     Initial counter value
   * @param  refpCounter   [OUT]    Reference to store pointer to counter.
   * @return Fully initialized CountObject.
   */
  virtual CountObject create
  ( const std::string& group    ,
    const std::string& name     ,
    longlong  initial_value = 0 ) = 0;
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
    const std::string& name  ) = 0;
  /** Remove all counters of a given group. If no such counter exists
   * the return code is COUNTER_NOT_PRESENT
   * @param  group          [IN]     Hint for smart printing
   * @return StatusCode indicating failure or success.
   */
  virtual StatusCode remove
  ( const std::string& group ) = 0;
  /** Print counter value
   * @param  group         [IN]     Hint for smart printing
   * @param  name          [IN]     Counter name
   * @param printer        [IN]     Print actor
   *
   * @return StatusCode indicating failure or success.
   */
  virtual StatusCode print
  ( const std::string& group   ,
    const std::string& name    ,
    Printout&          printer ) const = 0;
  /** If no such counter exists the return code is COUNTER_NOT_PRESENT
   * Note: This call is not direct access.
   * @param  group         [IN]     Hint for smart printing
   * @param printer        [IN]     Print actor
   * @return StatusCode indicating failure or success.
   */
  virtual StatusCode print
  ( const std::string& group   ,
    Printout&          printer ) const = 0;
  /** Print counter value
   * @param pCounter       [IN]     Pointer to Counter object
   * @param printer        [IN]     Print actor
   * @return StatusCode indicating failure or success.
   */
  virtual StatusCode print
  ( const Counter* pCounter ,
    Printout&      printer  ) const = 0;
  /** Print counter value
   * @param refCounter     [IN]     Reference to CountObject object
   * @param printer        [IN]     Print actor
   * @return StatusCode indicating failure or success.
   */
  virtual StatusCode print
  ( const CountObject& pCounter ,
    Printout&          printer  ) const = 0;
  /** @param printer        [IN]     Print actor
   *  @return StatusCode indicating failure or success.
   */
  virtual StatusCode print
  ( Printout& printer ) const = 0;
  /// Default Printout for counters
  virtual StatusCode defaultPrintout
  ( MsgStream&     log      ,
    const Counter* pCounter ) const = 0;
  // Return codes:
  enum {  COUNTER_NOT_PRESENT = 2,  // Error
          COUNTER_EXISTS      = 4,  // Error ?
          COUNTER_REMOVED     = 3   // Type of success. Low bit set
  } ;
protected:
  /// protected virtual destructor
  virtual ~ICounterSvc() ; ///< protected virtual destructor
};
// ============================================================================
/// output operator for the counter object
// ============================================================================
std::ostream& operator<<( std::ostream& , const ICounterSvc::CountObject& ) ;
// ============================================================================
/// The END
// ============================================================================
#endif // GAUDIKERNEL_ICOUNTERSVC_H
// ============================================================================
