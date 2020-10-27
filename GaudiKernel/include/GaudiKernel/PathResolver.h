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
#ifndef GAUDIKERNEL_PATHRESOLVER_H
#define GAUDIKERNEL_PATHRESOLVER_H

#include "GaudiKernel/Kernel.h"
#include <string>

namespace System {

  class GAUDI_API PathResolver

  {
  public:
    typedef enum { Ok, EnvironmentVariableUndefined, UnknownDirectory } SearchPathStatus;

    typedef enum { LocalSearch, RecursiveSearch } SearchType;

    /**

      @arg @c logical_file_name the name of the file to locate in the search path
      @arg @c search_path the name of a path-like environment variable
      @arg @c search_type characterizes the type of search. Can be either @c LocalSearch or @c RecursiveSearch

      @return the physical name of the located file or empty string if not found

     */
    static std::string find_file( const std::string& logical_file_name, const std::string& search_path,
                                  SearchType search_type = LocalSearch );

    /**

      @arg @c logical_file_name the name of the file to locate in the search path
      @arg @c search_list the prioritized list of possible locations separated by the usual path separator
      @arg @c search_type characterizes the type of search. Can be either @c LocalSearch or @c RecursiveSearch

      @return the physical name of the located file or empty string if not found

     */
    static std::string find_file_from_list( const std::string& logical_file_name, const std::string& search_list,
                                            SearchType search_type = LocalSearch );

    /**

      @arg @c logical_file_name the name of the directory to locate in the search path
      @arg @c search_path the name of a path-like environment variable
      @arg @c search_type characterizes the type of search. Can be either LocalSearch or RecursiveSearch

      @return the physical name of the located directory or empty string if not found

     */
    static std::string find_directory( const std::string& logical_file_name, const std::string& search_path,
                                       SearchType search_type = LocalSearch );

    /**

      @arg @c logical_file_name the name of the directory to locate in the search path
      @arg @c search_list the prioritized list of possible locations separated by the usual path separator
      @arg @c search_type characterizes the type of search. Can be either LocalSearch or RecursiveSearch

      @return the physical name of the located directory or empty string if not found

     */
    static std::string find_directory_from_list( const std::string& logical_file_name, const std::string& search_list,
                                                 SearchType search_type = LocalSearch );

    /**

    @arg @c search_path the name of a path-like environment variable

    @return the result of the verification. Can be one of @c Ok, @c EnvironmentVariableUndefined, @c UnknownDirectory

     */
    static SearchPathStatus check_search_path( const std::string& search_path );
  };

  GAUDI_API std::string PathResolverFindXMLFile( const std::string& logical_file_name );
  GAUDI_API std::string PathResolverFindDataFile( const std::string& logical_file_name );
} // namespace System

#endif
