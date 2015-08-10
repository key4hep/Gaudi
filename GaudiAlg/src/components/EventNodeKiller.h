#ifndef EVENTNODEKILLER_H 
#define EVENTNODEKILLER_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"
#include <vector>
#include <string>

/** @class EventNodeKiller EventNodeKiller.h
 *  Algorithm to kill nodes in the event data store. Useful if you want to
 *  make the application "forget" about the previous history of the input file
 *
 *  @author Marco Cattaneo
 *  @date   2005-07-14
 */
class EventNodeKiller : public GaudiAlgorithm {
public: 
  /// Standard constructor
  EventNodeKiller( const std::string& name, ISvcLocator* pSvcLocator );

  ~EventNodeKiller( ) override = default; ///< Destructor

  StatusCode execute() override;    ///< Algorithm execution

private:
  std::vector<std::string> m_nodes; ///< String property "Nodes"
};
#endif // EVENTNODEKILLER_H
