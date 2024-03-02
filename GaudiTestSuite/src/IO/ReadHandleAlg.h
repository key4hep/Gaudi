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
  class ReadHandleAlg : public ::Algorithm {

    DataObjectReadHandle<Collision> m_inputHandle{ this, "Input", "/Event/MyCollision" };

  public:
    ReadHandleAlg( const std::string& n, ISvcLocator* l ) : Algorithm( n, l ) {}

    bool isClonable() const override { return true; }

    /// Event callback
    StatusCode execute() override;
  };
} // namespace Gaudi::TestSuite
