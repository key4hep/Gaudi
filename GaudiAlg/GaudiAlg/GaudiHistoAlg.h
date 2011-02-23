// $Id: GaudiHistoAlg.h,v 1.11 2008/01/17 15:06:06 marcocle Exp $
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
  virtual ~GaudiHistoAlg( );

  /** standard initialization method
   *  @see  Algorithm
   *  @see      IAlgorithm
   *  @return status code
   */
  virtual StatusCode initialize ();

  /** standard finalization method
   *  @see       Algorithm
   *  @see      IAlgorithm
   *  @return status code
   */
  virtual StatusCode finalize  ();

private:

  // default constructor  is disabled
  GaudiHistoAlg ();
  // copy constructor     is disabled
  GaudiHistoAlg           ( const  GaudiHistoAlg& );
  // assignment operator is disabled
  GaudiHistoAlg& operator=( const  GaudiHistoAlg& );

}; // end of class GaudiHistoAlg

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_GAUDIHISTOAL_H
// ============================================================================
