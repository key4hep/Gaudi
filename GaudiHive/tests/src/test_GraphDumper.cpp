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
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_GraphDumper

#include "../../src/GraphDumper.h"
#include <boost/test/unit_test.hpp>
#include <istream>
#include <string>

namespace {
  void buildSimpleGraph( std::string const& fn, Gaudi::Hive::FileType type = Gaudi::Hive::FileType::UNKNOWN ) {
    Gaudi::Hive::Graph g{ fn, type };
    g.addNode( "Node1", "Id1" );
    g.addNode( "Node2", "Id2" );
    g.addEdge( "Node1", "Id1", "Node2", "Id2" );
  }
  void buildGraphWithLabel( std::string const& fn, Gaudi::Hive::FileType type ) {
    Gaudi::Hive::Graph g{ fn, type };
    g.addNode( "Node1", "Id1" );
    g.addNode( "Node2", "Id2" );
    g.addEdge( "Node1", "Id1", "Node2", "Id2", "EdgeLabel" );
  }
  std::string readFile( std::string const& fn ) {
    std::ifstream is( fn, std::ios::binary | std::ios::ate );
    auto          size = is.tellg();
    std::string   str( size, '\0' ); // construct string to stream size
    is.seekg( 0 );
    BOOST_CHECK( is.read( &str[0], size ) );
    return str;
  }

  const std::string simpleDotRef = "digraph datadeps {\n  rankdir=\"LR\";\n  Id1 [label=\"Node1\";shape=box];\n  Id2 "
                                   "[label=\"Node2\";shape=box];\n  Id1 -> Id2;\n}\n";
  const std::string labelDotRef  = "digraph datadeps {\n  rankdir=\"LR\";\n  Id1 [label=\"Node1\";shape=box];\n  Id2 "
                                   "[label=\"Node2\";shape=box];\n  Id1 -> Id2 [label=\"EdgeLabel\"];\n}\n";
  const std::string simpleMDRef  = "```mermaid\ngraph LR;\n\n  Id1{{Node1}}\n  Id2{{Node2}}\n  Id1 --> Id2\n```\n";
  const std::string labelMDRef =
      "```mermaid\ngraph LR;\n\n  Id1{{Node1}}\n  Id2{{Node2}}\n  Id1 --> Id2 : EdgeLabel\n```\n";
  const std::string simpleMLRef =
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\"\n         "
      "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n         "
      "xsi:schemaLocation=\"http://graphml.graphdrawing.org/xmlns "
      "http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd\">\n  <graph id=\"Data dependencies\" "
      "edgedefault=\"directed\">\n    <node id=\"Node1\"/>\n    <node id=\"Node2\"/>\n    <edge source=\"Node1\" "
      "target=\"Node2\"/>\n  </graph>\n</graphml>";
} // namespace

BOOST_AUTO_TEST_CASE( test_dot_simple ) {
  std::string fn = "test_GraphDumper_outputFile.dot";
  buildSimpleGraph( fn, Gaudi::Hive::FileType::DOT );
  auto str = readFile( fn );
  BOOST_CHECK_EQUAL( str, simpleDotRef );
  std::remove( fn.c_str() );
}

BOOST_AUTO_TEST_CASE( test_md_simple ) {
  std::string fn = "test_GraphDumper_outputFile.md";
  buildSimpleGraph( fn, Gaudi::Hive::FileType::MD );
  auto str = readFile( fn );
  BOOST_CHECK_EQUAL( str, simpleMDRef );
  std::remove( fn.c_str() );
}

BOOST_AUTO_TEST_CASE( test_ml_simple ) {
  std::string fn = "test_GraphDumper_outputFile.graphml";
  buildSimpleGraph( fn, Gaudi::Hive::FileType::ML );
  auto str = readFile( fn );
  BOOST_CHECK_EQUAL( str, simpleMLRef );
  std::remove( fn.c_str() );
}

BOOST_AUTO_TEST_CASE( test_auto_dot ) {
  std::string fn = "test_GraphDumper_outputFile_auto.dot";
  buildSimpleGraph( fn );
  auto str = readFile( fn );
  BOOST_CHECK_EQUAL( str, simpleDotRef );
  std::remove( fn.c_str() );
}

BOOST_AUTO_TEST_CASE( test_auto_md ) {
  std::string fn = "test_GraphDumper_outputFile_auto.md";
  buildSimpleGraph( fn );
  auto str = readFile( fn );
  BOOST_CHECK_EQUAL( str, simpleMDRef );
  std::remove( fn.c_str() );
}

BOOST_AUTO_TEST_CASE( test_auto_ml ) {
  std::string fn = "test_GraphDumper_outputFile_auto.graphml";
  buildSimpleGraph( fn );
  auto str = readFile( fn );
  BOOST_CHECK_EQUAL( str, simpleMLRef );
  std::remove( fn.c_str() );
}

BOOST_AUTO_TEST_CASE( test_auto_auto ) {
  std::string fn = "test_GraphDumper_outputFile_auto_auto";
  buildSimpleGraph( fn );
  auto str = readFile( fn + ".dot" );
  BOOST_CHECK_EQUAL( str, simpleDotRef );
  std::remove( ( fn + ".dot" ).c_str() );
}

BOOST_AUTO_TEST_CASE( test_dot_label ) {
  std::string fn = "test_GraphDumper_label_outputFile.dot";
  buildGraphWithLabel( fn, Gaudi::Hive::FileType::DOT );
  auto str = readFile( fn );
  BOOST_CHECK_EQUAL( str, labelDotRef );
  std::remove( fn.c_str() );
}

BOOST_AUTO_TEST_CASE( test_md_label ) {
  std::string fn = "test_GraphDumper_label_outputFile.md";
  buildGraphWithLabel( fn, Gaudi::Hive::FileType::MD );
  auto str = readFile( fn );
  BOOST_CHECK_EQUAL( str, labelMDRef );
  std::remove( fn.c_str() );
}

BOOST_AUTO_TEST_CASE( test_ml_label ) {
  std::string fn = "test_GraphDumper_label_outputFile.graphml";
  buildGraphWithLabel( fn, Gaudi::Hive::FileType::ML );
  auto str = readFile( fn );
  BOOST_CHECK_EQUAL( str, simpleMLRef ); // no label support in graphML
  std::remove( fn.c_str() );
}
