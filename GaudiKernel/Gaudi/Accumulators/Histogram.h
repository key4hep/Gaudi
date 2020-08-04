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

#include <cmath>
#include <type_traits>
#include <utility>
#include <array>

namespace Gaudi::Accumulators {

  namespace Details {
    
    /// Helper to provide uniform interface to different histograms implementations, at the moment ROOT7 and Boost.
    template <typename HistogramType>
    struct HistogramAdapter {};

    /// Adapter for ROOT7 histograms.
    template <int DIMENSIONS, class PRECISION, template <int D_, class P_> class... STAT>
    struct HistogramAdapter<ROOT::Experimental::RHist<DIMENSIONS, PRECISION, STAT...>> {
      using histogram_t = ROOT::Experimental::RHist<DIMENSIONS, PRECISION, STAT...>;
      using increment_t = typename histogram_t::CoordArray_t;

      HistogramAdapter( histogram_t&& h ) : hist{std::move( h )} {}
      histogram_t hist;

      /// @{
      /// Wrapper for filling the histogram.
      template <class... Ts>
      auto operator()( const Ts&... ts ) {
        return hist.Fill( ts... );
      }
      template <class... Ts>
      auto fill( const Ts&... ts ) {
        return hist.Fill( ts... );
      }
      auto operator+=( const increment_t& x ) { return hist.Fill( x ); }
      /// @}

      /// Basic formatting showing only total count.
      template <typename stream>
      stream& printImpl( stream& o, bool tableFormat ) const {
        auto fmt = boost::format{tableFormat ? "H%|10d| |" : "#=%|-7lu|"};
        return o << fmt % hist.GetEntries();
      }
      /// Histogram not empty.
      operator bool() const { return hist.GetEntries(); }

      /// get number of entries
      unsigned int nEntries() const { return hist.GetEntries(); }
    };

    /// Adapter for Boost histograms.
    template <class Axes, class Storage>
    struct HistogramAdapter<boost::histogram::histogram<Axes, Storage>> {
      using histogram_t = boost::histogram::histogram<Axes, Storage>;

      HistogramAdapter( histogram_t&& h ) : hist{std::move( h )} {}
      histogram_t hist;

      /// @{
      /// Wrapper for filling the histogram.
      template <class... Ts>
      auto operator()( const Ts&... ts ) {
        filled = true;
        return hist( ts... );
      }
      template <class... Ts>
      auto fill( const Ts&... ts ) {
        filled = true;
        return hist( ts... );
      }
      template <class T>
      auto operator+=( const T& x ) {
        filled = true;
        return hist( x );
      }
      /// @}

      /// Basic formatting showing only total count.
      template <typename stream>
      stream& printImpl( stream& o, bool tableFormat ) const {
        auto fmt = boost::format{tableFormat ? "H%|10d| |" : "#=%|-7lu|"};
        return o << fmt % boost::histogram::algorithm::sum( hist );
      }
      /// Histogram not empty.
      operator bool() const { return filled; }

      /// get number of entries
      unsigned int nEntries() const { return boost::histogram::algorithm::sum( hist ); }

    private:
      // flag to record if anything was added to the histogram (information not available in Boost histograms)
      bool filled = false;
    };
  } // namespace Details

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
  struct WeightedCountAccumulator : GenericAccumulator<std::pair<Arithmetic, Arithmetic>, Arithmetic, Atomicity, ExtractWeight> {
    using GenericAccumulator<std::pair<Arithmetic, Arithmetic>, Arithmetic, Atomicity, ExtractWeight>::GenericAccumulator;
    Arithmetic nEntries() const { return this->value(); }
  };

  /**
   * WeightedSumAccumulator. A WeightedSumAccumulator is an Accumulator storing a weighted sum of values.
   * It takes a pair (valueTuple, weight) and basically sums the product of the last item othe 2 part of its in put pair : weight and value
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic>
  struct WeightedSumAccumulator : GenericAccumulator<std::pair<Arithmetic, Arithmetic>, Arithmetic, Atomicity, WeightedProduct> {
    using GenericAccumulator<std::pair<Arithmetic, Arithmetic>, Arithmetic, Atomicity, WeightedProduct>::GenericAccumulator;
    Arithmetic sum() const { return this->value(); }
  };

  /**
   * WeightedSquareAccumulator. A WeightedSquareAccumulator is an Accumulator storing a weighted sum of squared values.
   * It basically takes a pair (weight, value) as input and sums weight*value*value
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic = double>
  struct WeightedSquareAccumulator : GenericAccumulator<std::pair<Arithmetic, Arithmetic>, Arithmetic, Atomicity, WeightedSquare> {
    using GenericAccumulator<std::pair<Arithmetic, Arithmetic>, Arithmetic, Atomicity, WeightedSquare>::GenericAccumulator;
    Arithmetic sum2() const { return this->value(); };
  };

  /**
   * WeightedAveragingAccumulator. An AveragingAccumulator is an Accumulator able to compute an average
   * This implementation takes a pair (weight, value) as input
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic>
  using WeightedAveragingAccumulator = AveragingAccumulatorBase<Atomicity, Arithmetic, WeightedCountAccumulator, WeightedSumAccumulator>;

  /**
   * WeightedSigmaAccumulator. A SigmaAccumulator is an Accumulator able to compute an average and variance/rms
   * This implementation takes a pair (weight, value) as input
   * @see Gaudi::Accumulators for detailed documentation
   */
  template <atomicity Atomicity, typename Arithmetic>
  using WeightedSigmaAccumulator = SigmaAccumulatorBase<Atomicity, Arithmetic, WeightedAveragingAccumulator, WeightedSquareAccumulator>;

  /**
   * Definition of an Histogram Axis
   */
  template <typename Arithmetic>
  struct Axis {
    Axis( unsigned int _nBins, Arithmetic _minValue, Arithmetic _maxValue )
        : nBins( _nBins ), minValue( _minValue ), maxValue( _maxValue ), ratio( _nBins / ( _maxValue - _minValue ) ){};
    /// number of bins for this Axis
    unsigned int nBins;
    /// min and max values on this axis
    Arithmetic minValue, maxValue;
    /**
     * precomputed ratio to convert a value into bin number
     * equal to nBins/(maxValue-minValue)
     */
    Arithmetic ratio;
  };
  
  /// automatic conversion of the Axis type to json
  template <typename Arithmetic>
  void to_json(nlohmann::json& j, const Axis<Arithmetic>& axis) {
    j = nlohmann::json{{"nBins", axis.nBins}, {"minValue", axis.minValue}, {"maxValue", axis.maxValue}};
  }

  /// small class used as InputType for regular Histograms
  template <typename Arithmetic, unsigned int ND, unsigned int NIndex = ND>
  struct HistoInputType : std::array<Arithmetic, ND> {
    unsigned int computeIndex( const std::array<Axis<Arithmetic>, NIndex>& axis ) const {
      unsigned int index = 0;
      for ( unsigned int dim = 0; dim < NIndex; dim++ ) {
        // compute local index for a given dimension
        int localIndex = std::floor( ( ( *this )[dim] - axis[dim].minValue ) * axis[dim].ratio ) + 1;
        localIndex     = ( localIndex < 0 )
                         ? 0
                         : ( ( (unsigned int)localIndex >= axis[dim].nBins ) ? axis[dim].nBins + 1
                             : (unsigned int)( localIndex + 1 ) );
        // compute global index. Bins are stored in a row first manner
        index = ( dim > 0 ? ( axis[dim-1].nBins + 2 ) : 0 ) * index + localIndex;
      }
      return index;
    }
    auto forInternalCounter() { return this->operator[](ND-1); }
  };

  /// specialization of HistoInputType for ND == 1 in order to have simpler syntax
  /// that is, no tuple of one item
  template <typename Arithmetic>
  class HistoInputType<Arithmetic, 1> {
  public:
    HistoInputType(Arithmetic a) : value(a) {}
    unsigned int computeIndex( const std::array<Axis<Arithmetic>, 1>& axis ) const {
      int index = std::floor( ( value - axis[0].minValue ) * axis[0].ratio ) + 1;
      return index < 0 ? 0 : ( (unsigned int)index > axis[0].nBins ? axis[0].nBins + 1 : (unsigned int)index );
    }
    Arithmetic& operator[](int index) { return value; }
    operator Arithmetic() const { return value; }
    auto forInternalCounter() { return value; }
  private:
    Arithmetic value;
  };

  /// small class used as InputType for weighted Histograms
  template <typename Arithmetic, unsigned int ND, unsigned int NIndex = ND>
  struct WeightedHistoInputType : std::pair<HistoInputType<Arithmetic, ND, NIndex>, Arithmetic> {
    using std::pair<HistoInputType<Arithmetic, ND, NIndex>, Arithmetic>::pair;
    unsigned int computeIndex( const std::array<Axis<Arithmetic>, NIndex>& axis ) const {
      return this->first.computeIndex( axis );
    }
    auto forInternalCounter() { return std::pair(this->first.forInternalCounter(), this->second); }
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
    using BaseAccumulator = BaseAccumulatorT<Atomicity, Arithmetic>;
    template <std::size_t... Is>
    HistogramingAccumulatorInternal( std::initializer_list<Axis<Arithmetic>> axis, std::index_sequence<Is...> ) :
      m_axis{{ *(axis.begin() + Is)... }}, m_totNBins{computeTotNBins()}, m_value( new BaseAccumulator[m_totNBins] ) {
      reset();
    }
    template <atomicity ato>
    HistogramingAccumulatorInternal( construct_empty_t,
                                     const HistogramingAccumulatorInternal<ato, InputType, Arithmetic, ND, BaseAccumulatorT>& other )
        : m_axis( other.m_axis ), m_totNBins{computeTotNBins()}, m_value( new BaseAccumulator[m_totNBins] ) {
      reset();
    }
    HistogramingAccumulatorInternal& operator+=( InputType v ) {
      accumulator( v.computeIndex( m_axis ) ) += v.forInternalCounter();
      return *this;
    }
    void reset() {
      for ( unsigned int index = 0; index < m_totNBins; index++ ) accumulator(index).reset();
    }
    template <atomicity ato>
    void mergeAndReset( HistogramingAccumulatorInternal<ato, InputType, Arithmetic, ND, BaseAccumulatorT>&& other ) {
      assert( m_totNBins == other.m_totNBins );
      for ( unsigned int index = 0; index < m_totNBins; index++ ) {
        accumulator( index ).mergeAndReset( std::move( other.accumulator( index ) ) );
      }
    }

  protected:
    auto& axis() const { return m_axis; }
    auto nBins( unsigned int i ) const { return m_axis[i].nBins; }
    auto minValue( unsigned int i ) const { return m_axis[i].minValue; }
    auto maxValue( unsigned int i ) const { return m_axis[i].maxValue; }
    auto ratio( unsigned int i ) const { return m_axis[i].ratio; }
    auto binValue( unsigned int i ) const { return accumulator( i ).value(); }
    auto nEntries( unsigned int i ) const { return accumulator( i ).nEntries(); }
    auto totNBins() const { return m_totNBins; }

  private:
    BaseAccumulator& accumulator( unsigned int index ) const {
      assert( index < m_totNBins );
      return m_value[index];
    }
    unsigned int         computeTotNBins() const {
      unsigned int nTotBins = 1;
      for ( unsigned int i = 0; i < ND::value; i++ ) { nTotBins *= ( m_axis[i].nBins + 2 ); }
      return nTotBins;
    }
    /// set of Axis of this Histogram
    std::array<Axis<Arithmetic>, ND::value> m_axis;
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
  using HistogramingAccumulator = HistogramingAccumulatorInternal<Atomicity, HistoInputType<Arithmetic, ND::value>, Arithmetic, ND, CountAccumulator>;

  /**
   * Class implementing a weighted histogram accumulator
   *
   * Actually only an alias to HistogramingAccumulatorInternal with proper template parameters
   */
  template <atomicity Atomicity, typename Arithmetic, typename ND>
  using WeightedHistogramingAccumulator = HistogramingAccumulatorInternal<Atomicity, WeightedHistoInputType<Arithmetic, ND::value>, Arithmetic, ND, WeightedCountAccumulator>;

  /**
   * Class implementing a profile histogram accumulator
   *
   * Actually only an alias to HistogramingAccumulatorInternal with proper template parameters
   */
  template <atomicity Atomicity, typename Arithmetic, typename ND>
  using ProfileHistogramingAccumulator = HistogramingAccumulatorInternal<Atomicity, HistoInputType<Arithmetic, ND::value+1, ND::value>, Arithmetic, ND, SigmaAccumulator>;

  /**
   * Class implementing a weighted profile histogram accumulator
   *
   * Actually only an alias to HistogramingAccumulatorInternal with proper template parameters
   */
  template <atomicity Atomicity, typename Arithmetic, typename ND>
  using WeightedProfileHistogramingAccumulator = HistogramingAccumulatorInternal<Atomicity, WeightedHistoInputType<Arithmetic, ND::value+1, ND::value>, Arithmetic, ND, WeightedSigmaAccumulator>;

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
   *    corresponding to the given values is then increased by 1/weight
   *  - the Counter is templated by the types of the values given to
   *    operator+ and also by the type stored into the bins
   *  - the counter can be atomic or not and supports buffering. Note that
   *    the atomicity is classical eventual consistency. So each bin is
   *    atomically updated but bins are not garanted to be coherent when
   *    reading all of them back
   *  - profile histograms are also supported, operator+= takes one more
   *    value in the array of values in that case
   *
   * This base class is then aliases for the 4 standard cases of HistogramingCounter,
   * WeightedHistogramingCounter, ProfileHistogramingCounter and WeightedProfileHistogramingCounter
   *
   * Typical usage :
   * \code
   * HistogramingCounter<2, double, atomicity::full>
   *   counter{owner, "CounterName", {nBins1, minVal1, maxVal1}, {nBins2, minVal2, maxVal2}};
   * counter += {val1, val2};
   * \endcode
   */
  template <unsigned int ND, atomicity Atomicity, typename Arithmetic, const char* SubType,
            template<atomicity, typename, typename> typename Accumulator>
  struct HistogramingCounterBase
    : BufferableCounter<Atomicity, Accumulator, Arithmetic, std::integral_constant<int, ND>> {
    using Parent =
      BufferableCounter<Atomicity, Accumulator, Arithmetic, std::integral_constant<int, ND>>;
    template <typename OWNER>
    HistogramingCounterBase( OWNER* owner, std::string const& name, std::initializer_list<Axis<Arithmetic>> axis )
      : Parent( owner, name, "histogram", axis, std::make_index_sequence<ND>{} ) {}
    template <typename OWNER>
    HistogramingCounterBase( OWNER* owner, std::string const& name, Axis<Arithmetic> axis )
      : HistogramingCounterBase( owner, name, {axis} ) {}
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
      bins.reserve(this->totNBins());
      unsigned int totNEntries{0};
      for (unsigned int i = 0; i < this->totNBins(); i++) {
        bins.push_back(this->binValue(i));
        totNEntries += this->nEntries(i);
      }
      // build json
      return nlohmann::json{{"dimension", ND},
                            {"empty", totNEntries == Arithmetic{}},
                            {"subtype", SubType},
                            {"nEntries", totNEntries},
                            {"internalType", typeid(Arithmetic).name()},
                            {"axis", this->axis() },
                            {"bins", bins }};
    }
  };

  namespace {
    static const char histogramString[] = "histogram";
    static const char profilehistogramString[] = "profilehistogram";
  }
  /// standard histograming counter. See HistogramingCounterBase for details
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double>
  using HistogramingCounter = HistogramingCounterBase<ND, Atomicity, Arithmetic, histogramString, HistogramingAccumulator>;

  /// standard histograming counter with weight. See HistogramingCounterBase for details
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double>
  using WeightedHistogramingCounter = HistogramingCounterBase<ND, Atomicity, Arithmetic, histogramString, WeightedHistogramingAccumulator>;

  /// profile histograming counter. See HistogramingCounterBase for details
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double>
  using ProfileHistogramingCounter = HistogramingCounterBase<ND, Atomicity, Arithmetic, profilehistogramString, ProfileHistogramingAccumulator>;

  /// weighted profile histograming counter. See HistogramingCounterBase for details
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double>
  using WeightedProfileHistogramingCounter = HistogramingCounterBase<ND, Atomicity, Arithmetic, profilehistogramString, WeightedProfileHistogramingAccumulator>;

  /** Counter class for histogram storage. (EXPERIMENTAL)
   *
   * **It is an experimental feature and the interface may change.**
   *
   * This Gaudi::Accumulator class can be used to add histograms as data members of algorithms,
   * similar to the simpler counters.
   *
   * For example:
   * \code
   * class MyAlgorithm: public Gaudi::Algorithm {
   *   // ...
   *   StatusCode execute( const EventContext& ) const {
   *     // ...
   *     m_myHisto( aValue );
   *     // ...
   *     return StatusCode::SUCCESS:
   *   }
   *   // ...
   *   Gaudi::Accumulators::Histogram<RH1D> m_myHisto{
   *       this, "histogram_id", {"Histogram Title", {100, -5, 5}}};
   * };
   * \endcode
   *
   * Known issues:
   * - No I/O support (writing to ROOT files or any other persistency should be done by hand)
   * - ROOT7 2D histograms can be created and filled, but not saved to disk
   *   (https://sft.its.cern.ch/jira/browse/ROOT-10143)
   * - Boost histograms are a bit cumbersome to declare (see GaudiExamples/src/Histograms/Boost.cpp)
   * - No explicit thread safety (depends on the underlying implementation)
   *
   */
  template <typename HistogramType>
  struct Histogram : PrintableCounter {
    inline static const std::string typeString{"oldhistogram"};
    using Adapter = Details::HistogramAdapter<HistogramType>;

    Histogram( HistogramType h ) : m_hist{std::move( h )} {}

    template <class OWNER>
    Histogram( OWNER* o, const std::string& tag, HistogramType h ) : Histogram{std::move( h )} {
      o->serviceLocator()->monitoringHub().registerEntity( o->name(), tag, typeString, *this );
    }

    /// @{
    /// Add something to the histogram (see documentation of the histogram implementation).
    template <class... Ts>
    Histogram& operator()( const Ts&... ts ) {
      m_hist( ts... );
      return *this;
    }
    template <typename T = HistogramType, typename INC = typename Details::HistogramAdapter<T>::increment_t>
    Histogram& operator+=( const INC& x ) {
      m_hist += x;
      return *this;
    }
    template <class... Ts>
    Histogram& fill( const Ts&... ts ) {
      m_hist.fill( ts... );
      return *this;
    }
    /// @}

    /// @{
    /// Accessor to the underlying histogram object.
    HistogramType&       hist() { return m_hist.hist; }
    const HistogramType& hist() const { return m_hist.hist; }
    /// @}

    // PrintableCounter specialization.
    using PrintableCounter::print;
    std::ostream& print( std::ostream& s, bool tableFormat = false ) const override {
      return m_hist.printImpl( s, tableFormat );
    }
    MsgStream& print( MsgStream& s, bool tableFormat = true ) const override {
      return m_hist.printImpl( s, tableFormat );
    }
    bool                   toBePrinted() const override { return m_hist; }
    virtual nlohmann::json toJSON() const override {
      return {{"type", "histogram"}, {"empty", !(bool)m_hist}, {"nEntries", m_hist.nEntries()}};
    }

  private:
    Adapter m_hist;
  };
} // namespace Gaudi::Accumulators
