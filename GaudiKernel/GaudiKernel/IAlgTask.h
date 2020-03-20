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

  virtual void operator()() = 0;
};

#endif /* GAUDIKERNEL_GAUDIKERNEL_IALGTASK_H_ */
