#ifndef GAUDIALG_GAUDICOMMON_H
#define GAUDIALG_GAUDICOMMON_H 1
// ============================================================================
// Include files
// ============================================================================
// from STL
// ============================================================================
#include <algorithm>
#include <functional>
#include <list>
#include <map>
#include <mutex>
#include <string>
#include <vector>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiAlg/FixTESPath.h"
#include "GaudiKernel/DataObjectHandle.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/HashMap.h"
#include "GaudiKernel/IAlgContextSvc.h"
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/ICounterSummarySvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/IUpdateManagerSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/StatEntity.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/System.h"

#ifdef __CLING__
#  define WARN_UNUSED
#else
#  define WARN_UNUSED [[nodiscard]]
#endif

// ============================================================================
// forward declarations
// ============================================================================
namespace Gaudi {
  class Algorithm; // GaudiKernel
}
class AlgTool; // GaudiKernel
class ISvcLocator;
namespace Gaudi {
  namespace Utils {
    template <class TYPE>
    struct GetData;
  }
} // namespace Gaudi

namespace GaudiCommon_details {
  constexpr const struct svc_eq_t {
    bool operator()( const std::string& n, const SmartIF<IService>& s ) const { return n == s->name(); };
    bool operator()( const SmartIF<IService>& s, const std::string& n ) const { return s->name() == n; };
    bool operator()( const SmartIF<IService>& s, const SmartIF<IService>& n ) const { return s->name() == n->name(); };
  } svc_eq{};
  constexpr const struct svc_lt_t {
    bool operator()( const std::string& n, const SmartIF<IService>& s ) const { return n < s->name(); };
    bool operator()( const SmartIF<IService>& s, const std::string& n ) const { return s->name() < n; };
    bool operator()( const SmartIF<IService>& s, const SmartIF<IService>& n ) const { return s->name() < n->name(); };
  } svc_lt{};
} // namespace GaudiCommon_details
// ============================================================================
/*  @file GaudiCommon.h
 *
 *  Header file for class : GaudiCommon
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @author Rob Lambert Rob.Lambert@cern.ch
 *  @date   2009-08-04
 */
// ============================================================================
/** @class GaudiCommon GaudiCommon.h GaudiAlg/GaudiCommon.h
 *
 *  Implements the common functionality between GaudiTools and GaudiAlgorithms
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @author Rob Lambert Rob.Lambert@cern.ch
 *  @date   2009-08-04
 */
// ============================================================================
template <class PBASE>
class GAUDI_API GaudiCommon : public FixTESPath<PBASE> {
protected: // definitions
  using base_class = FixTESPath<PBASE>;

  /** Simple definition to be used with the new useRootInTES argument get<TYPE>
   *  and put methods. If used with cause the RootInTES option to be IGNORED.
   *
   *  Useful to aid with code readability. e.g.
   *  @code
   *  // Get data, ignoring the setting of rootInTES()
   *  MyData * data = get<MyData>( "/Event/MyData", IgnoreRootInTES );
   *  @endcode
   */
  static const bool IgnoreRootInTES = false;
  /** Simple definition to be used with the new useRootInTES argument get<TYPE>
   *  and put methods. If used with cause the RootInTES option to be USED
   *
   *  Useful to aid with code readability. e.g.
   *  @code
   *  // Get data, using the setting of rootInTES()
   *  MyData * data = get<MyData>( "/Event/MyData", UseRootInTES );
   *  // note the default setting is true, so this is equivalent to
   *  MyData * data = get<MyData>( "/Event/MyData" );
   *  @endcode
   */
  static const bool UseRootInTES = true;
  // ==========================================================================
protected: // few actual data types
  // ==========================================================================
  /// the actual type of general counters
  typedef std::map<std::string, StatEntity>                                                    StatisticsOwn;
  typedef std::map<std::string, std::reference_wrapper<Gaudi::Accumulators::PrintableCounter>> Statistics;
  /// the actual type error/warning counter
  typedef std::map<std::string, unsigned int> Counter;
  /// storage for active tools
  typedef std::vector<IAlgTool*> AlgTools;
  /// storage for active services
  typedef std::vector<SmartIF<IService>> Services;

  // ==========================================================================
  // protected members such that they can be used in the derived classes
  /// a pointer to the CounterSummarySvc
  SmartIF<ICounterSummarySvc> m_counterSummarySvc;

public:
  // ==========================================================================
  /** @brief Templated access to the data in Gaudi Transient Store
   *
   *  Quick and safe access to the data in Gaudi transient store.
   *  The method located the data at given address and perform the
   *  debug printout about located data
   *
   *  @code
   *
   *  // Will access MCHits from the given location
   *  MCHits* hits = get<MCHits>( evtSvc () , "MC/Hits" );
   *
   *  @endcode
   *
   *  @attention The method respects the setting of the job option
   *             RootInTES by prepending the value of this to the
   *             data location that is passed.
   *             The default setting for RootInTES is "" so has no effect.
   *             This behavior can be suppressed by passing the argument
   *             useRootInTES = false
   *
   *  @see IDataProviderSvc
   *  @see SmartDataPtr
   *
   *  @exception      GaudiException for Invalid Data Provider Service
   *  @exception      GaudiException for invalid/unavailable data
   *
   *  @param svc      Pointer to data service (data provider)
   *  @param location data location/address in Gaudi Transient Store
   *  @param useRootInTES Flag to turn on(TRUE) off(FALSE) the use of
   *                      the RootInTES location property
   *
   *  @return pointer to the data object
   */
  template <class TYPE>
  typename Gaudi::Utils::GetData<TYPE>::return_type get( IDataProviderSvc* svc, const std::string& location,
                                                         const bool useRootInTES = true ) const;
  /** Quicker version of the get function which bypasses the check on the
   *  retrieved data.
   */
  template <class TYPE>
  typename Gaudi::Utils::GetData<TYPE>::return_type getIfExists( IDataProviderSvc* svc, const std::string& location,
                                                                 const bool useRootInTES = true ) const;
  /** @brief Check the existence of a data object or container
   *         in the Gaudi Transient Event Store
   *
   *  @code
   *
   *  bool a1 = exist<DataObject>( evtSvc() , "/Event/MyObject" ) ;
   *  bool a2 = exist<MyHits>    ( evtSvc() , "/Event/MyHits" ) ;
   *
   *  @endcode
   *
   *  @attention The method respects the setting of the job option
   *             RootInTES by prepending the value of this to the
   *             data location that is passed.
   *             The default setting for RootInTES is "" so has no effect.
   *             This behavior can be suppressed by passing the argument
   *             useRootInTES = false
   *
   *  @param  svc      Pointer to data provider service
   *  @param  location Address in Gaudi Transient Store
   *  @param useRootInTES Flag to turn on(TRUE) off(FALSE) the use of
   *                      the RootInTES location property
   *
   *  @return          Boolean indicating status of the request
   *  @retval true     Data object or container exists and implements a proper interface
   *  @retval true     Failed to locate the data object or container
   */
  template <class TYPE>
  bool exist( IDataProviderSvc* svc, const std::string& location, const bool useRootInTES = true ) const;
  /** @brief Get the existing data object from Gaudi Event Transient store.
   *        Alternatively, create new object and register it in TES
   *        and return if object does not exist.
   *
   *  @code
   *
   *  MyHits* hits = getOrCreate<MyHits,MyHits>( evtSvc() , "/Event/MyHits" ) ;
   *
   *  @endcode
   *
   *  @attention The method respects the setting of the job option
   *             RootInTES by prepending the value of this to the
   *             data location that is passed.
   *             The default setting for RootInTES is "" so has no effect.
   *             This behavior can be suppressed by passing the argument
   *             useRootInTES = false
   *
   *  @exception GaudiException for Invalid Data Provider Service
   *  @exception GaudiException for invalid/unavailable  data
   *
   *  @param  svc      Pointer to data provider service
   *  @param  location  Location in Gaudi Transient Event Store
   *  @param useRootInTES Flag to turn on(TRUE) off(FALSE) the use of
   *                      the RootInTES location property
   *
   *  @return A valid pointer to the data object
   */
  template <class TYPE, class TYPE2>
  typename Gaudi::Utils::GetData<TYPE>::return_type getOrCreate( IDataProviderSvc* svc, const std::string& location,
                                                                 const bool useRootInTES = true ) const;
  /** @brief Register a data object or container into Gaudi Event Transient Store
   *
   *  @see IDataProviderSvc
   *
   *  @attention The method respects the setting of the job option
   *             RootInTES by prepending the value of this to the
   *             data location that is passed.
   *             The default setting for RootInTES is "" so has no effect.
   *             This behavior can be suppressed by passing the argument
   *             useRootInTES = false
   *
   *  @param svc        Pointer to data provider service
   *  @param object     Data object or container to be registered
   *  @param location   Location in Gaudi Event Transient Store ("/Event" could be omitted )
   *  @param useRootInTES Flag to turn on(TRUE) off(FALSE) the use of
   *                      the RootInTES location property
   *
   *  @exception        GaudiException for invalid event data service
   *  @exception        GaudiException for invalid object
   *  @exception        GaudiException for error result from event data service
   *
   *  @return StatusCode
   *  @retval StatusCode::SUCCESS Data was successfully placed in the TES.
   *  @retval StatusCode::FAILURE Failed to store data in the TES.
   */
  DataObject* put( IDataProviderSvc* svc, std::unique_ptr<DataObject> object, const std::string& location,
                   const bool useRootInTES = true ) const;
  // [[deprecated( "please pass std::unique_ptr as 2nd argument" )]]
  DataObject* put( IDataProviderSvc* svc, DataObject* object, const std::string& location,
                   const bool useRootInTES = true ) const {
    return put( svc, std::unique_ptr<DataObject>( object ), location, useRootInTES );
  }
  /** Useful method for the easy location of tools.
   *
   *  @code
   *
   *  IMyTool* myTool =
   *      tool<IMyTool>( "MyToolType", "MyToolName", this );
   *
   *  @endcode
   *
   *  @attention The tools located with this method
   *             will be released automatically
   *
   *  @see IToolSvc
   *  @see IAlgTool
   *
   *  @exception GaudiException for invalid Tool Service
   *  @exception GaudiException for error from Tool Service
   *  @exception GaudiException for invalid tool
   *
   *  @param type   Tool type
   *  @param name   Tool name
   *  @param parent Tool parent
   *  @param create Flag for creation of nonexisting tools
   *  @return       A pointer to the tool
   */
  template <class TOOL>
  TOOL* tool( const std::string& type, const std::string& name, const IInterface* parent = 0,
              bool create = true ) const;
  /** A useful method for the easy location of tools.
   *
   *  @code
   *
   *  IMyTool* myTool = tool<IMyTool>( "PublicToolType" );
   *
   *  @endcode
   *
   *  @attention The tools located with this method
   *             will be released automatically
   *
   *  @see IToolSvc
   *  @see IAlgTool
   *
   *  @exception    GaudiException for invalid Tool Service
   *  @exception    GaudiException for error from Tool Service
   *  @exception    GaudiException for invalid tool
   *  @param type   Tool type, could be of "Type/Name" format
   *  @param parent Tool parent
   *  @param create Flag for creation of non-existing tools
   *  @return       A pointer to the tool
   */
  template <class TOOL>
  TOOL* tool( const std::string& type, const IInterface* parent = 0, bool create = true ) const;
  /** A useful method for the easy location of services
   *
   *  @code
   *
   *  IMyService* mySvc = svc<IMyService>( "MyServiceType" );
   *
   *  @endcode
   *
   *  @attention The services located with this method
   *             will be released automatically
   *
   *  @see IService
   *  @see ISevcLocator
   *  @see Service
   *
   *  @exception GaudiException for error in Algorithms::service
   *  @exception GaudiException for invalid service
   *
   *  @param name   service type name
   *  @param create Flag for creation of non-existing services
   *  @return       A pointer to the service
   */
  template <class SERVICE>
  SmartIF<SERVICE> svc( const std::string& name, const bool create = true ) const;
  /// Short-cut to locate the Update Manager Service.
  inline IUpdateManagerSvc* updMgrSvc() const;

public:
  /** Print the error message and return with the given StatusCode.
   *
   *  Also performs statistical analysis of the error messages and
   *  suppression after the defined number of error instances.
   *
   *  @code
   *
   *   if( a < 0 ) { return Error("a is negative!") ;}
   *   if( b < 0 ) { return Error("b is illegal!" , StatusCode(25) );
   *   if( c < 0 )
   *      { return Error("c is negative" , StatusCode(35) , 50 );
   *
   *  @endcode
   *
   *  @see MsgStream
   *  @see IMessageSvc
   *  @see StatusCode
   *
   *  @param msg    Error message
   *  @param st     StatusCode to return
   *  @param mx     Maximum number of printouts for this message
   *  @return       StatusCode
   */
  WARN_UNUSED
  StatusCode Error( const std::string& msg, const StatusCode st = StatusCode::FAILURE, const size_t mx = 10 ) const;
  /** Print the warning message and return with the given StatusCode.
   *
   *  Also performs statistical analysis of the warning messages and
   *  suppression after the defined number of error instances.
   *
   *  @code
   *
   *   if( a < 0 ) { return Warning("a is negative!") ;}
   *   if( b < 0 ) { return Warning("b is illegal!" , StatusCode(25) );
   *   if( c < 0 )
   *      { return Warning("c is negative" , StatusCode(35) , 50 );
   *
   *  @endcode
   *
   *  @see MsgStream
   *  @see IMessageSvc
   *  @see StatusCode
   *
   *  @param msg    Warning message
   *  @param st     StatusCode to return
   *  @param mx     Maximum number of printouts for this message
   *  @return       The given StatusCode
   */
  WARN_UNUSED
  StatusCode Warning( const std::string& msg, const StatusCode st = StatusCode::FAILURE, const size_t mx = 10 ) const;
  /** Print the info message and return with the given StatusCode.
   *
   *  Also performs statistical analysis of the info messages and
   *  suppression after the defined number of instances.
   *
   *  @see MsgStream
   *  @see IMessageSvc
   *  @see StatusCode
   *  @see GaudiCommon::Warning
   *
   *  @param msg    Info message
   *  @param st     StatusCode to return
   *  @param mx     Maximum number of printouts for this message
   *  @return       The given StatusCode
   */
  WARN_UNUSED
  StatusCode Info( const std::string& msg, const StatusCode st = StatusCode::SUCCESS, const size_t mx = 10 ) const;
  /** Print the message and return with the given StatusCode.
   *
   *  @see MsgStream
   *  @see IMessageSvc
   *  @see StatusCode
   *
   *  @param msg    Message to print
   *  @param st     StatusCode to return
   *  @param lev    Printout level for the given message
   *  @return       The given StatusCode
   */
  WARN_UNUSED
  StatusCode Print( const std::string& msg, const StatusCode st = StatusCode::SUCCESS,
                    const MSG::Level lev = MSG::INFO ) const;
  /** Assertion - throw exception if the given condition is not fulfilled
   *
   *  @see GaudiException
   *
   *  @exception          Exception for invalid condition
   *  @param ok           Condition which should be "true"
   *  @param message      Message to be associated with the exception
   */
  inline void Assert( const bool ok, const std::string& message = "",
                      const StatusCode sc = StatusCode( StatusCode::FAILURE, true ) ) const;
  /** Assertion - throw exception if the given condition is not fulfilled
   *
   *  @see GaudiException
   *
   *  @exception          Exception for invalid condition
   *  @param ok           Condition which should be "true"
   *  @param message      Message to be associated with the exception
   */
  inline void Assert( const bool ok, const char* message,
                      const StatusCode sc = StatusCode( StatusCode::FAILURE, true ) ) const;
  /** Create and (re)-throw a given GaudiException
   *
   *  @see GaudiException
   *
   *  @exception    GaudiException always thrown!
   *  @param msg    Exception message
   *  @param exc    (previous) exception of type GaudiException
   */
  void Exception( const std::string& msg, const GaudiException& exc,
                  const StatusCode sc = StatusCode( StatusCode::FAILURE, true ) ) const;
  /** Create and (re)-throw a given exception
   *
   *  @see GaudiException
   *
   *  @exception    std::exception always thrown!
   *  @param msg    Exception message
   *  @param exc    (previous) exception of type std::exception
   *  @param sc     StatusCode
   */
  void Exception( const std::string& msg, const std::exception& exc,
                  const StatusCode sc = StatusCode( StatusCode::FAILURE, true ) ) const;
  /** Create and throw an exception with the given message
   *
   *  @see GaudiException
   *
   *  @exception    GaudiException always thrown!
   *  @param msg    Exception message
   *  @param sc     StatusCode
   */
  void Exception( const std::string& msg = "no message",
                  const StatusCode   sc  = StatusCode( StatusCode::FAILURE, true ) ) const;

private:
  /// accessor to all owned counters
  inline StatisticsOwn countersOwn() const { return m_countersOwn; }

public:
  /** accessor to certain counter by name
   *
   *  @code
   *
   *  if ( OK ) { ++counter("OK") ; }
   *
   *  // additive counter ('Flag')
   *  counter("#Tracks") += tracks->size() ;
   *
   *  // multiplicative counter  ('Weight')
   *  counter("ProbTot") *= probability ;
   *
   *  @endcode
   *
   *  @see StatEntuty
   *  @param tag counter name
   *  @return the counter itself
   */
  //[[deprecated( "see LHCBPS-1758" )]]
  inline StatEntity& counter( const std::string& tag ) const {
    return const_cast<GaudiCommon<PBASE>*>( this )->counter( tag );
  }
  inline StatEntity& counter( const std::string& tag ) {
    std::lock_guard<std::mutex> lock( m_countersOwnMutex );
    // Return referenced StatEntity if it already exists, else create it
    auto p = this->findCounter( tag );
    if ( !p ) {
      auto& counter = m_countersOwn[tag];
      this->declareCounter( tag, counter );
      return counter;
    }
    return m_countersOwn[tag];
  }
  // ==========================================================================
public:
  /// Insert the actual C++ type of the algorithm/tool in the messages ?
  inline bool typePrint() const { return m_typePrint; }
  /// Print properties at initialization ?
  inline bool propsPrint() const { return m_propsPrint; }
  /// Print statistical counters at finalization ?
  inline bool statPrint() const { return m_statPrint; }
  /// Print error counters at finalization ?
  inline bool errorsPrint() const { return m_errorsPrint; }
  // ==========================================================================
public:
  /** perform the actual printout of statistical counters
   *  @param  level The message level to print at
   *  @return number of active statistical counters
   */
  long printStat( const MSG::Level level = MSG::ALWAYS ) const;
  /** perform the actual printout of error counters
   *  @param  level The message level to print at
   *  @return number of error counters
   */
  long printErrors( const MSG::Level level = MSG::ALWAYS ) const;
  /** perform the actual printout of properties
   *  @param  level The message level to print at
   *  @return number of properties
   */
  long printProps( const MSG::Level level = MSG::ALWAYS ) const;
  /** register the current instance to the UpdateManagerSvc as a consumer for a condition.
   *  @param condition  the path inside the Transient Detector Store to the condition object.
   *  @param mf         optional pointer to the member function to call when the condition object
   *                    is updated. If the pointer is omitted the user must explicitly provide
   *                    the class name to the method.
   *  @code
   *  StatusCode MyAlg::initialize(){
   *     // ...
   *     registerCondition("/dd/Conditions/Readout/MyDet/MyCond",&MyAlg::i_CallBack);
   *     registerCondition<MyAlg>("/dd/Conditions/Readout/MyDet/MyOtherCond");
   *     // ...
   *     return StatusCode.SUCCESS;
   *  }
   *  @endcode
   */
  template <class CallerClass>
  inline void registerCondition( const std::string& condition, StatusCode ( CallerClass::*mf )() = nullptr ) {
    updMgrSvc()->registerCondition( dynamic_cast<CallerClass*>( this ), condition, mf );
  }
  /** register the current instance to the UpdateManagerSvc as a consumer for a condition.
   *  This version of the method allow the user to specify where to put a copy of the pointer
   *  to the condition object.
   *  @param condition    the path inside the Transient Detector Store to the condition object.
   *  @param condPtrDest  pointer to fill with the location of the condition object.
   *                      Note: the pointer can be safely used only in the execute method or in the
   *                      member function triggered by the update.
   *  @param mf           optional pointer to the member function to call when the condition object
   *                      is updated. If the pointer is omitted the user must explicitly provide
   *                      the class name to the method.
   *  @code
   *  class MyAlg: public GaudiAlgorithm {
   *     // ...
   *  public:
   *     virtual StatusCode i_CallBack();
   *  private:
   *     Condition *m_MyCond;
   *     SpecialCondition *m_MyOtherCond;
   *     // ...
   *  };
   *
   *  StatusCode MyAlg::initialize(){
   *     // ...
   *     registerCondition("/dd/Conditions/Readout/MyDet/MyCond",m_MyCond,&MyAlg::i_CallBack);
   *     registerCondition<MyAlg>("/dd/Conditions/Readout/MyDet/MyOtherCond",m_MyOtherCond);
   *     // ...
   *     return StatusCode.SUCCESS;
   *  }
   *  @endcode
   */
  template <class CallerClass, class CondType>
  inline void registerCondition( const std::string& condition, CondType*& condPtrDest,
                                 StatusCode ( CallerClass::*mf )() = NULL ) {
    updMgrSvc()->registerCondition( dynamic_cast<CallerClass*>( this ), condition, mf, condPtrDest );
  }
  /// just to avoid conflicts with the version using a pointer to a template class.
  template <class CallerClass>
  inline void registerCondition( char* condition, StatusCode ( CallerClass::*mf )() = NULL ) {
    updMgrSvc()->registerCondition( dynamic_cast<CallerClass*>( this ), std::string( condition ), mf );
  }
  /** register the current instance to the UpdateManagerSvc as a consumer for a condition.
   *  @param condition  the path inside the Transient Detector Store to the condition object.
   *  @param mf         optional pointer to the member function to call when the condition object
   *                    is updated. If the pointer is omitted the user must explicitly provide
   *                    the class name to the method.
   *  @code
   *  StatusCode MyAlg::initialize(){
   *     // ...
   *     registerCondition("/dd/Conditions/Readout/MyDet/MyCond",&MyAlg::i_CallBack);
   *     registerCondition<MyAlg>("/dd/Conditions/Readout/MyDet/MyOtherCond");
   *     // ...
   *     return StatusCode.SUCCESS;
   *  }
   *  @endcode
   */
  template <class CallerClass, class TargetClass>
  inline void registerCondition( TargetClass* condition, StatusCode ( CallerClass::*mf )() = NULL ) {
    updMgrSvc()->registerCondition( dynamic_cast<CallerClass*>( this ), condition, mf );
  }
  /** asks the UpdateManagerSvc to perform an update of the instance (if needed) without waiting the
   *  next BeginEvent incident.
   *  It is useful if the instance can be instantiated after a BeginEvent incident, and used before
   *  the next one (for example with tools).
   *  @code
   *  StatusCode MyTool::initialize(){
   *    // ...
   *    return runUpdate();
   *  }
   *  @endcode
   */
  inline StatusCode runUpdate() { return updMgrSvc()->update( this ); }

public:
  /// Algorithm constructor - the SFINAE constraint below ensures that this is
  /// constructor is only defined if PBASE derives from Algorithm
  template <typename U = PBASE, typename = std::enable_if_t<std::is_base_of_v<Gaudi::Algorithm, PBASE>, U>>
  GaudiCommon( const std::string& name, ISvcLocator* pSvcLocator ) : base_class( name, pSvcLocator ) {
    initGaudiCommonConstructor();
  }
  /// Tool constructor - SFINAE-ed to insure this constructor is only defined
  /// if PBASE derives from AlgTool.
  template <typename U = PBASE, typename = std::enable_if_t<std::is_base_of_v<AlgTool, PBASE>, U>>
  GaudiCommon( const std::string& type, const std::string& name, const IInterface* ancestor )
      : base_class( type, name, ancestor ) {
    initGaudiCommonConstructor( this->parent() );
  }

public:
  /** standard initialization method
   *  @return status code
   */
  StatusCode initialize() override;
  /** standard finalization method
   *  @return status code
   */
  StatusCode finalize() override;

private:
  GaudiCommon()                     = delete;
  GaudiCommon( const GaudiCommon& ) = delete;
  GaudiCommon& operator=( const GaudiCommon& ) = delete;

protected:
  /// manual forced (and 'safe') release of the tool
  StatusCode releaseTool( const IAlgTool* tool ) const;
  /// manual forced (and 'safe') release of the service
  StatusCode releaseSvc( const IInterface* svc ) const;

public:
  /** Manual forced (and 'safe') release of the active tool or service
   *
   *  @code
   *
   *  IMyTool* mytool = tool<IMyTool>( .... ) ;
   *  mytool->spendCPUtime() ;
   *  release ( mytool ) ;
   *
   *  IMySvc* msvc = svc<IMySvc>( .... ) ;
   *  msvc->spendCPUtime() ;
   *  release ( msvc ) ;
   *
   *  @endcode
   *
   *  @param interface  Interface pointer to the interface to be released
   *  @return           StatusCode
   *  @retval           StatusCode::SUCCESS Tool or service was successfully released
   *  @retval           StatusCode::FAILURE Error releasing too or service
   */
  StatusCode release( const IInterface* interface ) const;

  /// Un-hide IInterface::release (ICC warning #1125)
  using PBASE::release;
  // ==========================================================================
public:
  // ==========================================================================
  /// get the list of aquired services
  const Services& services() const { return m_services; } // get all services
  // ==========================================================================
public:
  // ==========================================================================
  /// Returns the "context" string. Used to identify different processing states.
  inline const std::string& context() const { return m_context; }

private:
  // ==========================================================================
  /// Add the given service to the list of acquired services
  void addToServiceList( SmartIF<IService> svc ) const;
  /// Constructor initializations
  void initGaudiCommonConstructor( const IInterface* parent = nullptr );
  // ==========================================================================
private:
  /// List of active  tools
  mutable AlgTools m_managedTools;
  /// List of active  services
  mutable Services m_services;
  // ==========================================================================
  /// Counter of errors
  mutable Counter m_errors;
  /// counter of warnings
  mutable Counter m_warnings;
  /// counter of infos
  mutable Counter m_infos;
  /// Counter of exceptions
  mutable Counter m_exceptions;
  /// General counters
  StatisticsOwn m_countersOwn;
  /// The mutex for m_countersOwn
  std::mutex m_countersOwnMutex;
  // ==========================================================================
  /// Pointer to the Update Manager Service instance
  mutable IUpdateManagerSvc* m_updMgrSvc = nullptr;
  // ==========================================================================
  // Properties
  Gaudi::Property<bool> m_errorsPrint{this, "ErrorsPrint", true,
                                      [this]( auto& ) {
                                        // no action if not yet initialized
                                        if ( this->FSMState() >= Gaudi::StateMachine::INITIALIZED &&
                                             this->errorsPrint() ) {
                                          this->printErrors();
                                        }
                                      },
                                      "print the statistics of errors/warnings/exceptions"};
  Gaudi::Property<bool> m_propsPrint{this, "PropertiesPrint", false,
                                     [this]( auto& ) {
                                       // no action if not yet initialized
                                       if ( this->FSMState() >= Gaudi::StateMachine::INITIALIZED &&
                                            this->propsPrint() ) {
                                         this->printProps( MSG::ALWAYS );
                                       }
                                     },
                                     "print the properties of the component"};
  Gaudi::Property<bool> m_statPrint{this, "StatPrint", true,
                                    [this]( auto& ) {
                                      // no action if not yet initialized
                                      if ( this->FSMState() >= Gaudi::StateMachine::INITIALIZED && this->statPrint() ) {
                                        this->printStat( MSG::ALWAYS );
                                      }
                                    },
                                    "print the table of counters"};
  Gaudi::Property<bool> m_printEmptyCounters{this, "PrintEmptyCounters", false,
                                             "force printing of empty counters, otherwise only printed in DEBUG mode"};
  Gaudi::Property<bool> m_typePrint{this, "TypePrint", true, "add the actual C++ component type into the messages"};

  Gaudi::Property<std::string> m_context{this, "Context", {}, "note: overridden by parent settings"};
  Gaudi::Property<std::string> m_header{this, "StatTableHeader",
                                        " |    Counter                                      |     #     |   "
                                        " sum     | mean/eff^* | rms/err^*  |     min     |     max     |",
                                        "the header row for the output Stat-table"};
  Gaudi::Property<std::string> m_format1{
      this, "RegularRowFormat", " | %|-48.48s|%|50t||%|10d| |%|11.7g| |%|#11.5g| |%|#11.5g| |%|#12.5g| |%|#12.5g| |",
      "the format for regular row in the output Stat-table"};
  Gaudi::Property<std::string> m_format2{
      this, "EfficiencyRowFormat",
      " |*%|-48.48s|%|50t||%|10d| |%|11.5g| |(%|#9.6g| +- %|-#9.6g|)%%|   -------   |   -------   |",
      "The format for \"efficiency\" row in the output Stat-table"};
  Gaudi::Property<bool> m_useEffFormat{this, "UseEfficiencyRowFormat", true,
                                       "use the special format for printout of efficiency counters"};

  Gaudi::Property<std::vector<std::string>> m_counterList{
      this, "CounterList", {".*"}, "RegEx list, of simple integer counters for CounterSummary"};
  Gaudi::Property<std::vector<std::string>> m_statEntityList{
      this, "StatEntityList", {}, "RegEx list, of StatEntity counters for CounterSummary"};
};
// ============================================================================
#include "GaudiAlg/GaudiCommonImp.h"
// ============================================================================

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_GAUDICOMMON_H
