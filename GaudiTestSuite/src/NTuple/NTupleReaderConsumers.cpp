/***********************************************************************************\
* (c) Copyright 2026 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Algorithm.h>
#include <Gaudi/Functional/Consumer.h>
#include <Gaudi/TestSuite/NTuple/MyStruct.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiTestSuite/Counter.h>
#include <string>
#include <vector>

namespace Gaudi::TestSuite::NTuple {
  struct IntVectorDataConsumer final
      : Gaudi::Functional::Consumer<void( std::vector<int> const& ),
                                    Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>> {
    IntVectorDataConsumer( const std::string& name, ISvcLocator* svcLoc )
        : Consumer( name, svcLoc, { "InputLocation", "MyVector" } ) {}

    void operator()( std::vector<int> const& value ) const override {
      info() << "Received vector: " << value << endmsg;
    }
  };

  DECLARE_COMPONENT( IntVectorDataConsumer )

  struct FloatDataConsumer final
      : Gaudi::Functional::Consumer<void( float const& ), Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>> {
    FloatDataConsumer( const std::string& name, ISvcLocator* svcLoc )
        : Consumer( name, svcLoc, { "InputLocation", "MyFloat" } ) {}

    void operator()( float const& value ) const override { info() << "Received float: " << value << endmsg; }
  };

  DECLARE_COMPONENT( FloatDataConsumer )

  struct StrDataConsumer final : Gaudi::Functional::Consumer<void( std::string const& ),
                                                             Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>> {
    StrDataConsumer( const std::string& name, ISvcLocator* svcLoc )
        : Consumer( name, svcLoc, { "InputLocation", "MyString" } ) {}

    void operator()( std::string const& value ) const override { info() << "Received string: " << value << endmsg; }
  };

  DECLARE_COMPONENT( StrDataConsumer )

  struct StructDataConsumer final
      : Gaudi::Functional::Consumer<void( Gaudi::TestSuite::NTuple::MyStruct const& ),
                                    Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>> {
    StructDataConsumer( const std::string& name, ISvcLocator* svcLoc )
        : Consumer( name, svcLoc, { "InputLocation", "MyStruct" } ) {}

    void operator()( Gaudi::TestSuite::NTuple::MyStruct const& value ) const override {
      info() << "Received struct: MyStruct { id: " << value.id << ", name: " << value.name << " }" << endmsg;
    }
  };

  DECLARE_COMPONENT( StructDataConsumer )

  struct CounterDataConsumer final
      : Gaudi::Functional::Consumer<void( Gaudi::TestSuite::Counter const& ),
                                    Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>> {
    CounterDataConsumer( const std::string& name, ISvcLocator* svcLoc )
        : Consumer( name, svcLoc, { "InputLocation", "MyCounter" } ) {}

    void operator()( Gaudi::TestSuite::Counter const& value ) const override {
      info() << "Received counter: " << value << endmsg;
    }
  };
  DECLARE_COMPONENT( CounterDataConsumer )

} // namespace Gaudi::TestSuite::NTuple
