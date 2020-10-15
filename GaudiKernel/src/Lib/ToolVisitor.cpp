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
#include "GaudiKernel/ToolVisitor.h"
#include "GaudiKernel/AlgTool.h"

const std::regex ToolVisitor::s_noFilter;

void reverseAppend( const std::vector<IAlgTool*>& src, std::vector<IAlgTool*>& dest, const std::regex& reject_filter,
                    bool use_filter ) {
  dest.reserve( dest.size() + src.size() );
  std::copy_if( src.rbegin(), src.rend(), std::back_inserter( dest ), [&]( const auto& i ) {
    return !use_filter || !std::regex_match( i->name(), reject_filter );
  } ); // @TODO include type ?
}

void ToolVisitor::recursiveVisit( const std::vector<IAlgTool*>& tools, IVisitor const& visitor,
                                  const std::regex& reject_filter ) {
  bool use_filter = ( &reject_filter != &ToolVisitor::s_noFilter ) &&
                    ( !std::regex_match( "", reject_filter ) ); // @TODO remove empty string match-test ?
  std::vector<IAlgTool*> stack;
  // reverse tool lost to process tools in given order
  reverseAppend( tools, stack, reject_filter, use_filter );

  // keep track of tools which have been visited to prevent infinit loops in case of circular tool dependencies.
  std::set<IAlgTool*> visited;
  while ( !stack.empty() ) {
    auto* a_tool = stack.back();
    stack.pop_back();
    if ( a_tool ) {
      if ( visited.insert( a_tool ).second ) {
        visitor.visit( a_tool );
        // also visit all child tools
        if ( auto* tool_impl = dynamic_cast<AlgTool*>( a_tool ); tool_impl ) {
          reverseAppend( tool_impl->tools(), stack, reject_filter, use_filter );
        }
      }
      // @TODO warn about cicular tool dependencies ?
    }
  }
}
