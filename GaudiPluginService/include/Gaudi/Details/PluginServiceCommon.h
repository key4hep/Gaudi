/***********************************************************************************\
* (c) Copyright 2013-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef _GAUDI_PLUGIN_SERVICE_COMMON_H_

/// @author Marco Clemencic <marco.clemencic@cern.ch>

#  ifndef GAUDI_PLUGIN_SERVICE_USE_V2
#    if defined( GAUDI_PLUGIN_SERVICE_V2 ) || !defined( GAUDI_PLUGIN_SERVICE_V1 )
#      define GAUDI_PLUGIN_SERVICE_V2_INLINE inline
#      define GAUDI_PLUGIN_SERVICE_V1_INLINE
#      define GAUDI_PLUGIN_SERVICE_USE_V2 1
#    else
#      define GAUDI_PLUGIN_SERVICE_V2_INLINE
#      define GAUDI_PLUGIN_SERVICE_V1_INLINE inline
#      define GAUDI_PLUGIN_SERVICE_USE_V2 0
#    endif
#  endif

#  define GAUDIPS_HASCLASSVISIBILITY

#  if defined( GAUDIPS_HASCLASSVISIBILITY )
#    define GAUDIPS_IMPORT __attribute__( ( visibility( "default" ) ) )
#    define GAUDIPS_EXPORT __attribute__( ( visibility( "default" ) ) )
#    define GAUDIPS_LOCAL __attribute__( ( visibility( "hidden" ) ) )
#  else
#    define GAUDIPS_IMPORT
#    define GAUDIPS_EXPORT
#    define GAUDIPS_LOCAL
#  endif

#  ifdef GaudiPluginService_EXPORTS
#    define GAUDIPS_API GAUDIPS_EXPORT
#  else
#    define GAUDIPS_API GAUDIPS_IMPORT
#  endif

#endif
