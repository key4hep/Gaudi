// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/IRndmEngine.h,v 1.2 2001/01/19 14:22:55 mato Exp $
#ifndef GAUDIKERNEL_IRNDMENGINE_H
#define GAUDIKERNEL_IRNDMENGINE_H

// STL include files
#include <vector>

// Framework include files
#include "GaudiKernel/IInterface.h"

/** @class IRndmEngine IRndmEngine.h GaudiKernel/IRndmEngine.h

    Definition of a interface for a generic random number generator
    giving randomly distributed numbers in the range [0...1]
    number generators.

    @author  M.Frank
    @version 1.0
*/
class GAUDI_API IRndmEngine: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IRndmEngine,2,0);
  /// Single shot returning single random number
  virtual double rndm() const = 0;
  /** Multiple shots returning vector with flat random numbers.
      @param  array    Array containing random numbers
      @param  howmany  fill 'howmany' random numbers into array
      @param  start    ... starting at position start
      @return StatusCode indicating failure or success.
  */
  virtual StatusCode rndmArray( std::vector<double>& array, long howmany, long start = 0) const = 0;
  /// Allow to set new seeds
  virtual StatusCode setSeeds(const std::vector<long>& seeds) = 0;
  /// Allow to retrieve seeds
  virtual StatusCode seeds(std::vector<long>& seeds)   const  = 0;
};

#endif // GAUDI_INTERFACES_IRNDMENGINE_H
