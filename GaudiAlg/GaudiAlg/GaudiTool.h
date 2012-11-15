// $Id: GaudiTool.h,v 1.13 2008/10/10 13:38:28 marcocle Exp $
// ============================================================================
#ifndef GAUDIALG_GAUDITOOL_H
#define GAUDIALG_GAUDITOOL_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/DataObject.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlg.h"
#include "GaudiAlg/GaudiCommon.h"
// ============================================================================
// forward declarations
// ============================================================================
class          IDataProviderSvc ;
class          IChronoStatSvc   ;
class          IIncidentSvc     ;
class          IHistogramSvc    ;
class          GaudiException   ;
class          INTupleSvc       ;
class          IAlgContextSvc   ;
namespace std { class exception ; }
// ============================================================================
/** @file GaudiTool.h
 *
 * Header file for class GaudiAlgorithm.
 * The actual code is mainly imported from
 *   - LHCb Calorimeter software and
 *   - LHCb C++ toolkit for smart and friendly physics analysis "LoKi"
 *
 * This base class allows "easy"(=="1 line") access to tools
 * and services. This access is safe in the sense that there
 * is no need to worry about the reference counts for tools
 * and services.
 *
 * The base class allows "easy" (=="1 line") access to data in
 * Gaudi Transient Stores. The functionality includes the checking
 * of the presence of the data at the given location, checking the
 * validity of the data, retrieval of valid data and "forced" retrieve
 * of valid data (create if there is no data).
 *
 * The base class allows to perform an easy error, warning and exception
 * treatments, including the accumulated statistics of exceptions, errors
 * and warnings.
 *
 * The base class included also utilities for general statistical counters.
 *
 * It has been reported that usage of this base class results in
 * significant reduction of existing code lines.
 *
 *  @attention
 *  See also the class GaudiCommon, which implements some of the common
 *  functionality between GaudiTool and GaudiAlgorithm.
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date 01/11/2001
 */
// ============================================================================
/** @class GaudiTool GaudiTool.h GaudiTools/GaudiTool.h
 *
 *  The useful base class for tools.
 *  Small extension of ordinary the AlgTool base class.
 *
 *  This base class allows "easy"(=="1 line") access to tools
 *  and services. This access is safe in the sense that there
 *  is no need to worry about the reference counts for tools
 *  and services.
 *
 *  The base class allows "easy" (=="1 line") access to data in
 *  Gaudi Transient Stores. The functionality includes the checking
 *  of the presence of the data at the given location, checking the
 *  validity of the data, retrieval of valid data and "forced" retrieve
 *  of valid data (create if there is no data).
 *
 *  The base class allows to perform an easy error, warning and exception
 *  treatments, including the accumulated statistics of exceptions, errors
 *  and warnings.
 *
 *  The base class also includes utilities for general statistical counters.
 *
 *  It has been reported that usage of this base class results in
 *  significant reduction of existing code lines.
 *
 *  @attention
 *  See the class GaudiCommon, which implements some of the common functionality
 *  between GaudiTool and GaudiAlgorithm
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   2003-07-07
 */
// ============================================================================
class GAUDI_API GaudiTool: public GaudiCommon<AlgTool>
{
public:
  // ==========================================================================
  /** standard initialization method
   *  @see  AlgTool
   *  @see IAlgTool
   *  @return status code
   */
  virtual StatusCode    initialize ();
  /** standard finalization method
   *  @see  AlgTool
   *  @see IAlgTool
   *  @return status code
   */
  virtual StatusCode    finalize   ();
  // ==========================================================================
public: // accessors
  // ==========================================================================
  /** Access the standard N-Tuple
   *  @return pointer to N-Tuple service .
   */
  INTupleSvc*          ntupleSvc () const;
  /** Access the standard event collection service
   *  @return pointer to the event collection service
   */
  INTupleSvc*          evtColSvc () const;
  /** accessor to detector service
   *  @return pointer to detector service
   */
  IDataProviderSvc*    detSvc    () const ;
  /** accessor to event service  service
   *  @return pointer to detector service
   */
  IDataProviderSvc*    evtSvc    () const ;
  /** accessor to Incident Service
   *  @return pointer to the Incident Service
   */
  IIncidentSvc*        incSvc    () const ;
  /** accessor to Chrono & Stat Service
   *  @return pointer to the Chrono & Stat Service
   */
  IChronoStatSvc*      chronoSvc () const ;
  /** acessor to the histogram service
   *  @return pointer to the histogram service
   */
  IHistogramSvc*       histoSvc  () const ;
  /** acessor to the Algorithm Context Service
   *  @return pointer to the Algorithm Context Service
   */
  IAlgContextSvc*     contextSvc () const ;
  // ==========================================================================
public:
  // ==========================================================================
  // following methods cannot go in GaudiCommon since they use methods ( evtSvc()
  // and detDvc() ) that are not members of AlgTool.
  // Also some methods seem which are members of the base class do not seem
  // to be found unless forwarding methods are put here ??
  // ==========================================================================

  /** @brief Register a data object or container into Gaudi Event Transient Store
   *
   *  @code
   *
   *  MCHits * hits = new MCHits();
   *  put( evtSvc(), hits, "/Event/MC/Hits" );
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
   *
   *  @param svc        Pointer to data provider service
   *  @param object     Data object or container to be registered
   *  @param address    Address in Gaudi Event Transient Store ("/Event" could be omitted )
   *  @param useRootInTES Flag to turn on(TRUE) off(FALSE) the use of
   *                      the RootInTES location property
   *  @exception        GaudiException for invalid event data service
   *  @exception        GaudiException for invalid object
   *  @exception        GaudiException for error result from event data service
   *  @return StatusCode
   *  @retval StatusCode::SUCCESS Data was successfully placed in the TES.
   *  @retval StatusCode::FAILURE Failed to store data in the TES.
   */
  inline DataObject* put ( IDataProviderSvc*  svc     ,
                    DataObject*        object  ,
                    const std::string& address ,
                    const bool useRootInTES = true ) const
  {
    return GaudiCommon<AlgTool>::put ( svc , object , address , useRootInTES ) ;
  }

  /** @brief Register a data object or container into Gaudi Event Transient Store
   *
   *  @see IDataProviderSvc
   *
   *  @code
   *
   *  MCHits * hits = put( new MCHits(), "/Event/MC/Hits" );
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
   *  @param object     Data object or container to be registered
   *  @param address    Address in Gaudi Event Transient Store ("/Event" could be omitted )
   *  @param useRootInTES Flag to turn on(TRUE) off(FALSE) the use of
   *                      the RootInTES location property
   *  @exception        GaudiException for invalid event data service
   *  @exception        GaudiException for invalid object
   *  @exception        GaudiException for error result from event data service
   *  @return StatusCode
   *  @retval StatusCode::SUCCESS Data was successfully placed in the TES.
   *  @retval StatusCode::FAILURE Failed to store data in the TES.
   */
  inline DataObject* put ( DataObject*        object   ,
                    const std::string& address  ,
                    const bool useRootInTES = true ) const
  {
    return GaudiCommon<AlgTool>::put ( evtSvc() , object , address , useRootInTES ) ;
  }

  /** @brief Templated access to the data in Gaudi Transient Store
   *
   *  Quick and safe access to the data in Gaudi transient store.
   *  The method located the data at given address and perform the
   *  debug printout about located data
   *
   *  @code
   *
   *  MCHits* hits = get<MCHits>( evtSvc() , "/Event/MC/Hits" );
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
   *  @return pointer to the data object
   */
  template < class TYPE  >
  inline typename Gaudi::Utils::GetData<TYPE>::return_type
  get  (  IDataProviderSvc*  svc       ,
          const std::string& location  ,
          const bool useRootInTES = true ) const
  {
    return GaudiCommon<AlgTool>::get<TYPE> ( svc , location , useRootInTES ) ;
  }

  /** @brief Templated access to the data in Gaudi Transient Store
   *
   *  Quick and safe access to the data in Gaudi transient store.
   *  The method located the data at given address and perform the
   *  debug printout about located data.
   * 
   *  Skips the check on the data as performed by 'get'. No exception
   *  is thrown if the data is missing.
   *
   *  @code
   *
   *  MCHits* hits = getIfExists<MCHits>( evtSvc() , "/Event/MC/Hits" );
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
   *  @param svc      Pointer to data service (data provider)
   *  @param location data location/address in Gaudi Transient Store
   *  @param useRootInTES Flag to turn on(TRUE) off(FALSE) the use of
   *                      the RootInTES location property
   *
   *  @return pointer to the data object. 
   *  @retval NULL If data does not exist.
   */
  template < class TYPE  >
  inline typename Gaudi::Utils::GetData<TYPE>::return_type
  getIfExists ( IDataProviderSvc*  svc       ,
                const std::string& location  ,
                const bool useRootInTES = true ) const
  {
    return GaudiCommon<AlgTool>::getIfExists<TYPE> ( svc , location , useRootInTES ) ;
  }

  /** @brief Templated access to the data from Gaudi Event Transient Store
   *
   *  Quick and safe access to the data in Gaudi transient store.
   *
   *  The method located the data at the given address and perform the
   *  debug printout about located data.
   *
   *  @code
   *
   *  MCParticles* mcps = get<MCParticles>( MCParticleLocation::Default );
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
   *  @param location Data location/address in Gaudi Transient Store
   *  @param useRootInTES Flag to turn on(TRUE) off(FALSE) the use of
   *                      the RootInTES location property
   *  @return         Pointer to the data object
   */
  template < class TYPE  >
  inline typename Gaudi::Utils::GetData<TYPE>::return_type
  get  ( const std::string& location  ,
         const bool useRootInTES = true ) const
  {
    return GaudiCommon<AlgTool>::get<TYPE> ( evtSvc() , location , useRootInTES ) ;
  }

  /** @brief Templated access to the data in Gaudi Transient Store
   *
   *  Quick and safe access to the data in Gaudi transient store.
   *  The method located the data at given address and perform the
   *  debug printout about located data. 
   *
   *  Skips the check on the data as performed by 'get'. No exception
   *  is thrown if the data is missing.
   *
   *  @code
   *
   *  MCHits* hits = getIfExists<MCHits>( "/Event/MC/Hits" );
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
   *  @param location data location/address in Gaudi Transient Store
   *  @param useRootInTES Flag to turn on(TRUE) off(FALSE) the use of
   *                      the RootInTES location property
   *
   *  @return pointer to the data object. 
   *  @retval NULL If data does not exist.
   */
  template < class TYPE  >
  inline typename Gaudi::Utils::GetData<TYPE>::return_type
  getIfExists ( const std::string& location  ,
                const bool useRootInTES = true ) const
  {
    return GaudiCommon<AlgTool>::getIfExists<TYPE> ( evtSvc() , location , useRootInTES ) ;
  }

  /** @brief Templated access to the detector data from the
   *         Gaudi Detector Transient Store
   *
   *  Quick and safe access to the detector data in Gaudi transient store.
   *
   *  The method located the detector at the given address and perform the
   *  debug printout about located detector.
   *
   *  @code
   *
   *  MyDet* mdet = getDet<MyDet>( detSvc() , "/dd/Structure/LHCb/MyDet" );
   *
   *  @endcode
   *
   *  @param svc       Pointer to data service (data provider)
   *  @param location  Detector location/address in Gaudi Transient Store
   *  @return          Pointer to the detector object
   */
  template < class TYPE  >
  inline TYPE* getDet ( IDataProviderSvc*  svc        ,
                        const std::string& location   ) const
  {
    return GaudiCommon<AlgTool>::get<TYPE> ( svc , location , false ) ;
  }

  /** @brief Templated access to the detector data from the
   *         Gaudi Detector Transient Store
   *
   *  Quick and safe access to the detector data in Gaudi transient store.
   *
   *  The method located the detector at the given address and perform the
   *  debug printout about located detector.
   * 
   *  Skips the check on the data as performed by 'get'. No exception
   *  is thrown if the data is missing.
   *
   *  @code
   *
   *  MyDet* mdet = getDetIfExists<MyDet>( detSvc() , "/dd/Structure/LHCb/MyDet" );
   *
   *  @endcode
   *
   *  @param svc       Pointer to data service (data provider)
   *  @param location  Detector location/address in Gaudi Transient Store
   *  @return          Pointer to the detector object
   *  @retval NULL If the detector object does not exist.
   */
  template < class TYPE  >
  inline typename Gaudi::Utils::GetData<TYPE>::return_type
  getDetIfExists ( IDataProviderSvc*  svc       ,
                   const std::string& location  ) const
  {
    return GaudiCommon<AlgTool>::getIfExists<TYPE> ( svc , location , false ) ;
  }

  /** @brief Templated access to the detector data from the
   *         Gaudi Detector Transient Store
   *
   *  Quick and safe access to the detector data in Gaudi transient store.
   *
   *  The method located the detector at the given address and perform the
   *  debug printout about located detector.
   *
   *  @code
   *
   *  MyDet* mdet = getDet<MyDet>( "/dd/Structure/LHCb/MyDet" );
   *
   *  @endcode
   *
   *  @param location  Detector location/address in Gaudi Transient Store
   *  @return          Pointer to the detector object
   */
  template < class TYPE  >
  inline TYPE* getDet ( const std::string& location ) const
  {
    return GaudiCommon<AlgTool>::get<TYPE> ( detSvc() , location , false ) ;
  }

  /** @brief Templated access to the detector data from the
   *         Gaudi Detector Transient Store
   *
   *  Quick and safe access to the detector data in Gaudi transient store.
   *
   *  The method located the detector at the given address and perform the
   *  debug printout about located detector.
   * 
   *  Skips the check on the data as performed by 'get'. No exception
   *  is thrown if the data is missing.
   *
   *  @code
   *
   *  MyDet* mdet = getDetIfExists<MyDet>( "/dd/Structure/LHCb/MyDet" );
   *
   *  @endcode
   *
   *  @param location  Detector location/address in Gaudi Transient Store
   *  @return          Pointer to the detector object
   *  @retval NULL If the detector object does not exist.
   */
  template < class TYPE  >
  inline typename Gaudi::Utils::GetData<TYPE>::return_type
  getDetIfExists ( const std::string& location ) const
  {
    return GaudiCommon<AlgTool>::getIfExists<TYPE> ( detSvc() , location , false ) ;
  }

  /** @brief Check the existence of a data object or container
   *         in the Gaudi Transient Event Store
   *
   *  @code
   *
   *  bool a1 = exist<DataObject>( evtSvc(), "/Event/MyObject" ) ;
   *  bool a2 = exist<MyHits>    ( evtSvc(), "/Event/MyHits"   ) ;
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
   *  @param svc       Pointer to data service (data provider)
   *  @param  location Address in Gaudi Transient Event Store
   *  @param useRootInTES Flag to turn on(TRUE) off(FALSE) the use of
   *                      the RootInTES location property
   *  @return          Boolean indicating status of the request
   *  @retval true     Data object or container exists and implements a proper interface
   *  @retval true     Failed to locate the data object or container
   */
  template < class TYPE  >
  inline bool  exist    ( IDataProviderSvc*  svc      ,
                          const std::string& location ,
                          const bool useRootInTES = true ) const
  {
    return GaudiCommon<AlgTool>::exist<TYPE> ( svc , location , useRootInTES ) ;
  }

  /** @brief Check the existence of a data object or container
   *         in the Gaudi Transient Event Store
   *
   *  @code
   *
   *  bool a1 = exist<DataObject>( "/Event/MyObject" ) ;
   *  bool a2 = exist<MyHits>    ( "/Event/MyHits"   ) ;
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
   *  @param  location Address in Gaudi Transient Event Store
   *  @return          Boolean indicating status of the request
   *  @retval true     Data object or container exists and implements a proper interface
   *  @retval true     Failed to locate the data object or container
   */
  template < class TYPE  >
  inline bool  exist    ( const std::string& location ,
                          const bool useRootInTES = true ) const
  {
    return GaudiCommon<AlgTool>::exist<TYPE> ( evtSvc() , location , useRootInTES ) ;
  }

  /** @brief Check the existence of detector objects in the Gaudi
   *         Transient Detector Store
   *
   *  @code
   *
   *  bool a1 = existDet<DataObject> ( detSvc(), "/dd/Structure/MyObject"     ) ;
   *  bool a2 = existDet<Material>   ( detSvc(), "/dd/Structure/Material/Air" ) ;
   *
   *  @endcode
   *
   *  @param svc       Pointer to data service (data provider)
   *  @param  location Address in Gaudi Transient Detector Store
   *  @return          Boolean indicating status of the request
   *  @retval true     Detector object exists and implements a proper interface
   *  @retval false    Failed to locate the data object
   */
  template < class TYPE  >
  inline bool  existDet    ( IDataProviderSvc*  svc      ,
                             const std::string& location ) const
  {
    return GaudiCommon<AlgTool>::exist<TYPE> ( svc , location , false ) ;
  }

  /** @brief Check the existence of detector objects in the Gaudi
   *         Transient Detector Store
   *
   *  @code
   *
   *  bool a1 = existDet<DataObject> ( "/dd/Structure/MyObject"     ) ;
   *  bool a2 = existDet<Material>   ( "/dd/Structure/Material/Air" ) ;
   *
   *  @endcode
   *
   *  @param  location Address in Gaudi Transient Detector Store
   *  @return          Boolean indicating status of the request
   *  @retval true     Detector object exists and implements a proper interface
   *  @retval false    Failed to locate the data object
   */
  template < class TYPE  >
  inline bool  existDet    ( const std::string& location ) const
  {
    return GaudiCommon<AlgTool>::exist<TYPE> ( detSvc() , location , false ) ;
  }

  /** @brief Get the existing data object from Gaudi Event Transient store.
   *         Alternatively, create new object and register it in TES
   *         and return if object does not exist.
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
   *  @param svc        Pointer to data service (data provider)
   *  @param  location  Location in Gaudi Transient Event Store
   *  @param useRootInTES Flag to turn on(TRUE) off(FALSE) the use of
   *                      the RootInTES location property
   *  @return A valid pointer to the object
   */
  template < class TYPE , class TYPE2 >
  inline typename Gaudi::Utils::GetData<TYPE>::return_type
  getOrCreate ( IDataProviderSvc*  svc                 ,
                const std::string& location            ,
                const bool         useRootInTES = true ) const
  {
    return GaudiCommon<AlgTool>::getOrCreate<TYPE,TYPE2> ( svc , location , useRootInTES ) ;
  }

  /** @brief Get the existing data object from Gaudi Event Transient store.
   *         Alternatively, create new object and register it in TES
   *         and return if object does not exist.
   *
   *  @code
   *
   *  MyHits* hits = getOrCreate<MyHits,MyHits>( "/Event/MyHits" ) ;
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
   *  @param  location  Location in Gaudi Transient Event Store
   *  @param useRootInTES Flag to turn on(TRUE) off(FALSE) the use of
   *                      the RootInTES location property
   *  @return A valid pointer to the object
   */
  template < class TYPE , class TYPE2 >
  inline typename Gaudi::Utils::GetData<TYPE>::return_type
  getOrCreate ( const std::string& location            ,
                const bool         useRootInTES = true ) const
  {
    return GaudiCommon<AlgTool>::getOrCreate<TYPE,TYPE2> ( evtSvc() , location , useRootInTES ) ;
  }

  // ==========================================================================
public:
  // ==========================================================================
  /// enable/disable summary
  static bool enableSummary  ( bool ) ;               // enable/disable summary
  /// is summary enabled?
  static bool summaryEnabled (      ) ;               // is summary enabled?
  // ==========================================================================
protected:
  // ==========================================================================
  /** Standard constructor
   *  @see AlgTool
   *  @param type tool type (useless)
   *  @param name tool name
   *  @param parent pointer to parent object (service, algorithm or tool)
   */
  GaudiTool ( const std::string& type   ,
              const std::string& name   ,
              const IInterface*  parent );

  /// destructor, virtual and protected
  virtual ~GaudiTool();
  // ==========================================================================
private:
  // ==========================================================================
  /// no public default constructor
  GaudiTool();
  /// no public copy constructor
  GaudiTool             ( const GaudiTool& );
  /// no public assignment operator
  GaudiTool& operator = ( const GaudiTool& );
  // ==========================================================================
private:
  // ==========================================================================
  /// pointer to the N-Tuple service
  mutable INTupleSvc*    m_ntupleSvc          ;
  /// pointer to the event tag collection service
  mutable INTupleSvc*    m_evtColSvc          ;
  /// pointer to Event Data Service
  mutable IDataProviderSvc* m_evtSvc          ;
  /// pointer to Detector Data Service
  mutable IDataProviderSvc* m_detSvc          ;
  /// pointer to Chrono & Stat Service
  mutable IChronoStatSvc*   m_chronoSvc       ;
  /// pointer to Incident Service
  mutable IIncidentSvc*     m_incSvc          ;
  /// pointer for histogram service
  mutable IHistogramSvc *  m_histoSvc         ;
  // Pointer to the Algorithm Context Service
  mutable IAlgContextSvc* m_contextSvc     ; ///< Algorithm Context Service
  // The name of the Algorithm Context Service
  std::string             m_contextSvcName ; ///< Algorithm Context Service
  // ==========================================================================
private:
  // ==========================================================================
  /// full tool name "type/name"
  const std::string m_local ;
  // ==========================================================================
private:
  // ==========================================================================
  /// enable printout of summary?
  static bool s_enableSummary ;  // enable printout of summary?
  // ==========================================================================
};
// ============================================================================

// ============================================================================
// The END
// ============================================================================
#endif  // GAUDIALG_GAUDITOOL_H
// ============================================================================


