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
#ifndef EVENTNODEKILLER_H
#define EVENTNODEKILLER_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"
#include <string>
#include <vector>

/** @class EventNodeKiller EventNodeKiller.h
 *  Algorithm to kill nodes in the event data store. Useful if you want to
 *  make the application "forget" about the previous history of the input file
 *
 *  @author Marco Cattaneo
 *  @date   2005-07-14
 */
class EventNodeKiller : public GaudiAlgorithm {
public:
  using GaudiAlgorithm::GaudiAlgorithm;

  StatusCode execute() override;

private:
  Gaudi::Property<std::vector<std::string>> m_nodes{this, "Nodes"};
};
#endif // EVENTNODEKILLER_H
