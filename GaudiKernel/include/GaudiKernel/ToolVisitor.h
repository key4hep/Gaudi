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
#pragma once

#include "GaudiKernel/IAlgTool.h"
#include <functional>
#include <regex>
#include <type_traits>

/** Helper class to visit a list of tools and all their child tools.
 */
class ToolVisitor {
  struct IVisitor {
    virtual ~IVisitor()                   = default;
    virtual void visit( IAlgTool* ) const = 0;
  };

public:
  /** Visit the given tools and their child tools.
   * @param tools: list of top level tools to be visited
   * @param visitor: a helper class which will be called for each visited tool
   * @param reject_filter: an optional regular expression to filter out tools by name i.e. they and their child tools
   * are not visited.
   *
   * usage:
   * @verbatim
   *      auto visitor=[this](const IAlgTool *tool) { this->msg(MSG::DEBUG)  << tool->name() << endmsg; };
   *      ToolVisitor::visit(tools(), visitor);
   * @verbatim
   */
  template <typename Callable, typename = std::enable_if_t<std::is_invocable_r_v<void, Callable, IAlgTool*>>>
  static void visit( const std::vector<IAlgTool*>& tools, Callable& callable,
                     const std::regex& reject_filter = s_noFilter ) {
    class Visitor final : public IVisitor {
      Callable* m_func;

    public:
      Visitor( Callable* f ) : m_func{f} {}
      void visit( IAlgTool* alg_tool ) const override { std::invoke( *m_func, alg_tool ); }
    };
    recursiveVisit( tools, Visitor{&callable}, reject_filter );
  }

private:
  static void             recursiveVisit( const std::vector<IAlgTool*>& tools, IVisitor const& visitor,
                                          const std::regex& reject_filter );
  static const std::regex s_noFilter;
};
