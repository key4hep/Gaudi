// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiHistoAlg.h"
// ============================================================================
/** @file
 *  simple example, which illustrates the usage of "histogram properties"
 *  @author Vanay BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-09-18
 */
// ============================================================================
namespace Gaudi
{
  namespace Examples
  {
    /** @class HistoProps
     *  simple example, which illustrates the usage of "histogram properties"
     *  @author Vanay BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-09-18
     */
    class HistoProps : public GaudiHistoAlg
    {
    public:
      // ======================================================================
      /// Execute the algorithm
      virtual StatusCode execute () ;
      // ======================================================================
    public:
      // ======================================================================
      /** standard constructor
       *  @param name algorithm instance name
       *  @param pSvc  Service Locator
       */
      HistoProps
      ( const std::string& name ,
        ISvcLocator*       pSvc )
        : GaudiHistoAlg ( name , pSvc )
        , m_hist1 ( Gaudi::Histo1DDef ( "Histogram1" , -3 , 3 , 200 ) )
        , m_hist2 (                     "Histogram2" , -5 , 5 , 200 )
      {
        declareProperty
          ( "Histo1" ,
            m_hist1  ,
            "The parameters for the first  histogram" ) ;
        declareProperty
          ( "Histo2" ,
            m_hist2  ,
            "The parameters for the second histogram" ) ;
        ///
        setProperty ( "PropertiesPrint" , true ) . ignore () ;
        setProperty ( "HistoPrint"      , true ) . ignore () ;
      }
      /// virtual and protected destructor
      virtual ~HistoProps() {} ///< virtual and protected destructor
      // ======================================================================
    private:
      // ======================================================================
      // default constructor is disabled
      HistoProps () ;                   ///< default constructor is disabled
      // copy constructor is disabled
      HistoProps ( const HistoProps& ) ; ///< copy constructor is disabled
      // assignement operator is disabled
      HistoProps& operator=( const HistoProps& ) ; ///< no assignement
      // ======================================================================
    private:
      Histo1DProperty   m_hist1 ;
      Gaudi::Histo1DDef m_hist2 ;
    };
  } // end of namespace Gaudi::Examples
} // end of namespace Gaudi
// ============================================================================
/// the factory (necessary for instantiation)
using Gaudi::Examples::HistoProps;
DECLARE_COMPONENT(HistoProps)
// ============================================================================
// Execute the algorithm
// ============================================================================
StatusCode Gaudi::Examples::HistoProps::execute ()
{

  Rndm::Numbers  gauss   ( randSvc() , Rndm::Gauss ( 0.0 ,  1.0 ) ) ;

  plot ( gauss() , m_hist1 ) ;
  plot ( gauss() , m_hist2 ) ;

  return StatusCode::SUCCESS ;
}
// ============================================================================
// The END
// ============================================================================
