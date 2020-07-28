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
#ifndef KERNEL_IJOBOPTIONSSVC_H
#define KERNEL_IJOBOPTIONSSVC_H
// Include files
#include "GaudiKernel/IInterface.h"
#include <Gaudi/DeprecationHelpers.h>
#include <Gaudi/PropertyFwd.h>

#include <string>
#include <vector>

// Forward declaration
class StatusCode;
class IProperty;

#if GAUDI_MAJOR_VERSION < 999
#  if GAUDI_VERSION >= CALC_GAUDI_VERSION( 35, 0 )
#    error "deprecated header: removed in v35r0"
#  elif GAUDI_VERSION >= CALC_GAUDI_VERSION( 34, 0 )
#    warning "deprecated header: to be removed in v35r0"
#  endif
#endif

/** @class IJobOptionsSvc IJobOptionsSvc.h GaudiKernel/IJobOptionsSvc.h

    Main interface for the JobOptions service

    @author Pere Mato
*/
class GAUDI_API IJobOptionsSvc : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( IJobOptionsSvc, 4, 1 );

  /** Override default properties of the calling client
      @param client Name of the client algorithm or service
      @param me Address of the interface IProperty of the client
  */
  GAUDI_DEPRECATED_SINCE_v34r0( "will be removed in v34r0" ) virtual StatusCode
      setMyProperties( const std::string& client, IProperty* me ) = 0;

  /// Add a property into the JobOptions catalog
  GAUDI_DEPRECATED_SINCE_v34r0( "will be removed in v34r0" ) virtual StatusCode
      addPropertyToCatalogue( const std::string& client, const Gaudi::Details::PropertyBase& property ) = 0;
  /// Remove a property from the JobOptions catalog
  GAUDI_DEPRECATED_SINCE_v34r0( "will be removed in v34r0" ) virtual StatusCode
      removePropertyFromCatalogue( const std::string& client, const std::string& name ) = 0;
  /// Get the properties associated to a given client
  GAUDI_DEPRECATED_SINCE_v34r0( "will be removed in v34r0" ) virtual const
      std::vector<const Gaudi::Details::PropertyBase*>* getProperties( const std::string& client ) const = 0;

  /// Get a property for a client
  GAUDI_DEPRECATED_SINCE_v34r0( "will be removed in v34r0" ) virtual const
      Gaudi::Details::PropertyBase* getClientProperty( const std::string& client, const std::string& name ) const = 0;

  /// Get the list of clients
  GAUDI_DEPRECATED_SINCE_v34r0( "will be removed in v34r0" ) virtual std::vector<std::string> getClients() const = 0;

  /** look for file 'File' into search path 'Path'
   *  and read it to update existing JobOptionsCatalogue
   *  @param File file   name
   *  @param Path search path
   *  @return status code
   */
  GAUDI_DEPRECATED_SINCE_v34r0( "will be removed in v34r0" ) virtual StatusCode
      readOptions( const std::string& file, const std::string& path = "" ) = 0;
};

#endif // KERNEL_IJOBOPTIONSSVC_H
