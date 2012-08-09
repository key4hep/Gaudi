// $Id: GaudiAlgorithm.h,v 1.19 2008/11/04 22:49:25 marcocle Exp $
// ============================================================================
#ifndef GAUDIALG_GaudiAlgorithm_H
#define GAUDIALG_GaudiAlgorithm_H 1
// ============================================================================
// Include files
// ============================================================================
#include <vector>
#include <string>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Algorithm.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlg.h"
#include "GaudiAlg/GaudiCommon.h"
// ============================================================================
// forward declarations:
// ============================================================================
class                   INTupleSvc     ; // GaudiKernel
template<class T> class  AlgFactory    ; // GaudiKernel
// ============================================================================
/* @file GaudiAlgorithm.h
 *
 * Header file for class GaudiAlgorithm.
 * The actual code is mainly imported from
 *   - LHCb Calorimeter software and
 *   - LHCb C++ toolkit for smart and friendly physics analysis "LoKi"
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
 *  significant shrinkage of existing code lines.
 *
 *  @attention
 *  See also the class GaudiCommon, which implements some of the common
 *  functionality between GaudiTool and GaudiAlgorithm.
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date 30/06/2001
 */
// ============================================================================
/** @class  GaudiAlgorithm GaudiAlgorithm.h GaudiAlg/GaudiAlgorithm.h
 *
 *  The useful base class for data processing algorithms.
 *  Small extension of ordinary the Algorithm base class.
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
 *  significant shrinkage of existing code lines.
 *
 *  @attention
 *  See also the class GaudiCommon, which implements some of the common
 *  functionality between GaudiTool and GaudiAlgorithm.
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   30/06/2001
 */
// ============================================================================
class GAUDI_API GaudiAlgorithm: public GaudiCommon<Algorithm>
{
  // friend factory for instantiation
  friend class AlgFactory<GaudiAlgorithm>;
public:
  // ==========================================================================
  /** standard initialization method
   *  @see  Algorithm
   *  @see IAlgorithm
   *  @return status code
   */
  virtual StatusCode initialize();
  // ==========================================================================
  /** standard execution method
   *  @see  Algorithm
   *  @see IAlgorithm
   *  @return status code
   */
  virtual StatusCode execute   ();
  // ==========================================================================
  /** standard finalization method
   *  @see  Algorithm
   *  @see IAlgorithm
   *  @return status code
   */
  virtual StatusCode finalize  ();
  // ==========================================================================
  /** the generic actions for the execution.
   *  @see  Algorithm
   *  @see IAlgorithm
   *  @see Algorithm::sysExecute
   *  @return status code
   */
  virtual StatusCode sysExecute () ;
  // ==========================================================================
public:

  // following methods cannot go in GaudiCommon since they use methods ( evtSvc()
  // and detDvc() ) that are not members of AlgTool.
  // Also some methods seem which are members of the base class do not seem
  // to be found unless forwarding methods are put here ??

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
   *
   *  @exception        GaudiException for invalid event data service
   *  @exception        GaudiException for invalid object
   *  @exception        GaudiException for error result from event data service
   *
   *  @return StatusCode
   *  @retval StatusCode::SUCCESS Data was successfully placed in the TES.
   *  @retval StatusCode::FAILURE Failed to store data in the TES.
   */
  inline void put ( IDataProviderSvc*  svc     ,
                    DataObject*        object  ,
                    const std::string& address ,
                    const bool useRootInTES = true ) const
  {
    GaudiCommon<Algorithm>::put ( svc , object , address , useRootInTES ) ;
  }

  /** @brief Register a data object or container into Gaudi Event Transient Store
   *
   *  @see IDataProviderSvc
   *
   *  @code
   *
   *  MCHits * hits = new MCHits();
   *  put( hits, "/Event/MC/Hits" );
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
   *
   *  @exception        GaudiException for invalid event data service
   *  @exception        GaudiException for invalid object
   *  @exception        GaudiException for error result from event data service
   *
   *  @return StatusCode
   *  @retval StatusCode::SUCCESS Data was successfully placed in the TES.
   *  @retval StatusCode::FAILURE Failed to store data in the TES.
   */
  inline void put ( DataObject*        object   ,
                    const std::string& address  ,
                    const bool useRootInTES = true ) const
  {
    GaudiCommon<Algorithm>::put ( evtSvc() , object , address , useRootInTES ) ;
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
   *
   *  @return pointer to the data object
   */
  template < class TYPE  >
  inline typename Gaudi::Utils::GetData<TYPE>::return_type
  get  ( IDataProviderSvc*  svc       ,
         const std::string& location  ,
         const bool useRootInTES = true ) const
  {
    return GaudiCommon<Algorithm>::get<TYPE> ( svc , location , useRootInTES ) ;
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
    return GaudiCommon<Algorithm>::getIfExists<TYPE> ( svc , location , useRootInTES ) ;
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
   *
   *  @return         Pointer to the data object
   */
  template < class TYPE  >
  inline typename Gaudi::Utils::GetData<TYPE>::return_type
  get  ( const std::string& location,
         const bool useRootInTES = true ) const
  {
    return GaudiCommon<Algorithm>::get<TYPE> ( evtSvc() , location , useRootInTES ) ;
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
    return GaudiCommon<Algorithm>::getIfExists<TYPE> ( evtSvc() , location , useRootInTES ) ;
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
    return GaudiCommon<Algorithm>::get<TYPE> ( svc , location , false ) ;
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
    return GaudiCommon<Algorithm>::getIfExists<TYPE> ( svc , location , false ) ;
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
    return GaudiCommon<Algorithm>::get<TYPE> ( detSvc() , location , false ) ;
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
    return GaudiCommon<Algorithm>::getIfExists<TYPE> ( detSvc() , location , false ) ;
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
   *
   *  @return          Boolean indicating status of the request
   *  @retval true     Data object or container exists and implements a proper interface
   *  @retval true     Failed to locate the data object or container
   */
  template < class TYPE  >
  inline bool  exist    ( IDataProviderSvc*  svc      ,
                          const std::string& location ,
                          const bool useRootInTES = true ) const
  {
    return GaudiCommon<Algorithm>::exist<TYPE> ( svc , location , useRootInTES ) ;
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
   *  @param useRootInTES Flag to turn on(TRUE) off(FALSE) the use of
   *                      the RootInTES location property
   *
   *  @return          Boolean indicating status of the request
   *  @retval true     Data object or container exists and implements a proper interface
   *  @retval true     Failed to locate the data object or container
   */
  template < class TYPE  >
  inline bool  exist    ( const std::string& location ,
                          const bool useRootInTES = true ) const
  {
    return GaudiCommon<Algorithm>::exist<TYPE> ( evtSvc() , location , useRootInTES ) ;
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
    return GaudiCommon<Algorithm>::exist<TYPE> ( svc , location , false ) ;
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
    return GaudiCommon<Algorithm>::exist<TYPE> ( detSvc() , location , false ) ;
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
   *
   *  @return A valid pointer to the object
   */
  template < class TYPE , class TYPE2 >
  inline typename Gaudi::Utils::GetData<TYPE>::return_type
  getOrCreate ( IDataProviderSvc*  svc      ,
                const std::string& location ,
                const bool useRootInTES = true ) const
  {
    return GaudiCommon<Algorithm>::getOrCreate<TYPE,TYPE2> ( svc , location , useRootInTES ) ;
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
   *
   *  @return A valid pointer to the object
   */
  template < class TYPE , class TYPE2 >
  inline typename Gaudi::Utils::GetData<TYPE>::return_type
  getOrCreate ( const std::string& location            ,
                const bool         useRootInTES = true ) const
  {
    return GaudiCommon<Algorithm>::getOrCreate<TYPE,TYPE2> ( evtSvc() , location , useRootInTES ) ;
  }
public:
  // ==========================================================================
  /** Standard constructor (protected)
   *  @see  Algorithm
   *  @param name           name of the algorithm
   *  @param pSvcLocator    pointer to Service Locator
   */
  GaudiAlgorithm ( const std::string& name        ,
                   ISvcLocator*       pSvcLocator );
  // ==========================================================================
  /// destructor, virtual and protected
  virtual ~GaudiAlgorithm();
  // ==========================================================================
public:
  // ==========================================================================
  /** Access the standard event collection service
   *  @return pointer to the event collection service
   */
  SmartIF<INTupleSvc>&     evtColSvc  () const;
  // ==========================================================================
private:
  // ==========================================================================
  // no public default constructor
  GaudiAlgorithm(); ///< no public default constructor
  // ==========================================================================
  // no public copy constructor
  GaudiAlgorithm             ( const GaudiAlgorithm& ); ///< no public copy
  // ==========================================================================
  // no public assignment operator
  GaudiAlgorithm& operator = ( const GaudiAlgorithm& ); ///< no public assignment
  // ==========================================================================
private:
  // ==========================================================================
  // Pointer to the Event Tag Collection Service
  mutable SmartIF<INTupleSvc> m_evtColSvc ; ///< Event Tag Collection Service
  /// skip the event if any of these objects are present in TES
  std::vector<std::string> m_vetoObjs;
  /// process the event only if one or more of these objects are present in TES
  std::vector<std::string> m_requireObjs;
  // ==========================================================================
};
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_GaudiAlgorithm_H
// ============================================================================


