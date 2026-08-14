/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an intergovernmental organization or     *
* submit itself to any jurisdiction.                                                *
\***********************************************************************************/
#include <Gaudi/Property.h>
#include <GaudiKernel/Algorithm.h>
#include <nlohmann/json.hpp>

class JSONPropertyAlg final : public Algorithm {
public:
  using Algorithm::Algorithm;

  StatusCode execute() override { return StatusCode::SUCCESS; }

private:
  Gaudi::Property<nlohmann::json> m_json{ this, "JSON", nlohmann::json::object() };
};

DECLARE_COMPONENT( JSONPropertyAlg )
