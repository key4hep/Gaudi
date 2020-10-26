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
#pragma once

#include <GAUDI_VERSION.h>

#if GAUDI_MAJOR_VERSION < 999
#  if GAUDI_VERSION >= CALC_GAUDI_VERSION( 35, 0 )
#    error "deprecated header: removed in v35r0, use <Gaudi/PropertyFwd.h>"
#  elif GAUDI_VERSION >= CALC_GAUDI_VERSION( 34, 0 )
#    warning "deprecated header: to be removed in v35r0, use <Gaudi/PropertyFwd.h>"
#  endif
#endif

#include <Gaudi/PropertyFwd.h>
