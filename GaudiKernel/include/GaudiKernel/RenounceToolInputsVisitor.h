/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once
#include <GaudiKernel/DataObjID.h>
#include <GaudiKernel/ToolVisitor.h>
#include <functional>
#include <string>
#include <type_traits>
#include <vector>

#ifndef USAGE_IS_THREAD_SAFE
#  define USAGE_IS_THREAD_SAFE
#endif

class IAlgTool;

/** Helper class to be used in conjunction with the recursive tool visitor to renounce certain inputs.
 */
class RenounceToolInputsVisitor {
public:
  /** Helper class interface to optionally log renounce operations.
   */
  struct ILogger {
    virtual ~ILogger()                                                        = default;
    virtual void renounce( std::string_view tool_name, std::string_view key ) = 0;
  };

  /** A do-nothing helper class which implements the logger interface.
   */
  struct NoLogger : ILogger {
    void renounce( std::string_view, std::string_view ) override {}
  };

  class Logger final : public ILogger {
    std::function<void( std::string_view, std::string_view )> m_func;

  public:
    template <std::invocable<std::string_view, std::string_view> F>
    Logger( F func ) : m_func( std::move( func ) ) {}
    void renounce( std::string_view tool_name, std::string_view key ) override { m_func( tool_name, key ); }
  };
  /** @brief Create a logger from a function.
   * usage:
   * @verbatim
   * auto logger=RenounceToolInputsVisitor::createLogger( [this]( std::string_view tool_name, std::string_view key )
   * { this->msg(MSG::INFO) << " Renounce " << tool_name << " . " << key << endmsg;
   *                                                       });
   * @endverbatim
   */
  static Logger createLogger( std::function<void( std::string_view, std::string_view )> func ) {
    return Logger{ std::move( func ) };
  }

  /** construct the renounce visitor helper object
   * @param input_keys a list of input keys to be renounced.
   * @parame logger optional helper object to log inputs which are renounced.
   * By default this renounce visitor will renounce inputs with keys found in the inputs_names list, and it operates
   * silently.
   */
  RenounceToolInputsVisitor( std::vector<DataObjID> input_keys, ILogger& logger = s_noLogger )
      : m_renounceKeys( std::move( input_keys ) )
      , m_logger( &logger ) // @TODO possible source of use after delete
  {}

  void operator()( IAlgTool* );

private:
  std::vector<DataObjID> m_renounceKeys;
  ILogger*               m_logger;

  static NoLogger s_noLogger USAGE_IS_THREAD_SAFE;
};
