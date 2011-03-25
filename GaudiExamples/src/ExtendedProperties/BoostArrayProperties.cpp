// $Id: $
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/BoostArrayAsProperty.h"
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/AlgFactory.h"
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
      // ======================================================================
      /// the friend factory for instantiation
      friend class AlgFactory<Gaudi::Examples::BoostArrayProperties> ;
      // ======================================================================
    public:
      // ======================================================================
      /// execute it!
      StatusCode execute () ;
      // ======================================================================
    protected:
      // ======================================================================
      /** Standard constructor
       *
       */
      BoostArrayProperties ( const std::string& name ,    // algorithm instance name
                             ISvcLocator*       pSvc )    //         service locator
        : GaudiAlgorithm ( name , pSvc )
      {
        //
        std::fill( m_doubles.begin() , m_doubles.end() , -1 )      ;
        std::fill( m_strings.begin() , m_strings.end() , "bla-bla" ) ;
        //
        declareProperty ( "Doubles" , m_doubles , "Boost-array of doubles" ) ;
        declareProperty ( "Strings" , m_strings , "Boost-array of strings" ) ;
      }
      /// virtual destructor
      virtual ~BoostArrayProperties() {}
      // ======================================================================
    private:
      // ======================================================================
      /// the default constructor is disabled
      BoostArrayProperties () ;          // the default constructor is disabled
      /// copy constructor is disabled
      BoostArrayProperties ( const BoostArrayProperties& ) ;
      /// assignment operator is disabled
      BoostArrayProperties& operator=( const BoostArrayProperties& ) ;
      // ======================================================================
    private:
      // ======================================================================
      /// array  of doubles
      boost::array<double,5>       m_doubles ;             // array  of doubles
      /// array of strings
      boost::array<std::string,4>  m_strings ;              // array of strings
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
DECLARE_NAMESPACE_ALGORITHM_FACTORY( Gaudi::Examples , BoostArrayProperties )
// ============================================================================
// The END
// ============================================================================
