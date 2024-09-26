/***********************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "COPYING".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/IMessageSvc.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/Service.h>
#include <GaudiKernel/SmartIF.h>
#include <boost/thread/tss.hpp>
#include <memory>
#include <string>
#include <string_view>
#ifdef GAUDI_TEST_PUBLIC_HEADERS_BUILD
namespace rust {
  struct Str {
    const char* data() const noexcept;
    std::size_t size() const noexcept;
  };
} // namespace rust
#else
#  include <rust/cxx.h>
#endif

namespace Gaudi::Rust {
  /// @brief Wrapper around MessageSvc to simplify reporting from Rust components
  class Logger {
  public:
    Logger( const std::string& name, ISvcLocator* svcLoc ) : m_name( name ), m_msgSvc( svcLoc ) {}

    void report( MSG::Level level, rust::Str msg ) const {
      msgStream() << level << std::string_view( msg.data(), msg.size() ) << endmsg;
    }

    void verbose( rust::Str msg ) const { report( MSG::VERBOSE, msg ); }
    void debug( rust::Str msg ) const { report( MSG::DEBUG, msg ); }
    void info( rust::Str msg ) const { report( MSG::INFO, msg ); }
    void warning( rust::Str msg ) const { report( MSG::WARNING, msg ); }
    void error( rust::Str msg ) const { report( MSG::ERROR, msg ); }
    void fatal( rust::Str msg ) const { report( MSG::FATAL, msg ); }
    void always( rust::Str msg ) const { report( MSG::ALWAYS, msg ); }

  private:
    MsgStream& msgStream() const {
      if ( !m_msgStream.get() ) { m_msgStream.reset( new MsgStream( m_msgSvc, m_name ) ); }
      return *m_msgStream;
    }

    std::string          m_name;
    SmartIF<IMessageSvc> m_msgSvc;

    mutable boost::thread_specific_ptr<MsgStream> m_msgStream;
  };

  /// Factory function used by Rust components to create an instance of Logger
  inline std::unique_ptr<Logger> make_logger( const Service& svc ) {
    return std::make_unique<Logger>( svc.name(), svc.serviceLocator() );
  }
} // namespace Gaudi::Rust
