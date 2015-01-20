#ifndef PROPERTIES_CUSTOMPROPERTIESALG_H
#define PROPERTIES_CUSTOMPROPERTIESALG_H 1
// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

#include <unordered_map>

namespace Gaudi { namespace Examples {
  /** @class CustomPropertiesAlg CustomPropertiesAlg.h Properties/CustomPropertiesAlg.h
   *
   * Example on how to use custom property types.
   *
   * @author Marco Clemencic
   * @date 14/11/2014
   */
  class CustomPropertiesAlg: public GaudiAlgorithm {
  public:
    /// Standard constructor
    CustomPropertiesAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~CustomPropertiesAlg(); ///< Destructor

    virtual StatusCode initialize();    ///< Algorithm initialization
    virtual StatusCode execute   ();    ///< Algorithm execution
    virtual StatusCode finalize  ();    ///< Algorithm finalization
  protected:
  private:
    std::unordered_map<std::string, std::string> m_unorderedMap;
  };
}}

#endif // PROPERTIES_CUSTOMPROPERTIESALG_H
