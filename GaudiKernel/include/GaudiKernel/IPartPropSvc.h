/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_IPARTPROPSVC_H
#define GAUDIKERNEL_IPARTPROPSVC_H

// Include Files
#include <GaudiKernel/IService.h>
#include <map>

namespace HepPDT {
  class ParticleDataTable;
  class ProcessUnknownID;
} // namespace HepPDT

/* @class IPartPropSvc IPartPropSvc.h GaudiKernel/IPartPropSvc.h

    This class is an interface to the PartPropSvc.

    @author Charles Leggett

*/
class GAUDI_API IPartPropSvc : virtual public IService {

public:
  /// InterfaceID
  DeclareInterfaceID( IPartPropSvc, 2, 0 );

  virtual HepPDT::ParticleDataTable* PDT()                                                                      = 0;
  virtual void                       setUnknownParticleHandler( HepPDT::ProcessUnknownID*, const std::string& ) = 0;
};

#endif
