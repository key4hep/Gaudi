/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <GaudiAlg/EventCounter.h>
#include <GaudiAlg/GaudiSequencer.h>
#include <GaudiAlg/Prescaler.h>

DECLARE_COMPONENT( EventCounter )
DECLARE_COMPONENT( Prescaler )
DECLARE_COMPONENT( GaudiSequencer )

// GaudiAlg Sequencer has been moved to GaudiKernel as Gaudi::Sequencer and this
// allows for backward compatibility in options and tests.
#include <Gaudi/Sequencer.h>
struct Sequencer : Gaudi::Sequencer {
  using Gaudi::Sequencer::Sequencer;
};
DECLARE_COMPONENT( Sequencer )
