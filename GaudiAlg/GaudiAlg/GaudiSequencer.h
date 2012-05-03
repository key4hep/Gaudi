// $Id: GaudiSequencer.h,v 1.5 2008/01/10 13:11:11 marcocle Exp $
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
 *  filterPassed() = True. To then exit one must onter-intuitively set 
 *  ShortCircuit to False. If the default value ShortCircuit=True is left
 *  then all algorithms will be executed.
 *
 *  @author Olivier Callot
 *  @date   2004-05-13
 */
class GAUDI_API GaudiSequencer: public GaudiAlgorithm {
public:
  /// Standard constructor
  GaudiSequencer( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~GaudiSequencer( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

  virtual StatusCode beginRun  ();    ///< Algorithm beginRun
  virtual StatusCode endRun    ();    ///< Algorithm endRun

  void resetExecuted();         ///< Called before an event processing

  /** for asynchronous changes in the list of algorithms */
  void membershipHandler( Property& theProp );

protected:

  class AlgorithmEntry {
  public:
    /// Standard constructor
    AlgorithmEntry( Algorithm* alg ) {
      m_algorithm = alg;
      m_reverse = false;
      m_timer = 0;
    }

    virtual ~AlgorithmEntry( ) {}; ///< Destructor
    void setReverse( bool flag )            { m_reverse   = flag; }

    Algorithm* algorithm()        const  { return m_algorithm; }
    bool       reverse()          const  { return m_reverse;   }
    void       setTimer( int nb )        { m_timer = nb;       }
    int        timer()            const  { return m_timer;     }
  private:
    Algorithm*  m_algorithm;   ///< Algorithm pointer
    bool        m_reverse;     ///< Indicates that the flag has to be inverted
    int         m_timer;       ///< Timer number fo rthis algorithm
  };

  /** Decode a vector of string. */
  StatusCode decodeNames(  );

private:

  /** Private copy, copy not allowed **/
  GaudiSequencer( const GaudiSequencer& a );

  /** Private  assignment operator: This is not allowed **/
  GaudiSequencer& operator=( const GaudiSequencer& a );

  StringArrayProperty   m_names;         ///< Input string, list of algorithms
  std::vector<AlgorithmEntry> m_entries; ///< List of algorithms to process.
  bool m_modeOR;                         ///< Indicates that the OR is wanted instead of AND
  bool m_shortCircuit;                   ///< Indicates whether to stop processing as soon as possible,
                                         ///     or to always execute _all_ subalgorithms.
                                         ///     In MOdeOR=True the behaviour is the exact opposite.
  bool m_ignoreFilter;                   ///< True if one continues always.
  bool m_isInitialized;                  ///< Indicate that we are ready
  bool m_measureTime;                    ///< Flag to measure time
  bool m_returnOK;                       ///< Forces the sequencer to return a good status
  ISequencerTimerTool* m_timerTool;      ///< Pointer to the timer tool
  int  m_timer;                          ///< Timer number for the sequencer
};
#endif // GAUDISEQUENCER_H
