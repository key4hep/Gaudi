/***********************************************************************************\
* (c) Copyright 2023 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIEXAMPLES_PARTPROPALG_H
#define GAUDIEXAMPLES_PARTPROPALG_H

#include <Gaudi/Algorithm.h>

#include "GaudiPartProp/IParticlePropertySvc.h"
#include "GaudiPartProp/ParticleID.h"
#include "GaudiPartProp/ParticleProperty.h"

namespace Gaudi {
  namespace Examples {
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

      const Gaudi::IParticlePropertySvc* ppSvc() const;

    private:
      // the pointer to new particle property service
      mutable SmartIF<Gaudi::IParticlePropertySvc> m_ppSvc; // new part.property.svc
    };
  } // namespace Examples
} // namespace Gaudi

#endif // GAUDIEXAMPLES_PARTPROPALG_H
