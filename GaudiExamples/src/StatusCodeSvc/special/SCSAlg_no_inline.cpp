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
#include "../SCSAlg.h"
#include "HackedStatusCode.h"

#if __cplusplus >= 201703L && !defined( __CLING__ )
#  pragma GCC diagnostic ignored "-Wunused-result"
#endif

void SCSAlg::fncUnchecked2() { test(); }
