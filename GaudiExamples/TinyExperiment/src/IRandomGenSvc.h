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
#pragma once

#include <GaudiKernel/IInterface.h>

#include <random>

namespace Gaudi::Example::TinyExperiment {

  struct IRandomGenSvc : virtual IInterface {
    DeclareInterfaceID( IRandomGenSvc, 1, 0 );
    /**
     * return a random engine seeded based on the given context
     */
    virtual std::default_random_engine getEngine( unsigned long context ) const = 0;
  };

} // namespace Gaudi::Example::TinyExperiment
