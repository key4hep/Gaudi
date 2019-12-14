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
#ifndef GAUDIUTILS_HISTOSTATS_H
#define GAUDIUTILS_HISTOSTATS_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <utility>
// ============================================================================
// Gaudi
// ============================================================================
#include "GaudiKernel/Kernel.h"
// ============================================================================
// forward declarations
// ============================================================================
namespace AIDA {
  class IHistogram1D;
  class IProfile1D;
} // namespace AIDA
// ============================================================================
namespace Gaudi {
  namespace Utils {
    // ========================================================================
    /** @class HistoStats HistoStats.h GaudiUtils/HistoStats.h
     *  The collection of trivial functions to access  the
     *  statistical information for the histograms
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-08-06
     */
    class GAUDI_API HistoStats {
    public: // 1D Histograms
      // ======================================================================
      /** get the "bin-by-bin"-moment around the specified  "value"
       *  @param histo histogram
       *  @param order the moment parameter
       *  @param value central value
       *  @return the evaluated moment
       */
      static double moment( const AIDA::IHistogram1D* histo, const unsigned int order, const double value = 0 );
      // ======================================================================
      /** evaluate the uncertanty for 'bin-by-bin'-moment
       *  @param histo histogram
       *  @param order the moment parameter
       *  @param value central value
       *  @return the evaluated uncertanty in the moment
       */
      static double momentErr( const AIDA::IHistogram1D* histo, const unsigned int order );
      // ======================================================================
      /** evaluate the 'bin-by-bin'-central moment (around the mean value)
       *  @param histo histogram
       *  @param order the moment parameter
       *  @param value central value
       *  @return the evaluated central moment
       */
      static double centralMoment( const AIDA::IHistogram1D* histo, const unsigned int order );
      // ======================================================================
      /** evaluate the uncertanty for 'bin-by-bin'-central moment
       *  (around the mean value)
       *  ( the uncertanty is calculated with O(1/n2) precision)
       *  @param histo histogram
       *  @param order the moment parameter
       *  @param value central value
       *  @return the evaluated uncertanty in the central moment
       */
      static double centralMomentErr( const AIDA::IHistogram1D* histo, const unsigned int order );
      // ======================================================================
      /// get the skewness for the histogram
      static double skewness( const AIDA::IHistogram1D* histo );
      // ======================================================================
      /// get the error in skewness for the histogram
      static double skewnessErr( const AIDA::IHistogram1D* histo );
      // ======================================================================
      /// get the kurtosis for the histogram
      static double kurtosis( const AIDA::IHistogram1D* histo );
      // ======================================================================
      /// get the error in kurtosis for the histogram
      static double kurtosisErr( const AIDA::IHistogram1D* histo );
      // ======================================================================
      /// get the mean value for the histogram  (just for completeness)
      static double mean( const AIDA::IHistogram1D* histo );
      // ======================================================================
      /// get an error in the mean value
      static double meanErr( const AIDA::IHistogram1D* histo );
      // ======================================================================
      /// get the rms value for the histogram  (just for completeness)
      static double rms( const AIDA::IHistogram1D* histo );
      // ======================================================================
      /// get an error in the rms value
      static double rmsErr( const AIDA::IHistogram1D* histo );
      // ======================================================================
      /// get the effective entries   (just for completeness)
      static double nEff( const AIDA::IHistogram1D* histo );
      // ======================================================================
      /// get an error in the sum bin height ("in-range integral")
      static double sumBinHeightErr( const AIDA::IHistogram1D* histo );
      // ======================================================================
      /// get an error in the sum of all bin height ("integral")
      static double sumAllBinHeightErr( const AIDA::IHistogram1D* histo );
      // ======================================================================
      /// the fraction of overflow entries  (useful for shape comparison)
      static double overflowEntriesFrac( const AIDA::IHistogram1D* histo );
      /// the fraction of underflow entries (useful for shape comparison)
      static double underflowEntriesFrac( const AIDA::IHistogram1D* histo );
      /// error on fraction of overflow entries  (useful for shape comparison)
      static double overflowEntriesFracErr( const AIDA::IHistogram1D* histo );
      /// the error on fraction of underflow entries (useful for shape comparison)
      static double underflowEntriesFracErr( const AIDA::IHistogram1D* histo );
      // ======================================================================
      /// the fraction of overflow intergal  (useful for shape comparison)
      static double overflowIntegralFrac( const AIDA::IHistogram1D* histo );
      /// the fraction of underflow integral (useful for shape comparison)
      static double underflowIntegralFrac( const AIDA::IHistogram1D* histo );
      /// the error on fraction of overflow intergal
      static double overflowIntegralFracErr( const AIDA::IHistogram1D* histo );
      /// the error on fraction of underflow integral
      static double underflowIntegralFracErr( const AIDA::IHistogram1D* histo );
      // ======================================================================
      /** get number of entries in histogram up to
       *  the certain bin (not-included)
       *  @attention underflow bin is included!
       *  @param histo the pointer to the histogram
       *  @param imax  the bin number (not included)
       *  @param number of entries
       */
      static long nEntries( const AIDA::IHistogram1D* histo, const int imax );
      /** get number of entries in histogram form the certain
       *  minimal bin up to the certain maximal bin (not-included)
       *  @param histo the pointer to the histogram
       *  @param imin  the minimal bin number (included)
       *  @param imax  the maximal bin number (not included)
       *  @param number of entries
       */
      static long nEntries( const AIDA::IHistogram1D* histo,
                            const int                 imin, // minimal bin number (included)
                            const int                 imax );               // maximal bin number (not included)
      // ======================================================================
      /** get the fraction of entries in histogram up to
       *  the certain bin (not-included)
       *  @attention underflow bin is included!
       *  @param histo the pointer to the histogram
       *  @param imax  the bin number (not included)
       *  @param fraction of entries
       */
      static double nEntriesFrac( const AIDA::IHistogram1D* histo, const int imax );
      /** get fraction of entries in histogram form the certain
       *  minimal bin up to the certain maximal bin (not-included)
       *  @param histo the pointer to the histogram
       *  @param imin  the minimal bin number (included)
       *  @param imax  the maximal bin number (not included)
       *  @param fraction of entries
       */
      static double nEntriesFrac( const AIDA::IHistogram1D* histo, const int imin, const int imax );
      // ======================================================================
      /** get the (binominal) error for the fraction of entries
       *  in histogram up to the certain bin (not-included)
       *  @attention underflow bin is included!
       *  @param histo the pointer to the histogram
       *  @param imax  the bin number (not included)
       *  @param error for the fraction of entries
       */
      static double nEntriesFracErr( const AIDA::IHistogram1D* histo, const int imax );
      /** get the (binomial) error for the fraction of entries in histogram
       *  from the certain minimal bin up to the certain maximal bin (not-included)
       *  @param histo the pointer to the histogram
       *  @param imin  the minimal bin number (included)
       *  @param imax  the maximal bin number (not included)
       *  @param error for the fraction of entries
       */
      static double nEntriesFracErr( const AIDA::IHistogram1D* histo, const int imin, const int imax );
      // ======================================================================
    public: // 1D Profile Histograms
      // ======================================================================
      /** get the "bin-by-bin"-moment around the specified  "value"
       *  @param histo histogram
       *  @param order the moment parameter
       *  @param value central value
       *  @return the evaluated moment
       */
      static double moment( const AIDA::IProfile1D* histo, const unsigned int order, const double value = 0 );
      // ======================================================================
      /** evaluate the uncertanty for 'bin-by-bin'-moment
       *  @param histo histogram
       *  @param order the moment parameter
       *  @param value central value
       *  @return the evaluated uncertanty in the moment
       */
      static double momentErr( const AIDA::IProfile1D* histo, const unsigned int order );
      // ======================================================================
      /** evaluate the 'bin-by-bin'-central moment (around the mean value)
       *  @param histo histogram
       *  @param order the moment parameter
       *  @param value central value
       *  @return the evaluated central moment
       */
      static double centralMoment( const AIDA::IProfile1D* histo, const unsigned int order );
      // ======================================================================
      /** evaluate the uncertanty for 'bin-by-bin'-central moment
       *  (around the mean value)
       *  ( the uncertanty is calculated with O(1/n2) precision)
       *  @param histo histogram
       *  @param order the moment parameter
       *  @param value central value
       *  @return the evaluated uncertanty in the central moment
       */
      static double centralMomentErr( const AIDA::IProfile1D* histo, const unsigned int order );
      // ======================================================================
      /// get the skewness for the profile histogram
      static double skewness( const AIDA::IProfile1D* histo );
      // ======================================================================
      /// get the error in skewness for the profile histogram
      static double skewnessErr( const AIDA::IProfile1D* histo );
      // ======================================================================
      /// get the kurtosis for the profile histogram
      static double kurtosis( const AIDA::IProfile1D* histo );
      // ======================================================================
      /// get the error in kurtosis for the profile histogram
      static double kurtosisErr( const AIDA::IProfile1D* histo );
      // ======================================================================
      /// get the mean value for the profile histogram  (just for completeness)
      static double mean( const AIDA::IProfile1D* histo );
      // ======================================================================
      /// get an error in the mean value
      static double meanErr( const AIDA::IProfile1D* histo );
      // ======================================================================
      /// get the rms value for the profile histogram  (just for completeness)
      static double rms( const AIDA::IProfile1D* histo );
      // ======================================================================
      /// get an error in the rms value
      static double rmsErr( const AIDA::IProfile1D* histo );
      // ======================================================================
      /// get an error in the sum bin height ("in-range integral")
      static double sumBinHeightErr( const AIDA::IProfile1D* histo );
      // ======================================================================
      /// get an error in the sum of all bin height ("integral")
      static double sumAllBinHeightErr( const AIDA::IProfile1D* histo );
      // ======================================================================
      /// get the effective entries   (just for completeness)
      static double nEff( const AIDA::IProfile1D* histo );
      // ======================================================================
      /// the fraction of overflow entries  (useful for shape comparison)
      static double overflowEntriesFrac( const AIDA::IProfile1D* histo );
      /// the fraction of underflow entries (useful for shape comparison)
      static double underflowEntriesFrac( const AIDA::IProfile1D* histo );
      /// error on fraction of overflow entries  (useful for shape comparison)
      static double overflowEntriesFracErr( const AIDA::IProfile1D* histo );
      /// the error on fraction of underflow entries (useful for shape comparison)
      static double underflowEntriesFracErr( const AIDA::IProfile1D* histo );
      // ======================================================================
      /// the fraction of overflow intergal  (useful for shape comparison)
      static double overflowIntegralFrac( const AIDA::IProfile1D* histo );
      /// the fraction of underflow integral (useful for shape comparison)
      static double underflowIntegralFrac( const AIDA::IProfile1D* histo );
      /// the error on fraction of overflow intergal
      static double overflowIntegralFracErr( const AIDA::IProfile1D* histo );
      /// the error on fraction of underflow integral
      static double underflowIntegralFracErr( const AIDA::IProfile1D* histo );
      // ======================================================================
      /** get number of entries in profile histogram up to
       *  the certain bin (not-included)
       *  @attention underflow bin is included!
       *  @param histo the pointer to the histogram
       *  @param imax  the bin number (not included)
       *  @param number of entries
       */
      static long nEntries( const AIDA::IProfile1D* histo, const int imax );
      /** get number of entries in profile histogram form the certain
       *  minimal bin up to the certain maximal bin (not-included)
       *  @param histo the pointer to the histogram
       *  @param imin  the minimal bin number (included)
       *  @param imax  the maximal bin number (not included)
       *  @param number of entries
       */
      static long nEntries( const AIDA::IProfile1D* histo,
                            const int               imin, // minimal bin number (included)
                            const int               imax );             // maximal bin number (not included)
      // ======================================================================
      /** get the fraction of entries in profile histogram up to
       *  the certain bin (not-included)
       *  @attention underflow bin is included!
       *  @param histo the pointer to the histogram
       *  @param imax  the bin number (not included)
       *  @param fraction of entries
       */
      static double nEntriesFrac( const AIDA::IProfile1D* histo, const int imax );
      /** get fraction of entries in profile histogram form the certain
       *  minimal bin up to the certain maximal bin (not-included)
       *  @param histo the pointer to the histogram
       *  @param imin  the minimal bin number (included)
       *  @param imax  the maximal bin number (not included)
       *  @param fraction of entries
       */
      static double nEntriesFrac( const AIDA::IProfile1D* histo, const int imin, const int imax );
      // ======================================================================
      /** get the (binominal) error for the fraction of entries
       *  in profile histogram up to the certain bin (not-included)
       *  @attention underflow bin is included!
       *  @param histo the pointer to the histogram
       *  @param imax  the bin number (not included)
       *  @param error for the fraction of entries
       */
      static double nEntriesFracErr( const AIDA::IProfile1D* histo, const int imax );
      /** get the (binomial) error for the fraction of entries in the profile histogram
       *  from the certain minimal bin up to the certain maximal bin (not-included)
       *  @param histo the pointer to the histogram
       *  @param imin  the minimal bin number (included)
       *  @param imax  the maximal bin number (not included)
       *  @param error for the fraction of entries
       */
      static double nEntriesFracErr( const AIDA::IProfile1D* histo, const int imin, const int imax );
      // ======================================================================
    };
    // ========================================================================
  } // namespace Utils
  // ==========================================================================
} // end of namespace Gaudi
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIUTILS_HISTOSTATS_H
