/***********************************************************************************\
* (c) Copyright 2024-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/NTuple/Writer.h>
#include <numeric>
#include <tuple>

namespace Gaudi::TestSuite::NTuple {
  struct NTupleSimpleWriter_V : Gaudi::NTuple::SimpleWriter<std::tuple<int, size_t, float>( const std::vector<int>& )> {
    NTupleSimpleWriter_V( const std::string& name, ISvcLocator* svcLoc )
        : SimpleWriter( name, svcLoc, { KeyValue( "InputLocation", { "MyVector" } ) } ) {}

    void operator()( std::vector<int> const& vector ) const override {
      for ( size_t index = 0; auto& v : vector ) { fillTree( { v, index++, std::rand() % 20 } ); }
    }
  };

  DECLARE_COMPONENT( NTupleSimpleWriter_V )

  struct NTupleWriter_V : Gaudi::NTuple::Writer<std::tuple<int, size_t, float>( const std::vector<int>& )> {
    NTupleWriter_V( const std::string& name, ISvcLocator* svcLoc )
        : Writer( name, svcLoc, { KeyValue( "InputLocation", { "MyVector" } ) } ) {}

    std::tuple<int, size_t, float> transform( const std::vector<int>& vector ) const override {
      return std::make_tuple( std::accumulate( vector.begin(), vector.end(), 0 ), vector.size(), std::rand() % 20 );
    }
  };

  DECLARE_COMPONENT( NTupleWriter_V )

} // namespace Gaudi::TestSuite::NTuple
