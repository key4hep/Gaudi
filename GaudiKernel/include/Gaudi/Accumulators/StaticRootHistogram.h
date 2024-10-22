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

#include <Gaudi/Accumulators/StaticHistogram.h>

#include <type_traits>

namespace {
  template <typename tuple_t>
  constexpr auto get_array_from_tuple( tuple_t&& tuple ) {
    constexpr auto get_array = []( auto&&... x ) { return std::array{ std::forward<decltype( x )>( x )... }; };
    return std::apply( get_array, std::forward<tuple_t>( tuple ) );
  }
} // namespace

namespace Gaudi::Accumulators {

  /// number of items in sums for a given dimension
  /// = 1 (nb items) + ND (sums of each dimension) + ND*(ND+1)/2 (square sums)
  constexpr unsigned int NSUMS( unsigned int ND ) { return 1 + ND + ND * ( ND + 1 ) / 2; }

  template <typename Arithmetic, atomicity Atomicity, unsigned int ND>
  struct SigmasValueHandler {
    using InputType  = std::conditional_t<ND == 1, Arithmetic, std::array<Arithmetic, ND>>;
    using OutputType = std::array<Arithmetic, NSUMS( ND )>;
    struct OutputTypeTS : std::array<std::atomic<Arithmetic>, NSUMS( ND )> {
      /// copy constructor from non thread safe type
      using std::array<std::atomic<Arithmetic>, NSUMS( ND )>::array;
      explicit OutputTypeTS( OutputType const& other ) {
        for ( unsigned int i = 0; i < NSUMS( ND ); i++ ) { ( *this )[i] = other[i]; }
      }
      // operator= from non thread safe type
      OutputTypeTS& operator=( OutputType const& other ) {
        for ( unsigned int i = 0; i < NSUMS( ND ); i++ ) { ( *this )[i] = other[i]; }
        return *this;
      }
      // automatic conversion to non thread safe type
      operator OutputType() const {
        OutputType out;
        for ( unsigned int i = 0; i < NSUMS( ND ); i++ ) { out[i] = ( *this )[i].load( std::memory_order_relaxed ); }
        return out;
      }
    };
    using InternalType = std::conditional_t<Atomicity == atomicity::full, OutputTypeTS, OutputType>;
    static constexpr OutputType getValue( const InternalType& v ) noexcept {
      // Note automatic conversion will happen
      return v;
    };
    static OutputType exchange( InternalType& v, OutputType newv ) noexcept {
      if constexpr ( Atomicity == atomicity::full ) {
        OutputType old;
        for ( unsigned int i = 0; i < NSUMS( ND ); i++ ) { old[i] = v[i].exchange( newv[i] ); }
        return old;
      } else {
        return std::exchange( v, newv );
      }
    }
    static constexpr OutputType DefaultValue() { return InternalType{}; }
    static void                 merge( InternalType& a, OutputType b ) noexcept {
      if constexpr ( Atomicity == atomicity::full ) {
        for ( unsigned int i = 0; i < NSUMS( ND ); i++ ) { fetch_add( a[i], b[i] ); }
      } else {
        for ( unsigned int i = 0; i < ND * ( ND + 3 ); i++ ) a[i] += b[i];
      }
    }
    static void merge( InternalType& a, InputType b ) noexcept {
      // prepare values to increment internal values
      OutputType diff{};
      diff[0] = 1.;
      if constexpr ( ND == 1 ) {
        // no operator[] for b in this case
        diff[1] = b;
        diff[2] = b * b;
      } else {
        for ( unsigned int i = 0; i < ND; i++ ) diff[i + 1] += b[i];
        unsigned int n = 1 + ND;
        for ( unsigned int i = 0; i < ND; i++ ) {
          for ( unsigned int j = i; j < ND; j++ ) {
            diff[n] = b[i] * b[j];
            n++;
          }
        }
      }
      // Now increase original counter
      if constexpr ( Atomicity == atomicity::full ) {
        for ( unsigned int i = 0; i < NSUMS( ND ); i++ ) { fetch_add( a[i], diff[i] ); }
      } else {
        for ( unsigned int i = 0; i < NSUMS( ND ); i++ ) a[i] += diff[i];
      }
    }
  };

  template <typename Arithmetic, atomicity Atomicity, unsigned int ND>
  struct SigmaNAccumulator
      : GenericAccumulator<std::array<Arithmetic, ND>, std::array<Arithmetic, NSUMS( ND )>, Atomicity, Identity,
                           Identity, SigmasValueHandler<Arithmetic, Atomicity, ND>> {
    std::array<Arithmetic, NSUMS( ND )> const sums() const { return this->value(); }
  };

  /// specialization for ND=1 to allow for better syntax
  template <typename Arithmetic, atomicity Atomicity>
  struct SigmaNAccumulator<Arithmetic, Atomicity, 1>
      : GenericAccumulator<Arithmetic, std::array<Arithmetic, 3>, Atomicity, Identity, Identity,
                           SigmasValueHandler<Arithmetic, Atomicity, 1>> {
    std::array<Arithmetic, 3> const sums() const { return this->value(); }
  };

  /**
   * Internal Accumulator class dealing with RootHistograming.
   * Actually a simple extention on top of RootHistograming with an
   * extra SigmaCounter embeded
   */
  template <atomicity Atomicity, typename Arithmetic, unsigned int ND, typename AxisTupleType>
  class RootHistogramingAccumulatorInternal
      : public HistogramingAccumulatorInternal<Atomicity, HistoInputType<AxisToArithmetic_t<AxisTupleType>, ND>,
                                               unsigned long, IntegralAccumulator, AxisTupleType> {

    using InputType = HistoInputType<AxisToArithmetic_t<AxisTupleType>, ND>;
    using Parent =
        HistogramingAccumulatorInternal<Atomicity, InputType, unsigned long, IntegralAccumulator, AxisTupleType>;

    template <atomicity, typename, unsigned int, typename>
    friend class RootHistogramingAccumulatorInternal;

    static_assert( ND <= 3, "Root on supports histogrmas with dimension <= 3" );

    /**
     * Small procyclass allowing operator[] to work as expected on the RootHistogram
     * that is to return something having an operator+= updating the histogram properly
     */
    struct Proxy {
      Proxy( RootHistogramingAccumulatorInternal& histo, typename InputType::ValueType& v )
          : m_histo( histo ), m_v( v ) {}
      void                                 operator++() { m_histo.update( m_v ); }
      RootHistogramingAccumulatorInternal& m_histo;
      typename InputType::ValueType        m_v;
    };

  public:
    using Parent::Parent;
    friend struct Proxy;

    [[deprecated( "Use `++h1[x]`, `++h2[{x,y}]`, etc. instead." )]] RootHistogramingAccumulatorInternal&
    operator+=( typename InputType::ValueType v ) {
      update( v );
      return *this;
    }
    void reset() {
      m_accumulator.reset();
      Parent::reset();
    }
    template <atomicity ato>
    void mergeAndReset( RootHistogramingAccumulatorInternal<ato, Arithmetic, ND, AxisTupleType>& other ) {
      m_accumulator.mergeAndReset( other.m_accumulator );
      Parent::mergeAndReset( other );
    }
    [[nodiscard]] auto operator[]( typename InputType::ValueType v ) { return Proxy( *this, v ); }

    /// returns the nbentries, sums and "squared sums" of the inputs
    /// Practically we have first the number of entries, then the simple sums of each
    /// input dimension followed by all combinasions of product of 2 inputs, in "alphabetical" order,
    /// e.g. for ND=3 we have sums of n, x, y, z, x^2, xy, xz, y^2, yz, z^2
    auto sums2() const { return m_accumulator.sums(); }

  private:
    void update( typename InputType::ValueType v ) {
      // Do not accumulate in m_accumulator if we are outside the histo range
      // We mimic here the behavior of ROOT
      if ( v.inAcceptance( this->axis() ) ) {
        if constexpr ( ND == 1 ) {
          m_accumulator += std::get<0>( v );
        } else {
          m_accumulator += get_array_from_tuple( static_cast<typename InputType::InternalType>( v ) );
        }
      }
      ++Parent::operator[]( v );
    }
    // Accumulator for keeping squared sum of value stored in the histogram and correlation values
    // they get stored in "alphabetical" order, e.g. for ND=3 x^2, xy, xz, y^2, yz, z^2
    SigmaNAccumulator<Arithmetic, Atomicity, ND> m_accumulator;
  };

  namespace {
    /// helper function to compute standard_deviation
    template <typename Arithmetic>
    Arithmetic stddev( Arithmetic n, Arithmetic s, Arithmetic s2 ) {
      using Gaudi::Accumulators::sqrt;
      using std::sqrt;
      auto v = ( n > 0 ) ? ( ( s2 - s * ( s / n ) ) / n ) : Arithmetic{};
      return ( Arithmetic{ 0 } > v ) ? Arithmetic{} : sqrt( v );
    }
  } // namespace

  /**
   * Class implementing a root histogram accumulator
   */
  template <atomicity Atomicity, typename Arithmetic, typename ND, typename AxisTupleType>
  struct RootHistogramingAccumulator;

  template <atomicity Atomicity, typename Arithmetic, typename AxisTupleType>
  struct RootHistogramingAccumulator<Atomicity, Arithmetic, std::integral_constant<unsigned int, 1>, AxisTupleType>
      : RootHistogramingAccumulatorInternal<Atomicity, Arithmetic, 1, AxisTupleType> {
    using RootHistogramingAccumulatorInternal<Atomicity, Arithmetic, 1,
                                              AxisTupleType>::RootHistogramingAccumulatorInternal;
    using RootHistogramingAccumulatorInternal<Atomicity, Arithmetic, 1, AxisTupleType>::nEntries;
    Arithmetic nEntries() const { return this->sums2()[0]; }
    Arithmetic sum() const { return this->sums2()[1]; }
    Arithmetic sum2() const { return this->sums2()[2]; }
    Arithmetic mean() const { return sum() / nEntries(); }
    Arithmetic standard_deviation() const { return stddev( nEntries(), sum(), sum2() ); }
  };

  template <atomicity Atomicity, typename Arithmetic, typename AxisTupleType>
  struct RootHistogramingAccumulator<Atomicity, Arithmetic, std::integral_constant<unsigned int, 2>, AxisTupleType>
      : RootHistogramingAccumulatorInternal<Atomicity, Arithmetic, 2, AxisTupleType> {
    using RootHistogramingAccumulatorInternal<Atomicity, Arithmetic, 2,
                                              AxisTupleType>::RootHistogramingAccumulatorInternal;
    using RootHistogramingAccumulatorInternal<Atomicity, Arithmetic, 2, AxisTupleType>::nEntries;
    Arithmetic nEntries() const { return this->sums2()[0]; }
    Arithmetic sumx() const { return this->sums2()[1]; }
    Arithmetic sumy() const { return this->sums2()[2]; }
    Arithmetic sumx2() const { return this->sums2()[3]; }
    Arithmetic sumy2() const { return this->sums2()[5]; }
    Arithmetic sumxy() const { return this->sums2()[4]; }
    Arithmetic meanx() const { return sumx() / nEntries(); }
    Arithmetic meany() const { return sumy() / nEntries(); }
    Arithmetic standard_deviationx() const { return stddev( nEntries(), sumx(), sumx2() ); }
    Arithmetic standard_deviationy() const { return stddev( nEntries(), sumy(), sumy2() ); }
  };

  template <atomicity Atomicity, typename Arithmetic, typename AxisTupleType>
  struct RootHistogramingAccumulator<Atomicity, Arithmetic, std::integral_constant<unsigned int, 3>, AxisTupleType>
      : RootHistogramingAccumulatorInternal<Atomicity, Arithmetic, 3, AxisTupleType> {
    using RootHistogramingAccumulatorInternal<Atomicity, Arithmetic, 3,
                                              AxisTupleType>::RootHistogramingAccumulatorInternal;
    using RootHistogramingAccumulatorInternal<Atomicity, Arithmetic, 3, AxisTupleType>::nEntries;
    Arithmetic nEntries() const { return this->sums2()[0]; }
    Arithmetic sumx() const { return this->sums2()[1]; }
    Arithmetic sumy() const { return this->sums2()[2]; }
    Arithmetic sumz() const { return this->sums2()[3]; }
    Arithmetic sumx2() const { return this->sums2()[4]; }
    Arithmetic sumy2() const { return this->sums2()[7]; }
    Arithmetic sumz2() const { return this->sums2()[9]; }
    Arithmetic sumxy() const { return this->sums2()[5]; }
    Arithmetic sumxz() const { return this->sums2()[6]; }
    Arithmetic sumyz() const { return this->sums2()[8]; }
    Arithmetic meanx() const { return sumx() / nEntries(); }
    Arithmetic meany() const { return sumy() / nEntries(); }
    Arithmetic meanz() const { return sumz() / nEntries(); }
    Arithmetic standard_deviationx() const { return stddev( nEntries(), sumx(), sumx2() ); }
    Arithmetic standard_deviationy() const { return stddev( nEntries(), sumy(), sumy2() ); }
    Arithmetic standard_deviationz() const { return stddev( nEntries(), sumz(), sumz2() ); }
  };

  /**
   * Extension of the standard Gaudi histogram to provide similar functionnality as ROOT
   *
   * The main piece of extra functionnality is that ablity to compute a mean and sigma
   * of the data set hold by the histogram on top of the histogram itself
   * This will then be used when filling a real Root histogram so that one gets the
   * same values as if working directly with Root.
   * When using pure Gaudi Histograms, and converting to ROOT, ROOT automatically
   * recomputed average and sigma form the bins, but the value is not the expected one
   *
   * Usage is similar to HistogramingCounterBase so see the documentation there
   * Serialization has the following extra fields :
   *   - nTotEntries, sum, sum2, mean in 1D
   *   - nTotEntries, sumx, sumy, sumx2, sumy2, sumxy, meanx, meany in 2D
   *   - nTotEntries, sumx, sumy, sumz, sumx2, sumy2, sumz2, sumxy, sumxz, sumyz, meanx, meany, meanz in 3D
   * and uses same types as HistogramingCounterBase
   *
   */
  template <unsigned int ND, atomicity Atomicity, typename Arithmetic, const char* Type, typename AxisTupleType>
  class RootHistogramingCounterBase;

  template <atomicity Atomicity, typename Arithmetic, const char* Type, typename AxisTupleType>
  class RootHistogramingCounterBase<1, Atomicity, Arithmetic, Type, AxisTupleType>
      : public HistogramingCounterBase<1, Atomicity, Arithmetic, Type, RootHistogramingAccumulator, AxisTupleType> {
  public:
    using Parent = HistogramingCounterBase<1, Atomicity, Arithmetic, Type, RootHistogramingAccumulator, AxisTupleType>;
    using Parent::Parent;

    friend void to_json( nlohmann::json&                                                                   j,
                         RootHistogramingCounterBase<1, Atomicity, Arithmetic, Type, AxisTupleType> const& h ) {
      to_json( j, static_cast<Parent const&>( h ) );
      j["nTotEntries"]        = h.nEntries();
      j["sum"]                = h.sum();
      j["mean"]               = h.mean();
      j["sum2"]               = h.sum2();
      j["standard_deviation"] = h.standard_deviation();
    }
  };

  template <atomicity Atomicity, typename Arithmetic, const char* Type, typename AxisTupleType>
  class RootHistogramingCounterBase<2, Atomicity, Arithmetic, Type, AxisTupleType>
      : public HistogramingCounterBase<2, Atomicity, Arithmetic, Type, RootHistogramingAccumulator, AxisTupleType> {
  public:
    using Parent = HistogramingCounterBase<2, Atomicity, Arithmetic, Type, RootHistogramingAccumulator, AxisTupleType>;
    using Parent::Parent;

    friend void to_json( nlohmann::json&                                                                   j,
                         RootHistogramingCounterBase<2, Atomicity, Arithmetic, Type, AxisTupleType> const& h ) {
      to_json( j, static_cast<Parent const&>( h ) );
      j["nTotEntries"]         = h.nEntries();
      j["sumx"]                = h.sumx();
      j["sumy"]                = h.sumy();
      j["meanx"]               = h.meanx();
      j["meany"]               = h.meany();
      j["sumx2"]               = h.sumx2();
      j["sumy2"]               = h.sumy2();
      j["sumxy"]               = h.sumxy();
      j["standard_deviationx"] = h.standard_deviationx();
      j["standard_deviationy"] = h.standard_deviationy();
    }
  };

  template <atomicity Atomicity, typename Arithmetic, const char* Type, typename AxisTupleType>
  class RootHistogramingCounterBase<3, Atomicity, Arithmetic, Type, AxisTupleType>
      : public HistogramingCounterBase<3, Atomicity, Arithmetic, Type, RootHistogramingAccumulator, AxisTupleType> {
  public:
    using Parent = HistogramingCounterBase<3, Atomicity, Arithmetic, Type, RootHistogramingAccumulator, AxisTupleType>;
    using Parent::Parent;

    friend void to_json( nlohmann::json&                                                                   j,
                         RootHistogramingCounterBase<3, Atomicity, Arithmetic, Type, AxisTupleType> const& h ) {
      to_json( j, static_cast<Parent const&>( h ) );
      j["nTotEntries"]         = h.nEntries();
      j["sumx"]                = h.sumx();
      j["sumy"]                = h.sumy();
      j["sumz"]                = h.sumz();
      j["meanx"]               = h.meanx();
      j["meany"]               = h.meany();
      j["meanz"]               = h.meanz();
      j["sumx2"]               = h.sumx2();
      j["sumy2"]               = h.sumy2();
      j["sumz2"]               = h.sumz2();
      j["sumxy"]               = h.sumxy();
      j["sumxz"]               = h.sumxz();
      j["sumyz"]               = h.sumyz();
      j["standard_deviationx"] = h.standard_deviationx();
      j["standard_deviationy"] = h.standard_deviationy();
      j["standard_deviationz"] = h.standard_deviationz();
    }
  };

  /// Root histograming counter. See RootHistogramingCounterBase for details
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double,
            typename AxisTupleType = make_tuple_t<Axis<Arithmetic>, ND>>
  using StaticRootHistogram =
      RootHistogramingCounterBase<ND, Atomicity, Arithmetic, naming::histogramString, AxisTupleType>;

} // namespace Gaudi::Accumulators
