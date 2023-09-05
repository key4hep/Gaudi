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
#pragma once

#include "GaudiKernel/Kernel.h"

#include <string>

namespace AIDA {
  class IHistogram1D; // AIDA
  class IProfile1D;   // AIDA
} // namespace AIDA

class TH1;      // ROOT
class TProfile; // ROOT

namespace Gaudi {
  namespace Utils {
    namespace Histos {

      /** dump the text representation of the histogram
       *  @param histo  (INPUT) the histogram
       *  @param width  (INPUT) the maximal  column width
       *  @param height (INPUT) the proposed column height
       *  @param erorrs (INPUT) print/plot errors
       *  @return string representation of the histogram
       *  @author Vanya BELYAEV  Ivan.BElyaev@nikhef.nl
       *  @date 2009-09-19
       */
      GAUDI_API
      std::string histoDump( const AIDA::IHistogram1D* histo, const std::size_t width = 80,
                             const std::size_t height = 50, const bool errors = false );

      /** dump the text representation of the 1D-profile
       *  @param histo  (INPUT) the histogram
       *  @param width  (INPUT) the maximal  column width
       *  @param height (INPUT) the proposed column height
       *  @param spread (INPUT) print/plto spread vs rms
       *  @return string representation of the histogram
       *  @author Vanya BELYAEV  Ivan.BElyaev@nikhef.nl
       *  @date 2009-09-19
       */
      GAUDI_API
      std::string histoDump( const AIDA::IProfile1D* histo, const std::size_t width = 80, const std::size_t height = 50,
                             const bool spread = true );

      /** dump the text representation of the histogram
       *  @param histo  (INPUT) the histogram
       *  @param width  (INPUT) the maximal column width
       *  @param height (INPUT) the propsoed coulmn height
       *  @param erorrs (INPUT) print/plot errors
       *  @return string representation of the histogram
       *  @author Vanya BELYAEV  Ivan.Belyaev@nikhef.nl
       *  @date 2009-09-19
       */
      GAUDI_API
      std::string histoDump( const TProfile* histo, const std::size_t width = 80, const std::size_t height = 50 );

      /** dump the text representation of the histogram
       *  @param histo  (INPUT) the histogram
       *  @param width  (INPUT) the maximal column width
       *  @param height (INPUT) the propsoed coulmn height
       *  @param erorrs (INPUT) print/plot errors
       *  @return string representation of the histogram
       *  @author Vanya BELYAEV  Ivan.Belyaev@nikhef.nl
       *  @date 2009-09-19
       */
      GAUDI_API
      std::string histoDump( const TH1* histo, const std::size_t width = 80, const std::size_t height = 50,
                             const bool errors = false );
    } // namespace Histos

  } // namespace Utils

} //                                                     end of namespace Gaudi
