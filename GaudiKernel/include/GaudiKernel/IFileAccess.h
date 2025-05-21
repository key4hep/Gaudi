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

// Include Files
#include <GaudiKernel/IInterface.h>
#include <istream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

/** @class IFileAccess IFileAccess.h GaudiKernel/IFileAccess.h
 *
 * Abstract interface for a service or tool implementing a read access to files.
 *
 * @author Marco Clemencic
 * @date   2008-01-18
 */
class GAUDI_API IFileAccess : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( IFileAccess, 3, 1 );

  /// Find the URL and returns a unique_ptr to an input stream interface of an
  /// object that can be used to read from the file the URL is pointing to.
  /// Returns an empty pointer if the URL cannot be resolved.
  virtual std::unique_ptr<std::istream> open( std::string const& url ) = 0;

  /// Find the URL and returns an optional<string> to the content of
  /// the file the URL is pointing to.
  /// Returns an 'unengaged' optional if the URL cannot be resolved.
  virtual std::optional<std::string> read( std::string const& url ) {
    auto is = open( url );
    if ( !is || !is->good() ) return std::nullopt;
    return std::string{ std::istreambuf_iterator<char>{ *is }, std::istreambuf_iterator<char>{} };
  }

  /// Protocols supported by the instance.
  virtual const std::vector<std::string>& protocols() const = 0;
};
