#ifndef PROPERTIES_CUSTOMPROPERTIESALG_H
#define PROPERTIES_CUSTOMPROPERTIESALG_H 1
// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

#include <unordered_map>

namespace Gaudi {
  namespace Examples {
    /** @class CustomPropertiesAlg CustomPropertiesAlg.h Properties/CustomPropertiesAlg.h
     *
     * Example on how to use custom property types.
     *
     * @author Marco Clemencic
     * @date 14/11/2014
     */
    class CustomPropertiesAlg : public GaudiAlgorithm {
    public:
      /// Standard constructor
      using GaudiAlgorithm::GaudiAlgorithm;

      StatusCode initialize() override; ///< Algorithm initialization
      StatusCode execute() override;    ///< Algorithm execution
      StatusCode finalize() override;   ///< Algorithm finalization
    private:
      Gaudi::Property<std::unordered_map<std::string, std::string>> m_unorderedMap{this, "UnorderedMap"};
    };
  } // namespace Examples
} // namespace Gaudi

#endif // PROPERTIES_CUSTOMPROPERTIESALG_H
