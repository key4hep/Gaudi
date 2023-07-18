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
#ifndef GAUDIEXAMPLES_READALG_H
#define GAUDIEXAMPLES_READALG_H

// Framework include files
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/DataObjectHandle.h"

// Event Model related classes
#include "GaudiExamples/Counter.h"
#include "GaudiExamples/Event.h"
#include "GaudiExamples/MyTrack.h"

using namespace Gaudi::Examples;

/** @class ReadHandleAlg ReadHandleAlg.h
 */

class ReadHandleAlg : public Algorithm {

  DataObjectReadHandle<Collision> m_inputHandle{ this, "Input", "/Event/MyCollision" };

public:
  ReadHandleAlg( const std::string& n, ISvcLocator* l ) : Algorithm( n, l ) {}

  bool isClonable() const override { return true; }

  /// Event callback
  StatusCode execute() override;
};

#endif // GAUDIEXAMPLES_READALG_H
