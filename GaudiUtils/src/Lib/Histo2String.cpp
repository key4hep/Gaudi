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
// ============================================================================
// Include files
// ============================================================================
// ROOT
// ============================================================================
#include <TH1D.h>
#include <TH1F.h>
#include <TH2D.h>
#include <TH2F.h>
#include <TH3D.h>
#include <TH3F.h>
// ============================================================================
// AIDA
// ============================================================================
#include <AIDA/IHistogram1D.h>
#include <AIDA/IHistogram2D.h>
#include <AIDA/IHistogram3D.h>
// ============================================================================
// GaudiKernel
// ============================================================================
#include <GaudiKernel/ToStream.h>
// ============================================================================
// local
// ============================================================================
#include <GaudiUtils/Aida2ROOT.h>
#include <GaudiUtils/Histo2String.h>
#include <GaudiUtils/HistoXML.h>
// ============================================================================
/** @file
 *  Implementation file for utilities Histo -> string
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-10-21
 */
// ============================================================================
namespace {
  // ==========================================================================
  template <class HISTO>
  std::ostream& _toStream_1D_( const HISTO& histo, std::ostream& stream, const bool asXML ) {
    if ( asXML ) { return Gaudi::Utils::Histos::toXml( histo, stream ); }
    //
    stream << "{ ";
    //
    stream << "'name'  : ";
    Gaudi::Utils::toStream( std::string( histo.GetName() ), stream ) << " , ";
    stream << "'title' : ";
    Gaudi::Utils::toStream( std::string( histo.GetTitle() ), stream ) << " , ";
    //
    const TAxis*       axis  = histo.GetXaxis();
    const unsigned int nBins = axis->GetNbins();
    //
    stream << std::endl << "'X' : { ";
    if ( axis->IsVariableBinSize() ) {
      const TArrayD*      xbins = axis->GetXbins();
      const unsigned int  xsize = xbins->GetSize();
      std::vector<double> edges;
      for ( unsigned int iBin = 0; iBin < xsize; ++iBin ) { edges.push_back( xbins->At( iBin ) ); }
      stream << "'edges' : ";
      Gaudi::Utils::toStream( edges, stream ) << " }, " << std::endl;
    } else {
      stream << "'nbins' : ";
      Gaudi::Utils::toStream( nBins, stream ) << "  , ";
      stream << "'low'   : ";
      Gaudi::Utils::toStream( axis->GetXmin(), stream ) << "  , ";
      stream << "'high'  : ";
      Gaudi::Utils::toStream( axis->GetXmax(), stream ) << " }, " << std::endl;
    }
    // finally: the content
    std::vector<std::pair<double, double>> bins;
    for ( unsigned int iBin = 0; iBin <= nBins + 1; ++iBin ) {
      bins.emplace_back( histo.GetBinContent( iBin ), histo.GetBinError( iBin ) );
    }
    stream << "'bins'  : ";
    Gaudi::Utils::toStream( bins, stream );
    //
    stream << " }";
    //
    return stream;
  }
  // ==========================================================================
  template <class HISTO>
  std::ostream& _toStream_2D_( const HISTO& histo, std::ostream& stream, const bool asXML ) {
    if ( asXML ) { return Gaudi::Utils::Histos::toXml( histo, stream ); }
    //
    stream << "{ ";
    //
    stream << "'name'  : ";
    Gaudi::Utils::toStream( std::string( histo.GetName() ), stream ) << " , ";
    stream << "'title' : ";
    Gaudi::Utils::toStream( std::string( histo.GetTitle() ), stream ) << " , ";
    //
    const TAxis* xaxis = histo.GetXaxis();
    const int    xBins = xaxis->GetNbins();
    //
    stream << std::endl << "'X' : { ";
    if ( xaxis->IsVariableBinSize() ) {
      const TArrayD*      xbins = xaxis->GetXbins();
      const unsigned int  xsize = xbins->GetSize();
      std::vector<double> edges;
      for ( unsigned int iBin = 0; iBin < xsize; ++iBin ) { edges.push_back( xbins->At( iBin ) ); }
      // the edges
      stream << "'edges' : ";
      Gaudi::Utils::toStream( edges, stream ) << " }," << std::endl;
    } else {
      stream << "'nbins' : ";
      Gaudi::Utils::toStream( xBins, stream ) << " , ";
      stream << "'low'   : ";
      Gaudi::Utils::toStream( xaxis->GetXmin(), stream ) << " , ";
      stream << "'high'  : ";
      Gaudi::Utils::toStream( xaxis->GetXmax(), stream ) << " }, " << std::endl;
    }
    //
    const TAxis* yaxis = histo.GetYaxis();
    const int    yBins = yaxis->GetNbins();
    //
    stream << std::endl << "'Y' : { ";
    if ( yaxis->IsVariableBinSize() ) {
      const TArrayD*      ybins = yaxis->GetXbins();
      const unsigned int  ysize = ybins->GetSize();
      std::vector<double> edges;
      for ( unsigned int iBin = 0; iBin < ysize; ++iBin ) { edges.push_back( ybins->At( iBin ) ); }
      // the edges
      stream << " 'edges' : ";
      Gaudi::Utils::toStream( edges, stream ) << " }," << std::endl;
    } else {
      stream << "'nbins' : ";
      Gaudi::Utils::toStream( yBins, stream ) << " , ";
      stream << "'low'   : ";
      Gaudi::Utils::toStream( yaxis->GetXmin(), stream ) << " , ";
      stream << "'high'  : ";
      Gaudi::Utils::toStream( yaxis->GetXmax(), stream ) << " }, " << std::endl;
    }
    //
    // finally: the content
    stream << "'bins' : " << std::endl << " [ ";
    for ( int jBin = yBins + 1; jBin >= 0; --jBin ) {
      if ( yBins + 1 != jBin ) { stream << std::endl; }
      for ( int iBin = 0; iBin <= xBins + 1; ++iBin ) {
        //
        Gaudi::Utils::toStream( std::make_pair( histo.GetBinContent( iBin, jBin ), histo.GetBinError( iBin, jBin ) ),
                                stream );
        //
        if ( xBins + 1 != iBin || 0 != jBin ) { stream << " , "; }
      }
    }
    stream << " ]";
    //
    stream << " }";
    //
    return stream;
  }
  // ==========================================================================
  template <class HISTO>
  std::ostream& _toStream_3D_( const HISTO& histo, std::ostream& stream, const bool asXML ) {
    if ( asXML ) { return Gaudi::Utils::Histos::toXml( histo, stream ); }
    //
    stream << "{ ";
    //
    stream << "'name'  : ";
    Gaudi::Utils::toStream( std::string( histo.GetName() ), stream ) << " , ";
    stream << "'title' : ";
    Gaudi::Utils::toStream( std::string( histo.GetTitle() ), stream ) << " , ";
    //
    const TAxis* xaxis = histo.GetXaxis();
    const int    xBins = xaxis->GetNbins();
    //
    stream << std::endl << "'X' : { ";
    if ( xaxis->IsVariableBinSize() ) {
      const TArrayD*      xbins = xaxis->GetXbins();
      const unsigned int  xsize = xbins->GetSize();
      std::vector<double> edges;
      for ( unsigned int iBin = 0; iBin < xsize; ++iBin ) { edges.push_back( xbins->At( iBin ) ); }
      // the edges
      stream << "'edges' : ";
      Gaudi::Utils::toStream( edges, stream ) << " }," << std::endl;
    } else {
      stream << "'nbins' : ";
      Gaudi::Utils::toStream( xBins, stream ) << " , ";
      stream << "'low'   : ";
      Gaudi::Utils::toStream( xaxis->GetXmin(), stream ) << " , ";
      stream << "'high'  : ";
      Gaudi::Utils::toStream( xaxis->GetXmax(), stream ) << " }, " << std::endl;
    }
    //
    const TAxis* yaxis = histo.GetYaxis();
    const int    yBins = yaxis->GetNbins();
    //
    stream << std::endl << "'Y' : { ";
    if ( yaxis->IsVariableBinSize() ) {
      const TArrayD*      ybins = yaxis->GetXbins();
      const unsigned int  ysize = ybins->GetSize();
      std::vector<double> edges;
      for ( unsigned int iBin = 0; iBin < ysize; ++iBin ) { edges.push_back( ybins->At( iBin ) ); }
      // the edges
      stream << " 'edges' : ";
      Gaudi::Utils::toStream( edges, stream ) << " }," << std::endl;
    } else {
      stream << "'nbins' : ";
      Gaudi::Utils::toStream( yBins, stream ) << " , ";
      stream << "'low'   : ";
      Gaudi::Utils::toStream( yaxis->GetXmin(), stream ) << " , ";
      stream << "'high'  : ";
      Gaudi::Utils::toStream( yaxis->GetXmax(), stream ) << " }, " << std::endl;
    }
    //
    const TAxis* zaxis = histo.GetZaxis();
    const int    zBins = zaxis->GetNbins();
    //
    stream << std::endl << "'Z' : { ";
    if ( zaxis->IsVariableBinSize() ) {
      const TArrayD*      zbins = zaxis->GetXbins();
      const unsigned int  zsize = zbins->GetSize();
      std::vector<double> edges;
      for ( unsigned int iBin = 0; iBin < zsize; ++iBin ) { edges.push_back( zbins->At( iBin ) ); }
      // the edges
      stream << " 'edges' : ";
      Gaudi::Utils::toStream( edges, stream ) << " }," << std::endl;
    } else {
      stream << "'nbins' : ";
      Gaudi::Utils::toStream( zBins, stream ) << " , ";
      stream << "'low'   : ";
      Gaudi::Utils::toStream( zaxis->GetXmin(), stream ) << " , ";
      stream << "'high'  : ";
      Gaudi::Utils::toStream( zaxis->GetXmax(), stream ) << " }, " << std::endl;
    }
    //
    // finally: the content
    stream << "'bins' : " << std::endl << " [ ";
    for ( int kBin = 0; kBin <= zBins + 1; ++kBin ) {
      for ( int jBin = yBins + 1; jBin >= 0; --jBin ) {
        if ( yBins + 1 != jBin ) { stream << std::endl; }
        for ( int iBin = 0; iBin <= xBins + 1; ++iBin ) {
          //
          Gaudi::Utils::toStream(
              std::make_pair( histo.GetBinContent( iBin, jBin, kBin ), histo.GetBinError( iBin, jBin, kBin ) ),
              stream );
          //
          if ( !( ( ( xBins + 1 ) == iBin ) && ( 0 == jBin ) && ( ( zBins + 1 ) == kBin ) ) ) { stream << " , "; }
        }
      }
    }
    stream << " ]";
    //
    stream << " }";
    //
    return stream;
  }
  // ==========================================================================
} //                                                 end of anonymous namespace
// ============================================================================
/*  stream the ROOT histogram into output stream
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param stream (OUTPUT) the stream
 *  @param asXML  (INPUT)  use XML-format
 */
// ============================================================================
std::ostream& Gaudi::Utils::toStream( const TH1D& histo, std::ostream& stream, const bool asXML ) {
  return _toStream_1D_( histo, stream, asXML );
}
// ============================================================================
/*  stream the ROOT histogram into output stream
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param stream (OUTPUT) the stream
 *  @param asXML  (INPUT)  use XML-format
 */
// ============================================================================
std::ostream& Gaudi::Utils::toStream( const TH1F& histo, std::ostream& stream, const bool asXML ) {
  return _toStream_1D_( histo, stream, asXML );
}
// ============================================================================
/*  stream the ROOT histogram into output stream
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param stream (OUTPUT) the stream
 *  @param asXML  (INPUT)  use XML-format
 */
// ============================================================================
std::ostream& Gaudi::Utils::toStream( const TH2D& histo, std::ostream& stream, const bool asXML ) {
  return _toStream_2D_( histo, stream, asXML );
}
// ============================================================================
/*  stream the ROOT histogram into output stream
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param stream (OUTPUT) the stream
 *  @param asXML  (INPUT)  use XML-format
 */
// ============================================================================
std::ostream& Gaudi::Utils::toStream( const TH2F& histo, std::ostream& stream, const bool asXML ) {
  return _toStream_2D_( histo, stream, asXML );
}
// ============================================================================
/*  stream the ROOT histogram into output stream
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param stream (OUTPUT) the stream
 *  @param asXML  (INPUT)  use XML-format
 */
// ============================================================================
std::ostream& Gaudi::Utils::toStream( const TH3D& histo, std::ostream& stream, const bool asXML ) {
  return _toStream_3D_( histo, stream, asXML );
}
// ============================================================================
/*  stream the ROOT histogram into output stream
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param stream (OUTPUT) the stream
 *  @param asXML  (INPUT)  use XML-format
 */
// ============================================================================
std::ostream& Gaudi::Utils::toStream( const TH3F& histo, std::ostream& stream, const bool asXML ) {
  return _toStream_3D_( histo, stream, asXML );
}
// ============================================================================
/*  stream the AIDA histogram into output stream
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param stream (OUTPUT) the stream
 *  @param asXML  (INPUT)  use XML-format
 */
// ============================================================================
std::ostream& Gaudi::Utils::toStream( const AIDA::IHistogram1D& histo, std::ostream& stream, const bool asXML ) {
  auto root = Gaudi::Utils::Aida2ROOT::aida2root( &histo );
  return root ? toStream( *root, stream, asXML ) : stream;
}
// ============================================================================
/*  stream the AIDA histogram into output stream
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param stream (OUTPUT) the stream
 *  @param asXML  (INPUT)  use XML-format
 */
// ============================================================================
std::ostream& Gaudi::Utils::toStream( const AIDA::IHistogram2D& histo, std::ostream& stream, const bool asXML ) {
  //
  auto root = Gaudi::Utils::Aida2ROOT::aida2root( &histo );
  return root ? toStream( *root, stream, asXML ) : stream;
}
// ============================================================================
/*  stream the AIDA histogram into output stream
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param stream (OUTPUT) the stream
 *  @param asXML  (INPUT)  use XML-format
 */
// ============================================================================
std::ostream& Gaudi::Utils::toStream( const AIDA::IHistogram3D& histo, std::ostream& stream, const bool asXML ) {
  //
  auto root = Gaudi::Utils::Aida2ROOT::aida2root( &histo );
  return root ? toStream( *root, stream, asXML ) : stream;
}

// ============================================================================
/*  convert the histogram into the string
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param asXML  (INPUT)  use XML-format
 *  @return the string representation of the histogram
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-26
 */
// ============================================================================
std::string Gaudi::Utils::toString( const TH1D& histo, const bool asXML ) {
  std::ostringstream o;
  toStream( histo, o, asXML );
  return o.str();
}
// ============================================================================
/*  convert the histogram into the string
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param asXML  (INPUT)  use XML-format
 *  @return the string representation of the histogram
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-26
 */
// ============================================================================
std::string Gaudi::Utils::toString( const TH1F& histo, const bool asXML ) {
  std::ostringstream o;
  toStream( histo, o, asXML );
  return o.str();
}
// ============================================================================
/*  convert the histogram into the string
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param asXML  (INPUT)  use XML-format
 *  @return the string representation of the histogram
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-26
 */
// ============================================================================
std::string Gaudi::Utils::toString( const TH2D& histo, const bool asXML ) {
  std::ostringstream o;
  toStream( histo, o, asXML );
  return o.str();
}
// ============================================================================
/*  convert the histogram into the string
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param asXML  (INPUT)  use XML-format
 *  @return the string representation of the histogram
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-26
 */
// ============================================================================
std::string Gaudi::Utils::toString( const TH2F& histo, const bool asXML ) {
  std::ostringstream o;
  toStream( histo, o, asXML );
  return o.str();
}
// ============================================================================
/*  convert the histogram into the string
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param asXML  (INPUT)  use XML-format
 *  @return the string representation of the histogram
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-26
 */
// ============================================================================
std::string Gaudi::Utils::toString( const TH3D& histo, const bool asXML ) {
  std::ostringstream o;
  toStream( histo, o, asXML );
  return o.str();
}
// ============================================================================
/*  convert the histogram into the string
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param asXML  (INPUT)  use XML-format
 *  @return the string representation of the histogram
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-26
 */
// ============================================================================
std::string Gaudi::Utils::toString( const TH3F& histo, const bool asXML ) {
  std::ostringstream o;
  toStream( histo, o, asXML );
  return o.str();
}
// ============================================================================
/*  convert the histogram into the string
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param asXML  (INPUT)  use XML-format
 *  @return the string representation of the histogram
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-26
 */
// ============================================================================
std::string Gaudi::Utils::toString( const AIDA::IHistogram1D& histo, const bool asXML ) {
  std::ostringstream o;
  toStream( histo, o, asXML );
  return o.str();
}
// =============================================================================
/*  convert the histogram into the string
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param asXML  (INPUT)  use XML-format
 *  @return the string representation of the histogram
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-26
 */
// ============================================================================
std::string Gaudi::Utils::toString( const AIDA::IHistogram2D& histo, const bool asXML ) {
  std::ostringstream o;
  toStream( histo, o, asXML );
  return o.str();
}
// =============================================================================
/*  convert the histogram into the string
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param asXML  (INPUT)  use XML-format
 *  @return the string representation of the histogram
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-26
 */
// ============================================================================
std::string Gaudi::Utils::toString( const AIDA::IHistogram3D& histo, const bool asXML ) {
  std::ostringstream o;
  toStream( histo, o, asXML );
  return o.str();
}
// =============================================================================
/*  convert the histogram into the string
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param asXML  (INPUT)  use XML-format
 *  @return the string representation of the histogram
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-26
 */
// =============================================================================
std::string Gaudi::Utils::toString( const AIDA::IHistogram1D* histo ) { return histo ? toString( *histo ) : "{}"; }
// ============================================================================
/*  convert the histogram into the string
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param asXML  (INPUT)  use XML-format
 *  @return the string representation of the histogram
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-26
 */
// =============================================================================
std::string Gaudi::Utils::toString( AIDA::IHistogram1D* histo ) { return histo ? toString( *histo ) : "{}"; }
// ============================================================================
/*  convert the histogram into the string
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param asXML  (INPUT)  use XML-format
 *  @return the string representation of the histogram
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-26
 */
// =============================================================================
std::string Gaudi::Utils::toString( const TH1D* histo ) { return histo ? toString( *histo ) : "{}"; }
// ============================================================================
/*  convert the histogram into the string
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param asXML  (INPUT)  use XML-format
 *  @return the string representation of the histogram
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-26
 */
// =============================================================================
std::string Gaudi::Utils::toString( const TH2D* histo ) { return histo ? toString( *histo ) : "{}"; }
// ============================================================================
/*  convert the histogram into the string
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param asXML  (INPUT)  use XML-format
 *  @return the string representation of the histogram
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-26
 */
// =============================================================================
std::string Gaudi::Utils::toString( TH1D* histo ) { return histo ? toString( *histo ) : "{}"; }
// ============================================================================
/*  convert the histogram into the string
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param asXML  (INPUT)  use XML-format
 *  @return the string representation of the histogram
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-26
 */
// =============================================================================
std::string Gaudi::Utils::toString( TH2D* histo ) { return histo ? toString( *histo ) : "{}"; }
// ============================================================================
/*  convert the histogram into the string
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param asXML  (INPUT)  use XML-format
 *  @return the string representation of the histogram
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-26
 */
// =============================================================================
std::string Gaudi::Utils::toString( const TH3D* histo ) { return histo ? toString( *histo ) : "{}"; }
// ============================================================================
/*  convert the histogram into the string
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param asXML  (INPUT)  use XML-format
 *  @return the string representation of the histogram
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-26
 */
// =============================================================================
std::string Gaudi::Utils::toString( TH3D* histo ) { return histo ? toString( *histo ) : "{}"; }
// ============================================================================
// The END
// ============================================================================
