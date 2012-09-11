// $Id: ISequencerTimerTool.h,v 1.4 2005/07/29 16:49:43 hmd Exp $
#ifndef ISEQUENCERTIMERTOOL_H
#define ISEQUENCERTIMERTOOL_H 1

// Include files
// from Gaudi
#include "GaudiKernel/IAlgTool.h"

/** @class ISequencerTimerTool ISequencerTimerTool.h
 *  Implements the time measurement inside a sequencer
 *
 *  @author Olivier Callot
 *  @date   2004-05-19
 */

class GAUDI_API ISequencerTimerTool: public virtual IAlgTool {
public:
  /// InterfaceID
  DeclareInterfaceID(ISequencerTimerTool,2,1);

  /** add a timer entry with the specified name **/
  virtual int addTimer( std::string name ) = 0 ;

  /** Increase the indentation of the name **/
  virtual void increaseIndent() = 0 ;

  /** Decrease the indentation of the name **/
  virtual void decreaseIndent() = 0;

  using IAlgTool::start;
  /** start the counter, i.e. register the current time **/
  virtual void start( int index ) = 0;

  using IAlgTool::stop;
  /** stop the counter, return the elapsed time **/
  virtual double stop( int index ) = 0;

  using IAlgTool::name;
  /** returns the name of the counter **/
  virtual std::string name( int index ) = 0;

  /** returns the last measured time time **/
  virtual double lastTime( int index ) = 0;

  /** returns the index of the counter with that name, or -1 **/
  virtual int indexByName( std::string name ) = 0;

  /** returns the flag telling that global timing is wanted **/
  virtual bool globalTiming() = 0;

  /** prepares and saves the timing histograms **/
  virtual void saveHistograms() = 0;

  /** Destructor */
  virtual ~ISequencerTimerTool();
};
#endif // ISEQUENCERTIMERTOOL_H
