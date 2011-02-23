// $Id: HistoStats.h,v 1.3 2007/09/19 08:16:59 marcocle Exp $
// ============================================================================
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
namespace AIDA { class IHistogram1D ; }
// ============================================================================
namespace Gaudi
{
  namespace Utils
  {
    // ========================================================================
    /** @class HistoStats HistoStats.h GaudiUtils/HistoStats.h
     *  The collection of trivial functions to access  the
     *  statistical information for the histograms
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-08-06
     */
    class GAUDI_API HistoStats
    {
    public :
      // ======================================================================
      /** get the "bin-by-bin"-moment around the specified  "value"
       *  @param histo histogram
       *  @param order the moment parameter
       *  @param value central value
       *  @return the evaluated moment
       */
      static double moment
      ( const AIDA::IHistogram1D* histo     ,
        const unsigned int        order     ,
        const double              value = 0 ) ;
      // ======================================================================
      /** evaluate the uncertanty for 'bin-by-bin'-moment
       *  @param histo histogram
       *  @param order the moment parameter
       *  @param value central value
       *  @return the evaluated uncertanty in the moment
       */
      static double momentErr
      ( const AIDA::IHistogram1D* histo ,
        const unsigned int        order ) ;
      // ======================================================================
      /** evaluate the 'bin-by-bin'-central moment (around the mean value)
       *  @param histo histogram
       *  @param order the moment parameter
       *  @param value central value
       *  @return the evaluated central moment
       */
      static double centralMoment
      ( const AIDA::IHistogram1D* histo ,
        const unsigned int        order ) ;
      // ======================================================================
      /** evaluate the uncertanty for 'bin-by-bin'-central moment
       *  (around the mean value)
       *  ( the uncertanty is calculated with O(1/n2) precision)
       *  @param histo histogram
       *  @param order the moment parameter
       *  @param value central value
       *  @return the evaluated uncertanty in the central moment
       */
      static double centralMomentErr
      ( const AIDA::IHistogram1D* histo ,
        const unsigned int        order ) ;
      // ======================================================================
      /// get the skewness for the histogram
      static double skewness
      ( const AIDA::IHistogram1D* histo ) ;
      // ======================================================================
      /// get the error in skewness for the histogram
      static double skewnessErr
      ( const AIDA::IHistogram1D* histo ) ;
      // ======================================================================
      /// get the kurtosis for the histogram
      static double kurtosis
      ( const AIDA::IHistogram1D* histo ) ;
      // ======================================================================
      /// get the error in kurtosis for the histogram
      static double kurtosisErr
      ( const AIDA::IHistogram1D* histo ) ;
      // ======================================================================
      /// get the mean value for the histogram  (just for completeness)
      static double mean
      ( const AIDA::IHistogram1D* histo ) ;
      // ======================================================================
      /// get an error in the mean value
      static double meanErr
      ( const AIDA::IHistogram1D* histo ) ;
      // ======================================================================
      /// get the rms value for the histogram  (just for completeness)
      static double rms
      ( const AIDA::IHistogram1D* histo ) ;
      // ======================================================================
      /// get an error in the rms value
      static double rmsErr
      ( const AIDA::IHistogram1D* histo ) ;
      // ======================================================================
      /// get the effective entries   (just for completeness)
      static double nEff
      ( const AIDA::IHistogram1D* histo     ) ;
      // ======================================================================
      /// get an error in the sum bin height ("in-range integral")
      static double sumBinHeightErr
      ( const AIDA::IHistogram1D* histo ) ;
      // ======================================================================
      /// get an error in the sum of all bin height ("integral")
      static double sumAllBinHeightErr
      ( const AIDA::IHistogram1D* histo ) ;
      // ======================================================================
      /// the fraction of overflow entries  (useful for shape comparison)
      static double overflowEntriesFrac 
      ( const AIDA::IHistogram1D* histo     ) ;
      /// the fraction of underflow entries (useful for shape comparison)
      static double underflowEntriesFrac
      ( const AIDA::IHistogram1D* histo     ) ;
      /// error on fraction of overflow entries  (useful for shape comparison)
      static double overflowEntriesFracErr 
      ( const AIDA::IHistogram1D* histo     ) ;
      /// the error on fraction of underflow entries (useful for shape comparison)
      static double underflowEntriesFracErr  
      ( const AIDA::IHistogram1D* histo     ) ;
      // ======================================================================
      /// the fraction of overflow intergal  (useful for shape comparison)
      static double overflowIntegralFrac
      ( const AIDA::IHistogram1D* histo     ) ;
      /// the fraction of underflow integral (useful for shape comparison)
      static double underflowIntegralFrac
      ( const AIDA::IHistogram1D* histo     ) ;
      /// the error on fraction of overflow intergal 
      static double overflowIntegralFracErr 
      ( const AIDA::IHistogram1D* histo     ) ;
      /// the error on fraction of underflow integral
      static double underflowIntegralFracErr 
      ( const AIDA::IHistogram1D* histo     ) ;
      // ======================================================================
      /** get number of entries in histogram up to 
       *  the certain bin (not-included)
       *  @attention underflow bin is included! 
       *  @param histo the pointer to the histogram 
       *  @param imax  the bin number (not included) 
       *  @param number of entries 
       */
      static long   nEntries 
      ( const AIDA::IHistogram1D* histo , 
        const int                 imax  ) ;
      /** get number of entries in histogram form the certain 
       *  minimal bin up to the certain maximal bin (not-included)
       *  @param histo the pointer to the histogram 
       *  @param imin  the minimal bin number (included) 
       *  @param imax  the maximal bin number (not included) 
       *  @param number of entries 
       */
      static long   nEntries 
      ( const AIDA::IHistogram1D* histo , 
        const int                 imin  ,  //     minimal bin number (included) 
        const int                 imax  ) ;// maximal bin number (not included) 
      // ======================================================================
      /** get the fraction of entries in histogram up to 
       *  the certain bin (not-included)
       *  @attention underflow bin is included! 
       *  @param histo the pointer to the histogram 
       *  @param imax  the bin number (not included) 
       *  @param fraction of entries 
       */
      static double nEntriesFrac 
      ( const AIDA::IHistogram1D* histo , 
        const int                 imax  ) ;
      /** get fraction of entries in histogram form the certain 
       *  minimal bin up to the certain maximal bin (not-included)
       *  @param histo the pointer to the histogram 
       *  @param imin  the minimal bin number (included) 
       *  @param imax  the maximal bin number (not included) 
       *  @param fraction of entries 
       */
      static double nEntriesFrac 
      ( const AIDA::IHistogram1D* histo , 
        const int                 imin  ,  //     minimal bin number (included) 
        const int                 imax  ) ;// maximal bin number (not included) 
      // ======================================================================
      /** get the (binominal) error for the fraction of entries 
       *  in histogram up to the certain bin (not-included)
       *  @attention underflow bin is included! 
       *  @param histo the pointer to the histogram 
       *  @param imax  the bin number (not included) 
       *  @param error for the fraction of entries 
       */
      static double nEntriesFracErr
      ( const AIDA::IHistogram1D* histo , 
        const int                 imax  ) ;
      /** get the (binomial) error for the fraction of entries in histogram 
       *  from the certain minimal bin up to the certain maximal bin (not-included)
       *  @param histo the pointer to the histogram 
       *  @param imin  the minimal bin number (included) 
       *  @param imax  the maximal bin number (not included) 
       *  @param error for the fraction of entries 
       */
      static double nEntriesFracErr
      ( const AIDA::IHistogram1D* histo , 
        const int                 imin  ,  //     minimal bin number (included) 
        const int                 imax  ) ;// maximal bin number (not included) 
      // ======================================================================
    } ;
    // ========================================================================
  } // end of namespace Gaudi::Utils
  // ==========================================================================
} // end of namespace Gaudi
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIUTILS_HISTOSTATS_H
// ============================================================================
