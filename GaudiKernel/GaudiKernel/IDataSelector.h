#ifndef GAUDIKERNEL_IDATASELECTOR_H
#define GAUDIKERNEL_IDATASELECTOR_H

// Framework include files
#include "GaudiKernel/Kernel.h"

// Include files
#include <vector>

// Forward declarations
class DataObject;

/** 	This is only a placeholder to allow me compiling
	until the responsible guy does his work!
	M.Frank
*/
class IDataSelector : virtual public std::vector<DataObject*>    {
};

#endif  // KERNEL_IDATASELECTOR_H
