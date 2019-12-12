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
#ifndef GAUDIKERNEL_ICPUCRUNCHSVC_H
#define GAUDIKERNEL_ICPUCRUNCHSVC_H 1

#include "GaudiKernel/IService.h"
#include <chrono>

class GAUDI_API ICPUCrunchSvc : virtual public IService {

public:
  virtual ~ICPUCrunchSvc(){};

  DeclareInterfaceID( ICPUCrunchSvc, 1, 0 );

public:
  virtual std::chrono::milliseconds crunch_for( const std::chrono::milliseconds& ) const = 0;
};

#endif
