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
///////////////////////// -*- C++ -*- /////////////////////////////
#pragma once

/**
 * @class QotdAlg is a simple algorithm to output "famous" quotes of "famous"
 * people into a @c MsgStream stream. Nothing fancier.
 *
 */

// Include files
#include <GaudiKernel/Algorithm.h>
#include <GaudiKernel/MsgStream.h>

namespace GaudiEx {

  class QotdAlg : public Algorithm {

  public:
    // Constructor of this form must be provided
    QotdAlg( const std::string& name, ISvcLocator* pSvcLocator );

    // Three mandatory member functions of any algorithm
    StatusCode initialize() override;
    StatusCode execute() override;
    StatusCode finalize() override;

  private:
    /// a dumb event counter
    unsigned int m_evtCnt;
  };

} // namespace GaudiEx
