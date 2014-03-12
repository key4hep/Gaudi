// # Include files
// Standard:
#include <unistd.h>
#include <math.h>
// Gaudi:
#include "GaudiAlg/GaudiAlgorithm.h"

/// Algorithm which consume a lot of CPU.
///
/// Author: Alexander Mazurov (alexander.mazurov@gmail.com)
class CpuHungryAlg: public GaudiAlgorithm {
public:
  /// Standard constructor
  CpuHungryAlg(const std::string& name, ISvcLocator* pSvcLocator);
  /// ... and destructor
  virtual ~CpuHungryAlg(); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

private:
  double mysin();
  double mycos();
  double mytan();
  double myatan();
private:
  long m_loops;
  int m_nevent;
};


// Register algorithm
DECLARE_COMPONENT(CpuHungryAlg)

CpuHungryAlg::CpuHungryAlg(const std::string& name, ISvcLocator* pSvcLocator)
  : GaudiAlgorithm(name, pSvcLocator),m_nevent(0)
{
  declareProperty("Loops", m_loops = 1000000);
}


CpuHungryAlg::~CpuHungryAlg() {}

/// Initialization.
StatusCode CpuHungryAlg::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc; // error printed already by GaudiAlgorithm

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;
  return StatusCode::SUCCESS;
}

/// Execute.
double CpuHungryAlg::mysin() {
  double result = 0;
  for(long i=0; i <= m_loops; ++i) {
      result += sin(i);
  }
  return result;
}

double CpuHungryAlg::mycos() {
  double result = 0;
  for(long i=0; i<=m_loops; ++i) {
      result += cos(i);
  }
  return result;
}

double CpuHungryAlg::mytan() {
  double result = 0;
  for(long i=0; i<=m_loops; ++i) {
      result += tan(i);
  }
  result  += myatan();
  return result;
}

double CpuHungryAlg::myatan() {
  double result = 0;
  for(long i=0; i<=m_loops; ++i) {
      result += tan(i);
  }
  return result;
}

StatusCode CpuHungryAlg::execute() {
  m_nevent++;
  double result = 0;
  if (name() == "Alg1") {
    result = mysin();
  }else if (name() == "Alg2") {
    result = mycos();
  }else {
    result = mytan();
  }
  // This part is pointless, but prevent a warning about a
  // set, but unused, variable (result).
  if ( msgLevel(MSG::DEBUG) )
    debug() << "Result = " << result << endmsg;
  return StatusCode::SUCCESS;
}

/// Finalize
StatusCode CpuHungryAlg::finalize() {
  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize(); // must be called after all other actions
}

// ============================================================================
