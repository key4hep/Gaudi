/*****************************************************************************\
* (c) Copyright 2022-2026 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
#pragma once
#include "details.h"
#include "utilities.h"
#include <GaudiKernel/FunctionalFilterDecision.h>
#include <optional>
#include <string>
#include <vector>

namespace Gaudi::Functional {
  template <typename Container>
  using vector_of_ = std::vector<Container>;
  template <typename Container>
  using vector_of_optional_ = std::vector<std::optional<Container>>;
  using details::vector_of_const_;

  namespace details {

    template <typename Signature, typename Traits_>
    class SplittingMergingTransformer;

    template <typename Out, typename In, typename Traits_>
    class SplittingMergingTransformer<vector_of_<Out>( const vector_of_const_<In>& ), Traits_>
        : public BaseClass_t<Traits_> {
      using base_class = BaseClass_t<Traits_>;
      static_assert( std::is_base_of_v<Algorithm, base_class>, "BaseClass must inherit from Algorithm" );

      // if In is a pointer, it signals optional (as opposed to mandatory) input
      template <typename T>
      using IHandle_t = InputHandle_t<Traits_, std::remove_pointer_t<T>>;
      details::HandleVector<IHandle_t, In> m_ins;
      template <typename T>
      using OHandle_t = details::OutputHandle_t<Traits_, details::remove_optional_t<T>>;
      details::HandleVector<OHandle_t, Out> m_outs;

    public:
      using KeyValues = std::pair<std::string, std::vector<std::string>>;

      SplittingMergingTransformer( std::string name, ISvcLocator* locator, const KeyValues& inputs,
                                   const KeyValues& outputs )
          : base_class( std::move( name ), locator ), m_ins{ this, inputs }, m_outs{ this, outputs } {}

      // accessor to output Locations
      const std::string& outputLocation( unsigned int n ) const { return m_outs.at( n ).key(); }
      unsigned int       outputLocationSize() const { return m_outs.size(); }

      // accessor to input Locations
      const std::string& inputLocation( unsigned int n ) const { return m_ins.at( n ).key(); }
      unsigned int       inputLocationSize() const { return m_ins.size(); }

      // derived classes can NOT implement execute
      StatusCode execute( const EventContext& ctx ) const override final {
        return details::execute( *this, [&] {
          // TODO:FIXME: how does operator() know the number and order of expected outputs?
          m_outs.put( ( *this )( m_ins.get( ctx ) ) );
          return FilterDecision::PASSED;
        } );
      }

      // TODO/FIXME: how does the callee know in which order to produce the outputs?
      //             (note: 'missing' items can be specified by making Out an std::optional<Out>,
      //              and only those entries which contain an Out are stored)
      virtual vector_of_<Out> operator()( const vector_of_const_<In>& ) const = 0;
    };

  } // namespace details

  template <typename Signature, typename Traits_ = Traits::useDefaults>
  using SplittingMergingTransformer = details::SplittingMergingTransformer<Signature, Traits_>;

} // namespace Gaudi::Functional
