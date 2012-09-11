// $Id: SequencerTimerTool.h,v 1.7 2005/07/29 16:49:43 hmd Exp $
#ifndef SEQUENCERTIMERTOOL_H
#define SEQUENCERTIMERTOOL_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiHistoTool.h"
#include "GaudiAlg/ISequencerTimerTool.h"

// local
#include "TimerForSequencer.h"


/** @class SequencerTimerTool SequencerTimerTool.h
 *  Implements the time measurement inside a sequencer
 *
 *  The width of the timing table column printing the algorithm name
 *  is 30 by default. That can be changed via
 *  \verbatim
TimingAuditor().addTool(SequencerTimerTool, name = "TIMER")
TimingAuditor().TIMER.NameSize = 50 \endverbatim
 *
 *  @author Olivier Callot
 *  @date   2004-05-19
 */

class SequencerTimerTool : public GaudiHistoTool, virtual public ISequencerTimerTool{
public:

 /// Standard constructor
  SequencerTimerTool( const std::string& type,
                      const std::string& name,
                      const IInterface* parent);

  virtual ~SequencerTimerTool( ); ///< Destructor

  /** initialize method, to compute the normalization factor **/
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
    std::string space( m_headerSize, ' ' );
    myName += name + space ;
    myName = myName.substr( 0, m_headerSize );

    m_timerList.push_back( TimerForSequencer(myName, m_normFactor) );
    return m_timerList.size() -1;
  };

  /** Increase the indentation of the name **/
  virtual void increaseIndent()    { m_indent += 2; };

  /** Decrease the indentation of the name **/
  virtual void decreaseIndent()    {
    m_indent -= 2;
    if ( 0 > m_indent ) m_indent = 0;
  };

  using ISequencerTimerTool::start;
  /** start the counter, i.e. register the current time **/
  void start( int index )  {   m_timerList[index].start();  };

  using ISequencerTimerTool::stop;
  /** stop the counter, return the elapsed time **/
  double stop( int index )  {   return m_timerList[index].stop();  };

  /** returns the last time **/
  double lastTime( int index )  {   return m_timerList[index].lastTime();  };

  using ISequencerTimerTool::name;
  /** returns the name of the counter **/
  std::string name( int index )  {   return m_timerList[index].name();  };

  /** returns the index of the counter with that name, or -1 **/
  int indexByName( std::string name );

  /** returns the flag telling that global timing is wanted **/
  virtual bool globalTiming() { return m_globalTiming; };

  /** prepares and saves the timing histograms **/
  virtual void saveHistograms();

protected:

private:
  int m_shots;       ///< Number of shots for CPU normalization
  bool m_normalised; ///< Is the time scaled to a nominal PIII ?
  int m_indent;      ///< Amount of indentation
  std::vector<TimerForSequencer> m_timerList;
  double m_normFactor; ///< Factor to convert to standard CPU (1 GHz PIII)
  double m_speedRatio;
  bool   m_globalTiming;
  std::string::size_type m_headerSize;   ///< Size of the name field
};
#endif // SEQUENCERTIMERTOOL_H
