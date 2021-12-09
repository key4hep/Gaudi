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
#include "GaudiKernel/DataObjectHandle.h"
// ============================================================================
// forward declarations
// ============================================================================
class IDataProviderSvc;
class IChronoStatSvc;
class IIncidentSvc;
class IHistogramSvc;
class GaudiException;
class INTupleSvc;
class IAlgContextSvc;
namespace std {
  class exception;
}
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
class GAUDI_API GaudiTool : public GaudiCommon<AlgTool> {
public:
  // ==========================================================================
  /** standard initialization method
   *  @see  AlgTool
   *  @see IAlgTool
   *  @return status code
   */
  StatusCode initialize() override;
  /** standard finalization method
   *  @see  AlgTool
   *  @see IAlgTool
   *  @return status code
   */
  StatusCode finalize() override;
  // ==========================================================================
public: // accessors
        // ==========================================================================
        /** Access the standard N-Tuple
         *  @return pointer to N-Tuple service .
         */
  INTupleSvc* ntupleSvc() const;
  /** Access the standard event collection service
   *  @return pointer to the event collection service
   */
  INTupleSvc* evtColSvc() const;
  /** accessor to detector service
   *  @return pointer to detector service
   */
  IDataProviderSvc* detSvc() const;

  /** accessor to Incident Service
   *  @return pointer to the Incident Service
   */
  IIncidentSvc* incSvc() const;
  /** accessor to Chrono & Stat Service
   *  @return pointer to the Chrono & Stat Service
   */
  IChronoStatSvc* chronoSvc() const;
  /** acessor to the histogram service
   *  @return pointer to the histogram service
   */
  IHistogramSvc* histoSvc() const;
  /** acessor to the Algorithm Context Service
   *  @return pointer to the Algorithm Context Service
   */
  IAlgContextSvc* contextSvc() const;
  // ==========================================================================
public:
  // ==========================================================================
  // following methods cannot go in GaudiCommon since they use methods ( evtSvc()
  // and detSvc() ) that are not members of AlgTool.
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
  // [[deprecated( "please pass a std::unique_ptr as 2nd argument" )]]
  void put( IDataProviderSvc* svc, DataObject* object, const std::string& address,
            const bool useRootInTES = true ) const {
    put( svc, std::unique_ptr<DataObject>( object ), address, useRootInTES );
  }
  void put( IDataProviderSvc* svc, std::unique_ptr<DataObject> object, const std::string& address,
            const bool useRootInTES = true ) const {
    GaudiCommon<AlgTool>::put( svc, std::move( object ), address, useRootInTES );
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
  const DataObject* put( DataObject* object, const std::string& address, const bool useRootInTES = true ) const {
    return put( std::unique_ptr<DataObject>( object ), address, useRootInTES );
  }
  const DataObject* put( std::unique_ptr<DataObject> object, const std::string& address,
                         const bool useRootInTES = true ) const {
    return GaudiCommon<AlgTool>::put( evtSvc(), std::move( object ), address, useRootInTES );
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
  template <class TYPE>
  typename Gaudi::Utils::GetData<TYPE>::return_type get( IDataProviderSvc* svc, const std::string& location,
                                                         const bool useRootInTES = true ) const {
    return GaudiCommon<AlgTool>::get<TYPE>( svc, location, useRootInTES );
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
    return GaudiCommon<AlgTool>::getIfExists<TYPE>( svc, location, useRootInTES );
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
  template <class TYPE>
  typename Gaudi::Utils::GetData<TYPE>::return_type get( const std::string& location,
                                                         const bool         useRootInTES = true ) const {
    return GaudiCommon<AlgTool>::get<TYPE>( evtSvc(), location, useRootInTES );
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
    return GaudiCommon<AlgTool>::getIfExists<TYPE>( evtSvc(), location, useRootInTES );
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
    return GaudiCommon<AlgTool>::get<TYPE>( svc, location, false );
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
    return GaudiCommon<AlgTool>::getIfExists<TYPE>( svc, location, false );
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
    return GaudiCommon<AlgTool>::get<TYPE>( detSvc(), location, false );
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
    return GaudiCommon<AlgTool>::getIfExists<TYPE>( detSvc(), location, false );
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
  template <class TYPE>
  bool exist( IDataProviderSvc* svc, const std::string& location, const bool useRootInTES = true ) const {
    return GaudiCommon<AlgTool>::exist<TYPE>( svc, location, useRootInTES );
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
  template <class TYPE>
  bool exist( const std::string& location, const bool useRootInTES = true ) const {
    return GaudiCommon<AlgTool>::exist<TYPE>( evtSvc(), location, useRootInTES );
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
    return GaudiCommon<AlgTool>::exist<TYPE>( svc, location, false );
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
    return GaudiCommon<AlgTool>::exist<TYPE>( detSvc(), location, false );
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
  template <class TYPE, class TYPE2>
  typename Gaudi::Utils::GetData<TYPE>::return_type getOrCreate( IDataProviderSvc* svc, const std::string& location,
                                                                 const bool useRootInTES = true ) const {
    return GaudiCommon<AlgTool>::getOrCreate<TYPE, TYPE2>( svc, location, useRootInTES );
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
  template <class TYPE, class TYPE2>
  typename Gaudi::Utils::GetData<TYPE>::return_type getOrCreate( const std::string& location,
                                                                 const bool         useRootInTES = true ) const {
    return GaudiCommon<AlgTool>::getOrCreate<TYPE, TYPE2>( evtSvc(), location, useRootInTES );
  }
  // ==========================================================================
public:
  // ==========================================================================
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
  StatusCode Error( const std::string& msg, const StatusCode st = StatusCode::FAILURE, const size_t mx = 10 ) const {
    return GaudiCommon<AlgTool>::Error( m_isPublic ? msg + getCurrentAlgName() : msg, st, mx );
  }
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
  StatusCode Warning( const std::string& msg, const StatusCode st = StatusCode::FAILURE, const size_t mx = 10 ) const {
    return GaudiCommon<AlgTool>::Warning( m_isPublic ? msg + getCurrentAlgName() : msg, st, mx );
  }
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
  StatusCode Info( const std::string& msg, const StatusCode st = StatusCode::SUCCESS, const size_t mx = 10 ) const {
    return GaudiCommon<AlgTool>::Info( m_isPublic ? msg + getCurrentAlgName() : msg, st, mx );
  }
  // ==========================================================================
public:
  // ==========================================================================
  /// enable/disable summary
  static bool enableSummary( bool );
  /// is summary enabled?
  static bool summaryEnabled();

  // ==========================================================================
  /** Standard constructor
   *  @see AlgTool
   *  @param type tool type (useless)
   *  @param name tool name
   *  @param parent pointer to parent object (service, algorithm or tool)
   */
  GaudiTool( std::string type, std::string name, const IInterface* parent );

  /// destructor, virtual and protected
  ~GaudiTool() override;
  // ==========================================================================
private:
  // ==========================================================================
  /// no copy constructor, no assignment
  GaudiTool( const GaudiTool& ) = delete;
  GaudiTool& operator=( const GaudiTool& ) = delete;
  // ==========================================================================
private:
  // ==========================================================================
  /// Determines if this tool is public or not (i.e. owned by the ToolSvc).
  bool isPublic() const;
  /// Returns the current active algorithm name via the context service
  std::string getCurrentAlgName() const {
    const IAlgContextSvc* asvc    = this->contextSvc();
    const IAlgorithm*     current = ( asvc ? asvc->currentAlg() : NULL );
    return ( current ? " [" + current->name() + "]" : "" );
  }
  // ==========================================================================
private:
  // ==========================================================================
  /// pointer to the N-Tuple service
  mutable SmartIF<INTupleSvc> m_ntupleSvc;
  /// pointer to the event tag collection service
  mutable SmartIF<INTupleSvc> m_evtColSvc;
  /// pointer to Detector Data Service
  mutable SmartIF<IDataProviderSvc> m_detSvc;
  /// pointer to Chrono & Stat Service
  mutable SmartIF<IChronoStatSvc> m_chronoSvc;
  /// pointer to Incident Service
  mutable SmartIF<IIncidentSvc> m_incSvc;
  /// pointer for histogram service
  mutable SmartIF<IHistogramSvc> m_histoSvc;
  // Pointer to the Algorithm Context Service
  mutable SmartIF<IAlgContextSvc> m_contextSvc; ///< Algorithm Context Service

  Gaudi::Property<std::string> m_contextSvcName{ this, "ContextService", "AlgContextSvc",
                                                 "the name of Algorithm Context Service" };
  // ==========================================================================
private:
  // ==========================================================================
  /// Flag to say if the tool is a public or private tool
  bool m_isPublic = false;
  // ==========================================================================
private:
  // ==========================================================================
  /// full tool name "type/name"
  const std::string m_local;
  // ==========================================================================
private:
  // ==========================================================================
  /// enable printout of summary?
  static bool s_enableSummary;
  // ==========================================================================
};
// ============================================================================

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_GAUDITOOL_H
