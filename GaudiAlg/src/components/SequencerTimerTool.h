// $Id: SequencerTimerTool.h,v 1.7 2005/07/29 16:49:43 hmd Exp $
#ifndef SEQUENCERTIMERTOOL_H 
#define SEQUENCERTIMERTOOL_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiTool.h"
#include "GaudiAlg/ISequencerTimerTool.h"

// local
#include "TimerForSequencer.h"


/** @class SequencerTimerTool SequencerTimerTool.h
 *  Implements the time measurement inside a sequencer
 *
 *  @author Olivier Callot
 *  @date   2004-05-19
 */

class SequencerTimerTool : public GaudiTool, virtual public ISequencerTimerTool{
public: 

 /// Standard constructor
  SequencerTimerTool( const std::string& type, 
                      const std::string& name,
                      const IInterface* parent);

  virtual ~SequencerTimerTool( ); ///< Destructor

  /** initialize method, to compute the normalisation factor **/
  virtual StatusCode initialize();

  /** finalize method, to print the time summary table **/
  virtual StatusCode finalize();

  /** add a timer entry with the specified name **/
  virtual int addTimer( std::string name ) {
    std::string myName;
    if ( 0 < m_indent ) {
      std::string prefix( m_indent, ' ' );
      myName += prefix;
    }
    unsigned int headerSize = 30;
    
    std::string space( headerSize, ' ' );
    myName += name + space ;
    myName = myName.substr( 0, headerSize );
    
    m_timerList.push_back( TimerForSequencer(myName, m_normFactor) );
    return m_timerList.size() -1;
  };

  /** Increase the indentation of the name **/
  virtual void increaseIndent()    { m_indent += 2; };

  /** Decrease the indentation fo the name **/
  virtual void decreaseIndent()    { 
    m_indent -= 2; 
    if ( 0 > m_indent ) m_indent = 0; 
  };

  /** start the counter, i.e. register the current time **/
  void start( int index )  {   m_timerList[index].start();  };

  /** stop the counter, return the elapsed time **/
  double stop( int index )  {   return m_timerList[index].stop();  };
 
  /** returns the last time **/
  double lastTime( int index )  {   return m_timerList[index].lastTime();  };

 /** returns the name of the counter **/
  std::string name( int index )  {   return m_timerList[index].name();  };

  /** returns the index of the counter with that name, or -1 **/
  int indexByName( std::string name );

  /** returns the flag telling that global timing is wanted **/
  virtual bool globalTiming() { return m_globalTiming; };

protected:

private:
  int m_shots;       ///< Number of shots for CPU normalisation
  bool m_normalised; ///< Is the time scaled to a nominal PIII ?
  int m_indent;      ///< Amount of indentation
  std::vector<TimerForSequencer> m_timerList;
  double m_normFactor; ///< Factor to convert to standard CPU (1 GHz PIII)
  double m_speedRatio;
  bool   m_globalTiming;
};
#endif // SEQUENCERTIMERTOOL_H
