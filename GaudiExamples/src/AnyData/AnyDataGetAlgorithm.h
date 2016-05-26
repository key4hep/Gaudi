#ifndef ANYDATA_ANADATAGETALGORITHM_H
#define ANYDATA_ANADATAGETALGORITHM_H 1

// Include files
#include <string>
#include <vector>

// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

/** @class AnyDataGetAlgorithm AnyDataGetAlgorithm.h AnyData/AnyDataGetAlgorithm.h
 *
 *
 *  @author Roel Aaij
 *  @date   2016-05-26
 */
class AnyDataGetAlgorithm : public GaudiAlgorithm {
public:
  /// Standard constructor
  AnyDataGetAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);

  virtual ~AnyDataGetAlgorithm( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

private:

   std::vector<std::string> m_locations;

};
#endif // ANYDATA_ANADATAGETALGORITHM_H
