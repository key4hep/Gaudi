#ifndef GAUDIKERNEL_GAUDIKERNEL_IALGTASK_H_
#define GAUDIKERNEL_GAUDIKERNEL_IALGTASK_H_

#include "GaudiKernel/StatusCode.h"

/**@class IAlgTask IAlgTask.h GaudiKernel/IAlgTask.h
 *
 *  General interface for a wrapper around Gaudi algorithm.
 *
 *  @author  Illya Shapoval
 *  @version 1.0
 */

// Does it have to inherit from IInterface ?
class IAlgTask {
public:
  virtual ~IAlgTask() = default;

  virtual StatusCode execute() = 0;
};

#endif /* GAUDIKERNEL_GAUDIKERNEL_IALGTASK_H_ */
