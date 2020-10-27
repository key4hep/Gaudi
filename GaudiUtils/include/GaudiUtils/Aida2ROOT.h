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
#ifndef GAUDIALG_AIDA2ROOT_H
#define GAUDIALG_AIDA2ROOT_H 1
// ============================================================================
// Include files
// ============================================================================
#include "GaudiKernel/Kernel.h"
// ============================================================================
// Forward declarations
// ============================================================================
namespace AIDA {
  class IBaseHistogram;
  class IHistogram;
  class IHistogram1D;
  class IHistogram2D;
  class IHistogram3D;
  class IProfile1D;
  class IProfile2D;
} // namespace AIDA
// ============================================================================
// ROOT
// ============================================================================
class TH1D;
class TH2D;
class TH3D;
class TProfile;
class TProfile2D;
class TObject;
// ============================================================================
namespace Gaudi {
  namespace Utils {
    // ========================================================================
    /** @class Aida2ROOT Aida2ROOT.h GaudiKernel/Aida2ROOT.h
     *
     *  Accessor to underlying ROOT-representation of
     *  transient histograms
     *  The actual code is imported from Bender project.
     *
     *  It relies on PI implementation of AIDA interface,
     *  modified by Markus FRANK.
     *
     *  @see Gaudi::H1D
     *  @see Gaudi::H2D
     *  @see Gaudi::H3D
     *  @see Gaudi::Profile1D
     *  @see Gaudi::Profile2D
     *
     *  @see AIDA::IHistogram1D
     *  @see AIDA::IHistogram2D
     *  @see AIDA::IHistogram3D
     *  @see AIDA::IProfile1D
     *  @see AIDA::IProfile2D
     *
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date   2007-01-23
     */
    class GAUDI_API Aida2ROOT {
    public:
      /// get the underlying pointer for 1D-histogram
      static TH1D*       aida2root( AIDA::IHistogram1D* aida );
      static const TH1D* aida2root( const AIDA::IHistogram1D* aida );
      /// get the underlying pointer for 2D-histogram
      static TH2D*       aida2root( AIDA::IHistogram2D* aida );
      static const TH2D* aida2root( const AIDA::IHistogram2D* aida );
      /// get the underlying pointer for 3D-histogram
      static TH3D*       aida2root( AIDA::IHistogram3D* aida );
      static const TH3D* aida2root( const AIDA::IHistogram3D* aida );
      /// get the underlying pointer for 1D-profile
      static TProfile*       aida2root( AIDA::IProfile1D* aida );
      static const TProfile* aida2root( const AIDA::IProfile1D* aida );
      /// get the underlying pointer for 2D-profile
      static TProfile2D*       aida2root( AIDA::IProfile2D* aida );
      static const TProfile2D* aida2root( const AIDA::IProfile2D* aida );
      /// get root representation for other cases
      static TObject*       aida2root( AIDA::IHistogram* aida );
      static const TObject* aida2root( const AIDA::IHistogram* aida );
    };
    // ========================================================================
  } // namespace Utils
  // ==========================================================================
} // end of namespace Gaudi
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_AIDA2ROOT_H
