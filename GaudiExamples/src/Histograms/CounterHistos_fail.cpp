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
#include <Gaudi/Accumulators/Histogram.h>
#include <Gaudi/Algorithm.h>
#include <GaudiKernel/RndmGenerators.h>

namespace Gaudi {
  namespace Examples {
    namespace Counter {

      /// Testing expected compilation failures of hitogram usage
      class GaudiHistoAlgorithmFail : public Gaudi::Algorithm {
      public:
        using Gaudi::Algorithm::Algorithm;

        StatusCode execute( const EventContext& ) const override {
          // some random number generators, just to provide numbers
          static Rndm::Numbers Flat( randSvc(), Rndm::Flat( -10.0, 10.0 ) );
          const double         flat( Flat() );
          ++m_gaussVflat[flat]; // should not compile, as it's a 2D histogram and we give a single number
          return StatusCode::SUCCESS;
        }

      private:
        mutable Gaudi::Accumulators::Histogram<2> m_gaussVflat{
            this, "GaussFlat", "Gaussian V Flat, atomic", { { 50, -5, 5, "X" }, { 50, -5, 5, "Y" } } };
      };
      DECLARE_COMPONENT( GaudiHistoAlgorithmFail )
    } // namespace Counter
  }   // namespace Examples
} // namespace Gaudi
