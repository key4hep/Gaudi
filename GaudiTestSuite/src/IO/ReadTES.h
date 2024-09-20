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
#ifndef POOLIO_READTES_H
#define POOLIO_READTES_H 1

// Include files

#include <string>
#include <vector>

// from Gaudi
#include <GaudiKernel/Algorithm.h>

/** @class ReadTES ReadTES.h
 *
 *
 *  @author Marco Cattaneo
 *  @date   2008-11-03
 */
class ReadTES : public Algorithm {
public:
  /// Standard constructor
  using Algorithm::Algorithm;

  StatusCode initialize() override; ///< Algorithm initialization
  StatusCode execute() override;    ///< Algorithm execution

protected:
private:
  Gaudi::Property<std::vector<std::string>> m_locations{ this, "Locations", {}, "Locations to read" };
};
#endif // POOLIO_READTES_H
