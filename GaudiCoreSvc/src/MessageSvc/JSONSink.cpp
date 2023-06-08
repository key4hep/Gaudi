/***********************************************************************************\
* (c) Copyright 2022 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

#include <Gaudi/BaseSink.h>
#include <Gaudi/MonitoringHub.h>

#include <fstream>

#include <string>

namespace Gaudi::Monitoring {

  class JSONSink : public BaseSink {

  public:
    using BaseSink::BaseSink;

    StatusCode stop() override {
      if ( m_fileName.empty() ) { return StatusCode::SUCCESS; }
      nlohmann::json output;
      applytoAllEntities( [&output]( auto& ent ) {
        output.emplace_back( nlohmann::json{
            { "name", ent.name },
            { "component", ent.component },
            { "entity", ent.toJSON() },
        } );
      } );
      info() << "Writing JSON file " << m_fileName.value() << endmsg;
      std::ofstream os( m_fileName, std::ios::out );
      os << output.dump( 4 );
      os.close();
      // call parent's stop
      return BaseSink::stop();
    }

  private:
    Gaudi::Property<std::string> m_fileName{ this, "FileName", "json_output.json",
                                             "Name of output json file. Empty fileName means no output" };
  };

  DECLARE_COMPONENT( JSONSink )

} // namespace Gaudi::Monitoring
