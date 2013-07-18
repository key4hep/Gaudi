///////////////////////// -*- C++ -*- /////////////////////////////
#ifndef MULTIPLELOGSTREAMS_QOTDALG_H
#define MULTIPLELOGSTREAMS_QOTDALG_H 1

/**
 * @class QotdAlg is a simple algorithm to output "famous" quotes of "famous"
 * people into a @c MsgStream stream. Nothing fancier.
 *
 */

// Include files
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"

namespace GaudiEx {

class QotdAlg : public Algorithm
{

public:
  // Constructor of this form must be provided
  QotdAlg(const std::string& name, ISvcLocator* pSvcLocator);

  // Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

private:

  /// MsgStream instance (a std::cout like with print-out levels)
  MsgStream m_msg;

  /// a dumb event counter
  unsigned int m_evtCnt;
};

} //> end namespace GaudiEx

# endif    // MULTIPLELOGSTREAMS_QOTDALG_H
