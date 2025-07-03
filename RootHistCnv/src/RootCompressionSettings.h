/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

// STD
#include <string>

namespace RootHistCnv {

  /** @class RootCompressionSettings RootCompressionSettings.h
   *
   *  Simple class to decode a ROOT compression settings string,
   *  of the form '<Alg>:<level>' into the integer code to pass
   *  to ROOT.
   *
   *  @author Chris Jones
   *  @date   2013-10-24
   */

  class RootCompressionSettings {
    /// The cached ROOT compression level int
    int m_level;

  public:
    /// Standard constructor
    RootCompressionSettings( const std::string& settings );

    /// Get the level
    int level() const { return m_level; }
  };
} // namespace RootHistCnv
