/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
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
#include <string>

namespace Gaudi::Functional {

  namespace details {

    template <typename Sig>
    constexpr bool is_void_fun_v = false;
    template <typename... Args>
    constexpr bool is_void_fun_v<void( Args... )> = true;
    template <typename Sig>
    concept is_void_fun = is_void_fun_v<Sig>;

    template <typename Signature, typename Traits_, bool isLegacy>
    struct MergingTransformer;

    ////// Many of the same -> 1 or 0
    template <typename Out, typename... Ins, typename Traits_>
    struct MergingTransformer<Out( const vector_of_const_<Ins>&... ), Traits_, true>
        : DataHandleMixin<std::tuple<Out>, std::tuple<>, Traits_> {

      using base_class = DataHandleMixin<std::tuple<Out>, std::tuple<>, Traits_>;
      using KeyValue   = typename base_class::KeyValue;
      using KeyValues  = typename base_class::KeyValues;
      using InKeys     = details::RepeatValues_<KeyValues, sizeof...( Ins )>;

      // if In is a pointer, it signals optional (as opposed to mandatory) input
      template <typename T>
      using InputHandle = InputHandle_t<Traits_, std::remove_pointer_t<T>>;
      std::tuple<HandleVector<InputHandle, Ins>...> m_ins;

    public:
      MergingTransformer( std::string name, ISvcLocator* locator, InKeys inputs )
        requires std::is_void_v<Out>
          : base_class( std::move( name ), locator )
          , m_ins{ make_HandleVectorTuple<InputHandle, Ins...>( this, inputs ) } {}

      MergingTransformer( std::string name, ISvcLocator* locator, const KeyValues& inputs )
        requires( std::is_void_v<Out> && sizeof...( Ins ) == 1 )
          : MergingTransformer{ std::move( name ), locator, InKeys{ inputs } } {}

      MergingTransformer( std::string name, ISvcLocator* locator, InKeys inputs, const KeyValue& output )
        requires( !std::is_void_v<Out> )
          : base_class( std::move( name ), locator, output )
          , m_ins{ make_HandleVectorTuple<InputHandle, Ins...>( this, inputs ) } {}

      MergingTransformer( std::string name, ISvcLocator* locator, const KeyValues& inputs, const KeyValue& output )
        requires( !std::is_void_v<Out> && sizeof...( Ins ) == 1 )
          : MergingTransformer{ std::move( name ), locator, InKeys{ inputs }, output } {}

      // accessor to input Locations
      const std::string& inputLocation( unsigned int i, unsigned int j ) const {
        return getLocations( m_ins, i ).at( j ).key();
      }

      const std::string& inputLocation( unsigned int i ) const
        requires( sizeof...( Ins ) == 1 )
      {
        return inputLocation( 0, i );
      }

      unsigned int inputLocationSize( int i = 0 ) const { return getLocations( m_ins, i ).size(); }

      // derived classes can NOT implement execute
      StatusCode execute() override final {
        return details::execute( *this, [&] {
          const auto& ctx = Gaudi::Hive::currentContext();
          if constexpr ( std::is_void_v<Out> ) {
            std::apply( [&]( auto&&... handle ) { ( *this )( handle.get( ctx )... ); }, m_ins );
          } else {
            put( std::get<0>( this->m_outputs ),
                 std::apply( [&]( auto&&... handle ) { return ( *this )( handle.get( ctx )... ); }, m_ins ) );
          }
          return FilterDecision::PASSED;
        } );
      }

      virtual Out operator()( const vector_of_const_<Ins>&... inputs ) const = 0;
    };

    ////// Many of the same -> 1 or 0
    template <typename Out, typename... Ins, typename Traits_>
    struct MergingTransformer<Out( const vector_of_const_<Ins>&... ), Traits_, false>
        : DataHandleMixin<std::tuple<Out>, std::tuple<>, Traits_> {

      using base_class = DataHandleMixin<std::tuple<Out>, std::tuple<>, Traits_>;
      using KeyValue   = typename base_class::KeyValue;
      using KeyValues  = typename base_class::KeyValues;
      using InKeys     = details::RepeatValues_<KeyValues, sizeof...( Ins )>;

      // if In is a pointer, it signals optional (as opposed to mandatory) input
      template <typename T>
      using InputHandle = InputHandle_t<Traits_, std::remove_pointer_t<T>>;
      std::tuple<HandleVector<InputHandle, Ins>...> m_ins;

    public:
      MergingTransformer( std::string name, ISvcLocator* locator, InKeys inputs )
        requires std::is_void_v<Out>
          : base_class( std::move( name ), locator )
          , m_ins{ make_HandleVectorTuple<InputHandle, Ins...>( this, inputs ) } {}

      MergingTransformer( std::string name, ISvcLocator* locator, const KeyValues& inputs )
        requires( std::is_void_v<Out> && sizeof...( Ins ) == 1 )
          : MergingTransformer{ std::move( name ), locator, InKeys{ inputs } } {}

      MergingTransformer( std::string name, ISvcLocator* locator, InKeys inputs, const KeyValue& output )
        requires( !std::is_void_v<Out> )
          : base_class( std::move( name ), locator, output )
          , m_ins{ make_HandleVectorTuple<InputHandle, Ins...>( this, inputs ) } {}

      MergingTransformer( std::string name, ISvcLocator* locator, const KeyValues& inputs, const KeyValue& output )
        requires( !std::is_void_v<Out> && sizeof...( Ins ) == 1 )
          : MergingTransformer{ std::move( name ), locator, InKeys{ inputs }, output } {}

      // accessor to input Locations
      const std::string& inputLocation( unsigned int i, unsigned int j ) const {
        return getLocations( m_ins, i ).at( j ).key();
      }

      const std::string& inputLocation( unsigned int i ) const
        requires( sizeof...( Ins ) == 1 )
      {
        return inputLocation( 0, i );
      }

      unsigned int inputLocationSize( int i = 0 ) const { return getLocations( m_ins, i ).size(); }

      // derived classes can NOT implement execute
      StatusCode execute( const EventContext& ctx ) const override final {
        return details::execute( *this, [&] {
          if constexpr ( std::is_void_v<Out> ) {
            std::apply( [&]( auto&&... handle ) { ( *this )( handle.get( ctx )... ); }, m_ins );
          } else {
            put( std::get<0>( this->m_outputs ),
                 std::apply( [&]( auto&&... handle ) { return ( *this )( handle.get( ctx )... ); }, m_ins ) );
          }
          return FilterDecision::PASSED;
        } );
      }

      virtual Out operator()( const vector_of_const_<Ins>&... inputs ) const = 0;
    };

  } // namespace details

  template <typename Signature, typename Traits_ = Traits::useDefaults>
  using MergingTransformer = details::MergingTransformer<Signature, Traits_, details::isLegacy<Traits_>>;

  // more meaningful alias for cases where the return type in Signature is void
  template <details::is_void_fun Signature, typename Traits_ = Traits::useDefaults>
  using MergingConsumer = details::MergingTransformer<Signature, Traits_, details::isLegacy<Traits_>>;

  using details::vector_of_const_;

  // M vectors of the same -> N
  template <typename Signature, typename Traits_ = Traits::BaseClass_t<Gaudi::Algorithm>>
  struct MergingMultiTransformer;

  template <typename... Outs, typename... Ins, typename Traits_>
  struct MergingMultiTransformer<std::tuple<Outs...>( details::vector_of_const_<Ins> const&... ), Traits_>
      : details::DataHandleMixin<std::tuple<Outs...>, std::tuple<>, Traits_> {

    using base_class = details::DataHandleMixin<std::tuple<Outs...>, std::tuple<>, Traits_>;
    // if In is a pointer, it signals optional (as opposed to mandatory) input
    template <typename T>
    using InputHandle = details::InputHandle_t<Traits_, std::remove_pointer_t<T>>;
    std::tuple<details::HandleVector<InputHandle, Ins>...> m_ins;

  public:
    using KeyValue                 = typename base_class::KeyValue;
    using KeyValues                = typename base_class::KeyValues;
    using InKeys                   = details::RepeatValues_<KeyValues, sizeof...( Ins )>;
    using OutKeys                  = details::RepeatValues_<KeyValue, sizeof...( Outs )>;
    static constexpr size_t n_args = sizeof...( Ins );

    MergingMultiTransformer( std::string const& name, ISvcLocator* pSvcLocator, InKeys inputs, OutKeys outputs )
        : base_class{ name, pSvcLocator, std::move( outputs ) }
        , m_ins{ details::make_HandleVectorTuple<InputHandle, Ins...>( this, inputs ) } {}

    MergingMultiTransformer( std::string const& name, ISvcLocator* pSvcLocator, KeyValues inputs, OutKeys outputs )
      requires( sizeof...( Ins ) == 1 )
        : MergingMultiTransformer{ name, pSvcLocator, InKeys{ std::move( inputs ) }, std::move( outputs ) } {}

    // accessor to input Locations
    std::string const& inputLocation( unsigned int i, unsigned int j ) const {
      return getLocations( m_ins, i ).at( j ).key();
    }
    std::string const& inputLocation( unsigned int j ) const
      requires( n_args == 1 )
    {
      return inputLocation( 0, j );
    }
    unsigned int inputLocationSize( int i = 0 ) const { return getLocations( m_ins, i ).size(); }

    // derived classes can NOT implement execute
    StatusCode execute( EventContext const& ctx ) const override final {
      return details::execute( *this, [&] {
        std::apply(
            [&]( auto&... outhandle ) {
              std::apply(
                  [&outhandle...]( auto&&... data ) {
                    ( details::put( outhandle, std::forward<decltype( data )>( data ) ), ... );
                  },
                  std::apply( [&]( auto&&... in ) { return ( *this )( in.get( ctx )... ); }, this->m_ins ) );
            },
            this->m_outputs );
        return FilterDecision::PASSED;
      } );
    }

    virtual std::tuple<Outs...> operator()( const vector_of_const_<Ins>&... inputs ) const = 0;
  };

  // Many of the same -> N with filter functionality
  template <typename Signature, typename Traits_ = Traits::BaseClass_t<Gaudi::Algorithm>>
  struct MergingMultiTransformerFilter;

  template <typename... Outs, typename In, typename Traits_>
  struct MergingMultiTransformerFilter<std::tuple<Outs...>( vector_of_const_<In> const& ), Traits_>
      : details::DataHandleMixin<std::tuple<Outs...>, std::tuple<>, Traits_> {

  private:
    using base_class = details::DataHandleMixin<std::tuple<Outs...>, std::tuple<>, Traits_>;

    // if In is a pointer, it signals optional (as opposed to mandatory) input
    template <typename T>
    using InputHandle = details::InputHandle_t<Traits_, std::remove_pointer_t<T>>;
    details::HandleVector<InputHandle, In> m_ins;

  public:
    using KeyValue  = typename base_class::KeyValue;
    using KeyValues = typename base_class::KeyValues;
    using OutKeys   = details::RepeatValues_<KeyValue, sizeof...( Outs )>;

    MergingMultiTransformerFilter( std::string const& name, ISvcLocator* locator, KeyValues const& inputs,
                                   OutKeys const& outputs )
        : base_class( name, locator, outputs ), m_ins{ this, inputs } {}

    // accessor to input Locations
    std::string const& inputLocation( unsigned int n ) const { return m_ins.at( n ).key(); }
    unsigned int       inputLocationSize() const { return m_ins.size(); }

    // derived classes can NOT implement execute
    StatusCode execute( EventContext const& ctx ) const override final {
      return details::execute( *this, [&] {
        return std::apply(
                   [&]( auto&... outhandle ) {
                     return std::apply(
                         [&outhandle...]( bool passed, auto&&... data ) {
                           ( details::put( outhandle, std::forward<decltype( data )>( data ) ), ... );
                           return passed;
                         },
                         ( *this )( this->m_ins.get( ctx ) ) );
                   },
                   this->m_outputs )
                   ? FilterDecision::PASSED
                   : FilterDecision::FAILED;
      } );
    }

    virtual std::tuple<bool, Outs...> operator()( const vector_of_const_<In>& inputs ) const = 0;
  };

} // namespace Gaudi::Functional
