/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <fstream>
#include <string>
#include <string_view>

namespace Gaudi::Hive {

  enum class FileType : short { UNKNOWN, DOT, MD, ML };

  /** utilities to dump graphs in different formats
   *
   *  supported right now : .dot, mermaid and graphml
   *
   *  Usage :
   *    - create a Graph calling the constructor with type and name of the output file
   *    - use then addNode and addEdge to specify the graph
   *    - on destruction, the Graph will be written to file
   */
  class Graph {

  public:
    /**
     * Creates a graph with given type and given file name
     *
     * by default (and if UNKNOWN is provided) type is deduced from the file extension
     * in case extension is not recognized, dot format is used and filename gets
     * an extra ".dot" at the end
     */
    Graph( std::string_view fileName, FileType type = FileType::UNKNOWN );

    ~Graph();

    std::string const& fileName();

    void addNode( std::string_view id, std::string_view name );

    void addEdge( std::string_view srcId, std::string_view tgtId, std::string_view label = "" );

  private:
    // Utilities to write file headers and trailers in case a format needs them
    void writeHeader();
    void writeTrailer();

    /// name of the filed used for storing the graph
    std::string m_fileName;
    /// stream to the graph file
    std::ofstream m_stream;
    /// type of file used
    FileType m_type{ FileType::UNKNOWN };
  };

} // end namespace Gaudi::Hive
