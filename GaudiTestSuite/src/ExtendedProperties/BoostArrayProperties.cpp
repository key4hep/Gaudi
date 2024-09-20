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
// GaudiKernel
// ============================================================================
#include <GaudiKernel/BoostArrayAsProperty.h>
// ============================================================================
#include <Gaudi/TestSuite/TestAlg.h>
// ============================================================================
// Boots
// ============================================================================
#include <boost/array.hpp>
// ============================================================================
/** @file
 *  Simple example/test for "array"-properties
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-15
 */
// ============================================================================
namespace Gaudi {
  // ==========================================================================
  namespace TestSuite {
    // ========================================================================
    class BoostArrayProperties : public TestAlg {
    public:
      // ======================================================================
      /// execute it!
      StatusCode execute() override;
      // ======================================================================
    public:
      // ======================================================================
      /// Standard constructor
      using TestAlg::TestAlg;
      // ======================================================================
    private:
      // ======================================================================
      /// array  of doubles
      Gaudi::Property<boost::array<double, 5>> m_doubles{
          this, "Doubles", { -1, -1, -1, -1, -1 }, "Boost-array of doubles" };
      /// array of strings
      Gaudi::Property<boost::array<std::string, 4>> m_strings{
          this, "Strings", { "bla-bla", "bla-bla", "bla-bla", "bla-bla" }, "Boost-array of strings" };
      // ======================================================================
    };
    // ========================================================================
  } // namespace TestSuite
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
// execute it!
// ============================================================================
StatusCode Gaudi::TestSuite::BoostArrayProperties::execute() {
  //
  info() << " Doubles : " << Gaudi::Utils::toString( m_doubles ) << endmsg;
  info() << " Strings : " << Gaudi::Utils::toString( m_strings ) << endmsg;
  //
  return StatusCode::SUCCESS;
}
// ============================================================================
/// The Factory
// ============================================================================
using Gaudi::TestSuite::BoostArrayProperties;
DECLARE_COMPONENT( BoostArrayProperties )
// ============================================================================
// The END
// ============================================================================
