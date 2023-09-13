/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIALG_GaudiAlgorithm_H
#define GAUDIALG_GaudiAlgorithm_H 1
// ============================================================================
// Include files
// ============================================================================
#include <string>
#include <vector>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Algorithm.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlg.h"
#include "GaudiAlg/GaudiCommon.h"
#include "GaudiKernel/DataObjectHandle.h"
// ============================================================================
// forward declarations:
// ============================================================================
class INTupleSvc;

// ============================================================================
/** @file GaudiAlgorithm.h
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
class GAUDI_API GaudiAlgorithm : public GaudiCommon<Algorithm> {
public:
  // ==========================================================================
  /** standard initialization method
   *  @see  Algorithm
   *  @see IAlgorithm
   *  @return status code
   */
  StatusCode initialize() override;
  // ==========================================================================
  /** standard execution method
   *  @see  Algorithm
   *  @see IAlgorithm
   *  @return status code
   */
  StatusCode execute() override;
  // ==========================================================================
  /** standard finalization method
   *  @see  Algorithm
   *  @see IAlgorithm
   *  @return status code
   */
  StatusCode finalize() override;
  // ==========================================================================
  /** the generic actions for the execution.
   *  @see  Algorithm
   *  @see IAlgorithm
   *  @see Algorithm::sysExecute
   *  @return status code
   */
  StatusCode sysExecute( const EventContext& ctx ) override;
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
  // [[deprecated( "please pass a std::unique_ptr as 2nd argument" )]]
  void put( IDataProviderSvc* svc, DataObject* object, const std::string& address,
            const bool useRootInTES = true ) const {
    put( svc, std::unique_ptr<DataObject>( object ), address, useRootInTES );
  }
  void put( IDataProviderSvc* svc, std::unique_ptr<DataObject> object, const std::string& address,
            const bool useRootInTES = true ) const {
    GaudiCommon<Algorithm>::put( svc, std::move( object ), address, useRootInTES );
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
  const DataObject* put( DataObject* object, const std::string& address, const bool useRootInTES = true ) const {
    return put( std::unique_ptr<DataObject>( object ), address, useRootInTES );
  }
  const DataObject* put( std::unique_ptr<DataObject> object, const std::string& address,
                         const bool useRootInTES = true ) const {
    return GaudiCommon<Algorithm>::put( evtSvc(), std::move( object ), address, useRootInTES );
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
  template <class TYPE>
  typename Gaudi::Utils::GetData<TYPE>::return_type get( IDataProviderSvc* svc, const std::string& location,
                                                         const bool useRootInTES = true ) const {
    return GaudiCommon<Algorithm>::get<TYPE>( svc, location, useRootInTES );
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
  template <class TYPE>
  typename Gaudi::Utils::GetData<TYPE>::return_type getIfExists( IDataProviderSvc* svc, const std::string& location,
                                                                 const bool useRootInTES = true ) const {
    return GaudiCommon<Algorithm>::getIfExists<TYPE>( svc, location, useRootInTES );
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
  template <class TYPE>
  typename Gaudi::Utils::GetData<TYPE>::return_type get( const std::string& location,
                                                         const bool         useRootInTES = true ) const {
    return GaudiCommon<Algorithm>::get<TYPE>( evtSvc(), location, useRootInTES );
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
  template <class TYPE>
  typename Gaudi::Utils::GetData<TYPE>::return_type getIfExists( const std::string& location,
                                                                 const bool         useRootInTES = true ) const {
    return GaudiCommon<Algorithm>::getIfExists<TYPE>( evtSvc(), location, useRootInTES );
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
  template <class TYPE>
  TYPE* getDet( IDataProviderSvc* svc, const std::string& location ) const {
    return GaudiCommon<Algorithm>::get<TYPE>( svc, location, false );
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
  template <class TYPE>
  typename Gaudi::Utils::GetData<TYPE>::return_type getDetIfExists( IDataProviderSvc*  svc,
                                                                    const std::string& location ) const {
    return GaudiCommon<Algorithm>::getIfExists<TYPE>( svc, location, false );
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
  template <class TYPE>
  TYPE* getDet( const std::string& location ) const {
    return GaudiCommon<Algorithm>::get<TYPE>( detSvc(), location, false );
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
  template <class TYPE>
  typename Gaudi::Utils::GetData<TYPE>::return_type getDetIfExists( const std::string& location ) const {
    return GaudiCommon<Algorithm>::getIfExists<TYPE>( detSvc(), location, false );
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
  template <class TYPE>
  bool exist( IDataProviderSvc* svc, const std::string& location, const bool useRootInTES = true ) const {
    return GaudiCommon<Algorithm>::exist<TYPE>( svc, location, useRootInTES );
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
  template <class TYPE>
  bool exist( const std::string& location, const bool useRootInTES = true ) const {
    return GaudiCommon<Algorithm>::exist<TYPE>( evtSvc(), location, useRootInTES );
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
  template <class TYPE>
  bool existDet( IDataProviderSvc* svc, const std::string& location ) const {
    return GaudiCommon<Algorithm>::exist<TYPE>( svc, location, false );
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
  template <class TYPE>
  bool existDet( const std::string& location ) const {
    return GaudiCommon<Algorithm>::exist<TYPE>( detSvc(), location, false );
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
  template <class TYPE, class TYPE2>
  typename Gaudi::Utils::GetData<TYPE>::return_type getOrCreate( IDataProviderSvc* svc, std::string_view location,
                                                                 const bool useRootInTES = true ) const {
    return GaudiCommon<Algorithm>::getOrCreate<TYPE, TYPE2>( svc, location, useRootInTES );
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
  template <class TYPE, class TYPE2>
  typename Gaudi::Utils::GetData<TYPE>::return_type getOrCreate( const std::string& location,
                                                                 const bool         useRootInTES = true ) const {
    return GaudiCommon<Algorithm>::getOrCreate<TYPE, TYPE2>( evtSvc(), location, useRootInTES );
  }

  // ==========================================================================
  /** Standard constructor
   *  @see  Algorithm
   *  @param name           name of the algorithm
   *  @param pSvcLocator    pointer to Service Locator
   */
  GaudiAlgorithm( std::string name, ISvcLocator* pSvcLocator );
  // ==========================================================================
  //
  // no default/copy constructor, no assignment -- except that ROOT really
  // wants a default constructor declared. So we define it, and don't implement
  // it...
  GaudiAlgorithm( const GaudiAlgorithm& )            = delete;
  GaudiAlgorithm& operator=( const GaudiAlgorithm& ) = delete;
  // ==========================================================================
public:
  // ==========================================================================
  /** Access the standard event collection service
   *  @return pointer to the event collection service
   */
  SmartIF<INTupleSvc>& evtColSvc() const;

  // ==========================================================================
private:
  // ==========================================================================
  // Pointer to the Event Tag Collection Service
  mutable SmartIF<INTupleSvc> m_evtColSvc; ///< Event Tag Collection Service

  Gaudi::Property<std::vector<std::string>> m_vetoObjs{
      this, "VetoObjects", {}, "skip execute if one or more of these TES objects exist" };
  Gaudi::Property<std::vector<std::string>> m_requireObjs{
      this, "RequireObjects", {}, "execute only if one or more of these TES objects exist" };
  // ==========================================================================
};
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_GaudiAlgorithm_H
