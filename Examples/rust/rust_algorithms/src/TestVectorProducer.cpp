/***********************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "COPYING".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Examples/TestVector.h>
#include <Gaudi/Functional/Producer.h>

namespace Gaudi::Examples::TestAlgorithms {
  class TestVectorProducer : public Gaudi::Functional::Producer<TestVector()> {
  public:
    TestVectorProducer( const std::string& name, ISvcLocator* svcLoc )
        : Producer( name, svcLoc, { "OutputLocation", "/Event/TestVector" } ) {}

    TestVector operator()() const override {
      TestVector v;
      v.setX( 1.0 );
      v.setY( 2.0 );
      v.setZ( 3.0 );
      info() << "putting vector (" << v.x() << ", " << v.y() << ", " << v.z() << ") into " << outputLocation()
             << endmsg;
      return v;
    }
  };
  DECLARE_COMPONENT( TestVectorProducer )
} // namespace Gaudi::Examples::TestAlgorithms
