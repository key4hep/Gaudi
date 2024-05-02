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
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "Gaudi/Accumulators/Histogram.h"
#include "GaudiKernel/Algorithm.h"
#include <fmt/format.h>
// ============================================================================
/** @file
 *  simple example, which illustrates the usage of "histogram properties"
 *  @author Vanay BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-09-18
 */
// ============================================================================
namespace Gaudi {
  namespace TestSuite {
    /** @class HistoProps
     *  simple example, which illustrates the usage of "histogram properties"
     *  @author Vanay BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-09-18
     */
    class HistoProps : public ::Algorithm {
    public:
      using Algorithm::Algorithm;

      StatusCode initialize() override {
        return Algorithm::initialize().andThen( [&] {
          using hist_t = Gaudi::Accumulators::Histogram<1>;
          using axis_t = Gaudi::Accumulators::Axis<double>;
          m_hist1      = std::make_unique<hist_t>( this, "Histo1", "Histogram 1", axis_t{ m_hist1def.value() } );
          m_hist2      = std::make_unique<hist_t>( this, "Histo2", "Histogram 2", axis_t( m_hist2def.value() ) );
        } );
      }
      StatusCode execute() override {
        Rndm::Numbers gauss( randSvc(), Rndm::Gauss( 0.0, 1.0 ) );

        ++( *m_hist1 )[gauss()];
        ++( *m_hist2 )[gauss()];

        return StatusCode::SUCCESS;
      }
      StatusCode finalize() override {
        auto format = []( std::string_view name, const nlohmann::json& data ) {
          return fmt::format( " | {:25} | {:25} | {:7} |", name, data["axis"][0]["title"].get<std::string>(),
                              data["nEntries"].get<unsigned long long>() );
        };
        always() << fmt::format( "Histograms report\n | {:25} | {:25} | {:7} |\n", "name", "title", "entries" )
                 << format( "Histo1", *m_hist1 ) << '\n'
                 << format( "Histo2", *m_hist2 ) << endmsg;
        return Algorithm::finalize();
      }
      // ======================================================================
    private:
      Gaudi::Property<Gaudi::Histo1DDef> m_hist1def{ this, "Histo1", Gaudi::Histo1DDef( "Histogram1", -3, 3, 200 ),
                                                     "The parameters for the first  histogram" };
      Gaudi::Property<Gaudi::Histo1DDef> m_hist2def{
          this, "Histo2", { "Histogram2", -5, 5, 200 }, "The parameters for the second histogram" };

      std::unique_ptr<Gaudi::Accumulators::Histogram<1>> m_hist1;
      std::unique_ptr<Gaudi::Accumulators::Histogram<1>> m_hist2;
    };
  } // namespace TestSuite
} // end of namespace Gaudi

DECLARE_COMPONENT( Gaudi::TestSuite::HistoProps )
