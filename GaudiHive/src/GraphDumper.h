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
   *    - create a Graph callin gthe constructor with type and name of the output file
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
    Graph( std::string_view fileName, FileType type = FileType::UNKNOWN ) : m_fileName{ fileName }, m_type{ type } {
      // find out type of file if not given
      if ( type == FileType::UNKNOWN ) {
        // Check file extension
        if ( m_fileName.ends_with( ".dot" ) ) {
          m_type = FileType::DOT;
        } else if ( m_fileName.ends_with( ".md" ) ) {
          m_type = FileType::MD;
        } else if ( m_fileName.ends_with( ".graphml" ) ) {
          m_type = FileType::ML;
        } else {
          m_type = FileType::DOT;
          m_fileName += ".dot";
        }
      }
      // open file
      m_stream = std::ofstream{ m_fileName, std::ofstream::out };
      writeHeader();
    }

    ~Graph() {
      writeTrailer();
      m_stream.close();
    }

    std::string const& fileName() { return m_fileName; }

    void addNode( std::string_view name, std::string_view id ) {
      switch ( m_type ) {
      case FileType::DOT:
        m_stream << "  " << id << " [label=\"" << name << "\";shape=box];\n";
        break;
      case FileType::MD:
        m_stream << "  " << id << "{{" << name << "}}\n";
        break;
      case FileType::ML:
        m_stream << "    <node id=\"" << name << "\"/>\n";
        break;
      case FileType::UNKNOWN:
        break;
      }
    };

    void addEdge( std::string_view srcName, std::string_view srcId, std::string_view tgtName, std::string_view tgtId,
                  std::string_view label = "" ) {
      switch ( m_type ) {
      case FileType::DOT:
        m_stream << "  " << srcId << " -> " << tgtId;
        if ( label != "" ) m_stream << " [label=\"" << label << "\"]";
        m_stream << ";\n";
        break;
      case FileType::MD:
        m_stream << "  " << srcId << " --> " << tgtId;
        if ( label != "" ) m_stream << " : " << label;
        m_stream << "\n";
        break;
      case FileType::ML:
        m_stream << "    <edge source=\"" << srcName << "\" target=\"" << tgtName << "\"/>\n";
        break;
      case FileType::UNKNOWN:
        break;
      }
    };

  private:
    void writeHeader() {
      switch ( m_type ) {
      case FileType::DOT:
        m_stream << "digraph datadeps {\n  rankdir=\"LR\";\n";
        break;
      case FileType::MD:
        m_stream << "```mermaid\ngraph LR;\n\n";
        break;
      case FileType::ML:
        m_stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\"\n"
                    "         xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
                    "         xsi:schemaLocation=\"http://graphml.graphdrawing.org/xmlns "
                    "http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd\">\n"
                    "  <graph id=\"Data dependencies\" edgedefault=\"directed\">\n";
        break;
      case FileType::UNKNOWN:
        break;
      }
    }

    void writeTrailer() {
      switch ( m_type ) {
      case FileType::DOT:
        m_stream << "}\n";
        break;
      case FileType::MD:
        m_stream << "```\n";
        break;
      case FileType::ML:
        m_stream << "  </graph>\n</graphml>";
        break;
      case FileType::UNKNOWN:
        break;
      }
    }

    /// name of the filed used for storing the graph
    std::string m_fileName;
    /// stream to the graph file
    std::ofstream m_stream;
    /// type of file used
    FileType m_type{ FileType::UNKNOWN };
  };

} // end namespace Gaudi::Hive
