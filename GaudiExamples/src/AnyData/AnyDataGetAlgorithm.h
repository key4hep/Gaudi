#ifndef ANYDATA_ANADATAGETALGORITHM_H
#define ANYDATA_ANADATAGETALGORITHM_H 1

// Include files
#include <string>
#include <vector>

// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/AnyDataHandle.h"
/** @class AnyDataGetAlgorithm AnyDataGetAlgorithm.h AnyData/AnyDataGetAlgorithm.h
 *
 *
 *  @author Roel Aaij
 *  @date   2016-05-26
 */
template <class T>
class AnyDataGetAlgorithm : public GaudiAlgorithm
{
public:
  /// Standard constructor
  AnyDataGetAlgorithm( const std::string& name, ISvcLocator* svcLoc ) : GaudiAlgorithm( name, svcLoc ) {}

  StatusCode execute() override; ///< Algorithm execution

private:
  Gaudi::Property<std::string> m_location{this, "Location"};

  AnyDataHandle<std::vector<int>> m_ids{"/Event/Test/Ids", Gaudi::DataHandle::Reader, this};
};
#endif // ANYDATA_ANADATAGETALGORITHM_H
