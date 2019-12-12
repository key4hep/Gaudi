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
namespace Gaudi {
  namespace Examples {
    /** @class HistoProps
     *  simple example, which illustrates the usage of "histogram properties"
     *  @author Vanay BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-09-18
     */
    class HistoProps : public GaudiHistoAlg {
    public:
      // ======================================================================
      /// Execute the algorithm
      StatusCode execute() override;
      // ======================================================================
    public:
      // ======================================================================
      /** standard constructor
       *  @param name algorithm instance name
       *  @param pSvc  Service Locator
       */
      HistoProps( const std::string& name, ISvcLocator* pSvc ) : GaudiHistoAlg( name, pSvc ) {
        setProperty( "PropertiesPrint", true ).ignore();
        setProperty( "HistoPrint", true ).ignore();
      }
      // ======================================================================
    private:
      Gaudi::Property<Gaudi::Histo1DDef> m_hist1{this, "Histo1", Gaudi::Histo1DDef( "Histogram1", -3, 3, 200 ),
                                                 "The parameters for the first  histogram"};
      Gaudi::Property<Gaudi::Histo1DDef> m_hist2{
          this, "Histo2", {"Histogram2", -5, 5, 200}, "The parameters for the second histogram"};
    };
  } // namespace Examples
} // end of namespace Gaudi
// ============================================================================
/// the factory (necessary for instantiation)
using Gaudi::Examples::HistoProps;
DECLARE_COMPONENT( HistoProps )
// ============================================================================
// Execute the algorithm
// ============================================================================
StatusCode Gaudi::Examples::HistoProps::execute() {

  Rndm::Numbers gauss( randSvc(), Rndm::Gauss( 0.0, 1.0 ) );

  plot( gauss(), m_hist1.value() );
  plot( gauss(), m_hist2.value() );

  return StatusCode::SUCCESS;
}
// ============================================================================
// The END
// ============================================================================
