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

template <class T_SrcToolPtr, class T_DestToolPtr>
void reverseAppend( const std::vector<T_SrcToolPtr>& src, std::vector<T_DestToolPtr>& dest,
                    const std::regex& reject_filter, bool use_filter ) {
  dest.reserve( dest.size() + src.size() );
  for ( auto rev_iter = src.rbegin(); rev_iter != src.rend(); ++rev_iter ) {
    if ( !use_filter || !std::regex_match( ( *rev_iter )->name(), reject_filter ) ) { // @TODO include type ?
      dest.push_back( *rev_iter );
    }
  }
}

// helper class to get the correct pointer type for the only supported tool implementation for the recursive visit
template <class T_ToolList>
class ToolImpl {
public:
  using InterfaceType = const IAlgTool*;
  using PtrType       = const AlgTool*;
};

// Specialisation to remove "const" for non const IAlgTool
template <>
class ToolImpl<std::vector<IAlgTool*>> {
public:
  using InterfaceType = IAlgTool*;
  using PtrType       = AlgTool*;
};

template <class T_ToolList, class T_Visitor>
void ToolVisitor::recursiveVisit( T_ToolList& tools, T_Visitor& visitor, const std::regex& reject_filter ) {
  bool use_filter = ( &reject_filter != &ToolVisitor::s_noFilter ) &&
                    ( !std::regex_match( "", reject_filter ) ); // @TODO remove empty string match-test ?
  std::vector<typename ToolImpl<T_ToolList>::InterfaceType> stack;
  // reverse tool lost to process tools in given order
  reverseAppend( tools, stack, reject_filter, use_filter );

  // keep track of tools which have been visited to prevent infinit loops in case of circular tool dependencies.
  std::set<typename ToolImpl<T_ToolList>::InterfaceType> visited;
  while ( !stack.empty() ) {
    typename ToolImpl<T_ToolList>::InterfaceType a_tool = stack.back();
    stack.pop_back();
    if ( a_tool ) {
      if ( visited.insert( a_tool ).second ) {
        visitor.visit( a_tool );
        // also visit all child tools
        typename ToolImpl<T_ToolList>::PtrType tool_impl =
            dynamic_cast<typename ToolImpl<T_ToolList>::PtrType>( a_tool );
        if ( tool_impl ) { reverseAppend( tool_impl->tools(), stack, reject_filter, use_filter ); }
      }
      // @TODO warn about cicular tool dependencies ?
    }
  }
}

template void ToolVisitor::recursiveVisit( const std::vector<IAlgTool*>& tools, IConstVisitor& visitor,
                                           const std::regex& reject_filter );
template void ToolVisitor::recursiveVisit( std::vector<IAlgTool*>& tools, IVisitor& visitor,
                                           const std::regex& reject_filter );
