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
#ifndef PERSISTENCYSVC_DETPERSISTENCYSVC_H
#define PERSISTENCYSVC_DETPERSISTENCYSVC_H 1

#include "PersistencySvc.h"

class DetPersistencySvc : virtual public PersistencySvc {
public:
  // Standard Constructor
  DetPersistencySvc( const std::string& name, ISvcLocator* svc );
};

#endif // PERSISTENCYSVC_DETPERSISTENCYSVC_H
