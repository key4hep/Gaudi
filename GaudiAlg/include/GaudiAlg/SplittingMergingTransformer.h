/*****************************************************************************\
* (c) Copyright 2022 CERN for the benefit of the LHCb Collaboration           *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/

#pragma once

#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"
#include "GaudiKernel/FunctionalFilterDecision.h"
#include <functional>
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

    template <typename Signature, typename Traits_, bool isLegacy>
    class SplittingMergingTransformer;

    template <typename Out, typename In, typename Traits_>
    class SplittingMergingTransformer<vector_of_<Out>( const vector_of_const_<In>& ), Traits_, false>
        : public BaseClass_t<Traits_> {
      using base_class = BaseClass_t<Traits_>;
      static_assert( std::is_base_of_v<Algorithm, base_class>, "BaseClass must inherit from Algorithm" );

    public:
      using KeyValues = std::pair<std::string, std::vector<std::string>>;

      SplittingMergingTransformer( std::string name, ISvcLocator* locator, const KeyValues& inputs,
                                   const KeyValues& outputs )
          : base_class( std::move( name ), locator )
          , m_inputLocations{ this, inputs.first, details::to_DataObjID( inputs.second ),
                              [this]( Gaudi::Details::PropertyBase& ) {
                                this->m_inputs =
                                    make_vector_of_handles<decltype( this->m_inputs )>( this, m_inputLocations );
                                if ( std::is_pointer_v<In> ) { // handle constructor does not (yet) allow to set
                                                               // optional flag... so do it
                                                               // explicitly here...
                                  std::for_each( this->m_inputs.begin(), this->m_inputs.end(),
                                                 []( auto& h ) { h.setOptional( true ); } );
                                }
                              },
                              Gaudi::Details::Property::ImmediatelyInvokeHandler{ true } }
          , m_outputLocations(
                this, outputs.first, details::to_DataObjID( outputs.second ),
                [this]( Gaudi::Details::PropertyBase& ) {
                  this->m_outputs =
                      details::make_vector_of_handles<decltype( this->m_outputs )>( this, m_outputLocations );
                  if constexpr ( details::is_optional_v<Out> ) { // handle constructor does not (yet) allow to
                                                                 // set optional flag... so
                                                                 // do it explicitly here...
                    std::for_each( this->m_outputs.begin(), this->m_outputs.end(),
                                   []( auto& h ) { h.setOptional( true ); } );
                  }
                },
                Gaudi::Details::Property::ImmediatelyInvokeHandler{ true } ) {}

      // accessor to output Locations
      const std::string& outputLocation( unsigned int n ) const { return m_outputLocations.value()[n].key(); }
      unsigned int       outputLocationSize() const { return m_outputLocations.value().size(); }

      // accessor to input Locations
      const std::string& inputLocation( unsigned int n ) const { return m_inputLocations.value()[n].key(); }
      unsigned int       inputLocationSize() const { return m_inputLocations.value().size(); }

      // derived classes can NOT implement execute
      StatusCode execute( const EventContext& ) const override final {
        try {
          vector_of_const_<In> ins;
          ins.reserve( m_inputs.size() );
          std::transform( m_inputs.begin(), m_inputs.end(), std::back_inserter( ins ),
                          details2::get_from_handle<In>{} );
          // TODO:FIXME: how does operator() know the number and order of expected outputs?
          auto out = ( *this )( std::as_const( ins ) );
          if ( out.size() != m_outputs.size() ) {
            throw GaudiException( "Error during transform: expected " + std::to_string( m_outputs.size() ) +
                                      " containers, got " + std::to_string( out.size() ) + " instead",
                                  this->name(), StatusCode::FAILURE );
          }
          for ( unsigned i = 0; i != out.size(); ++i ) details::put( m_outputs[i], std::move( out[i] ) );
          return FilterDecision::PASSED;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.tag() << " : " << e.message() << endmsg;
          return e.code();
        }
      }

      // TODO/FIXME: how does the callee know in which order to produce the outputs?
      //             (note: 'missing' items can be specified by making Out an std::optional<Out>,
      //              and only those entries which contain an Out are stored)
      virtual vector_of_<Out> operator()( const vector_of_const_<In>& ) const = 0;

    private:
      // if In is a pointer, it signals optional (as opposed to mandatory) input
      template <typename T>
      using InputHandle_t = InputHandle_t<Traits_, std::remove_pointer_t<T>>;
      std::vector<InputHandle_t<In>>          m_inputs;         //   and make the handles properties instead...
      Gaudi::Property<std::vector<DataObjID>> m_inputLocations; // TODO/FIXME: remove this duplication...
      // TODO/FIXME: replace vector of DataObjID property + call-back with a
      //             vector<handle> property ... as soon as declareProperty can deal with that.
      template <typename T>
      using OutputHandle = details::OutputHandle_t<Traits_, details::remove_optional_t<T>>;
      std::vector<OutputHandle<Out>>          m_outputs;
      Gaudi::Property<std::vector<DataObjID>> m_outputLocations; // TODO/FIXME  for now: use a call-back to update the
                                                                 // actual handles!
    };

  } // namespace details

  template <typename Signature, typename Traits_ = Traits::useDefaults>
  using SplittingMergingTransformer =
      details::SplittingMergingTransformer<Signature, Traits_, false>; // details::isLegacy<Traits_>>;

} // namespace Gaudi::Functional
