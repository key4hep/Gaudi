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
#ifndef HANDLEWRITE_ALG_H
#define HANDLEWRITE_ALG_H

#include "GaudiAlg/GaudiAlgorithm.h"

#include "GaudiKernel/DataObjectHandle.h"

// Event Model related classes
#include "GaudiExamples/Counter.h"
#include "GaudiExamples/Event.h"
#include "GaudiExamples/MyTrack.h"

using namespace Gaudi::Examples;

//------------------------------------------------------------------------------

class WriteHandleAlg : public GaudiAlgorithm {
public:
  WriteHandleAlg( const std::string& n, ISvcLocator* l ) : GaudiAlgorithm( n, l ) {}

  bool isClonable() const override { return true; }

  /// the execution of the algorithm
  StatusCode execute() override;

private:
  Gaudi::Property<bool> m_useHandle{ this, "UseHandle", true, "Specify the usage of the handle to write" };

  DataObjectWriteHandle<Collision> m_output_handle{ this, "Output", "/Event/MyCollision" };
};

#endif
