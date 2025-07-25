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

    void flush( bool ) override {
      if ( m_fileName.empty() ) { return; }
      nlohmann::json output;
      applyToAllSortedEntities(
          [&output]( std::string const& component, std::string const& name, nlohmann::json const& j ) {
            output.emplace_back( nlohmann::json{
                { "name", name },
                { "component", component },
                { "entity", j },
            } );
          } );
      info() << "Writing JSON file " << m_fileName.value() << endmsg;
      std::ofstream{ m_fileName, std::ios::out } << output.dump( 4 );
    }

  private:
    Gaudi::Property<std::string> m_fileName{ this, "FileName", "json_output.json",
                                             "Name of output json file. Empty fileName means no output" };
  };

  DECLARE_COMPONENT( JSONSink )

} // namespace Gaudi::Monitoring
