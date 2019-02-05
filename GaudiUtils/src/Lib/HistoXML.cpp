// ============================================================================
// Include files
// ============================================================================
// STD&STL
// ============================================================================
#include <memory>
// ============================================================================
// ROOT
// ============================================================================
#include "TBufferXML.h"
#include "TH1.h"
#include "TH1D.h"
#include "TH1F.h"
#include "TH2D.h"
#include "TH2F.h"
#include "TH3D.h"
#include "TH3F.h"
#include "TProfile.h"
#include "TProfile2D.h"
// ============================================================================
// AIDA
// ============================================================================
#include "AIDA/IHistogram1D.h"
#include "AIDA/IHistogram2D.h"
#include "AIDA/IHistogram3D.h"
#include "AIDA/IProfile1D.h"
#include "AIDA/IProfile2D.h"
// ============================================================================
// local
// ============================================================================
#include "GaudiUtils/Aida2ROOT.h"
#include "GaudiUtils/HistoXML.h"
// ============================================================================
/** @file
 *  Implementation file for histogram --> XML conversion functions
 *  @date 2009-10-21
 *  @author Vanya Belyaev Ivan.Belyaev@nikhef.nl
 */
// ============================================================================
namespace {
  // ==========================================================================
  /// parse the histogram from xml
  //
  template <typename TYPE>
  std::unique_ptr<TYPE> _Xml( const std::string& input ) {
    // 1) use XML-parser
    std::unique_ptr<TObject> obj{TBufferXML::ConvertFromXML( input.c_str() )};
    TYPE*                    histo = ( obj ? dynamic_cast<TYPE*>( obj.get() ) : nullptr );
    // slightly tricky: in case the dynamic cast succeeds, transfer owership
    // by invoking 'release' on obj, and 'at the same time' pass 'histo' into
    // the constructor of unique_ptr -- but if the cast fails, do NOT transfer
    // ownership... the , operator is the closest I can think of
    return std::unique_ptr<TYPE>{histo ? ( obj.release(), histo ) : nullptr};
  }
  //
  // ==========================================================================
} //                                                 end of anonymous namespace
// ============================================================================
/*  stream the ROOT histogram into output stream as XML
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param stream (OUTPUT) the stream
 */
// ============================================================================
std::ostream& Gaudi::Utils::Histos::toXml( const TH1D& histo, std::ostream& stream ) {
  return stream << TBufferXML::ConvertToXML( &histo );
}
// ============================================================================
/*  stream the ROOT histogram into output stream as XML
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param stream (OUTPUT) the stream
 */
// ============================================================================
std::ostream& Gaudi::Utils::Histos::toXml( const TH2D& histo, std::ostream& stream ) {
  return stream << TBufferXML::ConvertToXML( &histo );
}
// ============================================================================
/*  stream the ROOT histogram into output stream as XML
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param stream (OUTPUT) the stream
 */
// ============================================================================
std::ostream& Gaudi::Utils::Histos::toXml( const TH3D& histo, std::ostream& stream ) {
  return stream << TBufferXML::ConvertToXML( &histo );
}
// ============================================================================
/*  stream the ROOT histogram into output stream as XML
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param stream (OUTPUT) the stream
 */
// ============================================================================
std::ostream& Gaudi::Utils::Histos::toXml( const TH1F& histo, std::ostream& stream ) {
  return stream << TBufferXML::ConvertToXML( &histo );
}
// ============================================================================
/*  stream the ROOT histogram into output stream as XML
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param stream (OUTPUT) the stream
 */
// ============================================================================
std::ostream& Gaudi::Utils::Histos::toXml( const TH2F& histo, std::ostream& stream ) {
  return stream << TBufferXML::ConvertToXML( &histo );
}
// ============================================================================
/*  stream the ROOT histogram into output stream as XML
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param stream (OUTPUT) the stream
 */
// ============================================================================
std::ostream& Gaudi::Utils::Histos::toXml( const TH3F& histo, std::ostream& stream ) {
  return stream << TBufferXML::ConvertToXML( &histo );
}
// ============================================================================
/*  stream the ROOT histogram into output stream as XML
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param stream (OUTPUT) the stream
 */
// ============================================================================
std::ostream& Gaudi::Utils::Histos::toXml( const TProfile& histo, std::ostream& stream ) {
  return stream << TBufferXML::ConvertToXML( &histo );
}
// ============================================================================
/*  stream the ROOT histogram into output stream as XML
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param stream (OUTPUT) the stream
 */
// ============================================================================
std::ostream& Gaudi::Utils::Histos::toXml( const TProfile2D& histo, std::ostream& stream ) {
  return stream << TBufferXML::ConvertToXML( &histo );
}
// ============================================================================
/* stream the ROOT histogram into the output stream as XML
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param stream (OUTPUT) the stream
 */
// ============================================================================
std::ostream& Gaudi::Utils::Histos::toXml( const AIDA::IHistogram1D& histo, std::ostream& stream ) {
  auto root = Gaudi::Utils::Aida2ROOT::aida2root( &histo );
  return root ? toXml( *root, stream ) : stream;
}
// ============================================================================
/* stream the ROOT histogram into the output stream as XML
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param stream (OUTPUT) the stream
 */
// ============================================================================
std::ostream& Gaudi::Utils::Histos::toXml( const AIDA::IHistogram2D& histo, std::ostream& stream ) {
  auto root = Gaudi::Utils::Aida2ROOT::aida2root( &histo );
  return root ? toXml( *root, stream ) : stream;
}
// ============================================================================
/* stream the ROOT histogram into the output stream as XML
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param stream (OUTPUT) the stream
 */
// ============================================================================
std::ostream& Gaudi::Utils::Histos::toXml( const AIDA::IHistogram3D& histo, std::ostream& stream ) {
  auto root = Gaudi::Utils::Aida2ROOT::aida2root( &histo );
  return root ? toXml( *root, stream ) : stream;
}
// ============================================================================
/* stream the ROOT histogram into the output stream as XML
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param stream (OUTPUT) the stream
 */
// ============================================================================
std::ostream& Gaudi::Utils::Histos::toXml( const AIDA::IProfile1D& histo, std::ostream& stream ) {
  auto root = Gaudi::Utils::Aida2ROOT::aida2root( &histo );
  return root ? toXml( *root, stream ) : stream;
}
// ============================================================================
/* stream the ROOT histogram into the output stream as XML
 *  @param histo  (INPUT)  the histogram to be streamed
 *  @param stream (OUTPUT) the stream
 */
// ============================================================================
std::ostream& Gaudi::Utils::Histos::toXml( const AIDA::IProfile2D& histo, std::ostream& stream ) {
  auto root = Gaudi::Utils::Aida2ROOT::aida2root( &histo );
  return root ? toXml( *root, stream ) : stream;
}
// ============================================================================
/*  parse the histogram from standard ROOT XML
 *  @param result (OUTPUT)  the parsed histogram
 *  @param input  (INPUT)   the input XML string
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Utils::Histos::fromXml( TH1D& result, const std::string& input ) {
  //
  result.Reset(); // RESET old histogram
  //

  auto histo = _Xml<TH1D>( input );
  if ( !histo ) { return StatusCode::FAILURE; } // RETURN
  //
  histo->Copy( result );
  //
  return StatusCode::SUCCESS;
}
// ============================================================================
/*  parse the histogram from standard ROOT XML
 *  @param result (OUTPUT)  the parsed histogram
 *  @param input  (INPUT)   the input XML string
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Utils::Histos::fromXml( TH2D& result, const std::string& input ) {
  //
  result.Reset(); // RESET old histogram
  //
  auto histo = _Xml<TH2D>( input );
  if ( !histo ) { return StatusCode::FAILURE; } // RETURN
  //
  histo->Copy( result );
  //
  return StatusCode::SUCCESS;
}
// ============================================================================
/*  parse the histogram from standard ROOT XML
 *  @param result (OUTPUT)  the parsed histogram
 *  @param input  (INPUT)   the input XML string
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Utils::Histos::fromXml( TH3D& result, const std::string& input ) {
  //
  result.Reset(); // RESET old histogram
  //
  auto histo = _Xml<TH3D>( input );
  if ( !histo ) { return StatusCode::FAILURE; } // RETURN
  //
  histo->Copy( result );
  //
  return StatusCode::SUCCESS;
}
// ============================================================================
/*  parse the histogram from standard ROOT XML
 *  @param result (OUTPUT)  the parsed histogram
 *  @param input  (INPUT)   the input XML string
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Utils::Histos::fromXml( TH1F& result, const std::string& input ) {
  //
  result.Reset(); // RESET old histogram
  //
  auto histo = _Xml<TH1F>( input );
  if ( !histo ) { return StatusCode::FAILURE; } // RETURN
  //
  histo->Copy( result );
  //
  return StatusCode::SUCCESS;
}
// ============================================================================
/*  parse the histogram from standard ROOT XML
 *  @param result (OUTPUT)  the parsed histogram
 *  @param input  (INPUT)   the input XML string
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Utils::Histos::fromXml( TH2F& result, const std::string& input ) {
  //
  result.Reset(); // RESET old histogram
  //
  auto histo = _Xml<TH2F>( input );
  if ( !histo ) { return StatusCode::FAILURE; } // RETURN
  //
  histo->Copy( result );
  //
  return StatusCode::SUCCESS;
}
// ============================================================================
/*  parse the histogram from standard ROOT XML
 *  @param result (OUTPUT)  the parsed histogram
 *  @param input  (INPUT)   the input XML string
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Utils::Histos::fromXml( TH3F& result, const std::string& input ) {
  //
  result.Reset(); // RESET old histogram
  //
  auto histo = _Xml<TH3F>( input );
  if ( !histo ) { return StatusCode::FAILURE; } // RETURN
  //
  histo->Copy( result );
  //
  return StatusCode::SUCCESS;
}
// ============================================================================
/*  parse the histogram from standard ROOT XML
 *  @param result (OUTPUT)  the parsed histogram
 *  @param input  (INPUT)   the input XML string
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Utils::Histos::fromXml( TProfile& result, const std::string& input ) {
  //
  result.Reset(); // RESET old histogram
  //
  auto histo = _Xml<TProfile>( input );
  if ( !histo ) { return StatusCode::FAILURE; } // RETURN
  //
  histo->Copy( result );
  //
  return StatusCode::SUCCESS;
}
// ============================================================================
/*  parse the histogram from standard ROOT XML
 *  @param result (OUTPUT)  the parsed histogram
 *  @param input  (INPUT)   the input XML string
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Utils::Histos::fromXml( TProfile2D& result, const std::string& input ) {
  //
  result.Reset(); // RESET old histogram
  //
  auto histo = _Xml<TProfile2D>( input );
  if ( !histo ) { return StatusCode::FAILURE; } // RETURN
  //
  histo->Copy( result );
  //
  return StatusCode::SUCCESS;
}

// ============================================================================
/*  parse the histogram from standard ROOT XML
 *  @param result (OUTPUT)  the parsed histogram
 *  @param input  (INPUT)   the input XML string
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Utils::Histos::fromXml( TH1D*& result, const std::string& input ) {
  if ( result ) { return fromXml( *result, input ); }
  //
  auto histo = _Xml<TH1D>( input );
  if ( !histo ) { return StatusCode::FAILURE; } // RETURN
  //
  result = histo.release(); // ASSIGN
  //
  return StatusCode::SUCCESS;
}
// ============================================================================
/*  parse the histogram from standard ROOT XML
 *  @param result (OUTPUT)  the parsed histogram
 *  @param input  (INPUT)   the input XML string
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Utils::Histos::fromXml( TH2D*& result, const std::string& input ) {
  if ( result ) { return fromXml( *result, input ); }
  //
  auto histo = _Xml<TH2D>( input );
  if ( !histo ) { return StatusCode::FAILURE; } // RETURN
  //
  result = histo.release(); // ASSIGN
  //
  return StatusCode::SUCCESS;
}
// ============================================================================
/*  parse the histogram from standard ROOT XML
 *  @param result (OUTPUT)  the parsed histogram
 *  @param input  (INPUT)   the input XML string
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Utils::Histos::fromXml( TH3D*& result, const std::string& input ) {
  if ( result ) { return fromXml( *result, input ); }
  //
  auto histo = _Xml<TH3D>( input );
  if ( !histo ) { return StatusCode::FAILURE; } // RETURN
  //
  result = histo.release(); // ASSIGN
  //
  return StatusCode::SUCCESS;
}

// ============================================================================
/*  parse the histogram from standard ROOT XML
 *  @param result (OUTPUT)  the parsed histogram
 *  @param input  (INPUT)   the input XML string
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Utils::Histos::fromXml( TProfile*& result, const std::string& input ) {
  if ( result ) { return fromXml( *result, input ); }
  //
  auto histo = _Xml<TProfile>( input );
  if ( !histo ) { return StatusCode::FAILURE; } // RETURN
  //
  result = histo.release(); // ASSIGN
  //
  return StatusCode::SUCCESS;
}
// ============================================================================
/*  parse the histogram from standard ROOT XML
 *  @param result (OUTPUT)  the parsed histogram
 *  @param input  (INPUT)   the input XML string
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Utils::Histos::fromXml( TProfile2D*& result, const std::string& input ) {
  if ( result ) { return fromXml( *result, input ); }
  //
  auto histo = _Xml<TProfile2D>( input );
  if ( !histo ) { return StatusCode::FAILURE; } // RETURN
  //
  result = histo.release(); // ASSIGN
  //
  return StatusCode::SUCCESS;
}
// ============================================================================
/*  parse the histogram from standard ROOT XML
 *  @param result (OUTPUT)  the parsed histogram
 *  @param input  (INPUT)   the input XML string
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Utils::Histos::fromXml( AIDA::IHistogram1D& result, const std::string& input ) {
  auto root = Gaudi::Utils::Aida2ROOT::aida2root( &result );
  return root ? fromXml( *root, input ) : StatusCode::FAILURE; // RETURN
}
// ============================================================================
/*  parse the histogram from standard ROOT XML
 *  @param result (OUTPUT)  the parsed histogram
 *  @param input  (INPUT)   the input XML string
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Utils::Histos::fromXml( AIDA::IHistogram2D& result, const std::string& input ) {
  auto root = Gaudi::Utils::Aida2ROOT::aida2root( &result );
  return root ? fromXml( *root, input ) : StatusCode::FAILURE; // RETURN
}
// ============================================================================
/*  parse the histogram from standard ROOT XML
 *  @param result (OUTPUT)  the parsed histogram
 *  @param input  (INPUT)   the input XML string
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Utils::Histos::fromXml( AIDA::IHistogram3D& result, const std::string& input ) {
  auto root = Gaudi::Utils::Aida2ROOT::aida2root( &result );
  return root ? fromXml( *root, input ) : StatusCode::FAILURE; // RETURN
}
// ============================================================================
/*  parse the histogram from standard ROOT XML
 *  @param result (OUTPUT)  the parsed histogram
 *  @param input  (INPUT)   the input XML string
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Utils::Histos::fromXml( AIDA::IProfile1D& result, const std::string& input ) {
  auto root = Gaudi::Utils::Aida2ROOT::aida2root( &result );
  return root ? fromXml( *root, input ) : StatusCode::FAILURE; // RETURN
}
// ============================================================================
/*  parse the histogram from standard ROOT XML
 *  @param result (OUTPUT)  the parsed histogram
 *  @param input  (INPUT)   the input XML string
 *  @return status code
 */
// ============================================================================
StatusCode Gaudi::Utils::Histos::fromXml( AIDA::IProfile2D& result, const std::string& input ) {
  auto root = Gaudi::Utils::Aida2ROOT::aida2root( &result );
  return root ? fromXml( *root, input ) : StatusCode::FAILURE; // RETURN
}
// ============================================================================
// The END
// ============================================================================
