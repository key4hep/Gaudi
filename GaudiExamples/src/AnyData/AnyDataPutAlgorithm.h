#ifndef ANYDATA_ANADATAPUTALGORITHM_H
#define ANYDATA_ANADATAPUTALGORITHM_H 1


// Include files
#include <string>
#include <vector>

// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/AnyDataHandle.h"

/** @class AnyDataPutAlgorithm AnyDataPutAlgorithm.h AnyData/AnyDataPutAlgorithm.h
 *
 *
 *  @author Roel Aaij
 *  @date   2016-05-26
 */
class AnyDataPutAlgorithm : public GaudiAlgorithm {
public:
  /// Standard constructor
  AnyDataPutAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

private:

   std::string m_loc;
   AnyDataHandle<std::vector<int>> m_ids;
   std::vector<AnyDataHandle<int>> m_id_vec;
   
};
#endif // ANYDATA_ANADATAPUTALGORITHM_H
