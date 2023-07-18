/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/Algorithm.h>

/** @class RecordOutputStream
 *
 * Simple class that adds an entry to the Transient Event Store to record that
 * the processing flow triggered its execution.
 *
 * The main use case is to replace OutputStream instances appearing in the
 * normal processing flow of workers in GaudiMP jobs. The information is then
 * passed to the writer process where an instance of @c ReplayOutputStream will
 * trigger the actual writing.
 *
 * @author Marco Clemencic
 * @date 30/08/2013
 */
class RecordOutputStream : public Algorithm {
public:
  using Algorithm::Algorithm;

  StatusCode initialize() override; ///< Algorithm initialization
  StatusCode execute() override;    ///< Algorithm execution

  /// Return the path in the Transient Store used to record the triggered
  /// instances.
  static inline const std::string locationRoot() { return "TriggeredOutputStreams"; }

private:
  Gaudi::Property<std::string> m_streamName{
      this, "OutputStreamName", {}, "Name of the OutputStream instance should be triggered." };

  /// location of the DataObject flag used to record that this algorithm was
  /// called
  std::string m_flagLocation;
};
