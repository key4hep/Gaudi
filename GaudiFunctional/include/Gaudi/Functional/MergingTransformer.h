/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include "details.h"
#include "utilities.h"
#include <Gaudi/Algorithm.h>
#include <GaudiKernel/FunctionalFilterDecision.h>
#include <functional>
#include <string>
#include <vector>

namespace Gaudi::Functional {

  using details::vector_of_const_;

  namespace details {
    template <typename F, size_t... Is>
    auto for_impl( F&& f, std::index_sequence<Is...> ) {
      if constexpr ( std::disjunction_v<std::is_void<std::invoke_result_t<F, std::integral_constant<int, Is>>>...> ) {
        ( std::invoke( f, std::integral_constant<int, Is>{} ), ... );
      } else {
        return std::array{ std::invoke( f, std::integral_constant<int, Is>{} )... };
      }
    }

    template <auto N, typename F>
    decltype( auto ) for_( F&& f ) {
      return for_impl( std::forward<F>( f ), std::make_index_sequence<N>{} );
    }

    template <typename Sig>
    struct is_void_fun : std::false_type {};
    template <typename... Args>
    struct is_void_fun<void( Args... )> : std::true_type {};
    template <typename Sig>
    inline constexpr bool is_void_fun_v = is_void_fun<Sig>::value;

    template <typename Signature, typename Traits_, bool isLegacy>
    struct MergingTransformer;

    ////// Many of the same -> 1 or 0
    template <typename Out, typename In, typename Traits_>
    struct MergingTransformer<Out( const vector_of_const_<In>& ), Traits_, true>
        : DataHandleMixin<std::tuple<Out>, std::tuple<>, Traits_> {
    private:
      using base_class = DataHandleMixin<std::tuple<Out>, std::tuple<>, Traits_>;

    public:
      using KeyValue  = typename base_class::KeyValue;
      using KeyValues = typename base_class::KeyValues;

      MergingTransformer( std::string name, ISvcLocator* locator, const KeyValues& inputs )
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
                              Gaudi::Details::Property::ImmediatelyInvokeHandler{ true } } {
        static_assert( std::is_void_v<Out> );
      }

      MergingTransformer( std::string name, ISvcLocator* locator, const KeyValues& inputs, const KeyValue& output )
          : base_class( std::move( name ), locator, output )
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
                              Gaudi::Details::Property::ImmediatelyInvokeHandler{ true } } {
        static_assert( !std::is_void_v<Out> );
      }

      // accessor to input Locations
      const std::string& inputLocation( unsigned int n ) const { return m_inputLocations.value()[n].key(); }
      unsigned int       inputLocationSize() const { return m_inputLocations.value().size(); }

      // derived classes can NOT implement execute
      StatusCode execute() override final {
        vector_of_const_<In> ins;
        ins.reserve( m_inputs.size() );
        std::transform( m_inputs.begin(), m_inputs.end(), std::back_inserter( ins ), details2::get_from_handle<In>{} );
        try {
          if constexpr ( std::is_void_v<Out> ) {
            std::as_const ( *this )( std::as_const( ins ) );
          } else {
            put( std::get<0>( this->m_outputs ), std::as_const( *this )( std::as_const( ins ) ) );
          }
          return FilterDecision::PASSED;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.tag() << " : " << e.message() << endmsg;
          return e.code();
        }
      }

      virtual Out operator()( const vector_of_const_<In>& inputs ) const = 0;

    private:
      // if In is a pointer, it signals optional (as opposed to mandatory) input
      template <typename T>
      using InputHandle_t = InputHandle_t<Traits_, std::remove_pointer_t<T>>;
      std::vector<InputHandle_t<In>>          m_inputs;         //   and make the handles properties instead...
      Gaudi::Property<std::vector<DataObjID>> m_inputLocations; // TODO/FIXME: remove this duplication...
      // TODO/FIXME: replace vector of DataObjID property + call-back with a
      //             vector<handle> property ... as soon as declareProperty can deal with that.
    };

    template <typename Out, typename... Ins, typename Traits_>
    struct MergingTransformer<Out( const vector_of_const_<Ins>&... ), Traits_, false>
        : DataHandleMixin<std::tuple<Out>, std::tuple<>, Traits_> {

      using base_class = DataHandleMixin<std::tuple<Out>, std::tuple<>, Traits_>;
      using KeyValue   = typename base_class::KeyValue;
      using KeyValues  = typename base_class::KeyValues;
      using InKeys     = details::RepeatValues_<KeyValues, sizeof...( Ins )>;

    private:
      auto construct_properties( InKeys inputs ) {
        return details::for_<sizeof...( Ins )>( [&]( auto I ) {
          constexpr auto i   = decltype( I )::value;
          auto&          ins = std::get<i>( inputs );
          return Gaudi::Property<std::vector<DataObjID>>{
              this, ins.first, details::to_DataObjID( ins.second ),
              [this]( auto&& ) {
                auto& handles = std::get<i>( this->m_inputs );
                auto& ins     = std::get<i>( this->m_inputLocations );
                using Handles = typename std::decay_t<decltype( handles )>;
                handles       = make_vector_of_handles<Handles>( this, ins );
                if ( std::is_pointer_v<typename Handles::value_type> ) { // handle constructor does not (yet) allow to
                                                                         // set
                                                                         // optional flag... so do it
                                                                         // explicitly here...
                  std::for_each( handles.begin(), handles.end(), []( auto& h ) { h.setOptional( true ); } );
                }
              },
              Gaudi::Details::Property::ImmediatelyInvokeHandler{ true } };
        } );
      }

    public:
      MergingTransformer( std::string name, ISvcLocator* locator, InKeys inputs )
          : base_class( std::move( name ), locator ), m_inputLocations{ construct_properties( inputs ) } {
        static_assert( std::is_void_v<Out> );
      }

      MergingTransformer( std::string name, ISvcLocator* locator, const KeyValues& inputs )
          : MergingTransformer{ name, locator, InKeys{ inputs } } {
        static_assert( sizeof...( Ins ) == 1 );
      }

      MergingTransformer( std::string name, ISvcLocator* locator, InKeys inputs, const KeyValue& output )
          : base_class( std::move( name ), locator, output ), m_inputLocations{ construct_properties( inputs ) } {
        static_assert( !std::is_void_v<Out> );
      }

      MergingTransformer( std::string name, ISvcLocator* locator, const KeyValues& inputs, const KeyValue& output )
          : MergingTransformer{ name, locator, InKeys{ inputs }, output } {
        static_assert( sizeof...( Ins ) == 1 );
      }

      // accessor to input Locations
      const std::string& inputLocation( unsigned int i, unsigned int j ) const {
        return m_inputLocations.at( i ).value().at( j ).key();
      }
      const std::string& inputLocation( unsigned int i ) const {
        static_assert( sizeof...( Ins ) == 1 );
        return inputLocation( 0, i );
      }
      unsigned int inputLocationSize( int i = 0 ) const { return m_inputLocations.at( i ).value().size(); }

      // derived classes can NOT implement execute
      StatusCode execute( const EventContext& ) const override final {
        std::tuple<vector_of_const_<Ins>...> inss;
        details::for_<sizeof...( Ins )>( [&]( auto I ) {
          constexpr size_t i       = decltype( I )::value;
          auto&            ins     = std::get<i>( inss );
          auto&            handles = std::get<i>( m_inputs );
          ins.reserve( handles.size() );
          std::transform( handles.begin(), handles.end(), std::back_inserter( ins ),
                          details::details2::get_from_handle<typename std::decay_t<decltype( ins )>::value_type>{} );
        } );
        try {
          if constexpr ( std::is_void_v<Out> ) {
            std::apply( [&]( auto&&... ins ) { return std::as_const( *this )( std::as_const( ins )... ); }, inss );
          } else {
            put( std::get<0>( this->m_outputs ),
                 std::apply( [&]( auto&&... ins ) { return std::as_const( *this )( std::as_const( ins )... ); },
                             inss ) );
          }
          return FilterDecision::PASSED;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.tag() << " : " << e.message() << endmsg;
          return e.code();
        }
      }

      virtual Out operator()( const vector_of_const_<Ins>&... inputs ) const = 0;

    private:
      // if In is a pointer, it signals optional (as opposed to mandatory) input
      template <typename T>
      using InputHandle_t = InputHandle_t<Traits_, std::remove_pointer_t<T>>;
      std::tuple<std::vector<InputHandle_t<Ins>>...> m_inputs; //   and make the handles properties instead...
      std::array<Gaudi::Property<std::vector<DataObjID>>, sizeof...( Ins )> m_inputLocations; // TODO/FIXME: remove
                                                                                              // this duplication...
      // TODO/FIXME: replace vector of string property + call-back with a
      //             vector<handle> property ... as soon as declareProperty can deal with that.
    };

  } // namespace details

  template <typename Signature, typename Traits_ = Traits::useDefaults>
  using MergingTransformer = details::MergingTransformer<Signature, Traits_, details::isLegacy<Traits_>>;

  // more meaningful alias for cases where the return type in Signature is void
  template <typename Signature, typename Traits_ = Traits::useDefaults,
            typename = std::enable_if_t<details::is_void_fun_v<Signature>>>
  using MergingConsumer = details::MergingTransformer<Signature, Traits_, details::isLegacy<Traits_>>;

  // M vectors of the same -> N
  template <typename Signature, typename Traits_ = Traits::BaseClass_t<Gaudi::Algorithm>>
  struct MergingMultiTransformer;

  template <typename... Outs, typename... Ins, typename Traits_>
  struct MergingMultiTransformer<std::tuple<Outs...>( vector_of_const_<Ins> const&... ), Traits_>
      : details::DataHandleMixin<std::tuple<Outs...>, std::tuple<>, Traits_> {

  private:
    using base_class = details::DataHandleMixin<std::tuple<Outs...>, std::tuple<>, Traits_>;

  public:
    using KeyValue                 = typename base_class::KeyValue;
    using KeyValues                = typename base_class::KeyValues;
    using InKeys                   = details::RepeatValues_<KeyValues, sizeof...( Ins )>;
    using OutKeys                  = details::RepeatValues_<KeyValue, sizeof...( Outs )>;
    static constexpr size_t n_args = sizeof...( Ins );

    MergingMultiTransformer( std::string const& name, ISvcLocator* pSvcLocator, InKeys inputs, OutKeys outputs )
        : base_class{ name, pSvcLocator, std::move( outputs ) }
        , m_inputLocations{ details::for_<n_args>( [&]( auto I ) {
          constexpr auto i   = decltype( I )::value;
          auto&          ins = std::get<i>( inputs );
          return Gaudi::Property<std::vector<DataObjID>>{
              this, ins.first, details::to_DataObjID( ins.second ),
              [this]( auto&& ) {
                auto& handles = std::get<i>( this->m_inputs );
                auto& ins     = std::get<i>( this->m_inputLocations );
                using In      = typename std::decay_t<decltype( handles )>::value_type;
                handles       = details::make_vector_of_handles<std::decay_t<decltype( handles )>>( this, ins );
                if ( std::is_pointer_v<In> ) { // handle constructor does not (yet) allow to set
                                               // optional flag... so do it
                                               // explicitly here...
                  std::for_each( handles.begin(), handles.end(), []( auto& h ) { h.setOptional( true ); } );
                }
              },
              Gaudi::Details::Property::ImmediatelyInvokeHandler{ true } };
        } ) } {}

    MergingMultiTransformer( std::string const& name, ISvcLocator* pSvcLocator, KeyValues inputs, OutKeys outputs )
        : MergingMultiTransformer{ name, pSvcLocator, InKeys{ std::move( inputs ) }, std::move( outputs ) } {
      static_assert( sizeof...( Ins ) == 1 );
    }

    // accessor to input Locations
    std::string const& inputLocation( unsigned int i, unsigned int j ) const {
      return m_inputLocations.at( i ).value().at( j ).key();
    }
    std::string const& inputLocation( unsigned int j ) const {
      static_assert( n_args == 1 );
      return inputLocation( 0, j );
    }
    unsigned int inputLocationSize( int i = 0 ) const { return m_inputLocations.at( i ).value().size(); }

    // derived classes can NOT implement execute
    StatusCode execute( EventContext const& ) const override final {
      std::tuple<vector_of_const_<Ins>...> inss;
      details::for_<sizeof...( Ins )>( [&]( auto I ) {
        constexpr size_t i       = decltype( I )::value;
        auto&            ins     = std::get<i>( inss );
        auto&            handles = std::get<i>( m_inputs );
        ins.reserve( handles.size() );
        std::transform( handles.begin(), handles.end(), std::back_inserter( ins ),
                        details::details2::get_from_handle<typename std::decay_t<decltype( ins )>::value_type>{} );
      } );
      try {
        std::apply(
            [&]( auto&... outhandle ) {
              GF_SUPPRESS_SPURIOUS_CLANG_WARNING_BEGIN
              std::apply(
                  [&outhandle...]( auto&&... data ) {
                    ( details::put( outhandle, std::forward<decltype( data )>( data ) ), ... );
                  },
                  std::apply( [&]( auto&&... ins ) { return std::as_const( *this )( std::as_const( ins )... ); },
                              inss ) );
              GF_SUPPRESS_SPURIOUS_CLANG_WARNING_END
            },
            this->m_outputs );
        return FilterDecision::PASSED;
      } catch ( GaudiException& e ) {
        ( e.code() ? this->warning() : this->error() ) << e.tag() << " : " << e.message() << endmsg;
        return e.code();
      }
    }

    virtual std::tuple<Outs...> operator()( const vector_of_const_<Ins>&... inputs ) const = 0;

  private:
    // if In is a pointer, it signals optional (as opposed to mandatory) input
    template <typename T>
    using InputHandle_t = details::InputHandle_t<Traits_, typename std::remove_pointer<T>::type>;
    std::tuple<std::vector<InputHandle_t<Ins>>...> m_inputs; //   and make the handles properties instead...
    std::array<Gaudi::Property<std::vector<DataObjID>>, sizeof...( Ins )> m_inputLocations; // TODO/FIXME: remove this
                                                                                            // duplication...
    // TODO/FIXME: replace vector of string property + call-back with a
    //             vector<handle> property ... as soon as declareProperty can deal with that.
  };

  // Many of the same -> N with filter functionality
  template <typename Signature, typename Traits_ = Traits::BaseClass_t<Gaudi::Algorithm>>
  struct MergingMultiTransformerFilter;

  template <typename... Outs, typename In, typename Traits_>
  struct MergingMultiTransformerFilter<std::tuple<Outs...>( vector_of_const_<In> const& ), Traits_>
      : details::DataHandleMixin<std::tuple<Outs...>, std::tuple<>, Traits_> {

  private:
    using base_class = details::DataHandleMixin<std::tuple<Outs...>, std::tuple<>, Traits_>;

  public:
    using KeyValue  = typename base_class::KeyValue;
    using KeyValues = typename base_class::KeyValues;
    using OutKeys   = details::RepeatValues_<KeyValue, sizeof...( Outs )>;

    MergingMultiTransformerFilter( std::string const& name, ISvcLocator* locator, KeyValues const& inputs,
                                   OutKeys const& outputs );

    // accessor to input Locations
    std::string const& inputLocation( unsigned int n ) const { return m_inputLocations.value()[n].key(); }
    unsigned int       inputLocationSize() const { return m_inputLocations.value().size(); }

    // derived classes can NOT implement execute
    StatusCode execute( EventContext const& ) const override final {
      vector_of_const_<In> ins;
      ins.reserve( m_inputs.size() );
      std::transform( m_inputs.begin(), m_inputs.end(), std::back_inserter( ins ),
                      details::details2::get_from_handle<In>{} );
      try {
        return std::apply(
                   [&]( auto&... outhandle ) {
                     GF_SUPPRESS_SPURIOUS_CLANG_WARNING_BEGIN
                     return std::apply(
                         [&outhandle...]( bool passed, auto&&... data ) {
                           ( details::put( outhandle, std::forward<decltype( data )>( data ) ), ... );
                           return passed;
                         },
                         ( *this )( std::as_const( ins ) ) );
                     GF_SUPPRESS_SPURIOUS_CLANG_WARNING_END
                   },
                   this->m_outputs )
                   ? FilterDecision::PASSED
                   : FilterDecision::FAILED;
      } catch ( GaudiException& e ) {
        ( e.code() ? this->warning() : this->error() ) << e.tag() << " : " << e.message() << endmsg;
        return e.code();
      }
    }

    virtual std::tuple<bool, Outs...> operator()( const vector_of_const_<In>& inputs ) const = 0;

  private:
    // if In is a pointer, it signals optional (as opposed to mandatory) input
    template <typename T>
    using InputHandle_t = details::InputHandle_t<Traits_, typename std::remove_pointer<T>::type>;
    std::vector<InputHandle_t<In>>          m_inputs;         //   and make the handles properties instead...
    Gaudi::Property<std::vector<DataObjID>> m_inputLocations; // TODO/FIXME: remove this duplication...
    // TODO/FIXME: replace vector of string property + call-back with a
    //             vector<handle> property ... as soon as declareProperty can deal with that.
  };

  template <typename... Outs, typename In, typename Traits_>
  MergingMultiTransformerFilter<std::tuple<Outs...>( const vector_of_const_<In>& ),
                                Traits_>::MergingMultiTransformerFilter( std::string const& name,
                                                                         ISvcLocator*       pSvcLocator,
                                                                         KeyValues const&   inputs,
                                                                         OutKeys const&     outputs )
      : base_class( name, pSvcLocator, outputs )
      , m_inputLocations{
            this, inputs.first, details::to_DataObjID( inputs.second ),
            [this]( Gaudi::Details::PropertyBase& ) {
              this->m_inputs = details::make_vector_of_handles<decltype( this->m_inputs )>( this, m_inputLocations );
              if ( std::is_pointer_v<In> ) { // handle constructor does not (yet) allow to set
                                             // optional flag... so do it
                                             // explicitly here...
                std::for_each( this->m_inputs.begin(), this->m_inputs.end(), []( auto& h ) { h.setOptional( true ); } );
              }
            },
            Gaudi::Details::Property::ImmediatelyInvokeHandler{ true } } {}

} // namespace Gaudi::Functional
