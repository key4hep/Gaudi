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
#ifndef GAUDIKERNEL_GAUDIKERNEL_ITASK_H_
#define GAUDIKERNEL_GAUDIKERNEL_ITASK_H_

/**@class ITask ITask.h GaudiKernel/ITask.h
 *
 *  General interface for a Gaudi task.
 *
 *  @author  Illya Shapoval
 *  @version 1.0
 */

class ITask {
public:
  virtual ~ITask() = default;

  virtual ITask* execute() = 0;
};

#endif /* GAUDIKERNEL_GAUDIKERNEL_ITASK_H_ */
