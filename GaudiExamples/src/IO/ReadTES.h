#ifndef POOLIO_READTES_H
#define POOLIO_READTES_H 1

// Include files

#include <string>
#include <vector>

// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

/** @class ReadTES ReadTES.h
 *
 *
 *  @author Marco Cattaneo
 *  @date   2008-11-03
 */
class ReadTES : public GaudiAlgorithm {
public:
  /// Standard constructor
  using GaudiAlgorithm::GaudiAlgorithm;

  StatusCode initialize() override; ///< Algorithm initialization
  StatusCode execute() override;    ///< Algorithm execution

protected:
private:
  Gaudi::Property<std::vector<std::string>> m_locations{this, "Locations", {}, "Locations to read"};
};
#endif // POOLIO_READTES_H
