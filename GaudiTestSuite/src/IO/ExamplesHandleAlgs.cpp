/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <GaudiExamples/Counter.h>
#include <GaudiExamples/Event.h>
#include <GaudiExamples/MyTrack.h>
#include <GaudiKernel/Algorithm.h>
#include <GaudiKernel/DataObjectHandle.h>
#include <GaudiKernel/IDataManagerSvc.h>
#include <GaudiKernel/IDataProviderSvc.h>
#include <GaudiKernel/ThreadLocalContext.h>
#include <GaudiTestSuite/Counter.h>
#include <GaudiTestSuite/Event.h>
#include <GaudiTestSuite/MyTrack.h>

using namespace Gaudi::Examples;

class ReadHandleAlg : public ::Algorithm {
  DataObjectReadHandle<Collision> m_inputHandle{ this, "Input", "/Event/MyCollision" };

public:
  ReadHandleAlg( const std::string& n, ISvcLocator* l ) : Algorithm( n, l ) {}

  bool isClonable() const override { return true; }

  StatusCode execute() override {

    Collision* c = m_inputHandle.get();

    const int evtNum = Gaudi::Hive::currentContext().evt();

    info() << "Event " << evtNum << " Collision number " << c->collision() << endmsg;

    return StatusCode::SUCCESS;
  }
};
DECLARE_COMPONENT( ReadHandleAlg )

class WriteHandleAlg : public ::Algorithm {
public:
  WriteHandleAlg( const std::string& n, ISvcLocator* l ) : Algorithm( n, l ) {}

  bool isClonable() const override { return true; }

  StatusCode execute() override {
    info() << "Hello, I am executing" << endmsg;

    // Set collision to the current event number from the context;
    // if the context doesn't exist, set it to some dummy value
    // this fallback allows to stay compatible with non-hive infrastructure
    auto c = std::make_unique<Collision>( Gaudi::Hive::currentContext().evt() );

    if ( m_useHandle ) {
      m_output_handle.put( std::move( c ) );
      return StatusCode::SUCCESS;
    } else {
      return eventSvc()->registerObject( "/Event", "MyCollision", c.release() );
    }
  }

private:
  Gaudi::Property<bool> m_useHandle{ this, "UseHandle", true, "Specify the usage of the handle to write" };

  DataObjectWriteHandle<Collision> m_output_handle{ this, "Output", "/Event/MyCollision" };
};

DECLARE_COMPONENT( WriteHandleAlg )
