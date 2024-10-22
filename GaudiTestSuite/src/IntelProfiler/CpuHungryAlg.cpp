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
// # Include files
// Standard:
#include <math.h>
#include <unistd.h>
// Gaudi:
#include <GaudiKernel/Algorithm.h>

/// Algorithm which consume a lot of CPU.
///
/// Author: Alexander Mazurov (alexander.mazurov@gmail.com)
class CpuHungryAlg : public Algorithm {
public:
  /// Standard constructor
  using Algorithm::Algorithm;

  StatusCode initialize() override; ///< Algorithm initialization
  StatusCode execute() override;    ///< Algorithm execution
  StatusCode finalize() override;   ///< Algorithm finalization

private:
  double mysin();
  double mycos();
  double mytan();
  double myatan();

private:
  Gaudi::Property<long> m_loops{ this, "Loops", 1000000, "" };
  int                   m_nevent = 0;
};

// Register algorithm
DECLARE_COMPONENT( CpuHungryAlg )

/// Initialization.
StatusCode CpuHungryAlg::initialize() {
  StatusCode sc = Algorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;         // error printed already by Algorithm

  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Initialize" << endmsg;
  return StatusCode::SUCCESS;
}

/// Execute.
double CpuHungryAlg::mysin() {
  double result = 0;
  for ( long i = 0; i <= m_loops; ++i ) { result += sin( i ); }
  return result;
}

double CpuHungryAlg::mycos() {
  double result = 0;
  for ( long i = 0; i <= m_loops; ++i ) { result += cos( i ); }
  return result;
}

double CpuHungryAlg::mytan() {
  double result = 0;
  for ( long i = 0; i <= m_loops; ++i ) { result += tan( i ); }
  result += myatan();
  return result;
}

double CpuHungryAlg::myatan() {
  double result = 0;
  for ( long i = 0; i <= m_loops; ++i ) { result += tan( i ); }
  return result;
}

StatusCode CpuHungryAlg::execute() {
  m_nevent++;
  double result = 0;
  if ( name() == "Alg1" ) {
    result = mysin();
  } else if ( name() == "Alg2" ) {
    result = mycos();
  } else {
    result = mytan();
  }
  // This part is pointless, but prevent a warning about a
  // set, but unused, variable (result).
  if ( msgLevel( MSG::DEBUG ) ) debug() << "Result = " << result << endmsg;
  return StatusCode::SUCCESS;
}

/// Finalize
StatusCode CpuHungryAlg::finalize() {
  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Finalize" << endmsg;

  return Algorithm::finalize(); // must be called after all other actions
}

// ============================================================================
