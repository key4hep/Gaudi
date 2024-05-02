/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <Gaudi/Accumulators/Histogram.h>

namespace Gaudi::Accumulators {

  /**
   * A Wrapper of an Histogram base class using Properties to define title and axis
   *
   * Practically, this is an optional of the underlying Histogram with creation
   * on demand, via the createHistogram method so one can wait that properties' values
   * are known.
   * By default creation will happen at initialization of the owner, but this can be
   * disabled at construction time, in which case the user has to call createHistogram
   * manually.
   * Note that disabling automatic initialization requires using the plain
   * constructor and passing title and axis. They can of course be empty and overwritten
   * via Properties. But there was no easier way providing char* convert automagically
   * to bool in C++ creating potential confusion between title on doNotInitialize
   *
   * This wrapper expects the Axis type of the Histogram to be usable in a
   * Gaudi Property and thus to define the required Grammar, parse function and operator<<
   * It also requires the underlying Histogram type to define :
   *   - AxisTupleType : a tuple type of all Axis types
   *   - AxisTupleArithmeticType : a tuple type of all Axis Arithmetic types
   * FIXME : use concepts when available
   */
  template <typename HistogramType,
            typename Seq =
                std::make_integer_sequence<unsigned int, std::tuple_size_v<typename HistogramType::AxisTupleType>>>
  class CustomHistogramWrapperInternal;
  template <typename HistogramType, unsigned int... ND>
  class CustomHistogramWrapperInternal<HistogramType, std::integer_sequence<unsigned int, ND...>> {
  public:
    template <unsigned int I>
    using AxisType = std::tuple_element_t<I, typename HistogramType::AxisTupleType>;

    /// constructor, only creates a set of Properties
    template <typename OWNER>
    CustomHistogramWrapperInternal( OWNER* owner, std::string const& name, std::string const& title = "",
                                    typename HistogramType::AxisTupleType axis = {}, bool doNotInitialize = false )
        : m_name{ name }, m_title{ title }, m_axis{ axis } {
      // Create associated properties
      owner->declareProperty( titlePropertyName(), m_title, fmt::format( "Title of histogram {}", name ) )
          ->template setOwnerType<OWNER>();
      ( owner
            ->declareProperty( axisPropertyName<ND>(), std::get<ND>( m_axis ),
                               fmt::format( "Axis {} of histogram {}", ND, name ) )
            ->template setOwnerType<OWNER>(),
        ... );
      // register creation of the Histogram at initialization time
      if ( !doNotInitialize ) {
        owner->registerCallBack( StateMachine::INITIALIZE, [this, owner]() { createHistogram( *owner ); } );
      }
    }
    /// constructor with more natural syntax for axis
    template <typename OWNER>
    CustomHistogramWrapperInternal( OWNER* owner, std::string const& name, std::string const& title,
                                    AxisType<ND>... allAxis )
        : CustomHistogramWrapperInternal( owner, name, title, std::make_tuple( allAxis... ) ) {}

    /// override of operator[] with extra checking that initialization happened
    [[nodiscard]] auto operator[]( typename HistogramType::AxisTupleArithmeticType&& v ) {
      if ( !m_histo ) {
        throw std::logic_error( fmt::format( "Histogram {} is used before being initialized", m_name ) );
      }
      return m_histo.value()[std::forward<typename HistogramType::AxisTupleArithmeticType>( v )];
    }

    /// creation of the internal histogram, from the properties
    template <typename OWNER>
    void createHistogram( OWNER& owner ) {
      m_histo.emplace( &owner, m_name, m_title, m_axis );
    }

    friend void to_json( nlohmann::json& j, CustomHistogramWrapperInternal const& h ) {
      if ( !h.m_histo ) {
        throw std::logic_error( fmt::format( "Histogram {} is converted to json before being initialized", h.m_name ) );
      }
      j = h.m_histo.value();
    }

    // set directly some properties, only if histogram was not yet created
    void setTitle( std::string const& title ) {
      if ( m_histo )
        throw std::logic_error(
            fmt::format( "Cannot modify title of histogram {} after it has been initialized", m_name ) );
      m_title = title;
    }
    template <unsigned int N>
    void setAxis( std::tuple_element_t<N, typename HistogramType::AxisTupleType> const& axis ) {
      if ( m_histo )
        throw std::logic_error(
            fmt::format( "Cannot modify axis {} of histogram {} after it has been initialized", N, m_name ) );
      std::get<N>( m_axis ) = axis;
    }

    void reset() { m_histo.reset(); }

  private:
    std::string titlePropertyName() const { return fmt::format( "{}_Title", m_name ); }
    template <unsigned int N>
    std::string axisPropertyName() const {
      return fmt::format( "{}_Axis{}", m_name, N );
    }

    // Members of the custom histogrem
    std::string                           m_name{};
    std::string                           m_title{};
    typename HistogramType::AxisTupleType m_axis{};
    std::optional<HistogramType>          m_histo{};
  };

  template <typename HistogramType>
  using CustomHistogramWrapper = CustomHistogramWrapperInternal<HistogramType>;

} // namespace Gaudi::Accumulators
