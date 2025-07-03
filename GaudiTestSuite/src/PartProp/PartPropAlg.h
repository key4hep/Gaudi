/***********************************************************************************\
* (c) Copyright 2023-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <Gaudi/Algorithm.h>

#include <Gaudi/Interfaces/IParticlePropertySvc.h>
#include <Gaudi/ParticleID.h>
#include <Gaudi/ParticleProperty.h>

namespace Gaudi {
  namespace TestSuite {
    /** @class PartPropAlg
     * @brief  an algorithm to test the particle property service
     * @author Graeme Stewart
     * @author author Vanya Belyaev
     */
    class PartPropAlg : public Gaudi::Algorithm {

    public:
      PartPropAlg( const std::string& name, ISvcLocator* pSvcLocator );
      StatusCode initialize() override;
      StatusCode execute( const EventContext& ctx ) const override;
      StatusCode finalize() override;

      const Gaudi::Interfaces::IParticlePropertySvc* ppSvc() const;

    private:
      // the pointer to new particle property service
      mutable SmartIF<Gaudi::Interfaces::IParticlePropertySvc> m_ppSvc; // new part.property.svc
    };
  } // namespace TestSuite
} // namespace Gaudi
