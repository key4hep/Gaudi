/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "COPYING".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once
/** @file Declaration of preprocessor macros to help schedule deprecations. */

#include <GAUDI_VERSION.h>

#if GAUDI_VERSION >= CALC_GAUDI_VERSION( 34, 0 ) && GAUDI_MAJOR_VERSION < 999
#  define GAUDI_DEPRECATED_SINCE_v34r0( REASON ) [[deprecated( REASON )]]
#else
#  define GAUDI_DEPRECATED_SINCE_v34r0( REASON )
#endif
