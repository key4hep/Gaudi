/*****************************************************************************\
* (c) Copyright 2023 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/

#include <GaudiKernel/StatEntity.h>

void to_json( nlohmann::json& j, StatEntity const& s ) {
  j = { { "type", s.typeString },
        { "empty", s.nEntries() == 0 },
        { "nEntries", s.nEntries() },
        { "sum", s.sum() },
        { "mean", s.mean() },
        { "sum2", s.sum2() },
        { "standard_deviation", s.standard_deviation() },
        { "min", s.min() },
        { "max", s.max() },
        { "nTrueEntries", s.nTrueEntries() },
        { "nFalseEntries", s.nFalseEntries() },
        { "efficiency", s.efficiency() },
        { "efficiencyErr", s.efficiencyErr() } };
}
