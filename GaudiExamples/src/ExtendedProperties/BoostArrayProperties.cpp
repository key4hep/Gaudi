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
namespace Gaudi
{
  // ==========================================================================
  namespace Examples
  {
    // ========================================================================
    class BoostArrayProperties : public GaudiAlgorithm
    {
    public:
      // ======================================================================
      /// execute it!
      StatusCode execute () override;
      // ======================================================================
    public:
      // ======================================================================
      /// Standard constructor
      using GaudiAlgorithm::GaudiAlgorithm;
      /// virtual destructor
      ~BoostArrayProperties() override = default;
      // ======================================================================
    private:
      // ======================================================================
      /// array  of doubles
      PropertyWithValue<boost::array<double,5>>       m_doubles{this, "Doubles", {-1, -1, -1, -1, -1}, "Boost-array of doubles"};
      /// array of strings
      PropertyWithValue<boost::array<std::string,4>>  m_strings{this, "Strings", {"bla-bla", "bla-bla", "bla-bla", "bla-bla"}, "Boost-array of strings"};
      // ======================================================================
    } ;
    // ========================================================================
  } //                                         end of namespace Gaudi::Examples
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
// execute it!
// ============================================================================
StatusCode Gaudi::Examples::BoostArrayProperties::execute ()
{
  propsPrint () ;
  //
  info() << " Doubles : " << Gaudi::Utils::toString ( m_doubles ) << endmsg ;
  info() << " Strings : " << Gaudi::Utils::toString ( m_strings ) << endmsg ;
  //
  return StatusCode::SUCCESS ;
}
// ============================================================================
/// The Factory
// ============================================================================
using Gaudi::Examples::BoostArrayProperties;
DECLARE_COMPONENT(BoostArrayProperties)
// ============================================================================
// The END
// ============================================================================
