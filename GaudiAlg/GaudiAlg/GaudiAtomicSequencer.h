// $Id: GaudiAtomicSequencer.h,v 0.1 2014/05/26 13:11:11 dfunke Exp $
#ifndef GAUDIATOMICSEQUENCER_H
#define GAUDIATOMICSEQUENCER_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiSequencer.h"

/** @class GaudiAtomicSequencer GaudiAtomicSequencer.h
 *  Sequencer for executing several algorithms, stopping when one is faulty.
 *  The algorithms are treated as an atomic block of operations and are NOT unrolled by
 *  the GaudiHive AlgResourcePool
 *
 *  Default behaviour (ModeOR=False) is to execute all algorithms until one returns 
 *  filterPassed() = False. If ShortCircuit is set to False, then all algorithms 
 *  will be executed.
 *
 *  In OR mode, the logic is opposite. All algorithms until one returns 
 *  filterPassed() = True. To then exit one must onter-intuitively set 
 *  ShortCircuit to False. If the default value ShortCircuit=True is left
 *  then all algorithms will be executed.
 *
 *  @author Daniel Funke
 *  @date   2014-05-26
 */
class GAUDI_API GaudiAtomicSequencer: public GaudiSequencer {
public:
  /// Standard constructor
	GaudiAtomicSequencer( const std::string& name, ISvcLocator* pSvcLocator );

  ~GaudiAtomicSequencer() override; ///< Destructor

  StatusCode initialize() override;    ///< Algorithm initialization

protected:

private:

  /** Private copy, copy not allowed **/
  GaudiAtomicSequencer( const GaudiAtomicSequencer& a );

  /** Private  assignment operator: This is not allowed **/
  GaudiAtomicSequencer& operator=( const GaudiAtomicSequencer& a );
};
#endif // GAUDIATOMICSEQUENCER_H
