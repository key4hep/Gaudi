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
#ifndef ISEQUENCERTIMERTOOL_H
#define ISEQUENCERTIMERTOOL_H 1

// from Gaudi
#include "GaudiKernel/IAlgTool.h"

/** @class ISequencerTimerTool ISequencerTimerTool.h
 *  Implements the time measurement inside a sequencer
 *
 *  @author Olivier Callot
 *  @date   2004-05-19
 */

class GAUDI_API ISequencerTimerTool : public virtual IAlgTool {

public:
  using IAlgTool::name;
  using IAlgTool::start;
  using IAlgTool::stop;

public:
  /// InterfaceID
  DeclareInterfaceID( ISequencerTimerTool, 3, 0 );

  /** add a timer entry with the specified name **/
  virtual int addTimer( const std::string& name ) = 0;

  /** Increase the indentation of the name **/
  virtual void increaseIndent() = 0;

  /** Decrease the indentation of the name **/
  virtual void decreaseIndent() = 0;

  /** start the counter, i.e. register the current time **/
  virtual void start( int index ) = 0;

  /** stop the counter, return the elapsed time **/
  virtual double stop( int index ) = 0;

  /** returns the name of the counter **/
  virtual const std::string& name( int index ) = 0;

  /** returns the last measured time time **/
  virtual double lastTime( int index ) = 0;

  /** returns the index of the counter with that name, or -1 **/
  virtual int indexByName( const std::string& name ) = 0;

  /** returns the flag telling that global timing is wanted **/
  virtual bool globalTiming() = 0;

  /** prepares and saves the timing histograms **/
  virtual void saveHistograms() = 0;
};

#endif // ISEQUENCERTIMERTOOL_H
