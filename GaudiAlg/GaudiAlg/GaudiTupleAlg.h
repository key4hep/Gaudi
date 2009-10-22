// $Id: GaudiTupleAlg.h,v 1.10 2005/09/23 16:14:20 hmd Exp $
#ifndef GAUDIALG_GAUDITUPLEALG_H
#define GAUDIALG_GAUDITUPLEALG_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiHistoAlg.h"
#include "GaudiAlg/GaudiTuples.h"
// ============================================================================

// ============================================================================
/* @file GaudiTupleAlg.h
 *
 * Header file for class : GaudiTupleAlg
 *
 * @date 2004-01-23
 * @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 * @author Chris Jones   Christopher.Rob.Jones@cern.ch
 */
// ============================================================================

// ============================================================================
/** @class GaudiTupleAlg GaudiTupleAlg.h GaudiAlg/GaudiTupleAlg.h
 *
 *  Simple class to extend the functionality of class GaudiHistoAlg.
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
 *  @date   2004-01-23
 */
// ============================================================================

class GAUDI_API GaudiTupleAlg: public GaudiTuples<GaudiHistoAlg>
{

protected:

  /** standard constructor
   *  @see GaudiHistoAlg
   *  @see GaudiAlgorithm
   *  @see      Algorithm
   *  @see   IAlgFactory
   */
  GaudiTupleAlg( const std::string& name ,
                 ISvcLocator*       svc  );

  /// destructor
  virtual ~GaudiTupleAlg();

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
  virtual StatusCode finalize  ();

private:

  /// default constructor  is disabled
  GaudiTupleAlg ();
  /// copy constructor     is disabled
  GaudiTupleAlg           ( const  GaudiTupleAlg& );
  /// assignment operator is disabled
  GaudiTupleAlg& operator=( const  GaudiTupleAlg& );

};

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_GAUDITUPLEALG_H
// ============================================================================
