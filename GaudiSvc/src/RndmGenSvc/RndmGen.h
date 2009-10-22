//====================================================================
//	Random RndmGen class definition
//--------------------------------------------------------------------
//
//	Package    : Gaudi/RndmGen ( The LHCb Offline System)
//	Author     : M.Frank
//====================================================================
#ifndef GAUDI_RANDOMGENSVC_RNDMGEN_H
#define GAUDI_RANDOMGENSVC_RNDMGEN_H 1

// Framework include files
#include "GaudiKernel/IRndmGen.h"

// Forward declarations
class IRndmEngine;

/** Random Generator definition

    Description:
    Definition of a interface for a generic random
    number generators.

    Dependencies:
    <UL>
    <LI> Generic Service interface: Gaudi/Interfaces/IInterface.h
    </UL>

    <P> History    :
    <PRE>
    +---------+----------------------------------------------+--------+
    |    Date |                 Comment                      | Who    |
    +---------+----------------------------------------------+--------+
    | 21/11/99| Initial version.                             | MF     |
    +---------+----------------------------------------------+--------+
    </PRE>
    Author:  M.Frank
    Version: 1.0
*/
class RndmGen : public implements1<IRndmGen> {

protected:
  /// Generation parameters
  IRndmGen::Param*  m_params;
  /// Hosting service: Access must always be possible
  IRndmEngine*      m_engine;

  /// Standard Constructor
  RndmGen(IInterface* engine);
  /// Standard Destructor
  virtual ~RndmGen();

public:

  /** IRndmGen implementation    */
  /// Initialize the generator
  virtual StatusCode initialize(const IRndmGen::Param& par);
  /// Initialize the generator
  virtual StatusCode finalize();
  /// Random number generator type
  virtual const InterfaceID& type() const    {
    return (m_params != 0) ? m_params->type() : IID_IRndmFlat;
  }
  /// Random number generator ID
  virtual long ID() const    {
    return long(this);
  }
  /// Access to random number generator parameters
  virtual const IRndmGen::Param* parameters() const    {
    return m_params;
  }
  /// Single shot returning single random number according to specified distribution
  virtual double shoot() const;
  /** Multiple shots returning vector with random number according to specified distribution.
      @param  array    Array containing random numbers
      @param  howmany  fill 'howmany' random numbers into array
      @param  start    ... starting at position start
  */
  virtual StatusCode shootArray( std::vector<double>& array, long howmany, long start) const;
};

#endif // GAUDI_RANDOMGENSVC_RNDMGEN_H
