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
#include <Gaudi/Functional/Consumer.h>
#include <Gaudi/Functional/Producer.h>
#include <GaudiKernel/Algorithm.h>
#include <GaudiKernel/DataObjectHandle.h>
#include <gsl/span>
#include <string>
#include <string_view>
#include <vector>

using BaseClass_t = Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>;

template <typename Data, typename View>
using writeViewFor = Gaudi::Functional::Traits::use_<Gaudi::Functional::Traits::writeViewFor<Data, View>, BaseClass_t>;

struct ProduceStringView : Gaudi::Functional::Producer<std::string(), writeViewFor<std::string, std::string_view>> {
  ProduceStringView( const std::string& name, ISvcLocator* svcLoc )
      : Producer{ name, svcLoc, { "Output", "/Event/Test/StringView" } } {}

  std::string operator()() const override { return "Hello World"; }
};
DECLARE_COMPONENT( ProduceStringView )

struct ProduceIntView
    : Gaudi::Functional::Producer<std::vector<int>(), writeViewFor<std::vector<int>, gsl::span<const int>>> {
  ProduceIntView( const std::string& name, ISvcLocator* svcLoc )
      : Producer{ name, svcLoc, { "Output", "/Event/Test/IntView" } } {}

  std::vector<int> operator()() const override {
    std::vector<int> vi{ 1, 2, 3, 5, 8, 13, 21, 34 };
    info() << "made vector with data at : " << vi.data() << endmsg;
    return vi;
  }
};
DECLARE_COMPONENT( ProduceIntView )

struct GetStringView : Gaudi::Functional::Consumer<void( std::string_view const& ), BaseClass_t> {
  GetStringView( const std::string& name, ISvcLocator* svcLoc )
      : Consumer{ name, svcLoc, { "Input", "/Event/Test/StringView" } } {}

  void operator()( const std::string_view& sv ) const override { info() << sv << endmsg; }
};
DECLARE_COMPONENT( GetStringView )

struct GetIntView : Gaudi::Functional::Consumer<void( gsl::span<const int> const& ), BaseClass_t> {
  GetIntView( const std::string& name, ISvcLocator* svcLoc )
      : Consumer{ name, svcLoc, { "Input", "/Event/Test/IntView" } } {}

  void operator()( const gsl::span<const int>& view ) const override {
    info() << "got a span with data at : " << view.data() << endmsg;
    auto& out = info();
    for ( int i : view ) out << i << " ";
    out << endmsg;
  }
};
DECLARE_COMPONENT( GetIntView )
