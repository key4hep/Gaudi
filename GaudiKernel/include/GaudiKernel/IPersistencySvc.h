/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

// Include files
#include <GaudiKernel/IInterface.h>
#include <GaudiKernel/Kernel.h>

// Forward declarations
// Generic interface to data object class
class IConversionSvc;

/** @class IPersistencySvc IPersistencySvc.h GaudiKernel/IPersistencySvc.h

    Data persistency service interface.

    @author Markus Frank
    @version 1.0
*/
class GAUDI_API IPersistencySvc : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( IPersistencySvc, 2, 0 );

  /** Set default service type.
      @return     Status code indicating success or failure.
      @param      type Storage technology type.
  */
  virtual StatusCode setDefaultCnvService( long type ) = 0;

  /** Add new Conversion Service.
      @return     Status code indicating success or failure.
      @param      service Pointer to conversion service interface.
  */
  virtual StatusCode addCnvService( IConversionSvc* service ) = 0;

  /** Remove Conversion Service.
      @return     Status code indicating success or failure.
      @param      type Storage technology type.
  */
  virtual StatusCode removeCnvService( long type ) = 0;

  /** Retrieve conversion service identified by technology
      @return     Status code indicating success or failure.
      @param      service_type Storage technology type.
      @param      refpSvc      Reference to pointer to interface of required service.
  */
  virtual StatusCode getService( long service_type, IConversionSvc*& refpSvc ) = 0;
  /** Retrieve conversion service identified by technology
      @return     Status code indicating success or failure.
      @param      service_type Storage technology type (ROOT, MS Access, ....)
      @param      refpSvc      Reference to pointer to interface of required service.
  */
  virtual StatusCode getService( const std::string& service_type, IConversionSvc*& refpSvc ) = 0;
};
