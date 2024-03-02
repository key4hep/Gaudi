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
#include "FloatTool.h"
#include <Gaudi/Functional/Consumer.h>
#include <Gaudi/Functional/Producer.h>
#include <GaudiKernel/MsgStream.h>

namespace Gaudi {
  namespace TestSuite {
    using BaseClass_t = Gaudi::Functional::Traits::BaseClass_t<::Algorithm>;

    struct THDataProducer : Gaudi::Functional::Producer<int(), BaseClass_t> {

      THDataProducer( const std::string& name, ISvcLocator* svcLoc )
          : Producer( name, svcLoc, KeyValue( "OutputLocation", "/Event/MyInt" ) ) {}

      int operator()() const override {
        info() << "executing IntDataProducer, storing 7 into " << outputLocation() << endmsg;
        return 7;
      }
    };

    DECLARE_COMPONENT( THDataProducer )

    struct THDataProducer2 : Gaudi::Functional::Producer<float(), BaseClass_t> {

      THDataProducer2( const std::string& name, ISvcLocator* svcLoc )
          : Producer( name, svcLoc, KeyValue( "OutputLocation", "/Event/MyFloat" ) ) {}

      float operator()() const override {
        info() << "executing IntDataProducer, storing 7.0 into " << outputLocation() << endmsg;
        return 7.0;
      }
    };

    DECLARE_COMPONENT( THDataProducer2 )

    struct THDataConsumer : Gaudi::Functional::Consumer<void( const int& ), BaseClass_t> {

      THDataConsumer( const std::string& name, ISvcLocator* svcLoc )
          : Consumer( name, svcLoc, KeyValue( "InputLocation", "/Event/MyInt" ) ) {}

      void operator()( const int& input ) const override {
        info() << "executing IntDataConsumer, checking " << input << " from " << inputLocation() << " and "
               << m_floatTool->getFloat() << " from FloatTool are matching" << endmsg;
      }

      ToolHandle<FloatTool> m_floatTool{ this, "FloatTool", "Gaudi::TestSuite::FloatTool" };
    };

    DECLARE_COMPONENT( THDataConsumer )
  } // namespace TestSuite
} // namespace Gaudi
