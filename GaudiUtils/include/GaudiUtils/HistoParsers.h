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
#ifndef GAUDIUTILS_HISTOPARSERS_H
#define GAUDIUTILS_HISTOPARSERS_H 1
// =============================================================================
// Include files
// =============================================================================
// GaudiKernel
// =============================================================================
#include "GaudiKernel/StatusCode.h"
// =============================================================================
// forward declarations
// =============================================================================
// AIDA
// =============================================================================
namespace AIDA {
  class IHistogram1D; // AIDA
  class IHistogram2D; // AIDA
  class IHistogram3D; // AIDA
} // namespace AIDA
// =============================================================================
// ROOT
// =============================================================================
class TH1D; // ROOT
class TH2D; // ROOT
class TH3D; // ROOT
class TH1F; // ROOT
class TH2F; // ROOT
class TH3F; // ROOT
// =============================================================================
namespace Gaudi {
  // ===========================================================================
  namespace Parsers {
    // =========================================================================
    /** parse ROOT histogram from text representation
     *  @param result (OUTPUT) the histogram
     *  @param input  (INPUT)  the input to be parsed
     *  @return status code
     */
    GAUDI_API StatusCode parse( TH1D& result, const std::string& input );
    // =========================================================================
    /** parse ROOT histogram from text representation
     *  @param result (OUTPUT) the histogram
     *  @param input  (INPUT)  the input to be parsed
     *  @return status code
     */
    GAUDI_API StatusCode parse( TH2D& result, const std::string& input );
    // =========================================================================
    /** parse ROOT histogram from text representation
     *  @param result (OUTPUT) the histogram
     *  @param input  (INPUT)  the input to be parsed
     *  @return status code
     */
    GAUDI_API StatusCode parse( TH3D& result, const std::string& input );
    // =========================================================================
    /** parse ROOT histogram from text representation
     *  @param result (OUTPUT) the histogram
     *  @param input  (INPUT)  the input to be parsed
     *  @return status code
     */
    GAUDI_API StatusCode parse( TH1F& result, const std::string& input );
    // =========================================================================
    /** parse ROOT histogram from text representation
     *  @param result (OUTPUT) the histogram
     *  @param input  (INPUT)  the input to be parsed
     *  @return status code
     */
    GAUDI_API StatusCode parse( TH2F& result, const std::string& input );
    // =========================================================================
    /** parse AIDA histogram from text representation
     *  @param result (OUTPUT) the histogram
     *  @param input  (INPUT)  the input to be parsed
     *  @return status code
     */
    GAUDI_API StatusCode parse( TH3F& result, const std::string& input );
    // =========================================================================
    /** parse AIDA histogram from text representation
     *  @param result (OUTPUT) the histogram
     *  @param input  (INPUT)  the input to be parsed
     *  @return status code
     */
    GAUDI_API StatusCode parse( AIDA::IHistogram1D& result, const std::string& input );
    // =========================================================================
    /** parse AIDA histogram from text representation
     *  @param result (OUTPUT) the histogram
     *  @param input  (INPUT)  the input to be parsed
     *  @return status code
     */
    GAUDI_API StatusCode parse( AIDA::IHistogram2D& result, const std::string& input );
    // =========================================================================
    /** parse AIDA histogram from text representation
     *  @param result (OUTPUT) the histogram
     *  @param input  (INPUT)  the input to be parsed
     *  @return status code
     */
    GAUDI_API StatusCode parse( AIDA::IHistogram3D& result, const std::string& input );
    // =========================================================================
    /** parse ROOT histogram from text representation
     *  @param result (OUTPUT) the histogram
     *  @param input  (INPUT)  the input to be parsed
     *  @return status code
     */
    GAUDI_API StatusCode parse( TH1D*& result, const std::string& input );
    // =========================================================================
    /** parse ROOT histogram from text representation
     *  @param result (OUTPUT) the histogram
     *  @param input  (INPUT)  the input to be parsed
     *  @return status code
     */
    GAUDI_API StatusCode parse( TH2D*& result, const std::string& input );
    // =========================================================================
    /** parse ROOT histogram from text representation
     *  @param result (OUTPUT) the histogram
     *  @param input  (INPUT)  the input to be parsed
     *  @return status code
     */
    GAUDI_API StatusCode parse( TH3D*& result, const std::string& input );
    // =========================================================================
  } // namespace Parsers
  // ===========================================================================
} // end of namespace Gaudi
// =============================================================================
// The END
// =============================================================================
#endif // GAUDIUTILS_HISTOPARSERS_H
