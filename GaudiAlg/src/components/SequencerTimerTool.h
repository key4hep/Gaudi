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

class SequencerTimerTool : public GaudiHistoTool, virtual public ISequencerTimerTool {

public:
  using ISequencerTimerTool::name;
  using ISequencerTimerTool::start;
  using ISequencerTimerTool::stop;

public:
  /// Standard constructor
  SequencerTimerTool( const std::string& type, const std::string& name, const IInterface* parent );

  /** initialize method, to compute the normalization factor **/
  StatusCode initialize() override;

  /** finalize method, to print the time summary table **/
  StatusCode finalize() override;

  /** add a timer entry with the specified name **/
  int addTimer( const std::string& name ) override;

  /** Increase the indentation of the name **/
  void increaseIndent() override { m_indent += 1; }

  /** Decrease the indentation of the name **/
  void decreaseIndent() override { m_indent = std::max( m_indent - 1, 0 ); }

  /** start the counter, i.e. register the current time **/
  void start( int index ) override { m_timerList[index].start(); }

  /** stop the counter, return the elapsed time **/
  double stop( int index ) override { return m_timerList[index].stop(); }

  /** returns the last time **/
  double lastTime( int index ) override { return m_timerList[index].lastTime(); }

  /** returns the name of the counter **/
  const std::string& name( int index ) override { return m_timerList[index].name(); }

  /** returns the index of the counter with that name, or -1 **/
  int indexByName( const std::string& name ) override;

  /** returns the flag telling that global timing is wanted **/
  bool globalTiming() override { return m_globalTiming; };

  /** prepares and saves the timing histograms **/
  void saveHistograms() override;

private:
  Gaudi::Property<int>  m_shots{this, "Shots", 3500000, "number of shots for CPU normalization"};
  Gaudi::Property<bool> m_normalised{this, "Normalised", false, "normalise the time to a nominal PIII"};
  Gaudi::Property<bool> m_globalTiming{this, "GlobalTiming", false};
  Gaudi::Property<std::string::size_type> m_headerSize{this, "NameSize", 30,
                                                       "number of characters to be used in algorithm name column"};

  int                            m_indent = 0; ///< Amount of indentation
  std::vector<TimerForSequencer> m_timerList;
  double                         m_normFactor = 0.001; ///< Factor to convert to standard CPU (1 GHz PIII)
  double                         m_speedRatio = 0;
};
#endif // SEQUENCERTIMERTOOL_H
