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

  StatusCode initialize() override; ///< Algorithm initialization
  StatusCode execute() override;    ///< Algorithm execution

  StatusCode beginRun() override; ///< Algorithm beginRun
  StatusCode endRun() override;   ///< Algorithm endRun

  bool isSequence() const override final { return true; }

  /// Produce string represention of the control flow expression.
  std::ostream& toControlFlowExpression( std::ostream& os ) const override;

private:
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

  /** for asynchronous changes in the list of algorithms */
  void membershipHandler( Gaudi::Details::PropertyBase& theProp );

  /** copy/assignment not allowed **/
  GaudiSequencer( const GaudiSequencer& a ) = delete;
  GaudiSequencer& operator=( const GaudiSequencer& a ) = delete;

  Gaudi::Property<std::vector<std::string>> m_names = {this, "Members", {}, "list of algorithms"};
  Gaudi::Property<bool> m_sequential                = {this, "Sequential", false, "execute members one at a time"};
  Gaudi::Property<bool> m_modeOR                    = {this, "ModeOR", false, "use OR logic instead of AND"};
  Gaudi::Property<bool> m_ignoreFilter              = {this, "IgnoreFilterPassed", false, "always continue"};
  Gaudi::Property<bool> m_measureTime               = {this, "MeasureTime", false, "measure time"};
  Gaudi::Property<bool> m_returnOK     = {this, "ReturnOK", false, "forces the sequencer to return a good status"};
  Gaudi::Property<bool> m_shortCircuit = {this, "ShortCircuit", true, "stop processing as soon as possible"};
  Gaudi::Property<bool> m_invert       = {this, "Invert", false, "invert the logic result of the sequencer"};

  std::vector<AlgorithmEntry> m_entries;      ///< List of algorithms to process.
  ISequencerTimerTool* m_timerTool = nullptr; ///< Pointer to the timer tool
  int m_timer;                                ///< Timer number for the sequencer
};
#endif // GAUDISEQUENCER_H
