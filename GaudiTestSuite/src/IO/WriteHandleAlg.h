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

#include <GaudiKernel/Algorithm.h>
#include <GaudiKernel/DataObjectHandle.h>
#include <GaudiTestSuite/Counter.h>
#include <GaudiTestSuite/Event.h>
#include <GaudiTestSuite/MyTrack.h>

namespace Gaudi::TestSuite {
  class WriteHandleAlg : public ::Algorithm {
  public:
    WriteHandleAlg( const std::string& n, ISvcLocator* l ) : Algorithm( n, l ) {}

    bool isClonable() const override { return true; }

    /// the execution of the algorithm
    StatusCode execute() override;

  private:
    Gaudi::Property<bool> m_useHandle{ this, "UseHandle", true, "Specify the usage of the handle to write" };

    DataObjectWriteHandle<Collision> m_output_handle{ this, "Output", "/Event/MyCollision" };
  };
} // namespace Gaudi::TestSuite
