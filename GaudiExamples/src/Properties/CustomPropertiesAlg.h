/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef PROPERTIES_CUSTOMPROPERTIESALG_H
#define PROPERTIES_CUSTOMPROPERTIESALG_H 1
// Include files
// from Gaudi
#include <Gaudi/Examples/TestAlg.h>

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
    class CustomPropertiesAlg : public TestAlg {
    public:
      /// Standard constructor
      using TestAlg::TestAlg;

      StatusCode initialize() override; ///< Algorithm initialization
      StatusCode execute() override;    ///< Algorithm execution
      StatusCode finalize() override;   ///< Algorithm finalization
    private:
      Gaudi::Property<std::unordered_map<std::string, std::string>> m_unorderedMap{ this, "UnorderedMap" };
    };
  } // namespace Examples
} // namespace Gaudi

#endif // PROPERTIES_CUSTOMPROPERTIESALG_H
