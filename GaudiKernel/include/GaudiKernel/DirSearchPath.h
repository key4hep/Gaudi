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
#ifndef JOBOPTIONS_DIRSEARCHPATH_H
#define JOBOPTIONS_DIRSEARCHPATH_H
/** @file DirSearchPath.h
 * @brief search for files in a list of directories
 * @author Paolo Calafiura <pcalafiura@lbl.gov> - ATLAS Collaboration
 */
#include "GaudiKernel/Kernel.h"           /* GAUDI_API */
#include "boost/filesystem/exception.hpp" /*filesystem_error*/
#include "boost/filesystem/path.hpp"
#include <functional> /* binary_function */
#include <string>
#include <vector>
/** @class DirSearchPath
 * @brief search for files in a list of directories
 * @author Paolo Calafiura <pcalafiura@lbl.gov> - ATLAS Collaboration
 */
class GAUDI_API DirSearchPath {
public:
  typedef boost::filesystem::path path;

  /// \name constructors
  //@{
  DirSearchPath() { addCWD(); }
/// \throws boost::filesystem::filesystem_error
#ifdef _WIN32
  DirSearchPath( const std::string& stringifiedPath, const char* separator = ",;" );
#else
  DirSearchPath( const std::string& stringifiedPath, const char* separator = ",:" );
#endif
  //@}

  /// \name modifiers
  //@{
  //  bool add(const std::string& dirName); ///< \throws filesystem_error
  bool add( const path& dir ); ///< \throws filesystem_error
  bool addCWD();               ///< add current work dir (*nix pwd) to path
  //@}

  /// \name accessors
  //@{
  /// returns a flag if fileName found in search path, and sets ref to
  /// fully qualified file name (in native form)
  bool find( const std::string& fileName, std::string& fullFileName ) const;
  /// returns a flag if file found in search path. Sets ref to completed path
  bool find( const path& file, path& fileFound ) const;
  /// returns lists of files found in search path.
  std::list<path> find_all( const path& file ) const;
  //@}

  /// \name helpers
  //@{
  static bool existsDir( const std::string& dirName ); ///< check dirName is valid
  static bool existsDir( const path& dir );            ///< check dir path is valid
  //@}

private:
  //
  /// @class eqPath compare paths name
  struct eqPath {
    eqPath( const path& ref ) : m_ref( ref ) {}
    bool operator()( const path& p ) const { return p.string() == m_ref.string(); }

  private:
    path m_ref;
  };
  // @class lessPath order paths by (system-independent) name
  // struct lessPath : public std::binary_function<const path&,const path&,bool> {
  // bool operator() (const path& lhs, const path& rhs) const {
  //   return lhs.string() < rhs.string();
  // }
  //};
  ///

  //  typedef std::set<path, lessPath> PathSet; ///<a set ordered by path name
  //  PathSet m_dirs;   ///<the dir container
  //
  std::vector<path> m_dirs; ///< the dir container
};
#endif // JOBOPTIONS_DIRSEARCHPATH_H
