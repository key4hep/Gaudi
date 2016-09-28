#ifndef GAUDISEQUENCER_H
#define GAUDISEQUENCER_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

// Forward declarations
class ISequencerTimerTool;

/** @class GaudiSequencer GaudiSequencer.h
 *  Sequencer for executing several algorithms, stopping when one is faulty.
 *
 *  Default behaviour (ModeOR=False) is to execute all algorithms until one returns
 *  filterPassed() = False. If ShortCircuit is set to False, then all algorithms
 *  will be executed.
 *
 *  In OR mode, the logic is opposite. All algorithms until one returns
 *  filterPassed() = True. To then exit one must conter-intuitively set
 *  ShortCircuit to False. If the default value ShortCircuit=True is left
 *  then all algorithms will be executed.
 *
 *  @author Olivier Callot
 *  @date   2004-05-13
 */
class GAUDI_API GaudiSequencer : public GaudiAlgorithm
{
public:
  /// Standard constructor
  GaudiSequencer( const std::string& name, ISvcLocator* pSvcLocator );

  ~GaudiSequencer() override = default; ///< Destructor

  StatusCode initialize() override; ///< Algorithm initialization
  StatusCode execute() override;    ///< Algorithm execution
  StatusCode finalize() override;   ///< Algorithm finalization

  StatusCode beginRun() override; ///< Algorithm beginRun
  StatusCode endRun() override;   ///< Algorithm endRun

  /** for asynchronous changes in the list of algorithms */
  void membershipHandler( Gaudi::Details::PropertyBase& theProp );

protected:
  class AlgorithmEntry final
  {
  public:
    /// Standard constructor
    AlgorithmEntry( Algorithm* alg ) : m_algorithm( alg ) {}

    void setReverse( bool flag ) { m_reverse = flag; }

    Algorithm* algorithm() const { return m_algorithm; }
    bool reverse() const { return m_reverse; }
    void setTimer( int nb ) { m_timer = nb; }
    int timer() const { return m_timer; }
  private:
    Algorithm* m_algorithm = nullptr; ///< Algorithm pointer
    bool m_reverse         = false;   ///< Indicates that the flag has to be inverted
    int m_timer            = 0;       ///< Timer number for this algorithm
  };

  /** Decode a vector of string. */
  StatusCode decodeNames();

private:
  /** copy not allowed **/
  GaudiSequencer( const GaudiSequencer& a ) = delete;

  /** assignment not allowed **/
  GaudiSequencer& operator=( const GaudiSequencer& a ) = delete;

  StringArrayProperty m_names{this, "Members", {}, "list of algorithms"};
  BooleanProperty m_modeOR{this, "ModeOR", false, "use OR loginc instead of AND"};
  BooleanProperty m_ignoreFilter{this, "IgnoreFilterPassed", false, "always continue"};
  BooleanProperty m_measureTime{this, "MeasureTime", false, "measure time"};
  BooleanProperty m_returnOK{this, "ReturnOK", false, "forces the sequencer to return a good status"};
  BooleanProperty m_shortCircuit{this, "ShortCircuit", true, "stop processing as soon as possible"};

  bool m_isInitialized;                       ///< Indicate that we are ready
  std::vector<AlgorithmEntry> m_entries;      ///< List of algorithms to process.
  ISequencerTimerTool* m_timerTool = nullptr; ///< Pointer to the timer tool
  int m_timer;                                ///< Timer number for the sequencer
};
#endif // GAUDISEQUENCER_H
