/***********************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      *
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
#include <Gaudi/TestSuite/NTuple/MyStruct.h>
#include <string>
#include <vector>

namespace Gaudi::TestSuite::NTuple {
  struct IntVectorDataProducer final
      : Gaudi::Functional::Producer<std::vector<int>(), Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>> {
    IntVectorDataProducer( const std::string& name, ISvcLocator* svcLoc )
        : Producer( name, svcLoc, KeyValue( "OutputLocation", "MyVector" ) ) {}

    std::vector<int> operator()() const override { return std::vector<int>{ 0, 1, 2, 3, 4 }; }
  };

  DECLARE_COMPONENT( IntVectorDataProducer )

  struct FloatDataProducer final
      : Gaudi::Functional::Producer<float(), Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>> {
    FloatDataProducer( const std::string& name, ISvcLocator* svcLoc )
        : Producer( name, svcLoc, KeyValue( "OutputLocation", "MyFloat" ) ) {}

    float operator()() const override { return 2.5; }
  };

  DECLARE_COMPONENT( FloatDataProducer )

  struct StrDataProducer final
      : Gaudi::Functional::Producer<std::string(), Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>> {
    StrDataProducer( const std::string& name, ISvcLocator* svcLoc )
        : Producer( name, svcLoc, KeyValue( "OutputLocation", "MyString" ) ) {}

    std::string operator()() const override { return m_stringValue; }

  private:
    Gaudi::Property<std::string> m_stringValue{ this, "StringValue", "Default string", "Specify the string to write" };
  };

  DECLARE_COMPONENT( StrDataProducer )

  struct StructDataProducer final
      : Gaudi::Functional::Producer<Gaudi::TestSuite::NTuple::MyStruct(),
                                    Gaudi::Functional::Traits::BaseClass_t<Gaudi::Algorithm>> {
    StructDataProducer( const std::string& name, ISvcLocator* svcLoc )
        : Producer( name, svcLoc, KeyValue( "OutputLocation", "MyStruct" ) ) {}

    Gaudi::TestSuite::NTuple::MyStruct operator()() const override {
      Gaudi::TestSuite::NTuple::MyStruct myStruct = { 1, "myStruct" };
      return myStruct;
    }
  };

  DECLARE_COMPONENT( StructDataProducer )

} // namespace Gaudi::TestSuite::NTuple
