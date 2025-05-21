/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

// Include files
#include <GaudiKernel/Algorithm.h>
#include <GaudiKernel/NTuple.h>

// Forward declarations

class NTupleAlgorithm : public Algorithm {

public:
  // Constructor of this form must be provided
  NTupleAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );

  // Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  NTuple::Tuple* m_tuple1 = nullptr;
  NTuple::Tuple* m_tuple2 = nullptr;

  NTuple::Item<long>  m_ntrk;
  NTuple::Item<float> m_energy;
  // Items for the column wise n-tuple
  NTuple::Array<long>  m_iNumbers;
  NTuple::Array<float> m_fNumbers;
  NTuple::Item<long>   m_n;
};
