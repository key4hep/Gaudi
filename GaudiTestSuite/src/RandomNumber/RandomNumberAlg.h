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
#ifndef GAUDIEXAMPLES_RANDOMNUMBERALG_H
#define GAUDIEXAMPLES_RANDOMNUMBERALG_H

// Framework include files
#include <GaudiKernel/Algorithm.h>
#include <GaudiKernel/NTuple.h>
#include <GaudiKernel/RndmGenerators.h>

/** @class RandomNumberAlg
    A small algorithm class using the random number service

    @author M.Frank
    @date 1999
*/
class RandomNumberAlg : public Algorithm {
protected:
  /// Allocate wrapper for random number generator
  Rndm::Numbers m_numbers;
  /// Initial seed to fill deterministic random numbers
  uint32_t m_initial{ 0 };
  /// Pointer to N-tuple
  NTuple::Tuple* m_ntuple{ nullptr };
  /// N-tuple items
  NTuple::Item<int>      m_int;
  NTuple::Item<uint32_t> m_deter;
  NTuple::Item<float>    m_gauss;
  NTuple::Item<float>    m_exponential;
  NTuple::Item<float>    m_poisson;

public:
  /// Constructor: A constructor of this form must be provided.
  RandomNumberAlg( const std::string& name, ISvcLocator* pSvcLocator );
  /// Standard Destructor
  ~RandomNumberAlg() override;
  /// Customized initialisation
  StatusCode initialize() override;
  /// Customized finalisation
  StatusCode finalize() override;
  /// Event callback
  StatusCode execute() override;
};

#endif // GAUDIEXAMPLES_RANDOMNUMBERALG_H
