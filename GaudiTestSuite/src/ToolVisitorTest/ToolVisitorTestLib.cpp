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
#include "GaudiEnv.h"
#include "ITestTool.h"
#include <GaudiKernel/AlgTool.h>
#include <GaudiKernel/Algorithm.h>
#include <GaudiKernel/RenounceToolInputsVisitor.h>
#include <GaudiKernel/ToolVisitor.h>
#include <deque>
#include <sstream>

#ifndef NDEBUG
#  include <iostream>
#endif

namespace GaudiTesting {
  void throwConditionFailed( bool condition, const std::string file_name, int line_no,
                             const std::string condition_string ) {
    if ( !condition ) {
      std::stringstream msg;
      msg << "Conditoin Failed: " << file_name << ":" << line_no << " " << condition_string;
      throw std::runtime_error( msg.str() );
    }
  }

  std::string extractBaseName( const std::string& full_name ) {
    std::string::size_type pos  = full_name.rfind( "." );
    std::string::size_type pos2 = full_name.rfind( "/" );
    if ( pos == std::string::npos ) {
      pos = pos2;
    } else if ( pos2 > pos ) {
      pos = pos2;
    }
    return pos != std::string::npos ? full_name.substr( pos + 1 ) : full_name;
  }

  class TestTool : public extends<AlgTool, ITestTool> {
  public:
    TestTool( const std::string& type, const std::string& name, const IInterface* parent )
        : base_class( type, name, parent ) {
      setDefaults();
    }

    void process() const override {}
    void setDefaults() {
      std::string base_name( extractBaseName( name() ) );
      DEBUG_TRACE( std::cout << "DEBUG  " << name() << " setDefaults" << base_name << std::endl; );
      if ( base_name == "tool1" ) {
        addInput( "Renounce" );
        addInput( "Input11" );
      } else if ( base_name == "tool2" ) {
        addInput( "Input21" );
        addInput( "Input22" );
        addTool( "tool1" );
      } else if ( base_name == "tool3" ) {
        addInput( "Input31" );
        addInput( "Input32" );
        addTool( "tool4" );
        addTool( "tool2" );
      } else if ( base_name == "tool4" ) {
        addInput( "Input41" );
        addInput( "Input42" );
        addInput( "Renounce" );
      } else if ( base_name == "tool5" || base_name == "tool5Bug" ) {
        addInput( "Input51" );
        addInput( "Input52" );
        addOutput( "Renounce" );
        addTool( "tool3" );
      } else if ( base_name == "tool6" ) {
        addInput( "Input61" );
        addTool( "tool5" );
      }
      DEBUG_TRACE( std::cout << "DEBUG " << name() << " added tools:" << m_algTools.size()
                             << ", data handles:" << m_handles.size() << std::endl );
    }

    // a do-nothing helper class which implements the logger interface
    struct Logger : RenounceToolInputsVisitor::ILogger {
      void renounce( [[maybe_unused]] std::string_view tool, [[maybe_unused]] std::string_view input ) override {
        DEBUG_TRACE( std::cout << "renounce " << tool << " . " << input << std::endl );
      }
    };

    void addInput( const std::string key ) {
      m_keys.emplace_back( key );
      m_handles.emplace_back( m_keys.back(), Gaudi::DataHandle::Reader, this );
      this->declare( m_handles.back() );
    }
    void addOutput( const std::string key ) {
      m_keys.emplace_back( key );
      m_handles.emplace_back( m_keys.back(), Gaudi::DataHandle::Writer, this );
      this->declare( m_handles.back() );
    }
    void addTool( const std::string& name ) {
      m_algTools.emplace_back( this, name, "GaudiTesting::TestTool/" + name, "" );
      Ensures( this->declareTool( m_algTools.back(), m_algTools.back().typeAndName(), true ).isSuccess() );
    }
    StatusCode initialize() override {
      DEBUG_TRACE( std::cout << "DEBUG " << name() << " initialize." << std::endl );
      std::string base_name( extractBaseName( name() ) );
      for ( auto& handle : m_algTools ) {
        DEBUG_TRACE( std::cout << name() << " tool:" << handle.typeAndName() << std::endl );
        handle.retrieve().ignore();
      }
      if ( base_name == "tool5" ) {
        auto i = std::find_if( m_handles.begin(), m_handles.end(), []( const auto& data ) {
          return data.mode() == Gaudi::DataHandle::Writer && data.objKey() == "Renounce";
        } );
        if ( i != m_handles.end() ) {
          TestTool::Logger logger;
          DEBUG_TRACE( std::cout << "DEBUG " << name() << " renounce tools "
                                 << "Renounce" << std::endl );
          auto visitor = RenounceToolInputsVisitor{ { i->fullKey() }, logger };
          ToolVisitor::visit( tools(), visitor );
        }
      }
      DEBUG_TRACE( std::cout << "DEBUG " << name() << " INITIALIZED: tools " << m_algTools.size()
                             << ", data handles:" << m_handles.size() << std::endl );
      return StatusCode::SUCCESS;
    }

    std::vector<DataObjID>            m_keys;
    std::deque<Gaudi::DataHandle>     m_handles;
    std::deque<ToolHandle<ITestTool>> m_algTools;
  };

  class TestAlg : public Algorithm {
  public:
    TestAlg( std::string aname, ISvcLocator* svc_Locator ) : Algorithm( aname, svc_Locator ) {
      Ensures( setProperty( "FilterCircularDependencies", false ).isSuccess() );
      DEBUG_TRACE( std::cout << "DEBUG TestAlg ctor " << name() << std::endl );
      if ( extractBaseName( name() ) == "TestAlgBug" ) {
        addTool( "GaudiTesting::TestTool/tool5Bug" );
        Ensures( this->setProperty( "OutputLevel", static_cast<int>( MSG::FATAL ) ).isSuccess() );
      } else {
        addTool( "GaudiTesting::TestTool/tool5" );
      }
    }
    ~TestAlg() { DEBUG_TRACE( std::cout << "DEBUG " << name() << "::dtor" << std::endl ); }
    StatusCode initialize() override {
      for ( auto& handle : m_algTools ) {
        DEBUG_TRACE( std::cout << name() << " tool:" << handle.typeAndName() << std::endl );
        if ( handle.retrieve().isFailure() ) { return StatusCode::FAILURE; }
      }
      dumpData( "Renounce" );
      return StatusCode::SUCCESS;
    }
    void dumpData( const std::string& pattern ) const {
      auto dumper = [&pattern]( const IAlgTool* tool ) {
        const AlgTool* alg_tool = dynamic_cast<const AlgTool*>( tool );
        if ( alg_tool ) {
          for ( Gaudi::DataHandle* handle : alg_tool->inputHandles() ) {
            if ( handle->objKey().find( pattern ) != std::string::npos ) {
              DEBUG_TRACE( std::cout << "DEBUG input Handle " << tool->name() << " . " << handle->objKey()
                                     << std::endl );
            }
          }
          for ( Gaudi::DataHandle* handle : alg_tool->outputHandles() ) {
            if ( handle->objKey().find( pattern ) != std::string::npos ) {
              DEBUG_TRACE( std::cout << "DEBUG output Handle " << tool->name() << " . " << handle->objKey()
                                     << std::endl );
            }
          }
          for ( auto elm : alg_tool->inputDataObjs() ) {
            if ( elm.key().find( pattern ) != std::string::npos ) {
              DEBUG_TRACE( std::cout << "DEBUGinput Handle " << tool->name() << " . " << elm.key() << std::endl );
            }
          }
        }
      };
      ToolVisitor::visit( tools(), dumper );
    }
    StatusCode execute() override { return StatusCode::SUCCESS; }
    StatusCode finalize() override {
#ifndef NDEBUG
      for ( const DataObjID& const_obj_id : this->inputDataObjs() ) {
        DEBUG_TRACE( std::cout << "DEBUG " << name() << " input:" << const_obj_id.key() << std::endl );
      }
      for ( const DataObjID& const_obj_id : this->outputDataObjs() ) {
        DEBUG_TRACE( std::cout << "DEBUG " << name() << " output:" << const_obj_id.key() << std::endl );
      }
#endif
      return StatusCode::SUCCESS;
    }

  private:
    void addTool( const std::string& name ) {
      m_algTools.emplace_back( this, name, "GaudiTesting::TestTool/" + name, "" );
      Ensures( this->declareTool( m_algTools.back(), m_algTools.back().typeAndName(), true ).isSuccess() );
    }
    std::deque<ToolHandle<ITestTool>> m_algTools;
  };

  DECLARE_COMPONENT( TestTool )
  DECLARE_COMPONENT( TestAlg )
} // namespace GaudiTesting
