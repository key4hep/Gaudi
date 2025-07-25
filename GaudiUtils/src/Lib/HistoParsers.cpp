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
// =============================================================================
// Include files
// =============================================================================
// STD & STL
// =============================================================================
#include <map>
#include <memory>
// =============================================================================
// AIDA
// ============================================================================
#include <AIDA/IHistogram1D.h>
#include <AIDA/IHistogram2D.h>
#include <AIDA/IHistogram3D.h>
// ============================================================================
// ROOT
// ============================================================================
#include <TAxis.h>
#include <TH1D.h>
#include <TH1F.h>
#include <TH2D.h>
#include <TH2F.h>
#include <TH3D.h>
// ============================================================================
// GaudiKernel
// ============================================================================
#include <Gaudi/Parsers/Factory.h>
// ============================================================================
// local
// ============================================================================
#include <GaudiUtils/Aida2ROOT.h>
#include <GaudiUtils/HistoParsers.h>
#include <GaudiUtils/HistoXML.h>
// ============================================================================
// Local
// ============================================================================
#include "H1.h"
// ============================================================================
namespace Gaudi {
  // ==========================================================================
  namespace Parsers {
    // ========================================================================
    /** @class EdgeGrammar
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-26
     */
    template <typename Iterator, typename Skipper>
    class EdgeGrammar : public qi::grammar<Iterator, Edges(), qi::locals<char>, Skipper> {
      // ======================================================================
    public:
      // ======================================================================
      typedef Edges ResultT;
      // ======================================================================
    public:
      // ======================================================================
      EdgeGrammar() : EdgeGrammar::base_type( result ) {
        inner = ( ( qi::lit( "edges" ) | "'edges'" | "\"edges\"" ) >> ":" >> edges[qi::_val *= qi::_1] ) |
                inner_pairs[qi::_val = qi::_1];
        inner_pairs = ( ( ( qi::lit( "nbins" ) | "'nbins'" | "\"nbins\"" ) >> ":" >> nbins[qi::_val /= qi::_1] ) |
                        ( ( qi::lit( "low" ) | "'low'" | "\"low\"" ) >> ":" >> low[qi::_val -= qi::_1] ) |
                        ( ( qi::lit( "high" ) | "'high'" | "\"high\"" ) >> ":" >> high[qi::_val += qi::_1] ) ) %
                      ',';

        begin =
            enc::char_( '[' )[qi::_val = ']'] | enc::char_( '{' )[qi::_val = '}'] | enc::char_( '(' )[qi::_val = ')'];
        end    = enc::char_( qi::_r1 );
        result = begin[qi::_a = qi::_1] >> inner[qi::_val = qi::_1] >> end( qi::_a );
      }
      VectorGrammar<Iterator, std::vector<double>, Skipper>  edges;
      RealGrammar<Iterator, double, Skipper>                 low, high;
      IntGrammar<Iterator, unsigned int, Skipper>            nbins;
      qi::rule<Iterator, Edges(), qi::locals<char>, Skipper> result;
      qi::rule<Iterator, Edges(), Skipper>                   inner, inner_pairs;
      qi::rule<Iterator, char()>                             begin;
      qi::rule<Iterator, void( char )>                       end;
      // ======================================================================
    };
    REGISTER_GRAMMAR( Edges, EdgeGrammar );
    // ========================================================================
    template <typename Iterator, typename Skipper>
    class H1Grammar : public qi::grammar<Iterator, H1(), qi::locals<char>, Skipper> {
      // ======================================================================
    public:
      // ======================================================================
      typedef H1 ResultT;
      // ======================================================================
    public:
      // ======================================================================
      H1Grammar() : H1Grammar::base_type( result ) {
        inner = ( ( ( qi::lit( "name" ) | "'name'" | "\"name\"" ) >> ":" >> name[qi::_val *= qi::_1] ) |
                  ( ( qi::lit( "title" ) | "'title'" | "\"title\"" ) >> ":" >> title[qi::_val /= qi::_1] ) |
                  ( -( ( qi::lit( "X" ) | "'X'" | "\"X\"" | "x" | "'x'" | "\"x\"" ) >> ':' ) >>
                    edges[qi::_val &= qi::_1] ) |
                  ( ( qi::lit( "nbins" ) | "'nbins'" | "\"nbins\"" ) >> ":" >> nbins[qi::_val |= qi::_1] ) |
                  ( ( qi::lit( "low" ) | "'low'" | "\"low\"" ) >> ":" >> low[qi::_val -= qi::_1] ) |
                  ( ( qi::lit( "high" ) | "'high'" | "\"high\"" ) >> ":" >> high[qi::_val ^= qi::_1] ) |
                  ( ( qi::lit( "bins" ) | "'bins'" | "\"bins\"" ) >> ':' >> bins[qi::_val += qi::_1] ) ) %
                ',';

        begin =
            enc::char_( '[' )[qi::_val = ']'] | enc::char_( '{' )[qi::_val = '}'] | enc::char_( '(' )[qi::_val = ')'];
        end    = enc::char_( qi::_r1 );
        result = ( begin[qi::_a = qi::_1] >> inner[qi::_val = qi::_1] >> end( qi::_a ) ) | inner;
      }

      StringGrammar<Iterator, Skipper>            name, title;
      EdgeGrammar<Iterator, Skipper>              edges;
      RealGrammar<Iterator, double, Skipper>      low, high;
      IntGrammar<Iterator, unsigned int, Skipper> nbins;

      VectorGrammar<Iterator, std::vector<std::pair<double, double>>, Skipper> bins;
      qi::rule<Iterator, H1(), qi::locals<char>, Skipper>                      result;
      qi::rule<Iterator, H1(), Skipper>                                        inner;
      qi::rule<Iterator, char()>                                               begin;
      qi::rule<Iterator, void( char )>                                         end;
    };
    REGISTER_GRAMMAR( H1, H1Grammar );
    // ========================================================================
    template <typename Iterator, typename Skipper>
    class H2Grammar : public qi::grammar<Iterator, H2(), qi::locals<char>, Skipper> {
      // ======================================================================
    public:
      // ======================================================================
      typedef H2 ResultT;
      // ======================================================================
    public:
      // ======================================================================
      H2Grammar() : H2Grammar::base_type( result ) {
        inner = ( ( ( qi::lit( "name" ) | "'name'" | "\"name\"" ) >> ":" >> name[qi::_val *= qi::_1] ) |
                  ( ( qi::lit( "title" ) | "'title'" | "\"title\"" ) >> ":" >> title[qi::_val /= qi::_1] ) |
                  ( ( qi::lit( "X" ) | "'X'" | "\"X\"" | "x" | "'x'" | "\"x\"" ) >> ':' >> edges[qi::_val &= qi::_1] ) |
                  ( ( qi::lit( "Y" ) | "'Y'" | "\"Y\"" | "y" | "'y'" | "\"y\"" ) >> ':' >> edges[qi::_val |= qi::_1] ) |
                  ( ( qi::lit( "bins" ) | "'bins'" | "\"bins\"" ) >> ':' >> bins[qi::_val += qi::_1] ) ) %
                ',';

        begin =
            enc::char_( '[' )[qi::_val = ']'] | enc::char_( '{' )[qi::_val = '}'] | enc::char_( '(' )[qi::_val = ')'];
        end    = enc::char_( qi::_r1 );
        result = ( begin[qi::_a = qi::_1] >> inner[qi::_val = qi::_1] >> end( qi::_a ) ) | inner[qi::_val = qi::_1];
      }

      StringGrammar<Iterator, Skipper>                                         name, title;
      EdgeGrammar<Iterator, Skipper>                                           edges;
      VectorGrammar<Iterator, std::vector<std::pair<double, double>>, Skipper> bins;
      qi::rule<Iterator, H2(), qi::locals<char>, Skipper>                      result;
      qi::rule<Iterator, H2(), Skipper>                                        inner;
      qi::rule<Iterator, char()>                                               begin;
      qi::rule<Iterator, void( char )>                                         end;

      // ======================================================================
    };
    REGISTER_GRAMMAR( H2, H2Grammar );
    // ========================================================================
    template <typename Iterator, typename Skipper>
    class H3Grammar : public qi::grammar<Iterator, H3(), qi::locals<char>, Skipper> {
      // ======================================================================
    public:
      // ======================================================================
      typedef H3 ResultT;
      // ======================================================================
    public:
      // ======================================================================
      H3Grammar() : H3Grammar::base_type( result ) {
        inner = ( ( ( qi::lit( "name" ) | "'name'" | "\"name\"" ) >> ":" >> name[qi::_val *= qi::_1] ) |
                  ( ( qi::lit( "title" ) | "'title'" | "\"title\"" ) >> ":" >> title[qi::_val /= qi::_1] ) |
                  ( ( qi::lit( "X" ) | "'X'" | "\"X\"" | "x" | "'x'" | "\"x\"" ) >> ':' >> edges[qi::_val &= qi::_1] ) |
                  ( ( qi::lit( "Y" ) | "'Y'" | "\"Y\"" | "y" | "'y'" | "\"y\"" ) >> ':' >> edges[qi::_val |= qi::_1] ) |
                  ( ( qi::lit( "Z" ) | "'Z'" | "\"Z\"" | "z" | "'z'" | "\"z\"" ) >> ':' >> edges[qi::_val -= qi::_1] ) |
                  ( ( qi::lit( "bins" ) | "'bins'" | "\"bins\"" ) >> ':' >> bins[qi::_val += qi::_1] ) ) %
                ',';

        begin =
            enc::char_( '[' )[qi::_val = ']'] | enc::char_( '{' )[qi::_val = '}'] | enc::char_( '(' )[qi::_val = ')'];
        end    = enc::char_( qi::_r1 );
        result = ( begin[qi::_a = qi::_1] >> inner[qi::_val = qi::_1] >> end( qi::_a ) ) | inner[qi::_val = qi::_1];
      }

      StringGrammar<Iterator, Skipper>                                         name, title;
      EdgeGrammar<Iterator, Skipper>                                           edges;
      VectorGrammar<Iterator, std::vector<std::pair<double, double>>, Skipper> bins;
      qi::rule<Iterator, H3(), qi::locals<char>, Skipper>                      result;
      qi::rule<Iterator, H3(), Skipper>                                        inner;
      qi::rule<Iterator, char()>                                               begin;
      qi::rule<Iterator, void( char )>                                         end;

      // ======================================================================
    };
    REGISTER_GRAMMAR( H3, H3Grammar );
    // ========================================================================
  } // namespace Parsers
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
namespace {
  // ==========================================================================
  /// parse the histogram
  StatusCode _parse( H1& h1, std::string_view input ) {
    // check the parsing
    StatusCode sc = Gaudi::Parsers::parse_( h1, input );
    if ( sc.isFailure() ) { return sc; } // RETURN
    return h1.ok() ? StatusCode::SUCCESS : StatusCode::FAILURE;
  }
  // ==========================================================================
  /// parse the histogram
  StatusCode _parse( H2& h2, std::string_view input ) {
    // check the parsing
    StatusCode sc = Gaudi::Parsers::parse_( h2, input );
    if ( sc.isFailure() ) { return sc; } // RETURN
    return h2.ok() ? StatusCode::SUCCESS : StatusCode::FAILURE;
  }
  // ==========================================================================
  /// parse the histogram
  StatusCode _parse( H3& h3, std::string_view input ) {
    // check the parsing
    StatusCode sc = Gaudi::Parsers::parse_( h3, input );
    if ( sc.isFailure() ) { return sc; } // RETURN
    return h3.ok() ? StatusCode::SUCCESS : StatusCode::FAILURE;
  }
  // ==========================================================================
  template <class HISTO1>
  std::unique_ptr<HISTO1> _parse_1D( std::string_view input, std::string& name ) {
    //
    typedef std::unique_ptr<HISTO1> H1P;
    // ==========================================================================
    // 1) parse the custom format
    //
    H1         h1;
    StatusCode sc = _parse( h1, input );
    if ( sc.isFailure() || !h1.ok() ) { return H1P(); } // RETURN
    //
    // 2) create the histogram
    //
    H1P histo( h1.m_edges.edges.empty() ?          // FIXED binning?
                   new HISTO1( "",                 // h1.m_name.c_str   ()         ,           // NAME
                               h1.m_title.c_str(), // TITLE
                               h1.m_edges.nbins,   // #bins
                               h1.m_edges.low,     // low edge
                               h1.m_edges.high )
                                        :                     // high  edge
                   new HISTO1( "",                            // h1.m_name .c_str ()          ,     // NAME
                               h1.m_title.c_str(),            // TITLE
                               h1.m_edges.edges.size() - 1,   // #bins
                               &h1.m_edges.edges.front() ) ); // vector of edges

    // fill the histogram
    for ( unsigned int ibin = 0; ibin < h1.m_bins.size(); ++ibin ) {
      histo->SetBinContent( ibin, h1.m_bins[ibin].first );
      histo->SetBinError( ibin, h1.m_bins[ibin].second );
    }
    //
    name = h1.m_name;
    //
    return histo;
  }
  // ==========================================================================
  template <class HISTO2>
  std::unique_ptr<HISTO2> _parse_2D( std::string_view input, std::string& name ) {
    //
    typedef std::unique_ptr<HISTO2> H2P;
    // 1) parse the custom format
    //
    H2         h2;
    StatusCode sc = _parse( h2, input );
    if ( sc.isFailure() || !h2.ok() ) { return H2P(); } // RETURN
    //
    // 2) create the histogram
    //
    H2P histo( h2.m_xedges.edges.empty() && h2.m_yedges.edges.empty()
                   ?                               // FIXED binning?
                   new HISTO2( "",                 // h1.m_name.c_str   ()         ,           // NAME
                               h2.m_title.c_str(), // TITLE
                               h2.m_xedges.nbins,  // #bins
                               h2.m_xedges.low,    // low edge
                               h2.m_xedges.high,   // high edge
                               h2.m_yedges.nbins,  // #bins
                               h2.m_yedges.low,    // low edge
                               h2.m_yedges.high )
                   : h2.m_xedges.edges.empty() && !h2.m_xedges.edges.empty()
                         ? new HISTO2( "",                  // h1.m_name.c_str   ()         ,           // NAME
                                       h2.m_title.c_str(),  // TITLE
                                       h2.m_xedges.nbins,   // #bins
                                       h2.m_xedges.low,     // low edge
                                       h2.m_xedges.high,    // high edge
                                       h2.m_yedges.nBins(), // #bins
                                       &h2.m_yedges.edges.front() )
                         : // vector of edges
                         !h2.m_xedges.edges.empty() && h2.m_xedges.edges.empty()
                         ? new HISTO2( "",                         // h1.m_name.c_str   ()         ,           // NAME
                                       h2.m_title.c_str(),         // TITLE
                                       h2.m_xedges.nBins(),        // #bins
                                       &h2.m_xedges.edges.front(), // vector of edges
                                       h2.m_yedges.nbins,          // #bins
                                       h2.m_yedges.low,            // low edge
                                       h2.m_yedges.high )
                         :                                           // high edge
                         new HISTO2( "",                             // h1.m_name.c_str   ()         ,           // NAME
                                     h2.m_title.c_str(),             // TITLE
                                     h2.m_xedges.nBins(),            // #bins
                                     &h2.m_xedges.edges.front(),     // vector of edges
                                     h2.m_yedges.nBins(),            // #bins
                                     &h2.m_yedges.edges.front() ) ); // vector of edges

    int       ibin  = 0;
    const int xBins = h2.m_xedges.nBins();
    const int yBins = h2.m_yedges.nBins();

    for ( int jBin = yBins + 1; jBin >= 0; --jBin ) {
      for ( int iBin = 0; iBin <= xBins + 1; ++iBin ) {
        histo->SetBinContent( iBin, jBin, h2.m_bins[ibin].first );
        histo->SetBinError( iBin, jBin, h2.m_bins[ibin].second );
        ++ibin;
      }
    }
    //
    name = h2.m_name;
    //
    return histo;
  }
  // ==========================================================================
  template <class HISTO3>
  std::unique_ptr<HISTO3> _parse_3D( std::string_view input, std::string& name ) {
    //
    typedef std::unique_ptr<HISTO3> H3P;
    // 1) parse the custom format
    //
    H3         h3;
    StatusCode sc = _parse( h3, input );
    if ( sc.isFailure() || !h3.ok() ) { return H3P(); } // RETURN
    //
    // 2) create the histogram
    //
    H3P histo( h3.m_xedges.edges.empty() || h3.m_yedges.edges.empty() || h3.m_zedges.edges.empty() ? // FIXED binning?
                   new HISTO3( "",                 // h3.m_name.c_str   ()         ,           // NAME
                               h3.m_title.c_str(), // TITLE
                               h3.m_xedges.nbins,  // #bins
                               h3.m_xedges.low,    // low edge
                               h3.m_xedges.high,   // high edge
                               h3.m_yedges.nbins,  // #bins
                               h3.m_yedges.low,    // low edge
                               h3.m_yedges.high,   // high edge
                               h3.m_zedges.nbins,  // #bins
                               h3.m_zedges.low,    // low edge
                               h3.m_zedges.high )
                                                                                                   : // high edge
                   new HISTO3( "",                         // h3.m_name.c_str   ()         ,           // NAME
                               h3.m_title.c_str(),         // TITLE
                               h3.m_xedges.nBins(),        // #bins
                               &h3.m_xedges.edges.front(), // vector of edges
                               h3.m_yedges.nBins(),        // #bins
                               &h3.m_yedges.edges.front(), h3.m_zedges.nBins(),
                               &h3.m_zedges.edges.front() ) ); // vector of edges

    int       ibin  = 0;
    const int xBins = h3.m_xedges.nBins();
    const int yBins = h3.m_yedges.nBins();
    const int zBins = h3.m_yedges.nBins();

    for ( int kBin = 0; kBin <= zBins + 1; ++kBin ) {
      for ( int jBin = yBins + 1; jBin >= 0; --jBin ) {
        for ( int iBin = 0; iBin <= xBins + 1; ++iBin ) {
          histo->SetBinContent( iBin, jBin, kBin, h3.m_bins[ibin].first );
          histo->SetBinError( iBin, jBin, kBin, h3.m_bins[ibin].second );
          ++ibin;
        }
      }
    }
    //
    name = h3.m_name;
    //
    return histo;
  }
  // ==========================================================================
} //                                                 end of anonymous namespace
// ============================================================================
/*  parse ROOT histogram from text representation
 *  @param result (OUTPUT) the histogram
 *  @param input  (INPUT)  the input to be parsed
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Parsers::parse( TH1D& result, std::string_view input ) {
  // 1) check the parsing
  std::string name;
  //
  auto h1 = _parse_1D<TH1D>( input, name );
  if ( h1 ) {
    result.Reset();
    h1->Copy( result ); // ASSIGN
    result.SetName( name.c_str() );
    return StatusCode::SUCCESS; // RETURN
  }
  //
  // XML-like text?
  return ( std::string::npos != input.find( '<' ) ) ? Gaudi::Utils::Histos::fromXml( result, input )
                                                    : StatusCode::FAILURE;
}
// ============================================================================
/*  parse ROOT histogram from text representation
 *  @param result (OUTPUT) the histogram
 *  @param input  (INPUT)  the input to be parsed
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Parsers::parse( TH1F& result, std::string_view input ) {
  // 1) check the parsing
  std::string name;
  //
  auto h1 = _parse_1D<TH1F>( input, name );
  if ( h1 ) {
    result.Reset();
    h1->Copy( result ); // ASSIGN
    result.SetName( name.c_str() );
    return StatusCode::SUCCESS; // RETURN
  }
  //
  // XML-like text?
  return ( std::string::npos != input.find( '<' ) ) ? Gaudi::Utils::Histos::fromXml( result, input )
                                                    : StatusCode::FAILURE;
}
// ============================================================================
/*  parse ROOT histogram from text representation
 *  @param result (OUTPUT) the histogram
 *  @param input  (INPUT)  the input to be parsed
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Parsers::parse( TH2D& result, std::string_view input ) {
  // 1) check the parsing
  std::string name;
  auto        h2 = _parse_2D<TH2D>( input, name );
  if ( h2 ) {
    result.Reset();
    h2->Copy( result ); // ASSIGN
    result.SetName( name.c_str() );
    return StatusCode::SUCCESS; // RETURN
  }
  //
  // XML-like text?
  return ( std::string::npos != input.find( '<' ) ) ? Gaudi::Utils::Histos::fromXml( result, input )
                                                    : StatusCode::FAILURE;
}
// ============================================================================
/*  parse ROOT histogram from text representation
 *  @param result (OUTPUT) the histogram
 *  @param input  (INPUT)  the input to be parsed
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Parsers::parse( TH2F& result, std::string_view input ) {
  // 1) check the parsing
  std::string name;
  auto        h2 = _parse_2D<TH2F>( input, name );
  if ( h2 ) {
    result.Reset();
    h2->Copy( result ); // ASSIGN
    result.SetName( name.c_str() );
    return StatusCode::SUCCESS; // RETURN
  }
  //
  // XML-like text?
  if ( std::string::npos != input.find( '<' ) ) { return Gaudi::Utils::Histos::fromXml( result, input ); }
  //
  return StatusCode::FAILURE;
}
// ============================================================================
/*  parse ROOT histogram from text representation
 *  @param result (OUTPUT) the histogram
 *  @param input  (INPUT)  the input to be parsed
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Parsers::parse( TH3D& result, std::string_view input ) {
  // 1) check the parsing
  std::string name;
  auto        h3 = _parse_3D<TH3D>( input, name );
  if ( h3 ) {
    result.Reset();
    h3->Copy( result ); // ASSIGN
    result.SetName( name.c_str() );
    return StatusCode::SUCCESS; // RETURN
  }
  //
  // XML-like text?
  return ( std::string::npos != input.find( '<' ) ) ? Gaudi::Utils::Histos::fromXml( result, input )
                                                    : StatusCode::FAILURE;
}
// ============================================================================
/*  parse ROOT histogram from text representation
 *  @param result (OUTPUT) the histogram
 *  @param input  (INPUT)  the input to be parsed
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Parsers::parse( TH3F& result, std::string_view input ) {
  // 1) check the parsing
  std::string name;
  auto        h3 = _parse_3D<TH3F>( input, name );
  if ( h3 ) {
    result.Reset();
    h3->Copy( result ); // ASSIGN
    result.SetName( name.c_str() );
    return StatusCode::SUCCESS; // RETURN
  }
  //
  // XML-like text?
  if ( std::string::npos != input.find( '<' ) ) { return Gaudi::Utils::Histos::fromXml( result, input ); }
  //
  return StatusCode::FAILURE;
}
// ============================================================================
/*  parse ROOT histogram from text representation
 *  @param result (OUTPUT) the histogram
 *  @param input  (INPUT)  the input to be parsed
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Parsers::parse( TH1D*& result, std::string_view input ) {
  if ( result ) { return parse( *result, input ); } // RETURN

  // 1) check the parsing
  std::string name;
  auto        h1 = _parse_1D<TH1D>( input, name );
  if ( h1 ) {
    result = h1.release();
    result->SetName( name.c_str() );
    return StatusCode::SUCCESS; // RETURN
  }
  //
  // XML-like text?
  return ( std::string::npos != input.find( '<' ) ) ? Gaudi::Utils::Histos::fromXml( result, input )
                                                    : StatusCode::FAILURE;
}
// ============================================================================
/*  parse ROOT histogram from text representation
 *  @param result (OUTPUT) the histogram
 *  @param input  (INPUT)  the input to be parsed
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Parsers::parse( TH2D*& result, std::string_view input ) {
  if ( result ) { return parse( *result, input ); } // RETURN

  // 1) check the parsing
  std::string name;
  auto        h2 = _parse_2D<TH2D>( input, name );
  if ( h2 ) {
    result = h2.release();
    result->SetName( name.c_str() );
    return StatusCode::SUCCESS; // RETURN
  }
  //
  // XML-like text?
  return ( std::string::npos != input.find( '<' ) ) ? Gaudi::Utils::Histos::fromXml( result, input )
                                                    : StatusCode::FAILURE;
}
// ============================================================================
/*  parse ROOT histogram from text representation
 *  @param result (OUTPUT) the histogram
 *  @param input  (INPUT)  the input to be parsed
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Parsers::parse( TH3D*& result, std::string_view input ) {
  if ( result ) { return parse( *result, input ); } // RETURN

  // 1) check the parsing
  std::string name;
  auto        h3 = _parse_3D<TH3D>( input, name );
  if ( h3 ) {
    result = h3.release();
    result->SetName( name.c_str() );
    return StatusCode::SUCCESS; // RETURN
  }
  //
  // XML-like text?
  return ( std::string::npos != input.find( '<' ) ) ? Gaudi::Utils::Histos::fromXml( result, input )
                                                    : StatusCode::FAILURE;
}
// ============================================================================
/*  parse AIDA histogram from text representation
 *  @param result (OUTPUT) the histogram
 *  @param input  (INPUT)  the input to be parsed
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Parsers::parse( AIDA::IHistogram1D& result, std::string_view input ) {
  // 1) convert to ROOT
  auto root = Gaudi::Utils::Aida2ROOT::aida2root( &result );
  // 2) read ROOT histogram
  return root ? parse( *root, input ) : StatusCode::FAILURE;
}
// ============================================================================
/*  parse AIDA histogram from text representation
 *  @param result (OUTPUT) the histogram
 *  @param input  (INPUT)  the input to be parsed
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Parsers::parse( AIDA::IHistogram2D& result, std::string_view input ) {
  // 1) convert to ROOT
  auto root = Gaudi::Utils::Aida2ROOT::aida2root( &result );
  // 2) read ROOT histogram
  return root ? parse( *root, input ) : StatusCode::FAILURE;
}
// ============================================================================
/*  parse AIDA histogram from text representation
 *  @param result (OUTPUT) the histogram
 *  @param input  (INPUT)  the input to be parsed
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Parsers::parse( AIDA::IHistogram3D& result, std::string_view input ) {
  // 1) convert to ROOT
  auto root = Gaudi::Utils::Aida2ROOT::aida2root( &result );
  // 2) read ROOT histogram
  return root ? parse( *root, input ) : StatusCode::FAILURE;
}
// ============================================================================
// The END
// ============================================================================
