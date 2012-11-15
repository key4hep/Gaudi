// $Id: $
#ifndef GAUDIUTILS_HISTOLABELS_H
#define GAUDIUTILS_HISTOLABELS_H 1

#include <vector>
#include <string>
#include <utility>

#include "GaudiKernel/Kernel.h"

namespace AIDA
{
  class IHistogram1D;
  class IHistogram2D;
  class IProfile1D;
  class IProfile2D;
}

namespace Gaudi
{
  namespace Utils
  {
    namespace Histos
    {
      // --------------------------------------------------------------------------

      /// Typedef for a list of labels
      typedef std::vector<std::string>        Labels;
      /// Typedef for a bin number and its associated label
      typedef std::pair<unsigned,std::string> BinLabel;
      /// Typedef for a list of bin numbers and their associated label
      typedef std::vector<BinLabel>           BinLabels;

      // --------------------------------------------------------------------------

      /** @brief Set the Bin labels for a given 1D histogram
       *
       *  The labels will be applied in the order they appear in the list,
       *  starting at the first bin. If the list of labels is too short, the
       *  later bins will be missing a label. If the list is too long, only
       *  the first N will be used, where N is the number of bins in the histogram
       *
       *  @param hist   Pointer to the histogram
       *  @param labels The list of labels
       *
       *  @return Boolean indicating if the labels were successfully applied or not
       *  @retval TRUE  Labels were applied OK
       *  @retval FALSE Labels were NOT applied
       */
      GAUDI_API bool setBinLabels( AIDA::IHistogram1D* hist,
                                   const Labels& labels ) ;

      /** @brief Set the Bin labels for a given 1D histogram
       *
       *  Each entry in 'labels' gives the bin number and its associated label
       *
       *  @param hist   Pointer to the histogram
       *  @param labels The list of labels
       *
       *  @return Boolean indicating if the labels were successfully applied or not
       *  @retval TRUE  Labels were applied OK
       *  @retval FALSE Labels were NOT applied
       */
      GAUDI_API bool setBinLabels( AIDA::IHistogram1D* hist,
                                   const BinLabels& labels ) ;

      // --------------------------------------------------------------------------

      /** @brief Set the Bin labels for a given 1D profile histogram
       *
       *  The labels will be applied in the order they appear in the list,
       *  starting at the first bin. If the list of labels is too short, the
       *  later bins will be missing a label. If the list is too long, only
       *  the first N will be used, where N is the number of bins in the histogram
       *
       *  @param hist   Pointer to the histogram
       *  @param labels The list of labels
       *
       *  @return Boolean indicating if the labels were successfully applied or not
       *  @retval TRUE  Labels were applied OK
       *  @retval FALSE Labels were NOT applied
       */
      GAUDI_API bool setBinLabels( AIDA::IProfile1D* hist,
                                   const Labels& labels ) ;

      /** @brief Set the Bin labels for a given 1D profile histogram
       *
       *  Each entry in 'labels' gives the bin number and its associated label
       *
       *  @param hist   Pointer to the histogram
       *  @param labels The list of bin numbers and the associated label
       *
       *  @return Boolean indicating if the labels were successfully applied or not
       *  @retval TRUE  Labels were applied OK
       *  @retval FALSE Labels were NOT applied
       */
      GAUDI_API bool setBinLabels( AIDA::IProfile1D* hist,
                                   const BinLabels & labels ) ;

      // --------------------------------------------------------------------------

      /** @brief Set the Bin labels for a given 2D histogram
       *
       *  The labels will be applied in the order they appear in the lists,
       *  starting at the first bin. If the list of labels is too short, the
       *  later bins will be missing a label. If the list is too long, only
       *  the first N will be used, where N is the number of bins in the histogram
       *
       *  @param hist   Pointer to the histogram
       *  @param xlabels The list of x labels
       *  @param ylabels The list of y labels
       *
       *  @return Boolean indicating if the labels were successfully applied or not
       *  @retval TRUE  Labels were applied OK
       *  @retval FALSE Labels were NOT applied
       */
      GAUDI_API bool setBinLabels( AIDA::IHistogram2D* hist,
                                   const Labels& xlabels,
                                   const Labels& ylabels ) ;

      /** @brief Set the Bin labels for a given 2D histogram
       *
       *  Each entry in 'labels' lists gives the bin number and its associated label
       *
       *  @param hist   Pointer to the histogram
       *  @param xlabels The list of x bin numbers and the associated label
       *  @param ylabels The list of y bin numbers and the associated label
       *
       *  @return Boolean indicating if the labels were successfully applied or not
       *  @retval TRUE  Labels were applied OK
       *  @retval FALSE Labels were NOT applied
       */
      GAUDI_API bool setBinLabels( AIDA::IHistogram2D* hist,
                                   const BinLabels& xlabels,
                                   const BinLabels& ylabels ) ;

      // --------------------------------------------------------------------------

      /** @brief Set the Bin labels for a given 2D profile histogram
       *
       *  The labels will be applied in the order they appear in the lists,
       *  starting at the first bin. If the list of labels is too short, the
       *  later bins will be missing a label. If the list is too long, only
       *  the first N will be used, where N is the number of bins in the histogram
       *
       *  @param hist   Pointer to the histogram
       *  @param xlabels The list of x labels
       *  @param ylabels The list of y labels
       *
       *  @return Boolean indicating if the labels were successfully applied or not
       *  @retval TRUE  Labels were applied OK
       *  @retval FALSE Labels were NOT applied
       */
      GAUDI_API bool setBinLabels( AIDA::IProfile2D* hist,
                                   const Labels& xlabels,
                                   const Labels& ylabels ) ;

      /** @brief Set the Bin labels for a given 2D profile histogram
       *
       *  Each entry in 'labels' lists gives the bin number and its associated label
       *
       *  @param hist   Pointer to the histogram
       *  @param xlabels The list of x bin numbers and the associated label
       *  @param ylabels The list of y bin numbers and the associated label
       *
       *  @return Boolean indicating if the labels were successfully applied or not
       *  @retval TRUE  Labels were applied OK
       *  @retval FALSE Labels were NOT applied
       */
      GAUDI_API bool setBinLabels( AIDA::IProfile2D* hist,
                                   const BinLabels& xlabels,
                                   const BinLabels& ylabels ) ;

      // --------------------------------------------------------------------------

      /** @brief Set the axis labels for the given 1D histogram
       *
       *  @param hist  Pointer to the histogram
       *  @param xAxis Label for the x axis
       *  @param yAxis Label for the y axis
       *
       *  @return Boolean indicating if the labels were successfully applied or not
       *  @retval TRUE  Labels were applied OK
       *  @retval FALSE Labels were NOT applied
       */
      GAUDI_API bool setAxisLabels( AIDA::IHistogram1D* hist,
                                    const std::string & xAxis,
                                    const std::string & yAxis ) ;

      /** @brief Set the axis labels for the given 1D profile histogram
       *
       *  @param hist  Pointer to the histogram
       *  @param xAxis Label for the x axis
       *  @param yAxis Label for the y axis
       *
       *  @return Boolean indicating if the labels were successfully applied or not
       *  @retval TRUE  Labels were applied OK
       *  @retval FALSE Labels were NOT applied
       */
      GAUDI_API bool setAxisLabels( AIDA::IProfile1D* hist,
                                    const std::string & xAxis,
                                    const std::string & yAxis ) ;

      // --------------------------------------------------------------------------

      /** @brief Set the axis labels for the given 2D histogram
       *
       *  @param hist  Pointer to the histogram
       *  @param xAxis Label for the x axis
       *  @param yAxis Label for the y axis
       *
       *  @return Boolean indicating if the labels were successfully applied or not
       *  @retval TRUE  Labels were applied OK
       *  @retval FALSE Labels were NOT applied
       */
      GAUDI_API bool setAxisLabels( AIDA::IHistogram2D* hist,
                                    const std::string & xAxis,
                                    const std::string & yAxis ) ;

      /** @brief Set the axis labels for the given 2D profile histogram
       *
       *  @param hist  Pointer to the histogram
       *  @param xAxis Label for the x axis
       *  @param yAxis Label for the y axis
       *
       *  @return Boolean indicating if the labels were successfully applied or not
       *  @retval TRUE  Labels were applied OK
       *  @retval FALSE Labels were NOT applied
       */
      GAUDI_API bool setAxisLabels( AIDA::IProfile2D* hist,
                                    const std::string & xAxis,
                                    const std::string & yAxis ) ;

      // --------------------------------------------------------------------------

    }
  }
}

#endif // GAUDIUTILS_HISTOLABELS_H
