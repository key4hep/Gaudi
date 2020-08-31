/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_RENOUNCETOOLINPUTSVISITOR
#define GAUDIKERNEL_RENOUNCETOOLINPUTSVISITOR 1

#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/ToolVisitor.h"
#include <functional>
#include <string>
#include <vector>

#ifndef USAGE_IS_THREAD_SAFE
#  define USAGE_IS_THREAD_SAFE
#endif

class IAlgTool;

/** Helper class to be used in conjunction with the recursive tool visitor to renounce certain inputs.
 */
class RenounceToolInputsVisitor : public ToolVisitor::IVisitor {
public:
  /** Helper class interface to optionally log renounce operations.
   */
  class ILogger {
  public:
    virtual ~ILogger() {}
    virtual void renounce( const std::string& tool_name, const std::string& key ) = 0;
  };

  /** A do-nothing helper class which implements the logger interface.
   */
  class NoLogger : public ILogger {
  public:
    virtual void renounce( const std::string&, const std::string& ) override {}
  };

  class Logger : public ILogger {
  public:
    using Functor = std::function<void( const std::string&, const std::string& )>;
    Logger( Functor&& func ) : m_func( std::move( func ) ) {}
    virtual void renounce( const std::string& tool_name, const std::string& key ) override { m_func( tool_name, key ); }

  private:
    Functor m_func;
  };
  /** @brief Create a logger from a function.
   * usage:
   * @verbatim
   * auto logger=RenounceToolInputsVisitor::createLogger( [this]( const std::string& tool_name, const std::string& key )
   * { this->msg(MSG::INFO) << " Renounce " << tool_name << " . " << key << endmsg;
   *                                                       });
   * @endverbatim
   */
  static Logger createLogger( std::function<void( const std::string&, const std::string& )>&& func ) {
    return Logger( std::move( func ) );
  }

  /** construct the renounce visitor helper object
   * @param input_keys a list of input keys to be renounced.
   * @parame logger optional helper object to log inputs which are renounced.
   * By default this renounce visitor will renounce inputs with keys found in the inputs_names list, and it operates
   * silently.
   */
  RenounceToolInputsVisitor( std::vector<DataObjID>&& input_keys, ILogger& logger = s_noLogger )
      : m_renounceKeys( std::move( input_keys ) )
      , m_logger( &logger ) // @TODO possible source of use after delete
  {}

  virtual void visit( IAlgTool* alg_tool ) override;

private:
  std::vector<DataObjID> m_renounceKeys;
  ILogger*               m_logger;

  static NoLogger s_noLogger USAGE_IS_THREAD_SAFE;
};
#endif
