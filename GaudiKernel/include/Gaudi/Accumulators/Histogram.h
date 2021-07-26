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
#pragma once

#include <Gaudi/Accumulators.h>
#include <Gaudi/MonitoringHub.h>
#include <ROOT/RHist.hxx>

#include <boost/format.hpp>
#include <boost/histogram/algorithm/sum.hpp>
#include <boost/histogram/fwd.hpp>
#include <gsl/span>

#include <nlohmann/json.hpp>

#include <array>
#include <cmath>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace Gaudi::Accumulators {

  /**
   * A functor to extract weight, take a pair (valueTuple, weight) as input
   */
  struct ExtractWeight {
    template <typename Arithmetic>
    constexpr decltype( auto ) operator()( const std::pair<Arithmetic, Arithmetic>& v ) const noexcept {
      return v.second;
    }
  };

  /**
   * A Product functor, take a pair (value, weight) as input
   */
  struct WeightedProduct {
    template <typename Arithmetic>
    constexpr decltype( auto ) operator()( const std::pair<Arithmetic, Arithmetic>& v ) const noexcept {
      return v.first * v.second;
    }
  };

  /**
   * A WeightedSquare functor, take a pair (value, weight) as input
   */
  struct WeightedSquare {
    template <typename Arithmetic>
    constexpr decltype( auto ) operator()( const std::pair<Arithmetic, Arithmetic>& v ) const noexcept {
      return v.first * v.first * v.second;
    }
  };

  /**
   * WeightedCountAccumulator. A WeightedCountAccumulator is an Accumulator storing the number of provided values,
   * weighted. It basically sums the weights and thus is similar to the SumAccumulator except that it takes
   * a pair (valueTuple, weight) as input
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic>
  struct WeightedCountAccumulator
      : GenericAccumulator<std::pair<Arithmetic, Arithmetic>, Arithmetic, Atomicity, ExtractWeight> {
    using Base = GenericAccumulator<std::pair<Arithmetic, Arithmetic>, Arithmetic, Atomicity, ExtractWeight>;
    using Base::Base;
    using Base::operator+=;
    //// overload of operator+= to be able to only give weight and no value
    WeightedCountAccumulator operator+=( const Arithmetic weight ) {
      *this += {Arithmetic{}, weight};
      return *this;
    }
    Arithmetic nEntries() const { return this->value(); }
  };

  /**
   * WeightedSumAccumulator. A WeightedSumAccumulator is an Accumulator storing a weighted sum of values.
   * It takes a pair (valueTuple, weight) and basically sums the product of the last item othe 2 part of its in put pair
   * : weight and value
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic>
  struct WeightedSumAccumulator
      : GenericAccumulator<std::pair<Arithmetic, Arithmetic>, Arithmetic, Atomicity, WeightedProduct> {
    using GenericAccumulator<std::pair<Arithmetic, Arithmetic>, Arithmetic, Atomicity,
                             WeightedProduct>::GenericAccumulator;
    Arithmetic sum() const { return this->value(); }
  };

  /**
   * WeightedSquareAccumulator. A WeightedSquareAccumulator is an Accumulator storing a weighted sum of squared values.
   * It basically takes a pair (value, weight) as input and sums weight*value*value
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic = double>
  struct WeightedSquareAccumulator
      : GenericAccumulator<std::pair<Arithmetic, Arithmetic>, Arithmetic, Atomicity, WeightedSquare> {
    using GenericAccumulator<std::pair<Arithmetic, Arithmetic>, Arithmetic, Atomicity,
                             WeightedSquare>::GenericAccumulator;
    Arithmetic sum2() const { return this->value(); };
  };

  /**
   * WeightedAveragingAccumulator. An AveragingAccumulator is an Accumulator able to compute an average
   * This implementation takes a pair (value, weight) as input
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic>
  using WeightedAveragingAccumulator =
      AveragingAccumulatorBase<Atomicity, Arithmetic, WeightedCountAccumulator, WeightedSumAccumulator>;

  /**
   * WeightedSigmaAccumulator. A SigmaAccumulator is an Accumulator able to compute an average and variance/rms
   * This implementation takes a pair (value, weight) as input
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic>
  using WeightedSigmaAccumulator =
      SigmaAccumulatorBase<Atomicity, Arithmetic, WeightedAveragingAccumulator, WeightedSquareAccumulator>;

  /**
   * Definition of an Histogram Axis
   */
  template <typename Arithmetic>
  struct Axis {
    Axis( unsigned int _nBins, Arithmetic _minValue, Arithmetic _maxValue, std::string _title = {},
          std::vector<std::string> _labels = {} )
        : nBins( _nBins )
        , minValue( _minValue )
        , maxValue( _maxValue )
        , title( std::move( _title ) )
        , labels( std::move( _labels ) )
        , ratio( _nBins / ( _maxValue - _minValue ) ){};
    /// number of bins for this Axis
    unsigned int nBins;
    /// min and max values on this axis
    Arithmetic minValue, maxValue;
    /// title of this axis
    std::string title;
    /// labels for the bins
    std::vector<std::string> labels;
    /**
     * precomputed ratio to convert a value into bin number
     * equal to nBins/(maxValue-minValue)
     */
    Arithmetic ratio;
  };

  /// automatic conversion of the Axis type to json
  template <typename Arithmetic>
  void to_json( nlohmann::json& j, const Axis<Arithmetic>& axis ) {
    j = nlohmann::json{
        {"nBins", axis.nBins}, {"minValue", axis.minValue}, {"maxValue", axis.maxValue}, {"title", axis.title}};
    if ( !axis.labels.empty() ) { j["labels"] = axis.labels; }
  }

  /// small class used as InputType for regular Histograms
  template <typename Arithmetic, unsigned int ND, unsigned int NIndex = ND>
  struct HistoInputType : std::array<Arithmetic, ND> {
    // allow construction from set of values
    template <class... ARGS>
    HistoInputType( ARGS... args ) : std::array<Arithmetic, ND>{static_cast<Arithmetic>( args )...} {}
    // The change on NIndex == 1 allow to have simpler syntax in that case, that is no tuple of one item
    using ValueType          = HistoInputType<Arithmetic, NIndex == 1 ? 1 : ND, NIndex>;
    using AxisArithmeticType = Arithmetic;
    unsigned int computeIndex( const std::array<Axis<Arithmetic>, NIndex>& axis ) const {
      unsigned int index = 0;
      for ( unsigned int dim = 0; dim < NIndex; dim++ ) {
        // compute local index for a given dimension
        int localIndex = std::floor( ( ( *this )[dim] - axis[dim].minValue ) * axis[dim].ratio ) + 1;
        localIndex     = ( localIndex < 0 )
                         ? 0
                         : ( ( (unsigned int)localIndex > axis[dim].nBins ) ? axis[dim].nBins + 1
                                                                            : (unsigned int)( localIndex ) );
        // compute global index. Bins are stored in a row first manner
        index = ( dim > 0 ? ( axis[dim - 1].nBins + 2 ) : 0 ) * index + localIndex;
      }
      return index;
    }
    auto forInternalCounter() { return this->operator[]( ND - 1 ); }
  };

  /// specialization of HistoInputType for ND == 1 in order to have simpler syntax
  /// that is, no tuple of one item
  template <typename Arithmetic>
  class HistoInputType<Arithmetic, 1> {
  public:
    using ValueType          = HistoInputType;
    using AxisArithmeticType = Arithmetic;
    HistoInputType( Arithmetic a ) : value( a ) {}
    unsigned int computeIndex( const std::array<Axis<Arithmetic>, 1>& axis ) const {
      int index = std::floor( ( value - axis[0].minValue ) * axis[0].ratio ) + 1;
      return index < 0 ? 0 : ( (unsigned int)index > axis[0].nBins ? axis[0].nBins + 1 : (unsigned int)index );
    }
    Arithmetic& operator[]( int ) { return value; }
                operator Arithmetic() const { return value; }
    auto        forInternalCounter() { return value; }

  private:
    Arithmetic value;
  };

  /// small class used as InputType for weighted Histograms
  template <typename Arithmetic, unsigned int ND, unsigned int NIndex = ND>
  struct WeightedHistoInputType : std::pair<HistoInputType<Arithmetic, ND, NIndex>, Arithmetic> {
    // The change on NIndex == 1 allow to have simpler syntax in that case, that is no tuple of one item
    using ValueType          = HistoInputType<Arithmetic, NIndex == 1 ? 1 : ND, NIndex>;
    using AxisArithmeticType = Arithmetic;
    using std::pair<HistoInputType<Arithmetic, ND, NIndex>, Arithmetic>::pair;
    unsigned int computeIndex( const std::array<Axis<Arithmetic>, NIndex>& axis ) const {
      return this->first.computeIndex( axis );
    }
    auto forInternalCounter() { return std::pair( this->first.forInternalCounter(), this->second ); }
  };

  /**
   * Internal Accumulator class dealing with Histograming. Templates parameters are :
   *  - Atomicity : none or full
   *  - Arithmetic : the arithmetic type used for values filled into the histogram
   *  - ND : the number of dimensions of the histogram.
   *    Note that ND is given as an integral_constant as it needs to be a type for the internal template
   *    business of the Counter (more precisely the Buffer class)
   *  - InputType : the type given as input of the Histogram. Typically (Weighted)HistoInputType
   *  - BaseAccumulator : the underlying accumulator used in each bin
   *
   * This accumulator is simply an array of BaseAccumulator, one per bin. The number of bins is
   * the product of the number of bins for each dimension, each of them increased by 2 for storing
   * the values under min and above max
   */
  template <atomicity Atomicity, typename InputType, typename Arithmetic, typename ND,
            template <atomicity Ato, typename Arith> typename BaseAccumulatorT>
  class HistogramingAccumulatorInternal {
    template <atomicity, typename, typename, typename, template <atomicity, typename> typename>
    friend class HistogramingAccumulatorInternal;

  public:
    using BaseAccumulator    = BaseAccumulatorT<Atomicity, Arithmetic>;
    using AxisArithmeticType = typename InputType::AxisArithmeticType;
    template <std::size_t... Is>
    HistogramingAccumulatorInternal( std::initializer_list<Axis<AxisArithmeticType>> axis, std::index_sequence<Is...> )
        : m_axis{{*( axis.begin() + Is )...}}
        , m_totNBins{computeTotNBins()}
        , m_value( new BaseAccumulator[m_totNBins] ) {
      reset();
    }
    template <atomicity ato>
    HistogramingAccumulatorInternal(
        construct_empty_t,
        const HistogramingAccumulatorInternal<ato, InputType, Arithmetic, ND, BaseAccumulatorT>& other )
        : m_axis( other.m_axis ), m_totNBins{computeTotNBins()}, m_value( new BaseAccumulator[m_totNBins] ) {
      reset();
    }
    HistogramingAccumulatorInternal& operator+=( InputType v ) {
      accumulator( v.computeIndex( m_axis ) ) += v.forInternalCounter();
      return *this;
    }
    void reset() {
      for ( unsigned int index = 0; index < m_totNBins; index++ ) accumulator( index ).reset();
    }
    template <atomicity ato>
    void mergeAndReset( HistogramingAccumulatorInternal<ato, InputType, Arithmetic, ND, BaseAccumulatorT>&& other ) {
      assert( m_totNBins == other.m_totNBins );
      for ( unsigned int index = 0; index < m_totNBins; index++ ) {
        accumulator( index ).mergeAndReset( std::move( other.accumulator( index ) ) );
      }
    }
    auto operator[]( typename InputType::ValueType v ) {
      return Buffer<BaseAccumulatorT, Atomicity, Arithmetic>{accumulator( v.computeIndex( m_axis ) )};
    }

  protected:
    auto& axis() const { return m_axis; }
    auto  nBins( unsigned int i ) const { return m_axis[i].nBins; }
    auto  minValue( unsigned int i ) const { return m_axis[i].minValue; }
    auto  maxValue( unsigned int i ) const { return m_axis[i].maxValue; }
    auto  ratio( unsigned int i ) const { return m_axis[i].ratio; }
    auto  binValue( unsigned int i ) const { return accumulator( i ).value(); }
    auto  nEntries( unsigned int i ) const { return accumulator( i ).nEntries(); }
    auto  totNBins() const { return m_totNBins; }

  private:
    BaseAccumulator& accumulator( unsigned int index ) const {
      assert( index < m_totNBins );
      return m_value[index];
    }
    unsigned int computeTotNBins() const {
      unsigned int nTotBins = 1;
      for ( unsigned int i = 0; i < ND::value; i++ ) { nTotBins *= ( m_axis[i].nBins + 2 ); }
      return nTotBins;
    }
    /// set of Axis of this Histogram
    std::array<Axis<AxisArithmeticType>, ND::value> m_axis;
    /// total number of bins in this histogram, under and overflow included
    unsigned int m_totNBins;
    /// Histogram content
    std::unique_ptr<BaseAccumulator[]> m_value;
  };

  /**
   * Class implementing a regular histogram accumulator
   *
   * Actually only an alias to HistogramingAccumulatorInternal with proper template parameters
   */
  template <atomicity Atomicity, typename Arithmetic, typename ND>
  using HistogramingAccumulator = HistogramingAccumulatorInternal<Atomicity, HistoInputType<Arithmetic, ND::value>,
                                                                  unsigned long, ND, IntegralAccumulator>;

  /**
   * Class implementing a weighted histogram accumulator
   *
   * Actually only an alias to HistogramingAccumulatorInternal with proper template parameters
   */
  template <atomicity Atomicity, typename Arithmetic, typename ND>
  using WeightedHistogramingAccumulator =
      HistogramingAccumulatorInternal<Atomicity, WeightedHistoInputType<Arithmetic, ND::value>, Arithmetic, ND,
                                      WeightedCountAccumulator>;

  /**
   * Class implementing a profile histogram accumulator
   *
   * Actually only an alias to HistogramingAccumulatorInternal with proper template parameters
   */
  template <atomicity Atomicity, typename Arithmetic, typename ND>
  using ProfileHistogramingAccumulator =
      HistogramingAccumulatorInternal<Atomicity, HistoInputType<Arithmetic, ND::value + 1, ND::value>, Arithmetic, ND,
                                      SigmaAccumulator>;

  /**
   * Class implementing a weighted profile histogram accumulator
   *
   * Actually only an alias to HistogramingAccumulatorInternal with proper template parameters
   */
  template <atomicity Atomicity, typename Arithmetic, typename ND>
  using WeightedProfileHistogramingAccumulator =
      HistogramingAccumulatorInternal<Atomicity, WeightedHistoInputType<Arithmetic, ND::value + 1, ND::value>,
                                      Arithmetic, ND, WeightedSigmaAccumulator>;

  /**
   * A base counter dealing with Histograms
   *
   * Main features of that Counter :
   *  - can be any number of dimensions. The dimension is its first template parameter
   *  - for each dimension, a triplet of values have to be given at
   *    construction : nbins, minValue, maxValue. These triplets have to be
   *    embedded into braces, as the constructor takes an array of them
   *  - the operator+= takes either an array of values (one per dimension)
   *    or a tuple<array of values, weight>. The value inside the bin
   *    corresponding to the given values is then increased by 1 or weight
   *  - the prefered syntax is to avoid operator+= and use operator[] to get a
   *    buffer on the bin you're updating. Syntax becomes :
   *        ++counter[{x,y}]   or   wcounter[{x,y]] += w
   *  - the Counter is templated by the types of the values given to
   *    operator+ and also by the type stored into the bins
   *  - the counter can be atomic or not and supports buffering. Note that
   *    the atomicity is classical eventual consistency. So each bin is
   *    atomically updated but bins are not garanted to be coherent when
   *    reading all of them back
   *  - profile histograms are also supported, operator+= takes one more
   *    value in the array of values in that case
   *
   * This base class is then aliases for the 4 standard cases of Histogram,
   * WeightedHistogram, ProfileHistogram and WeightedProfileHistogram
   *
   * Typical usage :
   * \code
   * Histogram<2, double, atomicity::full>
   *   counter{owner, "CounterName", "HistoTitle", {{nBins1, minVal1, maxVal1}, {nBins2, minVal2, maxVal2}}};
   * ++counter[{val1, val2}];    // prefered syntax
   * counter += {val1, val2};    // original syntax inherited from counters
   *
   * WeightedHistogram<2, double, atomicity::full>
   *   wcounter{owner, "CounterName", "HistoTitle", {{nBins1, minVal1, maxVal1}, {nBins2, minVal2, maxVal2}}};
   * wcounter[{val1, val2}] += w;    // prefered syntax
   * wcounter += {{val1, val2}, w};  // original syntax inherited from counters
   * \endcode
   *
   * When serialized to json, this counter uses new types histogram:Histogram:<prec>, histogram:ProfileHistogram:<prec>,
   * histogram:WeightedHistogram:<prec> and histrogram:WeightedProfileHistogram:<prec>
   * <prec> id described in Accumulators.h and decribes here the precision of the bin data.
   * All these types have the same fields, namely :
   *   dimension(integer), title(string), empty(bool), nEntries(integer), axis(array), bins(array)
   * where :
   *     + axis is an array of tuples, one per dimension, with content (nBins(integer), minValue(number),
   * maxValue(number), title(string))
   *     + bins is an array of values. The length of the array is the product of (nBins+2) for all axis
   *       the +2 is because the bin 0 is the one for values below minValue and bin nBins+1 is the one for values above
   * maxValue bins are stored row first, so we iterate first on highest dimension For each bin the value is either a
   * number (for non profile histograms) or a triplet (for profile histograms) containing (nEntries(integer),
   * sum(number), sum2(number))
   */
  template <unsigned int ND, atomicity Atomicity, typename Arithmetic, const char* Type,
            template <atomicity, typename, typename> typename Accumulator>
  class HistogramingCounterBase
      : public BufferableCounter<Atomicity, Accumulator, Arithmetic, std::integral_constant<int, ND>> {
  public:
    using Parent = BufferableCounter<Atomicity, Accumulator, Arithmetic, std::integral_constant<int, ND>>;
    template <typename OWNER>
    HistogramingCounterBase( OWNER* owner, std::string const& name, std::string const& title,
                             std::initializer_list<Axis<Arithmetic>> axis )
        : Parent( owner, name, std::string( Type ) + ":" + typeid( Arithmetic ).name(), axis,
                  std::make_index_sequence<ND>{} )
        , m_title( title ) {}
    template <typename OWNER>
    HistogramingCounterBase( OWNER* owner, std::string const& name, std::string const& title, Axis<Arithmetic> axis )
        : HistogramingCounterBase( owner, name, title, {axis} ) {}
    using Parent::print;
    template <typename stream>
    stream& printImpl( stream& o, bool /*tableFormat*/ ) const {
      o << ND << "D Histogram with config ";
      for ( unsigned int i = 0; i < ND; i++ ) {
        o << this->nBins( i ) << " " << this->minValue( i ) << " " << this->maxValue( i ) << " ";
      }
      return o;
    }
    std::ostream& print( std::ostream& o, bool tableFormat = false ) const override {
      return printImpl( o, tableFormat );
    }
    MsgStream& print( MsgStream& o, bool tableFormat = false ) const override { return printImpl( o, tableFormat ); }
    nlohmann::json toJSON() const override {
      // get all bin values and compute total nbEntries
      using Acc = Accumulator<Atomicity, Arithmetic, std::integral_constant<int, ND>>;
      std::vector<typename Acc::BaseAccumulator::OutputType> bins;
      bins.reserve( this->totNBins() );
      unsigned int totNEntries{0};
      for ( unsigned int i = 0; i < this->totNBins(); i++ ) {
        bins.push_back( this->binValue( i ) );
        totNEntries += this->nEntries( i );
      }
      // build json
      return {{"type", std::string( Type ) + ":" + typeid( Arithmetic ).name()},
              {"title", m_title},
              {"dimension", ND},
              {"empty", totNEntries == Arithmetic{}},
              {"nEntries", totNEntries},
              {"axis", this->axis()},
              {"bins", bins}};
    }

  private:
    std::string const m_title;
  };

  namespace {
    static const char histogramString[]                = "histogram:Histogram";
    static const char weightedHistogramString[]        = "histogram:WeightedHistogram";
    static const char profilehistogramString[]         = "histogram:ProfileHistogram";
    static const char weightedProfilehistogramString[] = "histogram:WeightedProfileHistogram";
  } // namespace
  /// standard histograming counter. See HistogramingCounterBase for details
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double>
  using Histogram = HistogramingCounterBase<ND, Atomicity, Arithmetic, histogramString, HistogramingAccumulator>;

  /// standard histograming counter with weight. See HistogramingCounterBase for details
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double>
  using WeightedHistogram =
      HistogramingCounterBase<ND, Atomicity, Arithmetic, weightedHistogramString, WeightedHistogramingAccumulator>;

  /// profile histograming counter. See HistogramingCounterBase for details
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double>
  using ProfileHistogram =
      HistogramingCounterBase<ND, Atomicity, Arithmetic, profilehistogramString, ProfileHistogramingAccumulator>;

  /// weighted profile histograming counter. See HistogramingCounterBase for details
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double>
  using WeightedProfileHistogram = HistogramingCounterBase<ND, Atomicity, Arithmetic, weightedProfilehistogramString,
                                                           WeightedProfileHistogramingAccumulator>;

} // namespace Gaudi::Accumulators
