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

#include "IRandomGenSvc.h"

#include <GaudiKernel/Service.h>
#include <GaudiKernel/extends.h>

namespace Gaudi::Example::TinyExperiment {

  class RandomGenSvc : virtual public extends<Service, IRandomGenSvc> {
  public:
    using extends::extends;
    std::default_random_engine getEngine( unsigned long context ) const override {
      return std::default_random_engine( m_randomSeed + context );
    }

  private:
    Gaudi::Property<unsigned long> m_randomSeed{ this, "RandomSeed", 0 };
  };

  DECLARE_COMPONENT_WITH_ID( RandomGenSvc, "RandomGenSvc" )

} // namespace Gaudi::Example::TinyExperiment
