#ifndef GAUDIKERNEL_ANYDATAHANDLE_H
#define GAUDIKERNEL_ANYDATAHANDLE_H

#include <GaudiKernel/DataObjectHandle.h>

template <typename T>
using AnyDataHandle = DataObjectHandle<AnyDataWrapper<T>>;

#endif
