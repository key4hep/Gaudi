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
#ifndef GAUDIUTILS_HISTOXML_H
#define GAUDIUTILS_HISTOXML_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <iosfwd>
#include <string>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/StatusCode.h"
// ============================================================================
// Forward declarations
// ============================================================================
// AIDA
// ============================================================================
namespace AIDA {
  class IHistogram1D;
  class IHistogram2D;
  class IHistogram3D;
  class IProfile1D;
  class IProfile2D;
} // namespace AIDA
// ============================================================================
// ROOT
// ============================================================================
class TH1D;       // ROOT
class TH2D;       // ROOT
class TH3D;       // ROOT
class TH1F;       // ROOT
class TH2F;       // ROOT
class TH3F;       // ROOT
class TProfile;   // ROOT
class TProfile2D; // ROOT
// ============================================================================
namespace Gaudi {
  // ===========================================================================
  namespace Utils {
    // =========================================================================
    namespace Histos {
      // =======================================================================
      /** stream the ROOT histogram into output stream as XML
       *  @see TBufferXML
       *  @see TBufferXML::ConvertToXML
       *  @param histo  (INPUT)  the histogram to be streamed
       *  @param stream (OUTPUT) the stream
       */
      GAUDI_API std::ostream& toXml( const TH1D& histo, std::ostream& stream );
      // ======================================================================
      /** stream the ROOT histogram into output stream as XML
       *  @see TBufferXML
       *  @see TBufferXML::ConvertToXML
       *  @param histo  (INPUT)  the histogram to be streamed
       *  @param stream (OUTPUT) the stream
       */
      GAUDI_API std::ostream& toXml( const TH2D& histo, std::ostream& stream );
      // ======================================================================
      /** stream the ROOT histogram into output stream as XML
       *  @see TBufferXML
       *  @see TBufferXML::ConvertToXML
       *  @param histo  (INPUT)  the histogram to be streamed
       *  @param stream (OUTPUT) the stream
       */
      GAUDI_API std::ostream& toXml( const TH3D& histo, std::ostream& stream );
      // ======================================================================
      /** stream the ROOT histogram into output stream as XML
       *  @see TBufferXML
       *  @see TBufferXML::ConvertToXML
       *  @param histo  (INPUT)  the histogram to be streamed
       *  @param stream (OUTPUT) the stream
       */
      GAUDI_API std::ostream& toXml( const TProfile& histo, std::ostream& stream );
      // ======================================================================
      /** stream the ROOT histogram into output stream as XML
       *  @see TBufferXML
       *  @see TBufferXML::ConvertToXML
       *  @param histo  (INPUT)  the histogram to be streamed
       *  @param stream (OUTPUT) the stream
       */
      GAUDI_API std::ostream& toXml( const TProfile2D& histo, std::ostream& stream );
      // ======================================================================
      /** stream the AIDA histogram into the output stream as XML
       *  @param histo  (INPUT)  the histogram to be streamed
       *  @param stream (OUTPUT) the stream
       */
      GAUDI_API std::ostream& toXml( const AIDA::IHistogram1D& histo, std::ostream& stream );
      // ======================================================================
      /** stream the AIDA histogram into the output stream as XML
       *  @param histo  (INPUT)  the histogram to be streamed
       *  @param stream (OUTPUT) the stream
       */
      GAUDI_API std::ostream& toXml( const AIDA::IHistogram2D& histo, std::ostream& stream );
      // ======================================================================
      /** stream the AIDA histogram into the output stream as XML
       *  @param histo  (INPUT)  the histogram to be streamed
       *  @param stream (OUTPUT) the stream
       */
      GAUDI_API std::ostream& toXml( const AIDA::IHistogram3D& histo, std::ostream& stream );
      // ======================================================================
      /** stream the AIDA histogram into the output stream as XML
       *  @param histo  (INPUT)  the histogram to be streamed
       *  @param stream (OUTPUT) the stream
       */
      GAUDI_API std::ostream& toXml( const AIDA::IProfile1D& histo, std::ostream& stream );
      // =======================================================================
      /** stream the ROOT histogram into output stream as XML
       *  @see TBufferXML
       *  @see TBufferXML::ConvertToXML
       *  @param histo  (INPUT)  the histogram to be streamed
       *  @param stream (OUTPUT) the stream
       */
      GAUDI_API std::ostream& toXml( const TH1F& histo, std::ostream& stream );
      // ======================================================================
      /** stream the ROOT histogram into output stream as XML
       *  @see TBufferXML
       *  @see TBufferXML::ConvertToXML
       *  @param histo  (INPUT)  the histogram to be streamed
       *  @param stream (OUTPUT) the stream
       */
      GAUDI_API std::ostream& toXml( const TH2F& histo, std::ostream& stream );
      // ======================================================================
      /** stream the ROOT histogram into output stream as XML
       *  @see TBufferXML
       *  @see TBufferXML::ConvertToXML
       *  @param histo  (INPUT)  the histogram to be streamed
       *  @param stream (OUTPUT) the stream
       */
      GAUDI_API std::ostream& toXml( const TH3F& histo, std::ostream& stream );
      // ======================================================================
      /** stream the AIDA histogram into the output stream as XML
       *  @param histo  (INPUT)  the histogram to be streamed
       *  @param stream (OUTPUT) the stream
       */
      GAUDI_API std::ostream& toXml( const AIDA::IProfile2D& histo, std::ostream& stream );
      // ======================================================================
      /** parse the histogram from standard ROOT XML
       *
       *  @see TBufferXML
       *  @see TBufferXML::ConvertFromXML
       *
       *  @param result (OUTPUT)  the parsed histogram
       *  @param input  (INPUT)   the input XML string
       *  @return status code
       */
      GAUDI_API StatusCode fromXml( TH1D& result, std::string_view input );
      // ======================================================================
      /** parse the histogram from standard ROOT XML
       *
       *  @see TBufferXML
       *  @see TBufferXML::ConvertFromXML
       *
       *  @param result (OUTPUT)  the parsed histogram
       *  @param input  (INPUT)   the input XML string
       *  @return status code
       */
      GAUDI_API StatusCode fromXml( TH2D& result, std::string_view input );
      // ======================================================================
      /** parse the histogram from standard ROOT XML
       *
       *  @see TBufferXML
       *  @see TBufferXML::ConvertFromXML
       *
       *  @param result (OUTPUT)  the parsed histogram
       *  @param input  (INPUT)   the input XML string
       *  @return status code
       */
      GAUDI_API StatusCode fromXml( TH3D& result, std::string_view input );
      // ======================================================================
      /** parse the histogram from standard ROOT XML
       *
       *  @see TBufferXML
       *  @see TBufferXML::ConvertFromXML
       *
       *  @param result (OUTPUT)  the parsed histogram
       *  @param input  (INPUT)   the input XML string
       *  @return status code
       */
      GAUDI_API StatusCode fromXml( TProfile& result, std::string_view input );
      // ======================================================================
      /** parse the histogram from standard ROOT XML
       *
       *  @see TBufferXML
       *  @see TBufferXML::ConvertFromXML
       *
       *  @param result (OUTPUT)  the parsed histogram
       *  @param input  (INPUT)   the input XML string
       *  @return status code
       */
      GAUDI_API StatusCode fromXml( TProfile2D& result, std::string_view input );
      // ======================================================================
      /** parse the histogram from standard ROOT XML
       *
       *  @see TBufferXML
       *  @see TBufferXML::ConvertFromXML
       *
       *  @param result (OUTPUT)  the parsed histogram
       *  @param input  (INPUT)   the input XML string
       *  @return status code
       */
      GAUDI_API StatusCode fromXml( TH1F& result, std::string_view input );
      // ======================================================================
      /** parse the histogram from standard ROOT XML
       *
       *  @see TBufferXML
       *  @see TBufferXML::ConvertFromXML
       *
       *  @param result (OUTPUT)  the parsed histogram
       *  @param input  (INPUT)   the input XML string
       *  @return status code
       */
      GAUDI_API StatusCode fromXml( TH2F& result, std::string_view input );
      // ======================================================================
      /** parse the histogram from standard ROOT XML
       *
       *  @see TBufferXML
       *  @see TBufferXML::ConvertFromXML
       *
       *  @param result (OUTPUT)  the parsed histogram
       *  @param input  (INPUT)   the input XML string
       *  @return status code
       */
      GAUDI_API StatusCode fromXml( TH3F& result, std::string_view input );
      // ======================================================================
      /** parse the histogram from standard ROOT XML
       *
       *  @see TBufferXML
       *  @see TBufferXML::ConvertFromXML
       *
       *  @param result (OUTPUT)  the parsed histogram
       *  @param input  (INPUT)   the input XML string
       *  @return status code
       */
      GAUDI_API StatusCode fromXml( TH1D*& result, std::string_view input );
      // ======================================================================
      /** parse the histogram from standard ROOT XML
       *
       *  @see TBufferXML
       *  @see TBufferXML::ConvertFromXML
       *
       *  @param result (OUTPUT)  the parsed histogram
       *  @param input  (INPUT)   the input XML string
       *  @return status code
       */
      GAUDI_API StatusCode fromXml( TH2D*& result, std::string_view input );
      // ======================================================================
      /** parse the histogram from standard ROOT XML
       *
       *  @see TBufferXML
       *  @see TBufferXML::ConvertFromXML
       *
       *  @param result (OUTPUT)  the parsed histogram
       *  @param input  (INPUT)   the input XML string
       *  @return status code
       */
      GAUDI_API StatusCode fromXml( TH3D*& result, std::string_view input );
      // ======================================================================
      /** parse the histogram from standard ROOT XML
       *
       *  @see TBufferXML
       *  @see TBufferXML::ConvertFromXML
       *
       *  @param result (OUTPUT)  the parsed histogram
       *  @param input  (INPUT)   the input XML string
       *  @return status code
       */
      GAUDI_API StatusCode fromXml( TProfile*& result, std::string_view input );
      // ======================================================================
      /** parse the histogram from standard ROOT XML
       *
       *  @see TBufferXML
       *  @see TBufferXML::ConvertFromXML
       *
       *  @param result (OUTPUT)  the parsed histogram
       *  @param input  (INPUT)   the input XML string
       *  @return status code
       */
      GAUDI_API StatusCode fromXml( TProfile2D*& result, std::string_view input );
      // ======================================================================
      /** parse the histogram from standard ROOT XML
       *
       *  @see TBufferXML
       *  @see TBufferXML::ConvertFromXML
       *
       *  @param result (OUTPUT)  the parsed histogram
       *  @param input  (INPUT)   the input XML string
       *  @return status code
       */
      GAUDI_API StatusCode fromXml( AIDA::IHistogram1D& result, std::string_view input );
      // ======================================================================
      /** parse the histogram from standard ROOT XML
       *
       *  @see TBufferXML
       *  @see TBufferXML::ConvertFromXML
       *
       *  @param result (OUTPUT)  the parsed histogram
       *  @param input  (INPUT)   the input XML string
       *  @return status code
       */
      GAUDI_API StatusCode fromXml( AIDA::IHistogram2D& result, std::string_view input );
      // ======================================================================
      /** parse the histogram from standard ROOT XML
       *
       *  @see TBufferXML
       *  @see TBufferXML::ConvertFromXML
       *
       *  @param result (OUTPUT)  the parsed histogram
       *  @param input  (INPUT)   the input XML string
       *  @return status code
       */
      GAUDI_API StatusCode fromXml( AIDA::IHistogram3D& result, std::string_view input );
      // ======================================================================
      /** parse the histogram from standard ROOT XML
       *
       *  @see TBufferXML
       *  @see TBufferXML::ConvertFromXML
       *
       *  @param result (OUTPUT)  the parsed histogram
       *  @param input  (INPUT)   the input XML string
       *  @return status code
       */
      GAUDI_API StatusCode fromXml( AIDA::IProfile1D& result, std::string_view input );
      // ======================================================================
      /** parse the histogram from standard ROOT XML
       *
       *  @see TBufferXML
       *  @see TBufferXML::ConvertFromXML
       *
       *  @param result (OUTPUT)  the parsed histogram
       *  @param input  (INPUT)   the input XML string
       *  @return status code
       */
      GAUDI_API StatusCode fromXml( AIDA::IProfile2D& result, std::string_view input );
      // ======================================================================
    } // namespace Histos
    // ========================================================================
  } // namespace Utils
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIUTILS_HISTOXML_H
