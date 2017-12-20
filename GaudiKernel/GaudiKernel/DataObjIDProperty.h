#ifndef GAUDIKERNEL_DATAOBJIDPROPERTY_H
#define GAUDIKERNEL_DATAOBJIDPROPERTY_H 1

#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/Property.h"

[[deprecated( "replace DataObjIDProperty by Gaudi::Property<DataObjID>" )]] using DataObjIDProperty =
    Gaudi::Property<DataObjID>;
[[deprecated( "replace DataObjIDCollProperty by Gaudi::Property<DataObjIDColl>" )]] using DataObjIDCollProperty =
    Gaudi::Property<DataObjIDColl>;

#endif
