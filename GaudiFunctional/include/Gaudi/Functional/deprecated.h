/***********************************************************************************\
* (c) Copyright 2026 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once
#include <tuple>

namespace Gaudi::Functional::details {
  template <typename... T>
  struct type_list;

  template <typename OutputSpec, typename InputSpec, typename Traits_>
  class DataHandleMixin;

  template <typename... Out, typename... In, typename Traits_>
  class [[deprecated( "use DataHandleMixin<type_list<...>, type_list<...>, Traits> instead" )]] DataHandleMixin<
      std::tuple<Out...>, std::tuple<In...>, Traits_>
      : public DataHandleMixin<type_list<Out...>, type_list<In...>, Traits_> {
    using base_class = DataHandleMixin<type_list<Out...>, type_list<In...>, Traits_>;

  public:
    using base_class::base_class;
  };
} // namespace Gaudi::Functional::details
