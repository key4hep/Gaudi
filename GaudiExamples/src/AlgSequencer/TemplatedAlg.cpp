/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

#include <Gaudi/Algorithm.h>
#include <string>
#include <vector>

/** @class TemplateAlg
    Trivial Algorithm for tutotial purposes

    @author nobody
*/
template <typename T, typename R>
class TemplatedAlg : public Gaudi::Algorithm {
public:
  using Algorithm::Algorithm;

  StatusCode initialize() override {
    return Algorithm::initialize().andThen( [&]() {
      using Gaudi::PluginService::Details::demangle;
      info() << "Initializing TemplatedAlg instance " << name() << " of type " << demangle( typeid( *this ) ) << endmsg;
    } );
  }
  StatusCode execute( const EventContext& ) const override { return StatusCode::SUCCESS; }

private:
  Gaudi::Property<T> m_t{ this, "TProperty", {} };
  Gaudi::Property<R> m_r{ this, "RProperty", {} };
};

// Static Factory declaration
typedef TemplatedAlg<int, std::vector<std::string>> t1;
typedef TemplatedAlg<double, bool>                  t2;

DECLARE_COMPONENT( t1 )
DECLARE_COMPONENT_WITH_ID( t1, "TAlgIS" )
DECLARE_COMPONENT( t2 )
DECLARE_COMPONENT_WITH_ID( t2, "TAlgDB" )
