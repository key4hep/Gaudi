/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "ServiceA.h"

// Static Factory declaration
DECLARE_COMPONENT( ServiceA )

StatusCode ServiceA::initialize() {
  Service::initialize().ignore();
  auto pService = service<IService>( "ServiceB", true );
  return pService.isValid() ? StatusCode::SUCCESS : StatusCode::FAILURE;
}
