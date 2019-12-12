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
#ifndef GAUDIKERNEL_DATAOBJIDPROPERTY_H
#define GAUDIKERNEL_DATAOBJIDPROPERTY_H 1

#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/Property.h"

[[deprecated( "replace DataObjIDProperty by Gaudi::Property<DataObjID>" )]] typedef Gaudi::Property<DataObjID>
    DataObjIDProperty;

[[deprecated(
    "replace DataObjIDCollProperty by Gaudi::Property<DataObjIDColl>" )]] typedef Gaudi::Property<DataObjIDColl>
    DataObjIDCollProperty;

#endif
