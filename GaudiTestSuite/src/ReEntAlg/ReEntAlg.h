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

#include <Gaudi/Algorithm.h>

/** @class ReEntAlg
 * @brief  an algorithm to test reentrant Algorithms
 * @author Charles Leggett
 */
class ReEntAlg : public Gaudi::Algorithm {

public:
  ReEntAlg( const std::string& name, ISvcLocator* pSvcLocator );
  StatusCode initialize() override;
  StatusCode execute( const EventContext& ctx ) const override;
  StatusCode finalize() override;

private:
  Gaudi::Property<unsigned int> m_sleep{ this, "SleepFor", 1000, "ms to sleep in execute" };
};
