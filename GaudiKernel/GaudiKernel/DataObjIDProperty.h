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
