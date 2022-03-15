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
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IFileAccess.h"
#include <fstream>

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

  std::unique_ptr<std::istream> open( std::string const& url ) override {
    // remove the optional "file://" from the beginning of the url
    constexpr auto prefix = std::string_view{ "file://" };
    return std::make_unique<std::ifstream>( url.compare( 0, prefix.size(), prefix ) == 0 ? url.substr( prefix.size() )
                                                                                         : url );
  }

  /// Protocols supported by the instance.
  const std::vector<std::string>& protocols() const override {
    /// Vector of supported protocols.
    static const std::vector<std::string> s_protocols = { { "file" } };
    return s_protocols;
  }
};

DECLARE_COMPONENT( FileReadTool )
