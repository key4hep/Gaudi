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
#ifndef _FILEREADTOOL_H
#define _FILEREADTOOL_H

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IFileAccess.h"

/** @class FileReadTool FileReadTool.h
 *
 *  Basic implementation of the IFileAccess interface.
 *  This tool simply takes a path to a file as url and return the std::istream interface
 *  of std::ifstream.
 *
 *  @author Marco Clemencic
 *  @date 2008-01-18
 */
struct FileReadTool : extends<AlgTool, IFileAccess> {
  /// Standard constructor
  using extends::extends;

  std::unique_ptr<std::istream> open( const std::string& url ) override;

  /// Protocols supported by the instance.
  const std::vector<std::string>& protocols() const override;
};

#endif // _FILEREADTOOL_H
