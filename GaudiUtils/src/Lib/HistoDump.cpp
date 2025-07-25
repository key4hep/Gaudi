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
#ifdef __ICC
// disable icc remark #2259: non-pointer conversion from "X" to "Y" may lose significant bits
//   TODO: To be removed, since it comes from ROOT TMathBase.h
#  pragma warning( disable : 2259 )
#endif
#ifdef WIN32
// Disable warning
//   warning C4996: 'sprintf': This function or variable may be unsafe.
// coming from TString.h
#  pragma warning( disable : 4996 )
#endif

#include <AIDA/IAnnotation.h>
#include <AIDA/IAxis.h>
#include <AIDA/IHistogram1D.h>
#include <AIDA/IProfile1D.h>
#include <GaudiKernel/StatusCode.h>
#include <GaudiUtils/HistoDump.h>
#include <GaudiUtils/HistoStats.h>
#include <GaudiUtils/HistoTableFormat.h>
#include <TAxis.h>
#include <TH1.h>
#include <TProfile.h>
#include <cmath>
#include <fmt/format.h>
#include <iostream>
#include <numeric>
#include <sstream>
#include <utility>
#include <vector>

// ============================================================================
namespace {

  // idea coming from The art of computer programming by Knuth
  constexpr bool essentiallyEqual( double const a, double const b ) {
    return std::abs( a - b ) <= std::min( std::abs( a ), std::abs( b ) ) * std::numeric_limits<double>::epsilon();
  }

  // ==========================================================================
  /** @struct Histo
   *  helper structure to keep the representation of the histogram
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2009-09-18
   */
  struct Histo {
    // ========================================================================
    Histo() = default;
    // ========================================================================
    /** @struct Bin
     *  helper structure to keep the representation of bin
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-18
     */
    struct Bin {
      // ======================================================================
      Bin( const double h = 0, const double e = 0, const double l = -1 ) : height( h ), error( e ), lower( l ) {}
      // ======================================================================
      /// bin height
      double height; //  bin height
      /// bin error
      double error; //   bin error
      /// lower edge
      double lower; //  lower edge
      // ======================================================================
      Bin& operator+=( const Bin& right ) {
        height += right.height;
        const double e2 = error * error + right.error * right.error;
        error           = std::sqrt( e2 );
        return *this;
      }
      // ======================================================================
    };
    // ========================================================================
    /// get Y-max
    double maxY( const bool withErr ) const {
      return std::accumulate(
          std::begin( bins ), std::end( bins ), std::max( under.height, over.height ),
          [&]( double m, const Bin& b ) { return std::max( m, withErr ? b.height + b.error : b.height ); } );
    }
    /// get Y-min
    double minY( const bool withErr ) const {
      return std::accumulate(
          std::begin( bins ), std::end( bins ), std::min( under.height, over.height ),
          [&]( double m, const Bin& b ) { return std::min( m, withErr ? b.height - b.error : b.height ); } );
    }
    /// rebin the histogram
    Histo rebin( const unsigned int bin ) const {
      // create new histogram
      Histo nh;
      // copy overflow & underflow bins
      nh.under = under;
      nh.over  = over;
      // rebin
      for ( unsigned int ibin = 0; ibin < bins.size(); ++ibin ) {
        const Bin& current = bins[ibin];
        if ( nh.bins.empty() ) {
          nh.bins.push_back( current );
        } else if ( 0 == ibin % bin ) {
          nh.bins.push_back( current );
        } else {
          nh.bins.back() += current;
        }
      }
      return nh;
    }
    // find "null-bin", if any
    int nullBin() const {
      for ( auto ib = bins.cbegin(); bins.cend() != ib + 1; ++ib ) {
        if ( ib->lower <= 0 && 0 < ( ib + 1 )->lower ) { return ib - bins.cbegin(); }
      }
      return -1;
    }
    // ========================================================================
    typedef std::vector<Bin> Bins;
    // ========================================================================
    /// histogram bins:
    Bins bins; // histogram bins
    /// underflow bin
    Bin under; //  underflow bin
    /// overflow bin
    Bin over; //   overflow bin
    // ========================================================================
  };
  // ==========================================================================
  /** "convert ROOT histogram into "local" histogram
   *   @param root (INPUT)  pointer to ROOT histogram
   *   @param hist (OUTPUT) local histogram
   *   @return StatusCode
   *   @author Vanya BELYAEV  Ivan.Belyaev@nikhef.nl
   *   @date 2009-09-19
   */
  StatusCode _getHisto( const TH1* root, Histo& hist ) {
    // clear the histogram
    hist.bins.clear();
    //
    if ( !root ) { return StatusCode::FAILURE; } // RETURN
    const TAxis* axis = root->GetXaxis();
    if ( !axis ) { return StatusCode::FAILURE; } // RETURN
    const int nbins = axis->GetNbins();
    if ( 0 == nbins ) { return StatusCode::FAILURE; } // RETURN

    // underflow bin
    hist.under = Histo::Bin( root->GetBinContent( 0 ), root->GetBinError( 0 ), axis->GetXmin() );
    // overflow bin
    hist.over = Histo::Bin( root->GetBinContent( nbins + 1 ), root->GetBinError( nbins + 1 ), axis->GetXmax() );
    //
    //
    for ( int ibin = 1; ibin <= nbins; ++ibin ) {
      // add to the local histo
      hist.bins.emplace_back( root->GetBinContent( ibin ), root->GetBinError( ibin ), axis->GetBinLowEdge( ibin ) );
    }
    return StatusCode::SUCCESS;
  }
  // ==========================================================================
  /** "convert ROOT 1D-profile into "local" histogram
   *   @param root (INPUT)  pointer to ROOT 1D-profile
   *   @param hist (OUTPUT) local histogram
   *   @return StatusCode
   *   @author Vanya BELYAEV  Ivan.Belyaev@nikhef.nl
   *   @date 2009-09-19
   */
  StatusCode _getHisto( const TProfile* root, Histo& hist, const bool /* spread */ ) {
    const TH1* histo = root;
    return _getHisto( histo, hist );
  }
  // ==========================================================================
  /** "convert AIDA histogram into "local" histogram
   *   @param aida (INPUT)  pointer to AIDA histogram
   *   @param hist (OUTPUT) local histogram
   *   @return StatusCode
   *   @author Vanya BELYAEV  Ivan.BElyaev@nikhef.nl
   *   @date 2009-09-19
   */
  StatusCode _getHisto( const AIDA::IHistogram1D* aida, Histo& hist ) {
    // clear the histogram
    hist.bins.clear();
    //
    if ( !aida ) { return StatusCode::FAILURE; } // RETURN
    //
    const AIDA::IAxis& axis  = aida->axis();
    const int          nbins = axis.bins();
    //
    // underflow bin
    hist.under = Histo::Bin( aida->binHeight( AIDA::IAxis::UNDERFLOW_BIN ),
                             aida->binError( AIDA::IAxis::UNDERFLOW_BIN ), axis.lowerEdge() );
    // overflow bin
    hist.over = Histo::Bin( aida->binHeight( AIDA::IAxis::OVERFLOW_BIN ), aida->binError( AIDA::IAxis::OVERFLOW_BIN ),
                            axis.upperEdge() );
    //
    for ( int ibin = 0; ibin < nbins; ++ibin ) {
      // add to the local histo
      hist.bins.emplace_back( aida->binHeight( ibin ), aida->binError( ibin ), axis.binLowerEdge( ibin ) );
    }
    return StatusCode::SUCCESS;
  }
  // ==========================================================================
  /** "convert AIDA 1D-profile into "local" histogram
   *   @param aida (INPUT)  pointer to AIDA 1D-profile
   *   @param hist (OUTPUT) local histogram
   *   @return StatusCode
   *   @author Vanya BELYAEV  Ivan.Belyaev@nikhef.nl
   *   @date 2009-09-19
   */
  StatusCode _getHisto( const AIDA::IProfile1D* aida, Histo& hist, const bool spread ) {
    // clear the histogram
    hist.bins.clear();
    //
    if ( !aida ) { return StatusCode::FAILURE; } // RETURN
    //
    const AIDA::IAxis& axis  = aida->axis();
    const int          nbins = axis.bins();
    //
    // underflow bin
    hist.under =
        Histo::Bin( aida->binHeight( AIDA::IAxis::UNDERFLOW_BIN ),
                    spread ? aida->binRms( AIDA::IAxis::UNDERFLOW_BIN ) : aida->binError( AIDA::IAxis::UNDERFLOW_BIN ),
                    axis.lowerEdge() );
    // overflow bin
    hist.over =
        Histo::Bin( aida->binHeight( AIDA::IAxis::OVERFLOW_BIN ),
                    spread ? aida->binRms( AIDA::IAxis::OVERFLOW_BIN ) : aida->binError( AIDA::IAxis::OVERFLOW_BIN ),
                    axis.upperEdge() );
    //
    for ( int ibin = 0; ibin < nbins; ++ibin ) {
      // add to the local histo
      hist.bins.emplace_back( aida->binHeight( ibin ), spread ? aida->binRms( ibin ) : aida->binError( ibin ),
                              axis.binLowerEdge( ibin ) );
    }
    return StatusCode::SUCCESS;
  }
  // ==========================================================================
  /** find the appropriate rebin factor
   *  (I suspect that there is some intelligent STD/STL routine for this purpose)
   *  @author Vanya BELYAEV  Ivan.BElyaev@nikhef.nl
   *  @date 2009-09-19
   */
  inline unsigned int rebin( const unsigned int bins, const unsigned int imax ) {
    if ( 0 == imax ) { return 1; } // RETURN
    unsigned int ibin = 1;
    while ( bins > imax * ibin ) { ++ibin; }
    return ibin; // RETURN
  }
  // ==========================================================================
  /** decompose the double value into decimal significand
   *  (I suspect that there is some intelligent STD/STL routine for this purpose)
   *  @author Vanya BELYAEV  Ivan.BElyaev@nikhef.nl
   *  @date 2009-09-19
   */
  std::pair<double, int> decompose( double v ) {
    if ( std::abs( v ) < std::numeric_limits<double>::epsilon() ) {
      return { 0.0, 0 };
    } // RETURN
    else if ( essentiallyEqual( 1.0, v ) ) {
      return { 1.0, 0 };
    } // RETURN
    else if ( 0 > v ) {
      auto r = decompose( -v );
      return { -r.first, r.second }; // RETURN
    } else if ( 0.1 > v ) {
      int i = 0;
      while ( 0.1 > v ) {
        ++i;
        v *= 10;
      } // STUPID
      return { v, -i };
    } else if ( 1 < v ) {
      int i = 0;
      while ( 1 <= v ) {
        ++i;
        v /= 10;
      } // STUPID
      return { v, i };
    }
    return { v, 1 };
  }
  // ==========================================================================
  /** "efficient" pow-function
   *  @author Vanya BELYAEV  Ivan.Belyaev@nikhef.nl
   *  @date 2009-09-19
   */
  inline double _pow( double x, unsigned long n ) {
    double y = n % 2 ? x : 1;
    while ( n >>= 1 ) {
      x = x * x;
      if ( n % 2 ) { y *= x; }
    }
    return y;
  }
  // forward declaration to break cyclic dependency between rValMin and rValMax
  inline double rValMin( double v );
  // ==========================================================================
  /** find the proper "round" value
   *  @author Vanya BELYAEV  Ivan.BElyaev@nikhef.nl
   *  @date 2009-09-19
   */
  inline double rValMax( double v ) {
    if ( 0 > v ) { return -1 * rValMin( -v ); } // RETURN
    // decompose the double value into decimal significand and mantissa
    std::pair<double, int> r = decompose( v );
    //
    const double f = std::ceil( 20 * r.first ) / 2; // + 1 ;
    const int    l = r.second - 1;
    return 0 < l ? f * _pow( 10, l ) : f / _pow( 10, -l );
  }
  // ==========================================================================
  /** find the proper "round" value
   *  @author Vanya BELYAEV  Ivan.BElyaev@nikhef.nl
   *  @date 2009-09-19
   */
  inline double rValMin( double v ) {
    if ( 0 > v ) { return -1 * rValMax( -v ); } // RETURN
    // decompose the double value into decimal significand and mantissa
    std::pair<double, int> r = decompose( v );
    const double           f = std::floor( 20 * r.first ) / 2; // - 1 ;
    const int              l = r.second - 1;
    return 0 < l ? f * _pow( 10, l ) : f / _pow( 10, -l );
  }
  // ==========================================================================
  /** make the label for y-axis
   *  @author Vanya BELYAEV  Ivan.BElyaev@nikhef.nl
   *  @date 2009-09-19
   */
  inline std::string yLabel( double value ) { return fmt::format( "{:10.3g}", value ); }
  // ==========================================================================
  /** make the label for x-axis
   *  @author Vanya BELYAEV  Ivan.BElyaev@nikhef.nl
   *  @date 2009-09-19
   */
  inline std::string xLabel( const double value ) { return fmt::format( "{:9.3g}", value ); }
  // ==========================================================================
  /// get "correct" symbol
  char symbBin( const Histo::Bin& bin, const double yLow, const double yHigh, const bool yNull, const bool errors ) {
    if ( errors && yLow <= bin.height && bin.height < yHigh ) {
      return '*';
    } // O
    else if ( errors && yHigh < bin.height - bin.error ) {
      return ' ';
    } else if ( errors && yLow >= bin.height + bin.error ) {
      return ' ';
    } else if ( errors ) {
      return 'I';
    } else if ( yLow <= bin.height && bin.height < yHigh ) {
      return '*';
    } else if ( 0 <= bin.height && yLow <= bin.height && 0 < yHigh && !yNull ) {
      return '*';
    } // +
    else if ( 0 > bin.height && yHigh > bin.height && 0 >= yLow && !yNull ) {
      return '*';
    } // -
    //
    return ' ';
  }
  // ==========================================================================
  /** dump the local histogram in text form
   *  @author Vanya BELYAEV  Ivan.BElyaev@nikhef.nl
   *  @date 2009-09-19
   */
  std::ostringstream& dumpText( const Histo& histo, const std::size_t width, const std::size_t height,
                                const bool errors, std::ostringstream& stream ) {
    if ( 40 > width ) { return dumpText( histo, 40, height, errors, stream ); }
    if ( 200 < width ) { return dumpText( histo, 200, height, errors, stream ); }
    if ( 150 < height ) { return dumpText( histo, width, 150, errors, stream ); }
    if ( 20 > height ) { return dumpText( histo, width, 20, errors, stream ); }
    if ( height > width ) { return dumpText( histo, width, width, errors, stream ); }
    //
    const unsigned int nBins = histo.bins.size();
    if ( nBins > width ) {
      // rebin histogram
      Histo r = histo.rebin( rebin( nBins, width ) );
      return dumpText( r, width, height, errors, stream );
    }
    //
    // get the Y-scale
    double yMax = std::max( rValMax( histo.maxY( errors ) ), 0.0 );
    double yMin = std::min( rValMin( histo.minY( errors ) ), 0.0 );

    if ( essentiallyEqual( yMin, yMax ) ) { yMax = yMin + 1; }
    /// try to define the proper "Y-binning"
    std::pair<double, int> r   = decompose( yMax - yMin );
    double                 _ny = std::ceil( 10 * r.first ); //   1 <= ny < 10
    if ( 1 >= _ny ) { _ny = 10; }
    int yBins = (int)std::max( 1., std::ceil( height / _ny ) );

    yBins *= (int)_ny;
    if ( 20 > yBins ) { yBins = 20; }
    const double yScale = ( yMax - yMin ) / yBins;

    //
    const int ySkip = 0 == yBins % 13   ? 13
                      : 0 == yBins % 11 ? 11
                      : 0 == yBins % 9  ? 9
                      : 0 == yBins % 8  ? 8
                      : 0 == yBins % 7  ? 7
                      : 0 == yBins % 6  ? 6
                      : 0 == yBins % 5  ? 5
                      : 0 == yBins % 4  ? 4
                                        : 10;

    const int xSkip =
        // 0 == nBins % 8 ? 8 :
        0 == nBins % 7   ? 7
        : 0 == nBins % 6 ? 6
        : 0 == nBins % 5 ? 5
        : 0 == nBins % 4 ? 4
                         : 10;

    int iNull = histo.nullBin();

    if ( 0 <= iNull ) {
      iNull %= xSkip;
    } else {
      iNull = 0;
    }

    stream << std::endl;

    for ( int yLine = -1; yLine < yBins; ++yLine ) {
      const double yHigh = yMax - yScale * yLine;
      // const double yLow  = yHigh - yScale         ;
      const double yLow = yMax - yScale * ( yLine + 1 );
      //
      std::string line1 = " ";

      const bool ynull  = ( yLow <= 0 && 0 < yHigh );
      const bool yfirst = -1 == yLine || yBins - 1 == yLine;
      const bool ylab   = ( 0 == ( yLine + 1 ) % ySkip ) || yfirst || ynull;

      if ( ylab ) {
        line1 += yLabel( ( yLow <= 0 && 0 < yHigh ) ? 0.0 : yLow );
      } else {
        line1 += std::string( 10, ' ' );
      }
      //
      line1 += " ";
      //
      /// underflow bin:
      line1 += symbBin( histo.under, yLow, yHigh, ynull, errors );
      //
      line1 += ynull ? "-+" : ylab ? " +" : " |";
      //
      std::string line2;
      //
      for ( auto ibin = histo.bins.cbegin(); histo.bins.cend() != ibin; ++ibin ) {
        // char symb = ' ' ;
        const int i = ibin - histo.bins.cbegin();
        //
        const bool xnull = ibin->lower <= 0 && ( ibin + 1 ) != histo.bins.cend() && 0 < ( ibin + 1 )->lower;
        const bool xlab  = iNull == i % xSkip;
        //
        char symb = symbBin( *ibin, yLow, yHigh, ynull, errors );
        //
        if ( ' ' == symb ) {
          if ( ( ynull || yfirst ) && xlab ) {
            symb = '+';
          } else if ( ynull || yfirst ) {
            symb = '-';
          }
          //
          else if ( ylab && xnull ) {
            symb = '+';
          } else if ( xnull ) {
            symb = '|';
          }
          //
          else if ( ylab || xlab ) {
            symb = '.';
          }
          //
        }
        line2 += symb;
        //
      }
      //
      std::string line3 = ynull ? "->" : ylab ? "+ " : "| ";
      //
      /// overflow bin:
      line3 += symbBin( histo.over, yLow, yHigh, ynull, errors );
      //
      stream << line1 << line2 << line3 << std::endl;
      //
    }

    // get x-labels
    std::vector<std::string> xlabels;
    for ( auto ib = histo.bins.cbegin(); histo.bins.cend() != ib; ++ib ) { xlabels.push_back( xLabel( ib->lower ) ); }
    // overflow& underflow  label
    const std::string oLabel = xLabel( histo.over.lower );
    const std::string uLabel = xLabel( histo.under.lower );
    //
    static const std::string s_UNDERFLOW( "UNDERFLOW" );
    static const std::string s_OVERFLOW( " OVERFLOW" );
    //
    //
    for ( unsigned int yLine = 0; yLine < 12; ++yLine ) {
      std::string line = std::string( 12, ' ' );
      //
      if ( yLine < s_UNDERFLOW.size() ) {
        line += s_UNDERFLOW[yLine];
      } else {
        line += ' ';
      }
      //
      line += ' ';
      //
      if ( uLabel.size() > yLine ) {
        line += uLabel[yLine];
      } else {
        line += ' ';
      }
      //
      for ( auto ibin = histo.bins.cbegin(); histo.bins.cend() != ibin; ++ibin ) {
        int        ib   = ibin - histo.bins.cbegin();
        const bool xlab = ( iNull == ib % xSkip );
        if ( xlab && yLine < xlabels[ib].size() ) {
          line += xlabels[ib][yLine];
        } else {
          line += ' ';
        }
      }
      //
      if ( oLabel.size() > yLine ) {
        line += oLabel[yLine];
      } else {
        line += ' ';
      }
      //
      line += ' ';
      //
      if ( yLine < s_OVERFLOW.size() ) {
        line += s_OVERFLOW[yLine];
      } else {
        line += ' ';
      }
      //
      stream << line << std::endl;
    }
    //
    return stream; // RETURN
  }
} // namespace

// ============================================================================
/*  dump the text representation of the histogram
 *  @param histo the histogram
 *  @param stream the stream
 *  @param width  the maximal column width
 *  @param height (INPUT) the proposed coulmn height
 *  @param errors (INPUT) print/plot errors
 *  @param erorrs print/plot errors
 *  @author Vanya BELYAEV  Ivan.BElyaev@nikhef.nl
 *  @date 2009-09-19
 */
// ============================================================================
std::string Gaudi::Utils::Histos::histoDump( const AIDA::IHistogram1D* histo, const std::size_t width,
                                             const std::size_t height, const bool errors ) {
  std::ostringstream stream;
  stream << std::endl;
  if ( !histo ) { return stream.str(); } // RETURN
  Histo      hist;
  StatusCode sc = _getHisto( histo, hist );
  if ( sc.isFailure() ) { return stream.str(); } // RETURN

  stream << fmt::format( R"( Histo TES   : "{}"
 Histo Title : "{}"

 Mean        : {:11.5g} +- {:<10.4g}
 Rms         : {:11.5g} +- {:<10.4g}
 Skewness    : {:11.5g} +- {:<10.4g}
 Kurtosis    : {:11.5g} +- {:<10.4g}

 Entries     :
 |    All    |  In Range | Underflow |  Overflow | #Equivalent |   Integral  |    Total    |
 | {:^9} | {:^9} | {:^9} | {:^9} | {:^11.5g} | {:^11.5g} | {:^11.5g} |

)",
                         path( histo ), histo->title(), //
                         Gaudi::Utils::HistoStats::mean( histo ), Gaudi::Utils::HistoStats::meanErr( histo ),
                         Gaudi::Utils::HistoStats::rms( histo ), Gaudi::Utils::HistoStats::rmsErr( histo ),
                         Gaudi::Utils::HistoStats::skewness( histo ), Gaudi::Utils::HistoStats::skewnessErr( histo ),
                         Gaudi::Utils::HistoStats::kurtosis( histo ), Gaudi::Utils::HistoStats::kurtosisErr( histo ),
                         histo->allEntries(), histo->entries(), histo->binEntries( AIDA::IAxis::UNDERFLOW_BIN ),
                         histo->binEntries( AIDA::IAxis::OVERFLOW_BIN ), histo->equivalentBinEntries(),
                         histo->sumBinHeights(), histo->sumAllBinHeights() );

  const AIDA::IAnnotation& a = histo->annotation();
  if ( a.size() ) {
    stream << " Annotation\n";
    for ( int i = 0; i < a.size(); ++i ) {
      stream << fmt::format( " | {:<25.25s} : {:<45.45s} |\n", a.key( i ), a.value( i ) );
    }
    stream << '\n';
  }

  return dumpText( hist, width, height, errors, stream ).str();
}

/*  dump the text representation of the 1D-profile
 *  @param histo the histogram
 *  @param stream the stream
 *  @param width  the maximal column width
 *  @param height (INPUT) the proposed coulmn height
 *  @author Vanya BELYAEV  Ivan.BElyaev@nikhef.nl
 *  @date 2009-09-19
 */
std::string Gaudi::Utils::Histos::histoDump( const AIDA::IProfile1D* histo, const std::size_t width,
                                             const std::size_t height, const bool spread ) {
  std::ostringstream stream;
  stream << std::endl;
  if ( !histo ) { return stream.str(); } // RETURN
  Histo      hist;
  StatusCode sc = _getHisto( histo, hist, spread );
  if ( sc.isFailure() ) { return stream.str(); } // RETURN

  stream << fmt::format( R"( Histo TES   : "{}"
 Histo Title : "{}"

 Mean        : {:11.5g}
 Rms         : {:11.5g}

 Entries     :
 |    All    |  In Range | Underflow |  Overflow |   Integral  |    Total    |
 | {:^9} | {:^9} | {:^9} | {:^9} | {:^11.5g} | {:^11.5g} |

)",
                         path( histo ), histo->title(), //
                         histo->mean(), histo->rms(), histo->allEntries(), histo->entries(),
                         histo->binEntries( AIDA::IAxis::UNDERFLOW_BIN ),
                         histo->binEntries( AIDA::IAxis::OVERFLOW_BIN ), histo->sumBinHeights(),
                         histo->sumAllBinHeights() );

  const AIDA::IAnnotation& a = histo->annotation();
  if ( a.size() ) {
    stream << " Annotation\n";
    for ( int i = 0; i < a.size(); ++i ) {
      stream << fmt::format( " | {:<25.25s} : {:<45.45s} |\n", a.key( i ), a.value( i ) );
    }
    stream << std::endl;
  }

  return dumpText( hist, width, height, true, stream ).str();
}

/*  dump the text representation of the histogram
 *  @param histo  (INPUT) the histogram
 *  @param width  (INPUT) the maximal column width
 *  @param errors (INPUT) print/plot errors
 *  @return string representation of the histogram
 *  @author Vanya BELYAEV  Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-19
 */
std::string Gaudi::Utils::Histos::histoDump( const TH1* histo, const std::size_t width, const std::size_t height,
                                             const bool errors ) {
  const TProfile* profile = dynamic_cast<const TProfile*>( histo );
  if ( profile ) { return histoDump( profile, width, height ); }
  //
  std::ostringstream stream;
  stream << std::endl;
  if ( !histo ) { return stream.str(); } // RETURN
  Histo      hist;
  StatusCode sc = _getHisto( histo, hist );
  if ( sc.isFailure() ) { return stream.str(); } // RETURN

  stream << fmt::format( R"( Histo Name  : "{}"
 Histo Title : "{}"

 Mean        : {:11.5g} +- {:<10.4g}
 Rms         : {:11.5g} +- {:<10.4g}
 Skewness    : {:11.5g}
 Kurtosis    : {:11.5g}

 Entries     :
 |     All     |  Underflow  |   Overflow  | #Equivalent |   Integral  |
 | {:^11.5g} | {:^11.5g} | {:^11.5g} | {:^11.5g} | {:^11.5g} |

)",
                         histo->GetName(), histo->GetTitle(), //
                         histo->GetMean(), histo->GetMeanError(), histo->GetRMS(), histo->GetRMSError(),
                         histo->GetSkewness(), histo->GetKurtosis(), histo->GetEntries(), histo->GetBinContent( 0 ),
                         histo->GetBinContent( histo->GetNbinsX() + 1 ), histo->GetEffectiveEntries(),
                         histo->Integral() );

  return dumpText( hist, width, height, errors, stream ).str();
}

/*  dump the text representation of the histogram
 *  @param histo  (INPUT) the histogram
 *  @param width  (INPUT) the maximal column width
 *  @param errors (INPUT) print/plot errors
 *  @return string representation of the histogram
 *  @author Vanya BELYAEV  Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-19
 */
std::string Gaudi::Utils::Histos::histoDump( const TProfile* histo, const std::size_t width,
                                             const std::size_t height ) {
  std::ostringstream stream;
  stream << std::endl;
  if ( !histo ) { return stream.str(); } // RETURN
  Histo      hist;
  StatusCode sc = _getHisto( histo, hist, true );
  if ( sc.isFailure() ) { return stream.str(); } // RETURN

  stream << fmt::format( R"( Profile Name  : "{}"
 Profile Title : "{}"

 Mean        : {:11.5g}
 Rms         : {:11.5g}

 Entries     :
 |     All     |  Underflow  |   Overflow  |   Integral  |
 | {:^11.5g} | {:^11.5g} | {:^11.5g} | {:^11.5g} |

)",
                         histo->GetName(), histo->GetTitle(), //
                         histo->GetMean(), histo->GetRMS(), histo->GetSkewness(), histo->GetKurtosis(),
                         histo->GetEntries(), histo->GetBinContent( 0 ), histo->GetBinContent( histo->GetNbinsX() + 1 ),
                         histo->Integral() );

  return dumpText( hist, width, height, true, stream ).str();
}
