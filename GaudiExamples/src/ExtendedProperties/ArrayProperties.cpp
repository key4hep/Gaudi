// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/CArrayAsProperty.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlgorithm.h"
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
    class ArrayProperties : public GaudiAlgorithm
    {
    public:
      // ======================================================================
      /// execute it!
      StatusCode execute () ;
      // ======================================================================
    public:
      // ======================================================================
      /** Standard constructor
       *
       */
      ArrayProperties ( const std::string& name ,    // algorithm instance name
                        ISvcLocator*       pSvc )    //         service locator
        : GaudiAlgorithm ( name , pSvc )
      {
        //
        std::fill_n ( m_doubles , 5 , -10              ) ;
        std::fill_n ( m_strings , 4 , "blah-blah-blah" ) ;
        //
        declareProperty ( "Doubles" , m_doubles , "C-array of doubles" ) ;
        declareProperty ( "Strings" , m_strings , "C-array of strings" ) ;
      }
      /// virtual destructor
      virtual ~ArrayProperties() {}
      // ======================================================================
    private:
      // ======================================================================
      /// the default constructor is disabled
      ArrayProperties () ;               // the default constructor is disabled
      /// copy constructor is disabled
      ArrayProperties ( const ArrayProperties& ) ; // no copy constructor
      /// assignment operator is disabled
      ArrayProperties& operator=( const ArrayProperties& ) ; // no assignment
      // ======================================================================
    private:
      // ======================================================================
      /// array  of doubles
      double       m_doubles[5] ;                          // array  of doubles
      /// array of strings
      std::string  m_strings[4] ;                           // array of strings
      // ======================================================================
    } ;
    // ========================================================================
  } //                                         end of namespace Gaudi::Examples
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
// execute it!
// ============================================================================
StatusCode Gaudi::Examples::ArrayProperties::execute ()
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
using Gaudi::Examples::ArrayProperties;
DECLARE_COMPONENT(ArrayProperties)
// ============================================================================
// The END
// ============================================================================
