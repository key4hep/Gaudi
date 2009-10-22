// $Id: GaudiTupleTool.h,v 1.7 2005/09/23 16:14:20 hmd Exp $
// ============================================================================
#ifndef GAUDIALG_GAUDITUPLETOOL_H
#define GAUDIALG_GAUDITUPLETOOL_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiHistoTool.h"
#include "GaudiAlg/GaudiTuples.h"
// ============================================================================

// ============================================================================
/*  @file GaudiTupleTool.h
 *
 *  Header file for class : GaudiTupleTool
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   2004-06-28
 */
// ============================================================================

// ============================================================================
/** @class GaudiTupleTool GaudiTupleTool.h GaudiAlg/GaudiTupleTool.h
 *
 *  Simple class to extend the functionality of class GaudiHistoTool.
 *
 *  Class is instrumented with few methods
 *  for dealing with N-tuples and Event Tag collections
 *
 *  @attention
 *  See the class GaudiTuples, which implements the common functionality
 *  between GaudiTupleTool and GaudiTupleAlg
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   2004-06-28
 */
// ============================================================================

class GAUDI_API GaudiTupleTool: public GaudiTuples<GaudiHistoTool>
{

protected:

  /** standard constructor
   *  @see GaudiTool
   *  @see   AlgTool
   *  @param type tool type
   *  @param name tool name
   *  @param parent pointer to the parent component
   */
  GaudiTupleTool( const std::string& type   ,
                  const std::string& name   ,
                  const IInterface*  parent );

  /// destructor
  virtual ~GaudiTupleTool() ;

  /** standard initialization method
   *  @see  Algorithm
   *  @see IAlgorithm
   *  @return status code
   */
  virtual StatusCode initialize ();

  /** standard finalization method
   *  @see  Algorithm
   *  @see IAlgorithm
   *  @return status code
   */
  virtual StatusCode finalize   ();

private:

  /// default constructor  is disabled
  GaudiTupleTool() ;
  /// copy constructor     is disabled
  GaudiTupleTool           ( const GaudiTupleTool& ) ;
  /// assignment operator is disabled
  GaudiTupleTool& operator=( const GaudiTupleTool& ) ;

};

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_GAUDITUPLETOOL_H
// ============================================================================
