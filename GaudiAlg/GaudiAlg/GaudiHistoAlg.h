#ifndef GAUDIALG_GAUDIHISTOALG_H
#define GAUDIALG_GAUDIHISTOALG_H 1
// ============================================================================
/* @file GaudiHistoAlg.h
 *
 * Header file for class : GaudiHistoAlg
 *
 * @date 30/06/2001
 * @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 * @author Chris Jones   Christopher.Rob.Jones@cern.ch
 */
// ============================================================================
// Include files
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiAlg/GaudiHistos.h"
// ============================================================================
/** @class GaudiHistoAlg GaudiHistoAlg.h GaudiAlg/GaudiHistoAlg.h
 *
 *  Simple class to extend the functionality of class GaudiAlgorithm.
 *
 *  Class is instrumented with a selection of plotting functions for
 *  easy creation of histograms.
 *
 *  @attention
 *  See the class GaudiHistos, which implements the common functionality
 *  between GaudiHistoTool and GaudiHistoAlg
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   2003-12-11
 */
// ============================================================================

class GAUDI_API GaudiHistoAlg : public GaudiHistos<GaudiAlgorithm>
{

protected:

  /** Standard constructor (protected)
   *  @see  GaudiAlgorithm
   *  @see       Algorithm
   *  @param name           name of the algorithm
   *  @param pSvcLocator    poinetr to Service Locator
   */
  GaudiHistoAlg ( const std::string& name,
                  ISvcLocator*       pSvc );

  /// virtual destructor
  ~GaudiHistoAlg( ) override;

  /** standard initialization method
   *  @see  Algorithm
   *  @see      IAlgorithm
   *  @return status code
   */
  StatusCode initialize () override;

  /** standard finalization method
   *  @see       Algorithm
   *  @see      IAlgorithm
   *  @return status code
   */
  StatusCode finalize  () override;

private:

  // delete  default/copy constructor and assignment
  GaudiHistoAlg () = delete;
  GaudiHistoAlg           ( const  GaudiHistoAlg& ) = delete;
  GaudiHistoAlg& operator=( const  GaudiHistoAlg& ) = delete;

}; // end of class GaudiHistoAlg

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_GAUDIHISTOAL_H
// ============================================================================
