// $Id: GaudiHistoTool.h,v 1.8 2008/01/17 15:06:06 marcocle Exp $
// ============================================================================
#ifndef GAUDIALG_GAUDIHISTOTOOL_H
#define GAUDIALG_GAUDIHISTOTOOL_H 1
// ============================================================================
/** @file GaudiHistoTool.h
 *
 *  Header file for class : GaudiHistoTool
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date 2004-06-28
 */
// ============================================================================
// Include files
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiTool.h"
#include "GaudiAlg/GaudiHistos.h"
// ============================================================================

// forward declarations
class IHistogramSvc;

// ============================================================================
/** @class GaudiHistoTool GaudiHistoTool.h GaudiTools/GaudiHistoTool.h
 *
 *  Simple class to extend the functionality of class GaudiTool.
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
 *  @date   2004-06-28
 */
// ============================================================================

class GAUDI_API GaudiHistoTool: public GaudiHistos<GaudiTool>
{

protected:

  /** Standard constructor (protected)
   *  @see GaudiTool
   *  @see   AlgTool
   *  @param type tool type (useless)
   *  @param name tool name
   *  @param parent pointer to parent object (service, algorithm or tool)
   */
  GaudiHistoTool ( const std::string& type   ,
                   const std::string& name   ,
                   const IInterface*  parent );

  /// destructor, virtual and protected
  ~GaudiHistoTool() override;

  /** standard initialization method
   *  @see  Algorithm
   *  @see IAlgorithm
   *  @return status code
   */
  StatusCode initialize () override;

  /** standard finalization method
   *  @see  Algorithm
   *  @see IAlgorithm
   *  @return status code
   */
  StatusCode finalize   () override;

private:

  // delete default/copy constructor and assignment
  GaudiHistoTool() = delete;
  GaudiHistoTool           ( const GaudiHistoTool& ) = delete;
  GaudiHistoTool& operator=( const GaudiHistoTool& ) = delete;

};

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_GAUDIHISTOTOOL_H
// ============================================================================
