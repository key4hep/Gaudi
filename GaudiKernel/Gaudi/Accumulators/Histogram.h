#pragma once

#include <Gaudi/Accumulators.h>
#include <ROOT/RHist.hxx>
#include <boost/format.hpp>
#include <boost/histogram/algorithm/sum.hpp>
#include <boost/histogram/fwd.hpp>

namespace Gaudi::Accumulators {

  namespace Details {
    template <typename HistogramType>
    struct HistogramAdapter {};

    template <int DIMENSIONS, class PRECISION, template <int D_, class P_> class... STAT>
    struct HistogramAdapter<ROOT::Experimental::RHist<DIMENSIONS, PRECISION, STAT...>> {
      using histogram_t = ROOT::Experimental::RHist<DIMENSIONS, PRECISION, STAT...>;
      using increment_t = typename histogram_t::CoordArray_t;

      HistogramAdapter( histogram_t&& h ) : hist{std::move( h )} {}
      histogram_t hist;

      template <class... Ts>
      auto operator()( const Ts&... ts ) {
        return hist.Fill( ts... );
      }
      template <class... Ts>
      auto fill( const Ts&... ts ) {
        return hist.Fill( ts... );
      }
      auto operator+=( const increment_t& x ) { return hist.Fill( x ); }

      template <typename stream>
      stream& printImpl( stream& o, bool tableFormat ) const {
        auto fmt = boost::format{tableFormat ? "H%|10d| |" : "#=%|-7lu|"};
        return o << fmt % hist.GetEntries();
      }
      // histogram not empty
      operator bool() const { return hist.GetEntries(); }
    };

    template <class Axes, class Storage>
    struct HistogramAdapter<boost::histogram::histogram<Axes, Storage>> {
      using histogram_t = boost::histogram::histogram<Axes, Storage>;

      HistogramAdapter( histogram_t&& h ) : hist{std::move( h )} {}
      histogram_t hist;

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

      template <typename stream>
      stream& printImpl( stream& o, bool tableFormat ) const {
        auto fmt = boost::format{tableFormat ? "H%|10d| |" : "#=%|-7lu|"};
        return o << fmt % boost::histogram::algorithm::sum( hist );
      }
      // histogram not empty
      operator bool() const { return filled; }

    private:
      bool filled = false;
    };
  } // namespace Details

  template <typename HistogramType>
  struct Histogram : PrintableCounter {
    using Adapter = Details::HistogramAdapter<HistogramType>;

    Histogram( HistogramType h ) : m_hist{std::move( h )} {}

    template <class OWNER>
    Histogram( OWNER* o, const std::string& tag, HistogramType h ) : Histogram{std::move( h )} {
      // o->declareHistogram( tag, *this );
      o->declareCounter( tag, *this );
    }

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

    HistogramType&       hist() { return m_hist.hist; }
    const HistogramType& hist() const { return m_hist.hist; }

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
