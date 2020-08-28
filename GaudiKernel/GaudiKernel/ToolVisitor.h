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
#ifndef GAUDIKERNEL_TOOLVISITOR
#define GAUDIKERNEL_TOOLVISITOR 1

#include "GaudiKernel/IAlgTool.h"
#include <functional>
#include <regex>

/** Helper class to visit a list of tools and all their child tools.
 */
class ToolVisitor {
public:
  class IConstVisitor {
  public:
    virtual ~IConstVisitor() {}
    virtual void visit( const IAlgTool* ) = 0;
  };
  class IVisitor {
  public:
    virtual ~IVisitor() {}
    virtual void visit( IAlgTool* ) = 0;
  };

  class ConstVisitor : public IConstVisitor {
  public:
    using Functor = std::function<void( const IAlgTool* )>;
    ConstVisitor( Functor&& func ) : m_func( std::move( func ) ) {}
    virtual void visit( const IAlgTool* alg_tool ) override { m_func( alg_tool ); }

  private:
    Functor m_func;
  };

  /** @brief Convenience method to create a visitor from a lambda expression.
   *  @verbatim
   *  usage:
   *     auto visitor=Ath::ToolVisitor::constVisitor([this](const IAlgTool *alg_tool) { ... });
   *  @endverbatim
   */
  static ConstVisitor constVisitor( std::function<void( const IAlgTool* )>&& func ) {
    return ConstVisitor( std::move( func ) );
  }

  class Visitor : public IVisitor {
  public:
    using Functor = std::function<void( IAlgTool* )>;
    Visitor( Functor&& func ) : m_func( std::move( func ) ) {}
    virtual void visit( IAlgTool* alg_tool ) override { m_func( alg_tool ); }

  private:
    Functor m_func;
  };

  /** @brief Convenience method to create a visitor from a lambda expression.
   *  @verbatim
   *  usage:
   *       auto visitor=Ath::ToolVisitor::visitor([this](IAlgTool *alg_tool) { ... });
   *  @verbatim
   */
  static Visitor visitor( std::function<void( IAlgTool* )>&& func ) { return Visitor( std::move( func ) ); }

  /** Visit the given tools and their child tools.
   *  @param tools list of top level tools to be visited
   * @param visitor a helper class which will be called for each visited tool
   * @param reject_filter an optional regular expression to filter out tools by name i.e. they and their child tools are
   * not visited. usage:
   * @verbatim
   *      auto visitor=Ath::ToolVisitor::constVisitor([this](const IAlgTool *tool) {
   *                                                    this->msg(MSG::DEBUG)  << tool->name() << endmsg;
   *                                                });
   *      ToolVisitor::visit(tools(), visitor);
   * @verbatim
   */
  static void visit( const std::vector<IAlgTool*>& tools, IConstVisitor& visitor,
                     const std::regex& reject_filter = s_noFilter ) {
    recursiveVisit( tools, visitor, reject_filter );
  }
  /** Visit the given tools and their child tools (non-const access).
   */
  static void visit( std::vector<IAlgTool*>& tools, IVisitor& visitor, const std::regex& reject_filter = s_noFilter ) {
    recursiveVisit( tools, visitor, reject_filter );
  }

private:
  template <class T_ToolList, class T_Visitor>
  static void             recursiveVisit( T_ToolList& tools, T_Visitor& visitor, const std::regex& reject_filter );
  static const std::regex s_noFilter;
};

#endif
