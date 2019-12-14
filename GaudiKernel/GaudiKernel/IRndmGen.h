/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_IRNDMGEN_H
#define GAUDIKERNEL_IRNDMGEN_H

// STL include files
#include <string>
#include <vector>

// Framework include files
#include "GaudiKernel/IInterface.h"

// Declaration of the interface ID ( interface id, major version, minor version)
static const InterfaceID IID_IRndmBit( 150, 1, 0 );
static const InterfaceID IID_IRndmFlat( 151, 1, 0 );
static const InterfaceID IID_IRndmChi2( 152, 1, 0 );
static const InterfaceID IID_IRndmGamma( 153, 1, 0 );
static const InterfaceID IID_IRndmGauss( 154, 1, 0 );
static const InterfaceID IID_IRndmLandau( 155, 1, 0 );
static const InterfaceID IID_IRndmPoisson( 156, 1, 0 );
static const InterfaceID IID_IRndmStudentT( 157, 1, 0 );
static const InterfaceID IID_IRndmBinomial( 158, 1, 0 );
static const InterfaceID IID_IRndmExponential( 159, 1, 0 );
static const InterfaceID IID_IRndmBreitWigner( 160, 1, 0 );
static const InterfaceID IID_IRndmBreitWignerCutOff( 161, 1, 0 );
static const InterfaceID IID_IRndmDefinedPdf( 162, 1, 0 );
static const InterfaceID IID_IRndmGaussianTail( 163, 1, 0 );

/** @class IRndmGen IRndmGen.h GaudiKernel/IRndmGen.h

    Definition of a interface for a generic random number generators.

    @author  M.Frank
    @version 1.0
*/
class GAUDI_API IRndmGen : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( IRndmGen, 2, 0 );

  class Param {
  protected:
    /// Type of the generator
    const InterfaceID m_type;

  public:
    /// Standard constructor
    Param( const InterfaceID& type = IID_IRndmFlat ) : m_type( type ) {}
    /// Standard Destructor
    virtual ~Param() = default;
    /// Parameter's type
    virtual const InterfaceID& type() const { return m_type; }
    /// Clone parameters
    virtual Param* clone() const = 0;
  };

  /// Initialize the generator
  virtual StatusCode initialize( const IRndmGen::Param& par ) = 0;
  /// Finalize the generator
  virtual StatusCode finalize() = 0;
  /// Random number generator type
  virtual const InterfaceID& type() const = 0;
  /// Random generator ID
  virtual long ID() const = 0;
  /// Access to random number generator parameters
  virtual const IRndmGen::Param* parameters() const = 0;
  /// Single shot returning single random number according to specified distribution
  virtual double shoot() const = 0;
  /** Multiple shots returning vector with random number according to specified distribution.
      @param  array    Array containing random numbers
      @param  howmany  fill 'howmany' random numbers into array
      @param  start    ... starting at position start
      @param Statuscode indicating success or failure
  */
  virtual StatusCode shootArray( std::vector<double>& array, long howmany, long start = 0 ) const = 0;

  /// Virtual destructor
  virtual ~IRndmGen() = default;
};

#endif // GAUDIKERNEL_IRNDMGEN_H
