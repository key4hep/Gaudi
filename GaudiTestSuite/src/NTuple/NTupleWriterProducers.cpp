/***********************************************************************************\
* (c) Copyright 2024-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Algorithm.h>
#include <Gaudi/Functional/Producer.h>
#include <Gaudi/Functional/Transformer.h>
#include <Gaudi/TestSuite/NTuple/MyStruct.h>
#include <GaudiTestSuite/Counter.h>
#include <cstdint>
#include <string>
#include <vector>

namespace Gaudi::TestSuite::NTuple {
  struct IntVectorDataProducer final
      : Gaudi::Functional::Producer<std::vector<int>(), Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>> {
    IntVectorDataProducer( const std::string& name, ISvcLocator* svcLoc )
        : Producer( name, svcLoc, { "OutputLocation", "MyVector" } ) {}

    std::vector<int> operator()() const override { return std::vector<int>{ 0, 1, 2, 3, 4 }; }
  };

  DECLARE_COMPONENT( IntVectorDataProducer )

  struct FloatDataProducer final
      : Gaudi::Functional::Producer<float(), Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>> {
    FloatDataProducer( const std::string& name, ISvcLocator* svcLoc )
        : Producer( name, svcLoc, { "OutputLocation", "MyFloat" } ) {}

    float operator()() const override { return 2.5; }
  };

  DECLARE_COMPONENT( FloatDataProducer )

  struct StrDataProducer final
      : Gaudi::Functional::Producer<std::string(), Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>> {
    StrDataProducer( const std::string& name, ISvcLocator* svcLoc )
        : Producer( name, svcLoc, { "OutputLocation", "MyString" } ) {}

    std::string operator()() const override { return m_stringValue; }

  private:
    Gaudi::Property<std::string> m_stringValue{ this, "StringValue", "Default string", "Specify the string to write" };
  };

  DECLARE_COMPONENT( StrDataProducer )

  struct StructDataProducer final
      : Gaudi::Functional::Producer<Gaudi::TestSuite::NTuple::MyStruct(),
                                    Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>> {
    StructDataProducer( const std::string& name, ISvcLocator* svcLoc )
        : Producer( name, svcLoc, { "OutputLocation", "MyStruct" } ) {}

    Gaudi::TestSuite::NTuple::MyStruct operator()() const override {
      Gaudi::TestSuite::NTuple::MyStruct myStruct = { 1, "myStruct" };
      return myStruct;
    }
  };

  DECLARE_COMPONENT( StructDataProducer )

  struct CounterDataProducer final
      : Gaudi::Functional::Producer<Gaudi::TestSuite::Counter(),
                                    Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>> {
    CounterDataProducer( const std::string& name, ISvcLocator* svcLoc )
        : Producer( name, svcLoc, { "OutputLocation", "MyCounter" } ) {}

    Counter operator()() const override { return Counter(); }
  };
  DECLARE_COMPONENT( CounterDataProducer )

  // evt % (Size + 1) elements, or always Size without VariableSize
  struct FloatVectorProducer final
      : Gaudi::Functional::Transformer<std::vector<float>( const EventContext& ),
                                       Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>> {
    FloatVectorProducer( const std::string& name, ISvcLocator* svcLoc )
        : Transformer( name, svcLoc, { "OutputLocation", "MyFloatVector" } ) {}

    std::vector<float> operator()( const EventContext& ctx ) const override {
      const auto         evt  = ctx.evt();
      const auto         size = m_variableSize ? evt % ( m_size + 1 ) : m_size.value();
      std::vector<float> values;
      for ( std::size_t i = 0; i < size; ++i ) values.push_back( evt + 0.5f * i );
      return values;
    }

    Gaudi::Property<std::size_t> m_size{ this, "Size", 8 };
    Gaudi::Property<bool>        m_variableSize{ this, "VariableSize", true };
  };
  DECLARE_COMPONENT( FloatVectorProducer )

  struct EventNumberProducer final
      : Gaudi::Functional::Transformer<std::uint64_t( const EventContext& ),
                                       Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>> {
    EventNumberProducer( const std::string& name, ISvcLocator* svcLoc )
        : Transformer( name, svcLoc, { "OutputLocation", "EventNumber" } ) {}

    std::uint64_t operator()( const EventContext& ctx ) const override { return ctx.evt(); }
  };
  DECLARE_COMPONENT( EventNumberProducer )

  // The element writers need this to have as many entries as FloatVectorProducer gives with its
  // defaults (Size 8), hence the % 9.
  struct StructVectorProducer final
      : Gaudi::Functional::Transformer<std::vector<MyStruct>( const EventContext& ),
                                       Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>> {
    StructVectorProducer( const std::string& name, ISvcLocator* svcLoc )
        : Transformer( name, svcLoc, { "OutputLocation", "MyStructVector" } ) {}

    std::vector<MyStruct> operator()( const EventContext& ctx ) const override {
      const std::size_t     n = ctx.evt() % 9;
      std::vector<MyStruct> structs;
      for ( std::size_t i = 0; i < n; ++i ) structs.push_back( { int( i ), "s" + std::to_string( i ) } );
      return structs;
    }
  };
  DECLARE_COMPONENT( StructVectorProducer )
} // namespace Gaudi::TestSuite::NTuple
