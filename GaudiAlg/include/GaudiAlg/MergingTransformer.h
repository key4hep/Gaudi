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
#ifndef MERGING_TRANSFORMER_H
#define MERGING_TRANSFORMER_H

#include <functional>
#include <string>
#include <vector>

#include "Gaudi/Algorithm.h"
#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"
#include "GaudiKernel/FunctionalFilterDecision.h"

namespace Gaudi::Functional {

  using details::vector_of_const_;

  namespace details {

    template <typename Signature, typename Traits_, bool isLegacy>
    struct MergingTransformer;

    ////// Many of the same -> 1
    template <typename Out, typename In, typename Traits_>
    struct MergingTransformer<Out( const vector_of_const_<In>& ), Traits_, true>
        : DataHandleMixin<std::tuple<Out>, std::tuple<>, Traits_> {
    private:
      using base_class = DataHandleMixin<std::tuple<Out>, std::tuple<>, Traits_>;

    public:
      using KeyValue  = typename base_class::KeyValue;
      using KeyValues = typename base_class::KeyValues;

      MergingTransformer( std::string name, ISvcLocator* locator, const KeyValues& inputs, const KeyValue& output )
          : base_class( std::move( name ), locator, output )
          , m_inputLocations{this, inputs.first, inputs.second,
                             [=]( Gaudi::Details::PropertyBase& ) {
                               this->m_inputs =
                                   make_vector_of_handles<decltype( this->m_inputs )>( this, m_inputLocations );
                               if ( std::is_pointer_v<In> ) { // handle constructor does not (yet) allow to set
                                                              // optional flag... so do it
                                                              // explicitly here...
                                 std::for_each( this->m_inputs.begin(), this->m_inputs.end(),
                                                []( auto& h ) { h.setOptional( true ); } );
                               }
                             },
                             Gaudi::Details::Property::ImmediatelyInvokeHandler{true}} {}

      // accessor to input Locations
      const std::string& inputLocation( unsigned int n ) const { return m_inputLocations.value()[n]; }
      unsigned int       inputLocationSize() const { return m_inputLocations.value().size(); }

      // derived classes can NOT implement execute
      StatusCode execute() override final {
        vector_of_const_<In> ins;
        ins.reserve( m_inputs.size() );
        std::transform( m_inputs.begin(), m_inputs.end(), std::back_inserter( ins ), details2::get_from_handle<In>{} );
        try {
          put( std::get<0>( this->m_outputs ), std::as_const( *this )( std::as_const( ins ) ) );
          return FilterDecision::PASSED;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
      }

      virtual Out operator()( const vector_of_const_<In>& inputs ) const = 0;

    private:
      // if In is a pointer, it signals optional (as opposed to mandatory) input
      template <typename T>
      using InputHandle_t = InputHandle_t<Traits_, std::remove_pointer_t<T>>;
      std::vector<InputHandle_t<In>>            m_inputs;         //   and make the handles properties instead...
      Gaudi::Property<std::vector<std::string>> m_inputLocations; // TODO/FIXME: remove this duplication...
      // TODO/FIXME: replace vector of string property + call-back with a
      //             vector<handle> property ... as soon as declareProperty can deal with that.
    };

    template <typename Out, typename In, typename Traits_>
    struct MergingTransformer<Out( const vector_of_const_<In>& ), Traits_, false>
        : DataHandleMixin<std::tuple<Out>, std::tuple<>, Traits_> {
    private:
      using base_class = DataHandleMixin<std::tuple<Out>, std::tuple<>, Traits_>;

    public:
      using KeyValue  = typename base_class::KeyValue;
      using KeyValues = typename base_class::KeyValues;

      MergingTransformer( std::string name, ISvcLocator* locator, const KeyValues& inputs, const KeyValue& output )
          : base_class( std::move( name ), locator, output )
          , m_inputLocations{this, inputs.first, inputs.second,
                             [=]( Gaudi::Details::PropertyBase& ) {
                               this->m_inputs =
                                   make_vector_of_handles<decltype( this->m_inputs )>( this, m_inputLocations );
                               if ( std::is_pointer_v<In> ) { // handle constructor does not (yet) allow to set
                                                              // optional flag... so do it
                                                              // explicitly here...
                                 std::for_each( this->m_inputs.begin(), this->m_inputs.end(),
                                                []( auto& h ) { h.setOptional( true ); } );
                               }
                             },
                             Gaudi::Details::Property::ImmediatelyInvokeHandler{true}} {}

      // accessor to input Locations
      const std::string& inputLocation( unsigned int n ) const { return m_inputLocations.value()[n]; }
      unsigned int       inputLocationSize() const { return m_inputLocations.value().size(); }

      // derived classes can NOT implement execute
      StatusCode execute( const EventContext& ) const override final {
        vector_of_const_<In> ins;
        ins.reserve( m_inputs.size() );
        std::transform( m_inputs.begin(), m_inputs.end(), std::back_inserter( ins ), details2::get_from_handle<In>{} );
        try {
          put( std::get<0>( this->m_outputs ), ( *this )( std::as_const( ins ) ) );
          return FilterDecision::PASSED;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
      }

      virtual Out operator()( const vector_of_const_<In>& inputs ) const = 0;

    private:
      // if In is a pointer, it signals optional (as opposed to mandatory) input
      template <typename T>
      using InputHandle_t = InputHandle_t<Traits_, std::remove_pointer_t<T>>;
      std::vector<InputHandle_t<In>>            m_inputs;         //   and make the handles properties instead...
      Gaudi::Property<std::vector<std::string>> m_inputLocations; // TODO/FIXME: remove this duplication...
      // TODO/FIXME: replace vector of string property + call-back with a
      //             vector<handle> property ... as soon as declareProperty can deal with that.
    };

  } // namespace details

  template <typename Signature, typename Traits_ = Traits::useDefaults>
  using MergingTransformer = details::MergingTransformer<Signature, Traits_, details::isLegacy<Traits_>>;

  // Many of the same -> N
  template <typename Signature, typename Traits_ = Traits::BaseClass_t<Gaudi::Algorithm>>
  struct MergingMultiTransformer;

  template <typename... Outs, typename In, typename Traits_>
  struct MergingMultiTransformer<std::tuple<Outs...>( vector_of_const_<In> const& ), Traits_>
      : details::DataHandleMixin<std::tuple<Outs...>, std::tuple<>, Traits_> {

  private:
    using base_class = details::DataHandleMixin<std::tuple<Outs...>, std::tuple<>, Traits_>;

  public:
    using KeyValue  = typename base_class::KeyValue;
    using KeyValues = typename base_class::KeyValues;
    using OutKeys   = std::array<KeyValue, sizeof...( Outs )>;

    MergingMultiTransformer( std::string const& name, ISvcLocator* locator, KeyValues const& inputs,
                             OutKeys const& outputs );

    // accessor to input Locations
    std::string const& inputLocation( unsigned int n ) const { return m_inputLocations.value()[n]; }
    unsigned int       inputLocationSize() const { return m_inputLocations.value().size(); }

    // derived classes can NOT implement execute
    StatusCode execute( EventContext const& ) const override final {
      vector_of_const_<In> ins;
      ins.reserve( m_inputs.size() );
      std::transform( m_inputs.begin(), m_inputs.end(), std::back_inserter( ins ),
                      details::details2::get_from_handle<In>{} );
      try {
        std::apply(
            [&]( auto&... outhandle ) {
              GF_SUPPRESS_SPURIOUS_CLANG_WARNING_BEGIN
              std::apply(
                  [&outhandle...]( auto&&... data ) {
                    ( details::put( outhandle, std::forward<decltype( data )>( data ) ), ... );
                  },
                  std::as_const( *this )( std::as_const( ins ) ) );
              GF_SUPPRESS_SPURIOUS_CLANG_WARNING_END
            },
            this->m_outputs );
        return FilterDecision::PASSED;
      } catch ( GaudiException& e ) {
        ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
        return e.code();
      }
    }

    virtual std::tuple<Outs...> operator()( const vector_of_const_<In>& inputs ) const = 0;

  private:
    // if In is a pointer, it signals optional (as opposed to mandatory) input
    template <typename T>
    using InputHandle_t = details::InputHandle_t<Traits_, typename std::remove_pointer<T>::type>;
    std::vector<InputHandle_t<In>>            m_inputs;         //   and make the handles properties instead...
    Gaudi::Property<std::vector<std::string>> m_inputLocations; // TODO/FIXME: remove this duplication...
    // TODO/FIXME: replace vector of string property + call-back with a
    //             vector<handle> property ... as soon as declareProperty can deal with that.
  };

  template <typename... Outs, typename In, typename Traits_>
  MergingMultiTransformer<std::tuple<Outs...>( const vector_of_const_<In>& ), Traits_>::MergingMultiTransformer(
      std::string const& name, ISvcLocator* pSvcLocator, KeyValues const& inputs, OutKeys const& outputs )
      : base_class( name, pSvcLocator, outputs )
      , m_inputLocations{
            this, inputs.first, inputs.second,
            [=]( Gaudi::Details::PropertyBase& ) {
              this->m_inputs = details::make_vector_of_handles<decltype( this->m_inputs )>( this, m_inputLocations );
              if ( std::is_pointer_v<In> ) { // handle constructor does not (yet) allow to set
                                             // optional flag... so do it
                                             // explicitly here...
                std::for_each( this->m_inputs.begin(), this->m_inputs.end(), []( auto& h ) { h.setOptional( true ); } );
              }
            },
            Gaudi::Details::Property::ImmediatelyInvokeHandler{true}} {}

} // namespace Gaudi::Functional

#endif
