#pragma once

#include <Gaudi/Accumulators.h>
#include <ROOT/RHist.hxx>
#include <boost/format.hpp>
#include <boost/histogram/algorithm/sum.hpp>
#include <boost/histogram/fwd.hpp>

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

    private:
      // flag to record if anything was added to the histogram (information not available in Boost histograms)
      bool filled = false;
    };
  } // namespace Details

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
    using Adapter = Details::HistogramAdapter<HistogramType>;

    Histogram( HistogramType h ) : m_hist{std::move( h )} {}

    template <class OWNER>
    Histogram( OWNER* o, const std::string& tag, HistogramType h ) : Histogram{std::move( h )} {
      o->declareCounter( tag, *this );
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
    bool toBePrinted() const override { return m_hist; }

  private:
    Adapter m_hist;
  };
} // namespace Gaudi::Accumulators
