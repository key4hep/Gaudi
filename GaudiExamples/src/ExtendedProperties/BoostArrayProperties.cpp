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
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/BoostArrayAsProperty.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlgorithm.h"
// ============================================================================
// Boots
// ============================================================================
#include "boost/array.hpp"
// ============================================================================
/** @file
 *  Simple example/test for "array"-properties
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-15
 */
// ============================================================================
namespace Gaudi {
  // ==========================================================================
  namespace Examples {
    // ========================================================================
    class BoostArrayProperties : public GaudiAlgorithm {
    public:
      // ======================================================================
      /// execute it!
      StatusCode execute() override;
      // ======================================================================
    public:
      // ======================================================================
      /// Standard constructor
      using GaudiAlgorithm::GaudiAlgorithm;
      // ======================================================================
    private:
      // ======================================================================
      /// array  of doubles
      Gaudi::Property<boost::array<double, 5>> m_doubles{
          this, "Doubles", {-1, -1, -1, -1, -1}, "Boost-array of doubles"};
      /// array of strings
      Gaudi::Property<boost::array<std::string, 4>> m_strings{
          this, "Strings", {"bla-bla", "bla-bla", "bla-bla", "bla-bla"}, "Boost-array of strings"};
      // ======================================================================
    };
    // ========================================================================
  } // namespace Examples
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
// execute it!
// ============================================================================
StatusCode Gaudi::Examples::BoostArrayProperties::execute() {
  //
  info() << " Doubles : " << Gaudi::Utils::toString( m_doubles ) << endmsg;
  info() << " Strings : " << Gaudi::Utils::toString( m_strings ) << endmsg;
  //
  return StatusCode::SUCCESS;
}
// ============================================================================
/// The Factory
// ============================================================================
using Gaudi::Examples::BoostArrayProperties;
DECLARE_COMPONENT( BoostArrayProperties )
// ============================================================================
// The END
// ============================================================================
