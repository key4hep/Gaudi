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
/// Test file used to check if the new Plugin Service is able to handle the
/// strings used in the old one.
/// For example the class PluginServiceTest::MyAlg was identified by the string
/// "PluginServiceTest__MyAlg".

#include <GaudiKernel/Algorithm.h>

#include <string>
#include <vector>

namespace PluginServiceTest {
  class MyAlg : public Algorithm {
  public:
    using Algorithm::Algorithm;

    StatusCode execute() override { return StatusCode::SUCCESS; }
  };

  template <class T1, class T2>
  class MyTemplatedAlg : public Algorithm {
  public:
    using Algorithm::Algorithm;

    StatusCode execute() override { return StatusCode::SUCCESS; }
  };
} // namespace PluginServiceTest

DECLARE_COMPONENT( PluginServiceTest::MyAlg )
namespace {
  typedef PluginServiceTest::MyTemplatedAlg<int&, double*> _dummy;
}
DECLARE_COMPONENT( _dummy )
